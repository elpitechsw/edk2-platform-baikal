/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/BlockIo.h>
#include <Library/SdLib.h>

typedef struct {
  VENDOR_DEVICE_PATH        Vendor;
  EFI_DEVICE_PATH_PROTOCOL  End;
} BAIKAL_SD_DEVICE_PATH;

typedef struct {
  UINTN                  Signature;
  EFI_BLOCK_IO_PROTOCOL  BlockIo;
  EFI_BLOCK_IO_MEDIA     Media;
  BAIKAL_SD_DEVICE_PATH  DevicePath;
  UINT64                 Size;
} BAIKAL_SD_PRIVATE_DATA;

STATIC EFI_HANDLE                   mBaikalSdBlockHandle;
STATIC BAIKAL_SD_PRIVATE_DATA       mBaikalSd;
STATIC CONST BAIKAL_SD_DEVICE_PATH  mBaikalSdDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 0),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    {
      0xB55D548F, 0x9C89, 0x4F6E, {0x87, 0xE1, 0xB8, 0x72, 0x15, 0xB7, 0xCA, 0x3C}
    },
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) ((sizeof (EFI_DEVICE_PATH_PROTOCOL)) >> 0),
      (UINT8) ((sizeof (EFI_DEVICE_PATH_PROTOCOL)) >> 8)
    }
  }
};

STATIC
EFI_STATUS
EFIAPI
InstallBlock (
  IN  UINT64  TotalSize
  )
{
  EFI_STATUS  Status;

  // Misc
  mBaikalSd.Size                   = TotalSize;
  mBaikalSd.Signature              = SIGNATURE_32 ('M', 'S', 'H', 'C');
  mBaikalSd.DevicePath             = mBaikalSdDevicePath;

  mBaikalSd.BlockIo.Media          = &mBaikalSd.Media;
  mBaikalSd.BlockIo.Revision       = EFI_BLOCK_IO_PROTOCOL_REVISION;
  mBaikalSd.BlockIo.Reset          = SdResetIo;
  mBaikalSd.BlockIo.ReadBlocks     = SdReadBlocksIo;
  mBaikalSd.BlockIo.WriteBlocks    = SdWriteBlocksIo;
  mBaikalSd.BlockIo.FlushBlocks    = SdFlushBlocksIo;

  mBaikalSd.Media.MediaPresent     = TRUE;
  mBaikalSd.Media.RemovableMedia   = FALSE;
  mBaikalSd.Media.LogicalPartition = FALSE;
  mBaikalSd.Media.ReadOnly         = FALSE;
  mBaikalSd.Media.WriteCaching     = FALSE;
  mBaikalSd.Media.BlockSize        = SDHCI_BLOCK_SIZE_DEFAULT;
  mBaikalSd.Media.LastBlock        = mBaikalSd.Size / mBaikalSd.Media.BlockSize - 1;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mBaikalSdBlockHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mBaikalSd.DevicePath,
                  &gEfiBlockIoProtocolGuid,
                  &mBaikalSd.BlockIo,
                  NULL
                  );

  return Status;
}

EFI_STATUS
EFIAPI
SdBlockEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  UINT64      TotalSize;

  Status = SdIdentification();
  if (EFI_ERROR (Status)) {
    return EFI_NO_MEDIA;
  }

  TotalSize = SdTotalSize();
  if (TotalSize == 0) {
    return EFI_DEVICE_ERROR;
  }

  Status = InstallBlock (TotalSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}
