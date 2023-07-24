#include "PmBus.h"

#define PAGE_BITS   8, 16
#define SENSOR_BITS 0, 8

#define NUM_PAGES(PageMask) BitFieldCountOnes32(PageMask, 0, 31)
#define HAS_PAGE(PageMask, Page) ((((PageMask) >> (Page)) & 1) != 0)
#define GET_PAGE(PageMask, PageIdx) (BitFieldIndexOnes32 (PageMask, PageIdx))
#define GET_PAGE_IDX(PageMask, Page) ((Page) == 0 ? 0 : BitFieldCountOnes32(PageMask, 0, (Page) - 1))

#define NUM_SENSORS(SensorMask) BitFieldCountOnes32(SensorMask, 0, 15)
#define HAS_SENSOR(SensorMask, Sensor) ((((SensorMask) >> (Sensor)) & 1) != 0)
#define GET_SENSOR(SensorMask, SensorIdx) (BitFieldIndexOnes32(SensorMask, SensorIdx))
#define GET_SENSOR_IDX(SensorMask, Sensor) ((Sensor) == 0 ? 0 : BitFieldCountOnes32(SensorMask, 0, (Sensor) - 1))

STATIC CONST struct {
  PMB_CMD       Cmd;
  UINT8         Type;
  INT8          Exp2;
  CONST CHAR16 *Name;
} PmBusSensors[] = {
  {PMB_CMD_READ_VIN, SIMPLE_SENSOR_TYPE_VOLT, PMBUS_EXP2_VOLT, L"Vin"},
  {PMB_CMD_READ_VOUT, SIMPLE_SENSOR_TYPE_VOLT, PMBUS_EXP2_VOLT, L"Vout"},
  {PMB_CMD_READ_IIN, SIMPLE_SENSOR_TYPE_AMP, PMBUS_EXP2_AMP, L"Iin"},
  {PMB_CMD_READ_IOUT, SIMPLE_SENSOR_TYPE_AMP, PMBUS_EXP2_AMP, L"Iout"},
  {PMB_CMD_READ_PIN, SIMPLE_SENSOR_TYPE_WATT, PMBUS_EXP2_WATT, L"Pin"},
  {PMB_CMD_READ_POUT, SIMPLE_SENSOR_TYPE_WATT, PMBUS_EXP2_WATT, L"Pout"},
  {PMB_CMD_READ_EIN, 0, 0, L"Ein"},  // TODO:
  {PMB_CMD_READ_EOUT, 0, 0, L"Eout"}, // TODO:

  {PMB_CMD_READ_TEMPERATURE_1, SIMPLE_SENSOR_TYPE_CDEG, PMBUS_EXP2_CDEG, L"T1"},
  {PMB_CMD_READ_TEMPERATURE_2, SIMPLE_SENSOR_TYPE_CDEG, PMBUS_EXP2_CDEG, L"T2"},
  {PMB_CMD_READ_TEMPERATURE_3, SIMPLE_SENSOR_TYPE_CDEG, PMBUS_EXP2_CDEG, L"T3"},

  {PMB_CMD_READ_FAN_SPEED_1, SIMPLE_SENSOR_TYPE_RPM, PMBUS_EXP2_RPM, L"Fan1"},
  {PMB_CMD_READ_FAN_SPEED_2, SIMPLE_SENSOR_TYPE_RPM, PMBUS_EXP2_RPM, L"Fan2"},
  {PMB_CMD_READ_FAN_SPEED_3, SIMPLE_SENSOR_TYPE_RPM, PMBUS_EXP2_RPM, L"Fan3"},
  {PMB_CMD_READ_FAN_SPEED_4, SIMPLE_SENSOR_TYPE_RPM, PMBUS_EXP2_RPM, L"Fan4"},
};

