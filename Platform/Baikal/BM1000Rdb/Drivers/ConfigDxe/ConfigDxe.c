/** @file
 *
 *  Copyright (c) 2019 - 2020, ARM Limited. All rights reserved.
 *  Copyright (c) 2018 - 2021, Andrei Warkentin <andrey.warkentin@gmail.com>
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Platform/ConfigVars.h>
#include <Protocol/FruClient.h>
#include <Protocol/FdtClient.h>
#include "ConfigDxeFormSetGuid.h"

extern UINT8  ConfigDxeHiiBin[];
extern UINT8  ConfigDxeStrings[];

typedef struct {
  VENDOR_DEVICE_PATH        VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  End;
} HII_VENDOR_DEVICE_PATH;

STATIC HII_VENDOR_DEVICE_PATH  mVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    CONFIG_DXE_FORM_SET_GUID
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

STATIC EFI_EVENT  mReadyToBootEvent;

STATIC
EFI_STATUS
InstallHiiPages (
  VOID
  )
{
  EFI_STATUS      Status;
  EFI_HII_HANDLE  HiiHandle;
  EFI_HANDLE      DriverHandle;

  DriverHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mVendorDevicePath,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  HiiHandle = HiiAddPackages (
                &gConfigDxeFormSetGuid,
                DriverHandle,
                ConfigDxeStrings,
                ConfigDxeHiiBin,
                NULL
                );
  if (HiiHandle == NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           DriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mVendorDevicePath,
           NULL
           );
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

STATIC UINT32 mSavedPowerPolicy;

STATIC
EFI_STATUS
SetupVariables (
  VOID
  )
{
  UINTN                 Size;
  UINT32                Var32;
  FRU_CLIENT_PROTOCOL  *FruClient;
  EFI_STATUS            Status;

  Size = sizeof (UINT32);
  Status = gRT->GetVariable (
                  L"AcpiMsi",
                  &gConfigDxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var32
                  );
  if (EFI_ERROR (Status)) {
    Status = PcdSet32S (PcdAcpiMsiMode, PcdGet32 (PcdAcpiMsiMode));
    ASSERT_EFI_ERROR (Status);
  }

  Size = sizeof (UINT32);
  Status = gRT->GetVariable (
                  L"AcpiPcie",
                  &gConfigDxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var32
                  );
  if (EFI_ERROR (Status)) {
    Status = PcdSet32S (PcdAcpiPcieMode, PcdGet32 (PcdAcpiPcieMode));
    ASSERT_EFI_ERROR (Status);
  }

  Size = sizeof (UINT32);
  Status = gRT->GetVariable (
                  L"VduLvds",
                  &gConfigDxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var32
                  );
  if (EFI_ERROR (Status)) {
    Status = PcdSet32S (PcdVduLvdsMode, PcdGet32 (PcdVduLvdsMode));
    ASSERT_EFI_ERROR (Status);
  }

  Size = sizeof (UINT32);
  Status = gRT->GetVariable (
                  L"Uart1",
                  &gConfigDxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var32
                  );
  if (EFI_ERROR (Status)) {
    Status = PcdSet32S (PcdUart1Mode, PcdGet32 (PcdUart1Mode));
    ASSERT_EFI_ERROR (Status);
  }

  Status = gBS->LocateProtocol (
                  &gFruClientProtocolGuid,
                  NULL,
                  (VOID **) &FruClient
                  );
  if (Status == EFI_SUCCESS) {
    Var32 = FruClient->GetBoardPowerPolicy();
    if (Var32 == FRU_POWERPOLICY_ON) {
      mSavedPowerPolicy = PPOL_AUTO;
    } else {
      mSavedPowerPolicy = PPOL_OFF;
    }
  }

  Size = sizeof (UINT32);
  Status = gRT->GetVariable (
                  L"PowerPolicy",
                  &gConfigDxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var32
                  );
  if (EFI_ERROR (Status)) {
    Var32 = mSavedPowerPolicy;
    Status = gRT->SetVariable (
                  L"PowerPolicy",
                  &gConfigDxeFormSetGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof(Var32),
                  &Var32
                  );
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}

#if defined(ELP_2) || defined(ELP_3) || defined(ELP_5)
STATIC
VOID
EFIAPI
FixupFdt (
  VOID
  )
{
  FDT_CLIENT_PROTOCOL             *FdtClient;
  EFI_STATUS                       Status;
  INT32                            Node = 0, SubNode;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    return;
  }

#if defined(ELP_2) || defined(ELP_5) /* et-101 family board */
  if (PcdGet32(PcdVduLvdsMode) == 0) {
    Status = FdtClient->FindNodeByAlias (FdtClient, "vdu-lvds", &Node);
    if(Status == EFI_SUCCESS) {
      Status = FdtClient->SetNodeProperty (
            FdtClient,
            Node,
            "status",
            "disabled",
            9
	    );
    }
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Can't update vdu_lvds status - %r\n", Status));
    }
  }
#endif

  if (PcdGet32(PcdUart1Mode) == 0) {
    Status = FdtClient->FindNodeByAlias (FdtClient, "serial1", &Node);
    if(Status == EFI_SUCCESS) {
      Status = FdtClient->FindNextSubnode(FdtClient, "ps2mult", Node, &SubNode);
      if(Status == EFI_SUCCESS) {
        Status = FdtClient->DeleteNode(FdtClient, SubNode);
      }
    }
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Can't delete ps2mult node - %r\n", Status));
    }
  }
}
#endif

STATIC
VOID
EFIAPI
OnReadyToBoot (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  UINT32                Var32;
  UINTN                 Size;
  FRU_CLIENT_PROTOCOL  *FruClient;
  EFI_STATUS            Status;

#if defined(ELP_2) || defined(ELP_3) || defined(ELP_5)
  FixupFdt();
#endif

  Size = sizeof (UINT32);
  Status = gRT->GetVariable (
                  L"PowerPolicy",
                  &gConfigDxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var32
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: variable '%s' could not be read - bailing!\n", __FUNCTION__, L"PowerPolicy"));
    return;
  }

  if (Var32 == mSavedPowerPolicy) {
    DEBUG ((DEBUG_INFO, "%a: PowerPolicy not changed - doing nothing\n", __FUNCTION__));
    return;
  }

  Status = gBS->LocateProtocol (
                  &gFruClientProtocolGuid,
                  NULL,
                  (VOID **) &FruClient
                  );
  if (Status == EFI_SUCCESS) {
    Status = FruClient->SetBoardPowerPolicy(
                 (Var32 == PPOL_AUTO)?
                    FRU_POWERPOLICY_ON:
                    FRU_POWERPOLICY_OFF
                 );
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "%a: couldn't not save PowerPolicy: %r\n", __FUNCTION__, Status));
    }
  } else {
    DEBUG((DEBUG_ERROR, "%a: can't get FruClientProtocol: %r\n", __FUNCTION__, Status));
  }

}

EFI_STATUS
EFIAPI
ConfigDxeInitialize (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  Status = SetupVariables ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: couldn't not setup NV vars: %r\n", __FUNCTION__, Status));
  }

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gConfigDxeProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  Status = InstallHiiPages ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: couldn't install HiiPages: %r\n", __FUNCTION__, Status));
    return Status;
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  OnReadyToBoot,
                  NULL,
                  &gEfiEventReadyToBootGuid,
                  &mReadyToBootEvent
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: couldn't install OnReadyToBoot handler: %r\n", __FUNCTION__, Status));
    return Status;
  }
  return EFI_SUCCESS;
}
