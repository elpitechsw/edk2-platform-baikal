/** @file
  Copyright (c) 2021 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Guid/SystemNvDataGuid.h>
#include <Guid/VariableFormat.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Platform/FlashMap.h>
#include <Protocol/BlockIo.h>
#include <Protocol/FirmwareVolumeBlock.h>
#include <Library/EspiLib.h>
#include <BM1000.h>

#define ESPI_FLASH_PRIVATE_DATA_SIGNATURE  SIGNATURE_32 ('E', 'S', 'P', 'I')
#define ESPI_FLASH_PRIVATE_FROM_BLKIO(a)   CR (a, ESPI_FLASH_PRIVATE_DATA, BlockIo, ESPI_FLASH_PRIVATE_DATA_SIGNATURE)
#define ESPI_FLASH_GPIO_CS  0

#define DIVIDE_ROUND_UP(x,n)  (((x) + (n) - 1) / (n))
#define FAT_BLOCK_SIZE  512

typedef struct {
  VENDOR_DEVICE_PATH        Vendor;
  EFI_DEVICE_PATH_PROTOCOL  End;
} ESPI_FLASH_DEVICE_PATH;

typedef struct {
  UINTN                   Signature;
  EFI_BLOCK_IO_PROTOCOL   BlockIo;
  EFI_BLOCK_IO_MEDIA      Media;
  ESPI_FLASH_DEVICE_PATH  DevicePath;
  UINT64                  Size;
} ESPI_FLASH_PRIVATE_DATA;

STATIC UINTN  SectorSize;
STATIC UINTN  SectorCount;

STATIC EFI_HANDLE               mEspiFlashBlockIoHandle;
STATIC ESPI_FLASH_PRIVATE_DATA  mEspiFlash;
STATIC ESPI_FLASH_DEVICE_PATH   mEspiFlashBlockIoDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) (sizeof (VENDOR_DEVICE_PATH) >> 8)
      }
    },
    {
      0xE50024B7, 0x5945, 0x4EE1, {0x95, 0x31, 0x77, 0x57, 0xA8, 0xCC, 0xEC, 0xC6}
    },
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      0
    }
  }
};

//
// EFI_BLOCK_IO_PROTOCOL
//
EFI_STATUS
EFIAPI
EspiFlashBlockIoReset (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN BOOLEAN                ExtendedVerification
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
EspiFlashBlockIoReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  OUT VOID                   *Buffer
  )
{
  ESPI_FLASH_PRIVATE_DATA  *PrivateData;
  UINTN                     NumberOfBlocks;
  UINTN                     FlashOffset;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  PrivateData = ESPI_FLASH_PRIVATE_FROM_BLKIO (This);

  if (MediaId != PrivateData->Media.MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  if ((BufferSize % PrivateData->Media.BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  if (Lba > PrivateData->Media.LastBlock) {
    return EFI_INVALID_PARAMETER;
  }

  NumberOfBlocks = BufferSize / PrivateData->Media.BlockSize;
  if ((Lba + NumberOfBlocks - 1) > PrivateData->Media.LastBlock) {
    return EFI_INVALID_PARAMETER;
  }

  FlashOffset = Lba * FAT_BLOCK_SIZE;
  EspiRead (BM1000_ESPI_BASE, ESPI_FLASH_GPIO_CS, FlashOffset, Buffer, BufferSize);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
EspiFlashBlockIoWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                  MediaId,
  IN EFI_LBA                 Lba,
  IN UINTN                   BufferSize,
  IN VOID                   *Buffer
  )
{
  ESPI_FLASH_PRIVATE_DATA  *PrivateData;
  UINTN                    NumberOfBlocks;
  UINTN                    FlashOffset;
  UINTN                    Adr;
  UINTN                    Offset;
  UINTN                    TotalSize;
  UINTN                    Cnt;
  UINTN                    Size;
  VOID                     *Buf;

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  PrivateData = ESPI_FLASH_PRIVATE_FROM_BLKIO (This);

  if (MediaId != PrivateData->Media.MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  if (TRUE == PrivateData->Media.ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }

  if ((BufferSize % PrivateData->Media.BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  if (Lba > PrivateData->Media.LastBlock) {
    return EFI_INVALID_PARAMETER;
  }

  NumberOfBlocks = BufferSize / PrivateData->Media.BlockSize;
  if ((Lba + NumberOfBlocks - 1) > PrivateData->Media.LastBlock) {
    return EFI_INVALID_PARAMETER;
  }

  FlashOffset = Lba * FAT_BLOCK_SIZE;
  Adr = (FlashOffset / SectorSize) * SectorSize;
  Offset = FlashOffset - Adr;
  TotalSize = Offset + BufferSize;
  Cnt = DIVIDE_ROUND_UP (TotalSize, SectorSize);
  Size = Cnt * SectorSize;

  Buf = AllocatePool (Size);
  if (Buf == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  EspiRead (BM1000_ESPI_BASE, ESPI_FLASH_GPIO_CS, Adr, Buf, Size);
  CopyMem (Buf + Offset, Buffer, BufferSize);
  EspiErase (BM1000_ESPI_BASE, ESPI_FLASH_GPIO_CS, Adr, Size);
  EspiWrite (BM1000_ESPI_BASE, ESPI_FLASH_GPIO_CS, Adr, Buf, Size);

  if (Buf != NULL) {
    FreePool (Buf);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
EspiFlashBlockIoFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
{
  return EFI_SUCCESS;
}

EFI_BLOCK_IO_PROTOCOL  mEspiFlashBlockIoProtocol = {
  EFI_BLOCK_IO_PROTOCOL_REVISION,
  (EFI_BLOCK_IO_MEDIA *) 0,
  EspiFlashBlockIoReset,
  EspiFlashBlockIoReadBlocks,
  EspiFlashBlockIoWriteBlocks,
  EspiFlashBlockIoFlushBlocks
};

EFI_STATUS
EFIAPI
EspiFlashInstallBlock (
  VOID
  )
{
  ESPI_FLASH_PRIVATE_DATA  *PrivateData = &mEspiFlash;
  EFI_BLOCK_IO_PROTOCOL    *BlockIo;
  EFI_BLOCK_IO_MEDIA       *Media;
  EFI_STATUS               Status;

  BlockIo = &PrivateData->BlockIo;
  Media   = &PrivateData->Media;

  PrivateData->Size = SectorSize * SectorCount;
  PrivateData->Signature = ESPI_FLASH_PRIVATE_DATA_SIGNATURE;
  if (PrivateData->Size < FAT_BLOCK_SIZE) {
    return EFI_NO_MEDIA;
  }

  CopyMem (BlockIo, &mEspiFlashBlockIoProtocol, sizeof (EFI_BLOCK_IO_PROTOCOL));

  BlockIo->Media          = Media;
  Media->RemovableMedia   = FALSE;
  Media->MediaPresent     = TRUE;
  Media->LogicalPartition = FALSE;
  Media->ReadOnly         = FALSE;
  Media->WriteCaching     = FALSE;
  Media->BlockSize        = FAT_BLOCK_SIZE;
  Media->LastBlock        = DivU64x32 (PrivateData->Size + FAT_BLOCK_SIZE - 1, FAT_BLOCK_SIZE) - 1;
  PrivateData->DevicePath = mEspiFlashBlockIoDevicePath;

  if (Media->LastBlock < 1) {
    return EFI_NO_MEDIA;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mEspiFlashBlockIoHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mEspiFlash.DevicePath,
                  &gEfiBlockIoProtocolGuid,
                  &mEspiFlash.BlockIo,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

EFI_STATUS
EFIAPI
EspiFlashBlockIoDxeInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  INTN        Err;
  EFI_STATUS  Status;

  Err = EspiInit (BM1000_ESPI_BASE);
  if (Err) {
    return EFI_DEVICE_ERROR;
  }

  Err = EspiDetect (BM1000_ESPI_BASE, ESPI_FLASH_GPIO_CS);
  if (Err) {
    return EFI_SUCCESS; // No media
  }

  Err = EspiInfo (BM1000_ESPI_BASE, ESPI_FLASH_GPIO_CS, &SectorSize, &SectorCount);
  if (Err) {
    return EFI_DEVICE_ERROR;
  }

  Status = EspiFlashInstallBlock ();
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  return EFI_SUCCESS;
}