STATIC EFIAPI INTN
BitFieldIndexOnes32 (
  IN UINT32 Value,
  IN UINTN  Index
  )
{
  INTN Count;

  Index ++;

  for (Count = 0; Value != 0 && Index > 0; Value >>= 1, Count ++) {
    if (Value & 1) {
      Index --;
    }
  }

  return Value == 0 && Index > 0 ? -1 : Count - 1;
}

STATIC EFIAPI EFI_STATUS
PmBusSimpleSensorGetInfo (
  IN     SIMPLE_SENSOR_PROTOCOL *Proto,
  IN OUT SIMPLE_SENSOR_INFO     *Info
  )
{
  PMBUS_CONTEXT                 *Context = BASE_CR (Proto, PMBUS_CONTEXT, SimpleSensor);
  UINTN                          NumPages = NUM_PAGES (Context->PageMask);
  UINTN                          NumValues;
  UINT8                          Page;
  UINT8                          Sensor;
  UINT8                          PageBcd;
  INTN                           PageIdx;
  INTN                           ValueIdx;

  if (NumPages <= 0) {
    return EFI_NOT_FOUND;
  }

  if (Info->Id <= SIMPLE_SENSOR_ROOT_ID) {
    // Seek to first page id
    Info->Id = SIMPLE_SENSOR_ROOT_ID + 1;
  }

  if (Info->Id <= NumPages + SIMPLE_SENSOR_ROOT_ID) {
    // Is a page (group)
    PageIdx = Info->Id - 1 - SIMPLE_SENSOR_ROOT_ID;
    Page = GET_PAGE(Context->PageMask, PageIdx);
    PageBcd = DecimalToBcd8(Page);

    Info->GrpId = SIMPLE_SENSOR_ROOT_ID;
    Info->Type = SIMPLE_SENSOR_TYPE_GROUP;
    Info->Exp2 = 0;

    StrCpyS (Info->Name, ARRAY_SIZE (Info->Name) - 1, L"Page");
    if (PageBcd & 0xf0) {
      Info->Name[4] = (PageBcd >> 4) + '0';
      Info->Name[5] = (PageBcd & 0xf) + '0';
      Info->Name[6] = '\0';
    } else {
      Info->Name[4] = (PageBcd & 0xf) + '0';
      Info->Name[5] = '\0';
    }

    goto Exit;
  }

  NumValues = Context->PageData[NumPages - 1].ValueOffset +
    NUM_SENSORS (Context->PageData[NumPages - 1].SensorMask);

  ValueIdx = Info->Id - 1 - SIMPLE_SENSOR_ROOT_ID - NumPages;

  if (ValueIdx < NumValues) {
    // Is a sensor
    // Find page
    for (PageIdx = 0; PageIdx < NumPages - 1; PageIdx ++) {
      if (ValueIdx < Context->PageData[PageIdx + 1].ValueOffset) {
        break;
      }
    }

    ValueIdx -= Context->PageData[PageIdx].ValueOffset;
    Sensor = GET_SENSOR (Context->PageData[PageIdx].SensorMask, ValueIdx);

    Info->GrpId = PageIdx + 1;
    Info->Type = PmBusSensors[Sensor].Type;
    Info->Exp2 = PmBusSensors[Sensor].Exp2;

    StrCpyS (Info->Name, ARRAY_SIZE(Info->Name) - 1, PmBusSensors[Sensor].Name);

    goto Exit;
  }

  return EFI_NOT_FOUND;

Exit:
  return EFI_SUCCESS;
}

