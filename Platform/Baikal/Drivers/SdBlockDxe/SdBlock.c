/** @file
  Copyright (c) 2021 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/FdtClient.h>
#include <Protocol/FirmwareVolumeBlock.h>
#include <IndustryStandard/Sd.h>
#include <Protocol/Cpu.h>

#define IS_READ   TRUE
#define IS_WRITE  FALSE
#define SDHCI_BLOCK_SIZE_DEFAULT  512
#define SDHCI_PARTSIZE            (4 * 1024)

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

STATIC EFI_PHYSICAL_ADDRESS  Base;
STATIC EFI_PHYSICAL_ADDRESS  PhysicalBuffer;

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
    EFI_CALLER_ID_GUID
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

EFI_STATUS
SdPeimIdentification (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  OUT UINT64               *TotalSize
  );

EFI_STATUS
SdPeimRwBlocks (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Lba,
  IN  VOID                 *Buffer,
  IN  UINTN                 BufferSize,
  IN  UINTN                 IsRead
  );

STATIC
EFI_STATUS
EFIAPI
Reset (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  BOOLEAN                 ExtendedVerification
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  OUT VOID                   *Buffer
  )
{
  EFI_STATUS Status;
  UINT8  *Buf = Buffer;
  UINTN  Size = BufferSize;

  while (Size) {
    UINTN  Part = Size > SDHCI_PARTSIZE ? SDHCI_PARTSIZE : Size;
    Status = SdPeimRwBlocks (Base, Lba, (VOID *) PhysicalBuffer, Part, IS_READ);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    CopyMem ((VOID *) Buf, (VOID *) PhysicalBuffer, Part);
    Size -= Part;
    Buf  += Part;
    Lba  += Part / SDHCI_BLOCK_SIZE_DEFAULT;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
WriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  IN  VOID                   *Buffer
  )
{
  EFI_STATUS Status;
  UINT8  *Buf = Buffer;
  UINTN  Size = BufferSize;

  while (Size) {
    UINTN  Part = Size > SDHCI_PARTSIZE ? SDHCI_PARTSIZE : Size;
    CopyMem ((VOID *) PhysicalBuffer, (VOID *) Buf, Part);
    Status = SdPeimRwBlocks (Base, Lba, (VOID *) PhysicalBuffer, Part, IS_WRITE);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Size -= Part;
    Buf  += Part;
    Lba  += Part / SDHCI_BLOCK_SIZE_DEFAULT;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
FlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
GetBaseAdr (
  OUT  EFI_PHYSICAL_ADDRESS  *Reg
  )
{
  EFI_STATUS            Status;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  INT32                 Node;
  CONST VOID           *Prop;
  UINT32                PropSize;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Node = 0;
  Status = FdtClient->FindNextCompatibleNode (FdtClient, "snps,dwcmshc-sdhci", Node, &Node);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
    return EFI_DEVICE_ERROR;
  }

  Status = FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *Reg = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
  return Status;
}

STATIC
EFI_PHYSICAL_ADDRESS
AllocDmaBuffer (
  IN  UINTN  PhysicalBufferSize
  )
{
  EFI_STATUS Status;
  EFI_PHYSICAL_ADDRESS  PhysicalBuffer;
  EFI_CPU_ARCH_PROTOCOL *Cpu;

  if (!PhysicalBufferSize) {
    return 0;
  }

  PhysicalBuffer = (EFI_PHYSICAL_ADDRESS) (SIZE_4GB - 1);
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiReservedMemoryType,
                  EFI_SIZE_TO_PAGES (PhysicalBufferSize),
                  &PhysicalBuffer
                  );
  if (EFI_ERROR (Status)) {
    return 0;
  }

  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&Cpu);
  if (EFI_ERROR (Status)) {
    return 0;
  }

  Status = Cpu->SetMemoryAttributes (
                  Cpu, PhysicalBuffer,
                  PhysicalBufferSize,
                  EFI_MEMORY_WC
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePages (PhysicalBuffer, EFI_SIZE_TO_PAGES (PhysicalBufferSize));
    return 0;
  }

  return PhysicalBuffer;
}

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
  mBaikalSd.BlockIo.Reset          = Reset;
  mBaikalSd.BlockIo.ReadBlocks     = ReadBlocks;
  mBaikalSd.BlockIo.WriteBlocks    = WriteBlocks;
  mBaikalSd.BlockIo.FlushBlocks    = FlushBlocks;

  mBaikalSd.Media.MediaPresent     = TRUE;
  mBaikalSd.Media.RemovableMedia   = FALSE;
  mBaikalSd.Media.LogicalPartition = FALSE;
  mBaikalSd.Media.ReadOnly         = FALSE;
  mBaikalSd.Media.WriteCaching     = FALSE;
  mBaikalSd.Media.BlockSize        = SDHCI_BLOCK_SIZE_DEFAULT;
  mBaikalSd.Media.LastBlock        = mBaikalSd.Size / mBaikalSd.Media.BlockSize - 1;

  PhysicalBuffer = AllocDmaBuffer (SDHCI_PARTSIZE);
  if (!PhysicalBuffer) {
    return EFI_DEVICE_ERROR;
  }

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

  Status = GetBaseAdr (&Base);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: no device description in the device tree\n", __FUNCTION__));
    return Status;
  }

  Status = SdPeimIdentification (Base, &TotalSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = InstallBlock (TotalSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}
