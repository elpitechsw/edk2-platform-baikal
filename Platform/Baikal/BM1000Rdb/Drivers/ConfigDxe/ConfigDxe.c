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


STATIC
EFI_STATUS
SetupVariables (
  VOID
  )
{
  UINTN       Size;
  UINT32      Var32;
  EFI_STATUS  Status;

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

  return EFI_SUCCESS;
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
  }

  return EFI_SUCCESS;
}