STATIC EFIAPI EFI_STATUS
PmBusSimpleSensorGetData (
  IN    SIMPLE_SENSOR_PROTOCOL *Proto,
  IN    SIMPLE_SENSOR_ID        Id,
  OUT   SIMPLE_SENSOR_VAL      *Val
  )
{
  PMBUS_CONTEXT                *Context = BASE_CR (Proto, PMBUS_CONTEXT, SimpleSensor);
  UINTN                         NumPages = NUM_PAGES(Context->PageMask);
  UINTN                         NumValues;
  UINTN                         ValueIdx;

  if (NumPages == 0) {
    return EFI_NOT_FOUND;
  }

  if (Id <= NumPages + SIMPLE_SENSOR_ROOT_ID) {
    return EFI_INVALID_PARAMETER;
  }

  NumValues = Context->PageData[NumPages - 1].ValueOffset +
              NUM_SENSORS(Context->PageData[NumPages - 1].SensorMask);
  ValueIdx = Id - 1 - SIMPLE_SENSOR_ROOT_ID - NumPages;

  if (ValueIdx >= NumValues) {
    return EFI_NOT_FOUND;
  }

  *Val = Context->Value[ValueIdx];

  return EFI_SUCCESS;
}

STATIC EFIAPI EFI_STATUS
PmBusSimpleSensorListen (
  IN     SIMPLE_SENSOR_PROTOCOL *Proto,
  IN OUT SIMPLE_SENSOR_LISTENER *Listener,
  IN     BOOLEAN Register
  )
{
  PMBUS_CONTEXT *Context = BASE_CR (Proto, PMBUS_CONTEXT, SimpleSensor);

  if (Register) {
    if (!IsNodeInList (&Context->Listeners, &Listener->Entry)) {
      InsertHeadList (&Context->Listeners, &Listener->Entry);
    }
  } else {
    if (IsNodeInList (&Context->Listeners, &Listener->Entry)) {
      RemoveEntryList (&Listener->Entry);
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
PmBusIoSafe (
  IN     VOID          *IoCtx,
  IN     PMBUS_IO_FN    IoFn,
  IN     UINT8          Dir,
  IN OUT PMB_IO_TRANS  *Trans
  )
{
  PMB_IO_TRANS          TmpTrans;
  EFI_STATUS            Status;

  // Clear faults
  TmpTrans.Req.Cmd = PMB_CMD_CLEAR_FAULTS;
  Status = IoFn (IoCtx, PMB_IO_WRITE, &TmpTrans);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Send command
  Status = IoFn (IoCtx, Dir, Trans);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Read status
  TmpTrans.Req.Cmd = PMB_CMD_STATUS_BYTE;
  Status = IoFn (IoCtx, PMB_IO_READ, &TmpTrans);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (PMB_READ_BYTE(&TmpTrans.Res, 0) & PMB_STATUS_BYTE_CML) {
    DEBUG ((EFI_D_WARN, "%a: Error when executing PMBUS command 0x%x\n",
            __FUNCTION__, Trans->Req.Cmd));

    Status = EFI_DEVICE_ERROR;
  }

  return Status;
}

/**
  Scans the PMBUS device for sensors.
**/
EFI_STATUS
EFIAPI
PmBusScanSensors (
  IN     VOID                  *IoCtx,
  IN     PMBUS_IO_FN            IoFn,
  OUT    PMBUS_SCAN_DATA       *ScanData
  )
{
  EFI_STATUS                    Status;
  UINTN                         Page;
  UINTN                         Sensor;
  PMB_IO_TRANS                  Trans;

  ZeroMem (ScanData, sizeof (PMBUS_SCAN_DATA));

  //
  // Scan pages and sensors
  //
  for (Page = 0; Page <= PMB_MAX_PAGE; Page++) {
    Trans.Req.Cmd = PMB_CMD_PAGE;
    PMB_WRITE_BYTE(&Trans.Req, 0, Page);

    // Try to set page
    Status = IoFn (IoCtx, PMB_IO_WRITE, &Trans);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "%a: Unable to set PMBUS page: %r\n",
              __FUNCTION__, Status));
      return Status;
    }

    // Get current page
    Status = IoFn (IoCtx, PMB_IO_READ, &Trans);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "%a: Unable to get PMBUS page: %r\n", __FUNCTION__,
              Status));
      return Status;
    }

    if (PMB_READ_BYTE(&Trans.Res, 0) != Page) {
      continue;
    }

    // Page exists
    ScanData->PageMask |= 1 << Page;

    // Scan sensors
    for (Sensor = 0; Sensor < ARRAY_SIZE (PmBusSensors); Sensor++) {
      if (PmBusSensors[Sensor].Type == 0) {
        // Skip sensor if not implemented yet
        continue;
      }

      Trans.Req.Cmd = PmBusSensors[Sensor].Cmd;
      Status = PmBusIoSafe (IoCtx, IoFn, PMB_IO_READ, &Trans);

      if (!EFI_ERROR (Status)) {
        ScanData->SensorMask[ScanData->NumPages] |=
          1 << Sensor;
        ScanData->NumSensors ++;
      }
    }

    ScanData->NumPages ++;
  }

  DEBUG ((EFI_D_INFO, "%a: Found %u pages, %u sensors\n",
          __FUNCTION__, ScanData->NumPages, ScanData->NumSensors));

  return EFI_SUCCESS;
}

