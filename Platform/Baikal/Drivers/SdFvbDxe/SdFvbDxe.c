/** @file
  Copyright (c) 2020 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
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
#include <Library/SdLib.h>


/*
Device            Boot     Start       End   Sectors  Size Id Type
/dev/mmcblock0p1            2048     18432     16385    8M df BootIt
/dev/mmcblock0p2  *        20480    282623    262144  128M ef EFI (FAT-12/16/32)
/dev/mmcblock0p3          282624 120791039 120508416 57.5G 83 Linux
*/
#define SD_VAR_ADR  ((UINT64)((UINT64)2048 * (UINT64)SDHCI_BLOCK_SIZE_DEFAULT) + FLASH_MAP_VAR)


typedef struct {
  VENDOR_DEVICE_PATH        Vendor;
  EFI_DEVICE_PATH_PROTOCOL  End;
} SD_FVB_DEVICE_PATH;

STATIC SD_FVB_DEVICE_PATH  mSdFvbDevicePath = {
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
      0x1EB93F44, 0xDF73, 0x47B4, {0xB6, 0x01, 0xAE, 0xE3, 0xCD, 0x3F, 0xCB, 0xCA}
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

STATIC EFI_EVENT      mVirtualAddressChangeEvent;
STATIC EFI_HANDLE     mSdFvbHandle;
STATIC VOID          *mNvStorageBase;
STATIC CONST UINT64   mNvStorageSize =
                        FixedPcdGet32 (PcdFlashNvStorageVariableSize) +
                        FixedPcdGet32 (PcdFlashNvStorageFtwWorkingSize) +
                        FixedPcdGet32 (PcdFlashNvStorageFtwSpareSize);

/**
  The GetAttributes() function retrieves the attributes and
  current settings of the block.

  @param This       Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Attributes Pointer to EFI_FVB_ATTRIBUTES_2 in which the
                    attributes and current settings are
                    returned. Type EFI_FVB_ATTRIBUTES_2 is defined
                    in EFI_FIRMWARE_VOLUME_HEADER.

  @retval EFI_SUCCESS The firmware volume attributes were
                      returned.
**/
STATIC
EFI_STATUS
EFIAPI
SdFvbGetAttributes (
  IN  CONST EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  OUT       EFI_FVB_ATTRIBUTES_2                 *Attributes
  )
{
  *Attributes = EFI_FVB2_READ_ENABLED_CAP  | // Reads may be enabled
                EFI_FVB2_READ_STATUS       | // Reads are currently enabled
                EFI_FVB2_WRITE_STATUS      | // Writes are currently enabled
                EFI_FVB2_WRITE_ENABLED_CAP | // Writes may be enabled
                EFI_FVB2_STICKY_WRITE      | // A block erase is required to flip bits into EFI_FVB2_ERASE_POLARITY
                EFI_FVB2_MEMORY_MAPPED     | // It is memory mapped
                EFI_FVB2_ERASE_POLARITY;     // After erasure all bits take this value (i.e. '1')

  return EFI_SUCCESS;
}

/**
  The SetAttributes() function sets configurable firmware volume
  attributes and returns the new settings of the firmware volume.

  @param This         Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Attributes   On input, Attributes is a pointer to
                      EFI_FVB_ATTRIBUTES_2 that contains the
                      desired firmware volume settings. On
                      successful return, it contains the new
                      settings of the firmware volume. Type
                      EFI_FVB_ATTRIBUTES_2 is defined in
                      EFI_FIRMWARE_VOLUME_HEADER.

  @retval EFI_SUCCESS           The firmware volume attributes were returned.

  @retval EFI_INVALID_PARAMETER The attributes requested are in
                                conflict with the capabilities
                                as declared in the firmware
                                volume header.
**/
STATIC
EFI_STATUS
EFIAPI
SdFvbSetAttributes (
  IN     CONST EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  IN OUT       EFI_FVB_ATTRIBUTES_2                 *Attributes
  )
{
  return EFI_UNSUPPORTED;
}

/**
  The GetPhysicalAddress() function retrieves the base address of
  a memory-mapped firmware volume. This function should be called
  only for memory-mapped firmware volumes.

  @param This     Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Address  Pointer to a caller-allocated
                  EFI_PHYSICAL_ADDRESS that, on successful
                  return from GetPhysicalAddress(), contains the
                  base address of the firmware volume.

  @retval EFI_SUCCESS       The firmware volume base address was returned.

  @retval EFI_UNSUPPORTED   The firmware volume is not memory mapped.
**/
STATIC
EFI_STATUS
EFIAPI
SdFvbGetPhysicalAddress (
  IN  CONST EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  OUT       EFI_PHYSICAL_ADDRESS                 *Address
  )
{
  *Address = (EFI_PHYSICAL_ADDRESS) mNvStorageBase;
  return EFI_SUCCESS;
}

/**
  The GetBlockSize() function retrieves the size of the requested
  block. It also returns the number of additional blocks with
  the identical size. The GetBlockSize() function is used to
  retrieve the block map (see EFI_FIRMWARE_VOLUME_HEADER).

  @param This           Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Lba            Indicates the block for which to return the size.

  @param BlockSize      Pointer to a caller-allocated UINTN in which
                        the size of the block is returned.

  @param NumberOfBlocks Pointer to a caller-allocated UINTN in
                        which the number of consecutive blocks,
                        starting with Lba, is returned. All
                        blocks in this range have a size of
                        BlockSize.

  @retval EFI_SUCCESS             The firmware volume base address was returned.

  @retval EFI_INVALID_PARAMETER   The requested LBA is out of range.
**/
STATIC
EFI_STATUS
EFIAPI
SdFvbGetBlockSize (
  IN  CONST EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  IN        EFI_LBA                              Lba,
  OUT       UINTN                                *BlockSize,
  OUT       UINTN                                *NumberOfBlocks
  )
{
  *BlockSize = SDHCI_BLOCK_SIZE_DEFAULT;
  *NumberOfBlocks = mNvStorageSize / SDHCI_BLOCK_SIZE_DEFAULT - (UINTN) Lba;
  return EFI_SUCCESS;
}

/**
  Reads the specified number of bytes into a buffer from the specified block.

  The Read() function reads the requested number of bytes from the
  requested block and stores them in the provided buffer.
  Implementations should be mindful that the firmware volume
  might be in the ReadDisabled state. If it is in this state,
  the Read() function must return the status code
  EFI_ACCESS_DENIED without modifying the contents of the
  buffer. The Read() function must also prevent spanning block
  boundaries. If a read is requested that would span a block
  boundary, the read must read up to the boundary but not
  beyond. The output parameter NumBytes must be set to correctly
  indicate the number of bytes actually read. The caller must be
  aware that a read may be partially completed.

  @param This     Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Lba      The starting logical block index
                  from which to read.

  @param Offset   Offset into the block at which to begin reading.

  @param NumBytes Pointer to a UINTN. At entry, *NumBytes
                  contains the total size of the buffer. At
                  exit, *NumBytes contains the total number of
                  bytes read.

  @param Buffer   Pointer to a caller-allocated buffer that will
                  be used to hold the data that is read.

  @retval EFI_SUCCESS         The firmware volume was read successfully,
                              and contents are in Buffer.

  @retval EFI_BAD_BUFFER_SIZE Read attempted across an LBA
                              boundary. On output, NumBytes
                              contains the total number of bytes
                              returned in Buffer.

  @retval EFI_ACCESS_DENIED   The firmware volume is in the
                              ReadDisabled state.

  @retval EFI_DEVICE_ERROR    The block device is not
                              functioning correctly and could
                              not be read.
**/
STATIC
EFI_STATUS
EFIAPI
SdFvbRead (
  IN     CONST EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  IN           EFI_LBA                              Lba,
  IN           UINTN                                Offset,
  IN OUT       UINTN                                *NumBytes,
  IN OUT       UINT8                                *Buffer
  )
{
  if (EfiAtRuntime ()) {
    return EFI_UNSUPPORTED;
  }

  UINTN  LocalOffset = Lba * SDHCI_BLOCK_SIZE_DEFAULT + Offset;
  VOID   *Ram = (VOID *) (LocalOffset + (UINTN) mNvStorageBase);
  UINT64 Adr = LocalOffset + SD_VAR_ADR;
  SdRead (Adr, Ram, *NumBytes);
  CopyMem (Buffer, Ram, *NumBytes);
  return EFI_SUCCESS;
}

/**
  Writes the specified number of bytes from the input buffer to the block.

  The Write() function writes the specified number of bytes from
  the provided buffer to the specified block and offset. If the
  firmware volume is sticky write, the caller must ensure that
  all the bits of the specified range to write are in the
  EFI_FVB_ERASE_POLARITY state before calling the Write()
  function, or else the result will be unpredictable. This
  unpredictability arises because, for a sticky-write firmware
  volume, a write may negate a bit in the EFI_FVB_ERASE_POLARITY
  state but cannot flip it back again.  Before calling the
  Write() function,  it is recommended for the caller to first call
  the EraseBlocks() function to erase the specified block to
  write. A block erase cycle will transition bits from the
  (NOT)EFI_FVB_ERASE_POLARITY state back to the
  EFI_FVB_ERASE_POLARITY state. Implementations should be
  mindful that the firmware volume might be in the WriteDisabled
  state. If it is in this state, the Write() function must
  return the status code EFI_ACCESS_DENIED without modifying the
  contents of the firmware volume. The Write() function must
  also prevent spanning block boundaries. If a write is
  requested that spans a block boundary, the write must store up
  to the boundary but not beyond. The output parameter NumBytes
  must be set to correctly indicate the number of bytes actually
  written. The caller must be aware that a write may be
  partially completed. All writes, partial or otherwise, must be
  fully flushed to the hardware before the Write() service
  returns.

  @param This     Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL instance.

  @param Lba      The starting logical block index to write to.

  @param Offset   Offset into the block at which to begin writing.

  @param NumBytes The pointer to a UINTN. At entry, *NumBytes
                  contains the total size of the buffer. At
                  exit, *NumBytes contains the total number of
                  bytes actually written.

  @param Buffer   The pointer to a caller-allocated buffer that
                  contains the source for the write.

  @retval EFI_SUCCESS         The firmware volume was written successfully.

  @retval EFI_BAD_BUFFER_SIZE The write was attempted across an
                              LBA boundary. On output, NumBytes
                              contains the total number of bytes
                              actually written.

  @retval EFI_ACCESS_DENIED   The firmware volume is in the
                              WriteDisabled state.

  @retval EFI_DEVICE_ERROR    The block device is malfunctioning
                              and could not be written.
**/
STATIC
EFI_STATUS
EFIAPI
SdFvbWrite (
  IN     CONST EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  IN           EFI_LBA                              Lba,
  IN           UINTN                                Offset,
  IN OUT       UINTN                                *NumBytes,
  IN           UINT8                                *Buffer
  )
{
  if (EfiAtRuntime ()) {
    return EFI_WRITE_PROTECTED;
  }

  UINTN  LocalOffset = Lba * SDHCI_BLOCK_SIZE_DEFAULT + Offset;
  VOID   *Ram = (VOID *) (LocalOffset + (UINTN) mNvStorageBase);
  UINT64  Adr = LocalOffset + SD_VAR_ADR;
  CopyMem (Ram, Buffer, *NumBytes);
  SdWrite (Adr, Ram, *NumBytes);
  return EFI_SUCCESS;
}

/**
  Erases and initializes a firmware volume block.

  The EraseBlocks() function erases one or more blocks as denoted
  by the variable argument list. The entire parameter list of
  blocks must be verified before erasing any blocks. If a block is
  requested that does not exist within the associated firmware
  volume (it has a larger index than the last block of the
  firmware volume), the EraseBlocks() function must return the
  status code EFI_INVALID_PARAMETER without modifying the contents
  of the firmware volume. Implementations should be mindful that
  the firmware volume might be in the WriteDisabled state. If it
  is in this state, the EraseBlocks() function must return the
  status code EFI_ACCESS_DENIED without modifying the contents of
  the firmware volume. All calls to EraseBlocks() must be fully
  flushed to the hardware before the EraseBlocks() service
  returns.

  @param This   Indicates the EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL
                instance.

  @param ...    The variable argument list is a list of tuples.
                Each tuple describes a range of LBAs to erase
                and consists of the following:
                - An EFI_LBA that indicates the starting LBA
                - A UINTN that indicates the number of blocks to
                  erase.

                The list is terminated with an
                EFI_LBA_LIST_TERMINATOR. For example, the
                following indicates that two ranges of blocks
                (5-7 and 10-11) are to be erased: EraseBlocks
                (This, 5, 3, 10, 2, EFI_LBA_LIST_TERMINATOR);

  @retval EFI_SUCCESS The erase request successfully
                      completed.

  @retval EFI_ACCESS_DENIED   The firmware volume is in the
                              WriteDisabled state.

  @retval EFI_DEVICE_ERROR  The block device is not functioning
                            correctly and could not be written.
                            The firmware device may have been
                            partially erased.

  @retval EFI_INVALID_PARAMETER One or more of the LBAs listed
                                in the variable argument list do
                                not exist in the firmware volume.
**/
STATIC
EFI_STATUS
EFIAPI
SdFvbErase (
  IN  CONST EFI_FIRMWARE_VOLUME_BLOCK2_PROTOCOL  *This,
  ...
  )
{
  if (EfiAtRuntime ()) {
    return EFI_UNSUPPORTED;
  }

  VA_LIST  Args;
  EFI_LBA  Lba;

  VA_START (Args, This);

  for (Lba = VA_ARG (Args, EFI_LBA);
       Lba != EFI_LBA_LIST_TERMINATOR;
       Lba = VA_ARG (Args, EFI_LBA)) {
    UINTN   Cnt = VA_ARG (Args, UINTN);
    UINTN   Size = Cnt * SDHCI_BLOCK_SIZE_DEFAULT;
    UINTN   Offset = Lba * SDHCI_BLOCK_SIZE_DEFAULT;
    VOID   *Ram = (VOID *) (Offset + (UINTN) mNvStorageBase);
    UINT64 Adr = Offset + SD_VAR_ADR;
    SetMem64 (Ram, Size, ~0UL);
    SdWrite (Adr, Ram, Size);
  }

  VA_END (Args);
  return EFI_SUCCESS;
}

STATIC EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  mSdFvbProtocol = {
  SdFvbGetAttributes,
  SdFvbSetAttributes,
  SdFvbGetPhysicalAddress,
  SdFvbGetBlockSize,
  SdFvbRead,
  SdFvbWrite,
  SdFvbErase
};

/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It convers pointer to new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.
**/
STATIC
VOID
EFIAPI
SdFvbConvertPointerEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  // NonVolatile
  EfiConvertPointer (0x0, (VOID **) &mNvStorageBase);

  // Fvb
  EfiConvertPointer (0x0, (VOID **) &mSdFvbProtocol.GetAttributes);
  EfiConvertPointer (0x0, (VOID **) &mSdFvbProtocol.SetAttributes);
  EfiConvertPointer (0x0, (VOID **) &mSdFvbProtocol.GetPhysicalAddress);
  EfiConvertPointer (0x0, (VOID **) &mSdFvbProtocol.GetBlockSize);
  EfiConvertPointer (0x0, (VOID **) &mSdFvbProtocol.Read);
  EfiConvertPointer (0x0, (VOID **) &mSdFvbProtocol.Write);
  EfiConvertPointer (0x0, (VOID **) &mSdFvbProtocol.EraseBlocks);
}

