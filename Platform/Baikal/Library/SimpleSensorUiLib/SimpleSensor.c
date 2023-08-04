#include "SimpleSensor.h"
#include "SimpleSensorData.h"

// HII support
STATIC EFI_GUID                  mConfigFormSetGuid = SIMPLE_SENSOR_FORMSET_GUID;
//STATIC CHAR16                  mConfigStorageName[] = L"SimpleSensor";

// HII context
STATIC SIMPLE_SENSOR_CONTEXT    *mViewContext = NULL;

// HII support for Device Path
STATIC HII_VENDOR_DEVICE_PATH    mHiiVendorDevicePath = {
    {
        {
            HARDWARE_DEVICE_PATH,
            HW_VENDOR_DP,
            {
                (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
                (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
            }
        },
        SIMPLE_SENSOR_FORMSET_GUID
    },
    {
        END_DEVICE_PATH_TYPE,
        END_ENTIRE_DEVICE_PATH_SUBTYPE,
        {
            (UINT8) (END_DEVICE_PATH_LENGTH),
            (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
        }
    }
};

STATIC UINT8
NumDigs (
    IN UINT32 Data
    )
{
    return Data < 10 ? 1 : Data < 100 ? 2 : Data < 1000 ? 3 : Data < 10000 ? 4 : Data < 100000 ? 5 : Data < 1000000 ? 6 : Data < 10000000 ? 7 : Data < 100000000 ? 8 : Data < 1000000000 ? 9 : 10;
}

STATIC UINT32
DecPow (
    IN UINT8 Pow
    )
{
    return Pow == 1 ? 10 : Pow == 2 ? 100 : Pow == 3 ? 1000 : Pow == 4 ? 10000 : Pow == 5 ? 100000 : Pow == 6 ? 1000000 : Pow == 7 ? 10000000 : Pow == 8 ? 100000000 : Pow == 9 ? 1000000000 : 10000000000;
}

STATIC UINTN
FixToStr (
   IN OUT CHAR16 *Str,
   IN     UINTN   Len,
   IN     UINT8   IntDigs,
   IN     UINT8   FracDigs,
   IN     INT32   Data,
   IN     INT8    Exp2
   )
{
    CHAR16 *Ptr;
    CHAR16 *End;
    UINT32  Tmp;
    UINT32  Int;
    UINT32  Frac;
    UINT8   Size;

    ASSERT (IntDigs + FracDigs + (FracDigs > 0 ? 1 /*.*/ : 0) + (Data < 0 ? 1 /*-*/ : 0) < Len);

    if (Data < 0) {
        IntDigs --;
        Tmp = -Data;
    } else {
        Tmp = Data;
    }

    Int = Exp2 < 0 ? (Tmp >> -Exp2) : (Tmp << Exp2);
    Size = NumDigs(Int);
    ASSERT (Size <= IntDigs);

    if (Exp2 < 0) {
        Frac = ((Tmp - (Int << -Exp2)) * DecPow(FracDigs)) >> -Exp2;
    } else {
        Frac = 0;
    }

    Ptr = Str;
    End = Ptr + IntDigs - Size;

    // pad with spaces
    for (; Ptr < End; ) {
        *Ptr ++ = ' ';
    }

    // place negative marker
    if (Data < 0) {
        *Ptr ++ = '-';
    }

    // place integer part
    if (Int == 0) {
        *Ptr ++ = '0';
    } else {
        Ptr += Size;
        for (; Int > 0; Int /= 10) {
            *--Ptr = (Int % 10) + '0';
        }
        Ptr += Size;
    }

    // place point
    if (FracDigs > 0) {
        *Ptr ++ = '.';
    }

    End = Ptr;
    Ptr += FracDigs;

    // place fraction part
    for (; Ptr > End; Frac /= 10) {
        *--Ptr = (Frac % 10) + '0';
    }
    Ptr += FracDigs;
    *Ptr = '\0';

    return Ptr - Str;
}

#define DATA_TYPES                                                             \
  _(COUNT, L"", 9, 0)                                                          \
  _(VOLT, L" V", 5, 3)                                                         \
  _(AMP, L" A", 5, 3)                                                          \
  _(WATT, L" W", 5, 3)                                                         \
  _(CDEG, L" °C", 7, 1)                                                        \
  _(RPM, L" RPM", 9, 0)

STATIC
EFI_STATUS
UpdateData (
    IN OUT SIMPLE_SENSOR_CONTEXT *ViewContext,
    IN     BOOLEAN                Force
    )
{
    UINTN                  SensorIdx;
    SIMPLE_SENSOR_VAL      OldValue;
    SIMPLE_SENSOR_ELEMENT *Sensor;
    UINTN                  StrLen;
    UINT8                  IntDigs;
    UINT8                  FracDigs;
    CONST CHAR16          *Units;
    EFI_STATUS             Status;

    for (SensorIdx = 0; SensorIdx < ViewContext->NumSensors; SensorIdx++) {
        Sensor = &ViewContext->Sensor[SensorIdx];

        if (Sensor->Info.Type == SIMPLE_SENSOR_TYPE_GROUP) {
            continue;
        }

        OldValue = Sensor->Value;
        Status = ViewContext->SimpleSensor->GetData (
            ViewContext->SimpleSensor, Sensor->Info.Id, &Sensor->Value);

        if (EFI_ERROR (Status)) {
            // ignore errors
            continue;
        }

        if (!Force && Sensor->Value == OldValue) {
            // nothing to do
            continue;
        }

        switch (Sensor->Info.Type) {
#define _(type, units, int_digs, frac_digs)     \
            case SIMPLE_SENSOR_TYPE_##type:     \
                Units = units;                  \
                IntDigs = int_digs;             \
                FracDigs = frac_digs;           \
                break;
            DATA_TYPES
#undef _
        default:
            Units = L"";
            IntDigs = 0;
            FracDigs = 0;
        }

        // place value
        StrLen = FixToStr(Sensor->ValueStr, ARRAY_SIZE (Sensor->ValueStr) - 1,
                          IntDigs, FracDigs,
                          Sensor->Value, Sensor->Info.Exp2);

        // place units
        StrCpyS (&Sensor->ValueStr[StrLen],
                 ARRAY_SIZE(Sensor->ValueStr) - 1 - StrLen,
                 Units);

        HiiSetString(ViewContext->HiiHandle, Sensor->ValueId, Sensor->ValueStr, NULL);
    }

    return EFI_SUCCESS;
}

STATIC
VOID
OnDataChange (
    IN OUT SIMPLE_SENSOR_LISTENER *Listener
    )
{
    SIMPLE_SENSOR_CONTEXT *ViewContext =
        BASE_CR(Listener, SIMPLE_SENSOR_CONTEXT, SensorsListener);

    UpdateData (ViewContext, FALSE);
}

EFI_STATUS
EFIAPI
SimpleSensorUiInitialize (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
    )
{
    SIMPLE_SENSOR_CONTEXT *ViewContext;
    EFI_IFR_GUID_LABEL    *StartLabel;
    EFI_IFR_GUID_LABEL    *EndLabel;
    VOID                  *StartOpCodeHandle;
    VOID                  *EndOpCodeHandle;
    SIMPLE_SENSOR_INFO     SensorInfo;
    UINTN                  GroupIdx;
    UINTN                  SensorIdx;
    SIMPLE_SENSOR_ELEMENT *Group;
    SIMPLE_SENSOR_ELEMENT *Sensor;
    EFI_STATUS             Status;

    //
    // Initialize ViewContext data
    //
    ViewContext = AllocateZeroPool (sizeof (SIMPLE_SENSOR_CONTEXT));

    if (ViewContext == NULL) {
        DEBUG ((EFI_D_ERROR, "%a: Error when allocating memory", __FUNCTION__));
        Status = EFI_OUT_OF_RESOURCES;
        goto Exit;
    }

    //
    // Open SimpleSensor protocol
    //
    Status = gBS->LocateProtocol(&gSimpleSensorProtocolGuid, NULL,
                                 (VOID **)&ViewContext->SimpleSensor);

    if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "%a: Error when locating sensor protocol (%r)",
               __FUNCTION__, Status));
        goto Exit;
    }

    ViewContext->Signature = SIMPLE_SENSOR_CONTEXT_SIGNATURE;

#if 0
    ViewContext->ConfigAccess.ExtractConfig = SimpleSensorHiiConfigAccessExtractConfig;
    ViewContext->ConfigAccess.RouteConfig   = SimpleSensorHiiConfigAccessRouteConfig;
    ViewContext->ConfigAccess.Callback      = SimpleSensorHiiConfigAccessCallback;

    //
    // Locate ConfigRouting protocol
    //
    Status = gBS->LocateProtocol (
        &gEfiHiiConfigRoutingProtocolGuid,
        NULL,
        (VOID **) &ViewContext->ConfigRouting);

    if (EFI_ERROR (Status)) {
        DEBUG((EFI_D_ERROR, "%a: Error when locating config protocol (%r)",
               __FUNCTION__, Status));
        goto Exit;
    }
#endif

    //
    // Publish froms and config access
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
        &ViewContext->DriverHandle,
        &gEfiDevicePathProtocolGuid,
        &mHiiVendorDevicePath,
        //&gEfiHiiConfigAccessProtocolGuid,
        //&ViewContext->ConfigAccess,
        NULL);

    if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "%a: Error when installing protocols (%r)", __FUNCTION__, Status));
        goto Exit;
    }

    // Count available sensors
    for (SensorIdx = 0, SensorInfo.Id = 0;
         (Status = ViewContext->SimpleSensor->GetInfo(
             ViewContext->SimpleSensor, &SensorInfo)) == EFI_SUCCESS;
         SensorIdx++, SensorInfo.Id++) {
    }

    if (EFI_ERROR (Status) && Status != EFI_NOT_FOUND) {
        DEBUG ((EFI_D_ERROR, "%a: Error when requesting sensor #%u info (%r)",
                __FUNCTION__, SensorIdx, Status));
        goto Exit;
    }

    ViewContext->NumSensors = SensorIdx;
    ViewContext->Sensor = AllocateZeroPool (ViewContext->NumSensors *
                                            sizeof(SIMPLE_SENSOR_ELEMENT));

    if (ViewContext->Sensor == NULL) {
        DEBUG ((EFI_D_ERROR, "%a: Error when allocating memory", __FUNCTION__));
        Status = EFI_OUT_OF_RESOURCES;
        goto Exit;
    }

    for (SensorIdx = 0, SensorInfo.Id = 0;
         (Status = ViewContext->SimpleSensor->GetInfo(
             ViewContext->SimpleSensor, &SensorInfo)) == EFI_SUCCESS;
         SensorIdx++, SensorInfo.Id++) {
      ViewContext->Sensor[SensorIdx].Info = SensorInfo;
    }

    if (EFI_ERROR(Status) && Status != EFI_NOT_FOUND) {
      DEBUG((EFI_D_ERROR, "%a: Error when requesting sensor #%u info (%r)",
             __FUNCTION__, SensorIdx, Status));
      goto Exit;
    }

    //
    // Publish HII packages
    //
    ViewContext->HiiHandle = HiiAddPackages (
        &mConfigFormSetGuid,
        ViewContext->DriverHandle,
        SimpleSensorFormBin,
        SimpleSensorUiLibStrings,
        NULL);

    ASSERT (ViewContext->HiiHandle != NULL);

    StartOpCodeHandle = HiiAllocateOpCodeHandle();
    ASSERT(StartOpCodeHandle != NULL);

    EndOpCodeHandle = HiiAllocateOpCodeHandle();
    ASSERT(EndOpCodeHandle != NULL);

    StartLabel = (EFI_IFR_GUID_LABEL *)HiiCreateGuidOpCode(
        StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof(EFI_IFR_GUID_LABEL));
    StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
    StartLabel->Number = SIMPLE_SENSOR_LIST_START;

    EndLabel = (EFI_IFR_GUID_LABEL *)HiiCreateGuidOpCode (
        EndOpCodeHandle, &gEfiIfrTianoGuid, NULL,
        sizeof(EFI_IFR_GUID_LABEL));
    EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
    EndLabel->Number = SIMPLE_SENSOR_LIST_END;

    for (GroupIdx = 0; GroupIdx < ViewContext->NumSensors; GroupIdx++) {
        Group = &ViewContext->Sensor[GroupIdx];

        if (Group->Info.Type != SIMPLE_SENSOR_TYPE_GROUP) {
            continue;
        }

        HiiCreateTextOpCode (StartOpCodeHandle,
                             STRING_TOKEN(STR_SIMPLE_SENSOR_EMPTY),
                             STRING_TOKEN(STR_SIMPLE_SENSOR_EMPTY),
                             STRING_TOKEN(STR_SIMPLE_SENSOR_EMPTY));

        Group->NameId = HiiSetString (ViewContext->HiiHandle, 0, Group->Info.Name, NULL);

        HiiCreateSubTitleOpCode (StartOpCodeHandle, Group->NameId,
                                 STRING_TOKEN(STR_SIMPLE_SENSOR_EMPTY),
                                 0, 0);

        for (SensorIdx = 0; SensorIdx < ViewContext->NumSensors; SensorIdx++) {
            Sensor = &ViewContext->Sensor[SensorIdx];

            if (Sensor->Info.Type == SIMPLE_SENSOR_TYPE_GROUP ||
                Sensor->Info.GrpId != Group->Info.Id) {
                continue;
            }

            Sensor->NameId = HiiSetString (ViewContext->HiiHandle, 0, Sensor->Info.Name, NULL);
            Sensor->ValueId = HiiSetString (ViewContext->HiiHandle, 0, Sensor->ValueStr, NULL);

            HiiCreateTextOpCode (StartOpCodeHandle, Sensor->NameId,
                                 STRING_TOKEN(STR_SIMPLE_SENSOR_EMPTY),
                                 Sensor->ValueId);
        }
    }

    HiiUpdateForm (ViewContext->HiiHandle,
                   &mConfigFormSetGuid,
                   SIMPLE_SENSOR_VIEW_FORM_ID,
                   StartOpCodeHandle,
                   EndOpCodeHandle);

    UpdateData (ViewContext, TRUE);

    HiiFreeOpCodeHandle (StartOpCodeHandle);
    HiiFreeOpCodeHandle (EndOpCodeHandle);

    // register sensors listener
    ViewContext->SensorsListener.Function = OnDataChange;
    ViewContext->SimpleSensor->Listen (ViewContext->SimpleSensor,
                                       &ViewContext->SensorsListener, TRUE);

    mViewContext = ViewContext;

    Status = EFI_SUCCESS;

 Exit:
    if (EFI_ERROR (Status)) {
        if (ViewContext->Sensor != NULL) {
            FreePool(ViewContext->Sensor);
        }

        if (ViewContext != NULL) {
            FreePool(ViewContext);
        }
    }

    return Status;
 }

EFI_STATUS
EFIAPI
SimpleSensorUiFinalize (
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
    SIMPLE_SENSOR_CONTEXT  *ViewContext;
    EFI_STATUS            Status;

    ViewContext = mViewContext;

    if (ViewContext == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    mViewContext = NULL;

    ASSERT (ViewContext->DriverHandle != NULL);
    ASSERT (ViewContext->HiiHandle != NULL);
    ASSERT (ViewContext->Sensor != NULL);

    // unregister sensors listener
    ViewContext->SimpleSensor->Listen(ViewContext->SimpleSensor,
                                      &ViewContext->SensorsListener, FALSE);

    FreePool(ViewContext->Sensor);

    //
    // Unpublish HII packages
    //
    HiiRemovePackages (ViewContext->HiiHandle);

    //
    // Unpublish forms and config access
    //
    Status = gBS->UninstallMultipleProtocolInterfaces (
        ViewContext->DriverHandle,
        &gEfiDevicePathProtocolGuid,
        &mHiiVendorDevicePath,
        NULL);

    //
    // Free ViewContext data
    //
    FreePool(ViewContext);

    return Status;
}