VOID
PmBusInitSensors (
  IN     VOID             *Memory,
  IN     PMBUS_SCAN_DATA  *ScanData,
  OUT    PMBUS_CONTEXT    *Context
  )
{
  UINTN PageIdx;
  UINTN ValueOff;

  Context->Value = (PMBUS_SENSOR_DATA*)Memory;
  Memory = (UINT8*)Memory + sizeof(PMBUS_SENSOR_DATA) * ScanData->NumSensors;

  Context->PageData = (PMBUS_PAGE_DATA*)Memory;

  Context->PageMask = ScanData->PageMask;

  ZeroMem(Context->Value,
          ScanData->NumSensors * sizeof(PMBUS_SENSOR_DATA));

  for (PageIdx = 0, ValueOff = 0;
       PageIdx < ScanData->NumPages;
       ValueOff += NUM_SENSORS(ScanData->SensorMask[PageIdx ++])) {
    Context->PageData[PageIdx] = (PMBUS_PAGE_DATA) {
      .SensorMask = ScanData->SensorMask[PageIdx],
      .ValueOffset = ValueOff,
    };
  }

  InitializeListHead (&Context->Listeners);

  Context->SimpleSensor = (SIMPLE_SENSOR_PROTOCOL) {
    .GetInfo = PmBusSimpleSensorGetInfo,
    .GetData = PmBusSimpleSensorGetData,
    .Listen  = PmBusSimpleSensorListen,
  };
}

STATIC
PMBUS_SENSOR_DATA
PmBusFromLinear11 (
  IN UINT16 Src,
  IN INT8 DstExp
  )
{
  PMBUS_SENSOR_DATA Val;
  INT8 Exp;

  Val = ((INT16)((Src & 0x7ff) << (16 - 11))) >> (16 - 11);
  Exp = ((INT16)((Src & (0x1f << 11)))) >> 11;

  Exp -= DstExp;

  return Exp < 0 ? Val >> -Exp : Val << Exp;
}

STATIC
PMBUS_SENSOR_DATA
PmBusFromULinear16(
  IN UINT16 SrcMnt,
  IN UINT8 SrcExp,
  IN INT8 DstExp
  )
{
  PMBUS_SENSOR_DATA Val;
  INT16 Exp;

  Val = (INT16)SrcMnt;
  Exp = ((INT8)((SrcExp & 0x1f) << (8 - 5))) >> (8 - 5);

  Exp -= DstExp;

  return Exp < 0 ? Val >> -Exp : Val << Exp;
}