STATIC
EFI_STATUS
SdFvbInitializeFvAndVariableStoreHeaders (
  IN VOID  *FvMem
  )
{
  EFI_STATUS                   Status = EFI_SUCCESS;
  VOID                        *Headers;
  UINTN                        HeadersLength;
  EFI_FIRMWARE_VOLUME_HEADER  *FirmwareVolumeHeader;
  VARIABLE_STORE_HEADER       *VariableStoreHeader;

  HeadersLength = sizeof (EFI_FIRMWARE_VOLUME_HEADER) +
                  sizeof (EFI_FV_BLOCK_MAP_ENTRY) +
                  sizeof (VARIABLE_STORE_HEADER);

  Headers = (VOID *) AllocateZeroPool (HeadersLength);

  //
  // FirmwareVolumeHeader->FvLength is declared to have the Variable area AND
  // the FTW working area AND the FTW Spare contiguous.
  //
  ASSERT (PcdGet32 (PcdFlashNvStorageVariableBase) +
          PcdGet32 (PcdFlashNvStorageVariableSize) ==
          PcdGet32 (PcdFlashNvStorageFtwWorkingBase));

  ASSERT (PcdGet32 (PcdFlashNvStorageFtwWorkingBase) +
          PcdGet32 (PcdFlashNvStorageFtwWorkingSize) ==
          PcdGet32 (PcdFlashNvStorageFtwSpareBase));

  FirmwareVolumeHeader = (EFI_FIRMWARE_VOLUME_HEADER *) Headers;
  CopyGuid (&FirmwareVolumeHeader->FileSystemGuid, &gEfiSystemNvDataFvGuid);
  FirmwareVolumeHeader->FvLength = PcdGet32 (PcdFlashNvStorageVariableSize) +
                                   PcdGet32 (PcdFlashNvStorageFtwWorkingSize) +
                                   PcdGet32 (PcdFlashNvStorageFtwSpareSize);

  FirmwareVolumeHeader->Signature  = EFI_FVH_SIGNATURE;
  FirmwareVolumeHeader->Attributes = (EFI_FVB_ATTRIBUTES_2) (
    EFI_FVB2_READ_ENABLED_CAP | // Reads may be enabled
    EFI_FVB2_READ_STATUS      | // Reads are currently enabled
    EFI_FVB2_STICKY_WRITE     | // A block erase is required to flip bits into EFI_FVB2_ERASE_POLARITY
    EFI_FVB2_MEMORY_MAPPED    | // It is memory mapped
    EFI_FVB2_ERASE_POLARITY   | // After erasure all bits take this value (i.e. '1')
    EFI_FVB2_WRITE_STATUS     | // Writes are currently enabled
    EFI_FVB2_WRITE_ENABLED_CAP  // Writes may be enabled
    );

  FirmwareVolumeHeader->HeaderLength = sizeof (EFI_FIRMWARE_VOLUME_HEADER) +
                                       sizeof (EFI_FV_BLOCK_MAP_ENTRY);

  FirmwareVolumeHeader->Revision = EFI_FVH_REVISION;
  FirmwareVolumeHeader->BlockMap[0].NumBlocks = 255 + 1;
  FirmwareVolumeHeader->BlockMap[0].Length    = PcdGet32 (PcdFlashNvStorageVariableSize);
  FirmwareVolumeHeader->BlockMap[1].NumBlocks = 0;
  FirmwareVolumeHeader->BlockMap[1].Length    = 0;
  FirmwareVolumeHeader->Checksum = CalculateCheckSum16 (
                                     (UINT16 *) FirmwareVolumeHeader,
                                     FirmwareVolumeHeader->HeaderLength
                                     );

  VariableStoreHeader = (VARIABLE_STORE_HEADER *) ((UINTN) Headers + FirmwareVolumeHeader->HeaderLength);
  CopyGuid (&VariableStoreHeader->Signature, &gEfiAuthenticatedVariableGuid);
  VariableStoreHeader->Size = PcdGet32 (PcdFlashNvStorageVariableSize) - FirmwareVolumeHeader->HeaderLength;
  VariableStoreHeader->Format = VARIABLE_STORE_FORMATTED;
  VariableStoreHeader->State  = VARIABLE_STORE_HEALTHY;

  CopyMem (FvMem, Headers, HeadersLength);
  FreePool (Headers);

  return Status;
}

