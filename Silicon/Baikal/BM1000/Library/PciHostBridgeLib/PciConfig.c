/** @file
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HiiLib.h>
//#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include "PciConfig.h"

extern UINT8  PciConfigHiiBin[];
extern UINT8  PciHostBridgeLibStrings[];

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
    PCI_CONFIG_GUID
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

EFI_STATUS
PciConfigInstallHii(
  UINT32 SegmentMask
  )
{
  EFI_STATUS      Status;
  EFI_HII_HANDLE  HiiHandle;
  EFI_HANDLE      DriverHandle;
  UINT32          SegmentMaskVar;
  PCI_CONFIG_VARSTORE_DATA PciConfig;
  UINTN           Size;

  Size = sizeof (UINT32);
  Status = gRT->GetVariable (
                  L"SegmentMask",
                  &gPciConfigGuid,
                  NULL,
                  &Size,
                  &SegmentMaskVar
                  );
  if (EFI_ERROR(Status) || SegmentMaskVar != SegmentMask) {
    /* First start: Create 'SegmentMask' and 'PciConfig' variables. */
    SegmentMaskVar = SegmentMask;
    Size = sizeof (UINT32);
    Status = gRT->SetVariable (
                    L"SegmentMask",
                    &gPciConfigGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                    Size,
                    &SegmentMaskVar
                    );
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Can't set SegmentMask var (%r)\n", Status));
    }
    Size = sizeof (PCI_CONFIG_VARSTORE_DATA);
    ZeroMem(&PciConfig, Size);
    Status = gRT->SetVariable (
                    L"PciConfig",
                    &gPciConfigGuid,
                    EFI_VARIABLE_NON_VOLATILE |
                    EFI_VARIABLE_BOOTSERVICE_ACCESS |
                    EFI_VARIABLE_RUNTIME_ACCESS,
                    Size,
                    &PciConfig
                    );
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Can't set PciConfig var (%r)\n", Status));
    }
  }

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
                &gPciConfigGuid,
                DriverHandle,
                PciHostBridgeLibStrings,
                PciConfigHiiBin,
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