EFI_STATUS
PmBusPollSensors (
  IN     VOID             *IoCtx,
  IN     PMBUS_IO_FN       IoFn,
  IN OUT PMBUS_CONTEXT    *Context
  )
{
  EFI_STATUS               Status;
  UINT8                    Page;
  UINT8                    Sensor;
  UINTN                    PageIdx;
  UINTN                    SensorIdx;
  PMB_IO_TRANS             Trans;
  UINTN                    ValueIdx;
  UINT16                   Temp;
  PMBUS_SENSOR_DATA        Data;
  PMBUS_PAGE_MASK          PageMask;
  PMBUS_SENSOR_MASK        SensorMask;

  //DEBUG((EFI_D_INFO, "%a: Poll sensors (page mask: %08x)\n", __FUNCTION__,
  //       Context->PageMask));

  for (Page = 0, PageMask = Context->PageMask;
       PageMask != 0; Page ++, PageMask >>= 1) {
    if ((PageMask & 1) == 0) {
      continue;
    }

    Trans.Req.Cmd = PMB_CMD_PAGE;
    PMB_WRITE_BYTE (&Trans.Req, 0, Page);

    // Set page
    Status = PmBusIoSafe (IoCtx, IoFn, PMB_IO_WRITE, &Trans);

    if (EFI_ERROR (Status)) {
      // ignore errors
      continue;
    }

    PageIdx = GET_PAGE_IDX (Context->PageMask, Page);

    //DEBUG((EFI_D_INFO, "%a: Read page: %u (#%u) (sensor mask: %04x)\n",
    //       __FUNCTION__, Page, PageIdx, Context->PageData[PageIdx].SensorMask));

    for (Sensor = 0, SensorMask = Context->PageData[PageIdx].SensorMask;
         SensorMask != 0 && Sensor < ARRAY_SIZE (PmBusSensors); Sensor ++, SensorMask >>= 1) {
      if ((SensorMask & 1) == 0) {
        continue;
      }

      //DEBUG((EFI_D_INFO, "%a: Read sensor: %u\n",
      //       __FUNCTION__, Sensor));

      Trans.Req.Cmd = PmBusSensors[Sensor].Cmd;
      // Read data
      Status = PmBusIoSafe (IoCtx, IoFn, PMB_IO_READ, &Trans);

      if (EFI_ERROR (Status)) {
        // ignore errors
        continue;
      }

      switch (PmBusSensors[Sensor].Cmd) {
      case PMB_CMD_READ_VOUT:
        Temp = PMB_READ_WORD (&Trans.Res, 0);

        Trans.Req.Cmd = PMB_CMD_VOUT_MODE;
        // Read VOUT_MODE
        Status = PmBusIoSafe (IoCtx, IoFn, PMB_IO_READ, &Trans);

        if (EFI_ERROR(Status)) {
          // ignore errors
          ValueIdx ++;
          continue;
        }

        Data = PmBusFromULinear16 (Temp, PMB_READ_BYTE(&Trans.Res, 0),
                                   PmBusSensors[Sensor].Exp2);
        break;
      default:
        Data = PmBusFromLinear11 (PMB_READ_WORD(&Trans.Res, 0),
                                  PmBusSensors[Sensor].Exp2);
        break;
      }

      SensorIdx =
        GET_PAGE_IDX(Context->PageData[PageIdx].SensorMask, Sensor);
      ValueIdx = Context->PageData[PageIdx].ValueOffset + SensorIdx;

      Context->Value[ValueIdx] = Data;
    }
  }

  return EFI_SUCCESS;
}

VOID
PmBusNotifyListeners(
  IN OUT PMBUS_CONTEXT *Context
  )
{
  LIST_ENTRY *Listener;
  SIMPLE_SENSOR_LISTENER *ListenerContainer;

  // Notify listeners
  if (!IsListEmpty(&Context->Listeners)) {
    for (Listener = GetFirstNode(&Context->Listeners);;
         Listener = GetNextNode(&Context->Listeners, Listener)) {
      ListenerContainer = BASE_CR(Listener, SIMPLE_SENSOR_LISTENER, Entry);
      // Call listener function
      ListenerContainer->Function(ListenerContainer);

      if (IsNodeAtEnd(&Context->Listeners, Listener)) {
        break;
      }
    }
  }
}