STATIC
EFI_STATUS
SdFvbValidateFvHeader (
  IN VOID  *FvMem
  )
{
  UINT16                       Checksum;
  EFI_FIRMWARE_VOLUME_HEADER  *Header = FvMem;
  VARIABLE_STORE_HEADER       *Variable;
  UINTN                        VariableStoreLength;

  if (Header->Revision  != EFI_FVH_REVISION  ||
      Header->Signature != EFI_FVH_SIGNATURE ||
      Header->FvLength  != mNvStorageSize) {
    return EFI_NOT_FOUND;
  }

  if (!CompareGuid (&Header->FileSystemGuid, &gEfiSystemNvDataFvGuid)) {
    return EFI_NOT_FOUND;
  }

  Checksum = CalculateCheckSum16 ((UINT16 *) Header, Header->HeaderLength);
  if (Checksum != 0) {
    return EFI_NOT_FOUND;
  }

  Variable = (VOID *) ((UINTN) FvMem + Header->HeaderLength);
  if (!CompareGuid (&Variable->Signature, &gEfiVariableGuid) &&
      !CompareGuid (&Variable->Signature, &gEfiAuthenticatedVariableGuid)) {
    return EFI_NOT_FOUND;
  }

  VariableStoreLength = PcdGet32 (PcdFlashNvStorageVariableSize) - Header->HeaderLength;
  if (Variable->Size != VariableStoreLength) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
SdFvbBaikalFvHeader (
  VOID
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  DEBUG ((EFI_D_INFO, "Reading headers from sd\n"));
  SdRead (SD_VAR_ADR, mNvStorageBase, mNvStorageSize);
  if (SdFvbValidateFvHeader (mNvStorageBase) != EFI_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "Writing default headers to sd\n"));
    SetMem64 ((VOID *) mNvStorageBase, mNvStorageSize, ~0UL);
    SdFvbInitializeFvAndVariableStoreHeaders (mNvStorageBase);
    SdWrite (SD_VAR_ADR, mNvStorageBase, mNvStorageSize);
  }
  return Status;
}

EFI_STATUS
EFIAPI
SdFvbInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  /* init */
  Status = SdIdentification();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  /* base */
  mNvStorageBase = (VOID *) (PcdGet64 (PcdFlashNvStorageVariableBase64) != 0 ?
                             PcdGet64 (PcdFlashNvStorageVariableBase64) :
                             PcdGet32 (PcdFlashNvStorageVariableBase));
  Status = gBS->AllocatePages (
                  AllocateAddress,
                  EfiRuntimeServicesData,
                  EFI_SIZE_TO_PAGES (mNvStorageSize),
                  (EFI_PHYSICAL_ADDRESS *) &mNvStorageBase
                  );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;  // EFI_ALREADY_STARTED
  }

  /* header */
  Status = SdFvbBaikalFvHeader ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  /* event */
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  SdFvbConvertPointerEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &mVirtualAddressChangeEvent
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  /* install */
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mSdFvbHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mSdFvbDevicePath,
                  &gEfiFirmwareVolumeBlockProtocolGuid,
                  &mSdFvbProtocol,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}
