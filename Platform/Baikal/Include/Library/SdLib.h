/** @file
  Copyright (c) 2020 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef SD_LIB_H_
#define SD_LIB_H_

#define SDHCI_BLOCK_SIZE_DEFAULT  512

EFI_STATUS
SdIdentification (VOID);

EFI_STATUS
SdWriteBlocks (
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  IN  VOID                   *Buffer
  );

EFI_STATUS
SdReadBlocks (
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  OUT VOID                   *Buffer
  );

EFI_STATUS
SdWriteBlocksIo (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  IN  VOID                   *Buffer
  );

EFI_STATUS
SdReadBlocksIo (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  OUT VOID                   *Buffer
  );

EFI_STATUS
SdFlushBlocksIo (
  IN  EFI_BLOCK_IO_PROTOCOL  *This
  );

EFI_STATUS
SdResetIo (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  BOOLEAN                 ExtendedVerification
  );

UINT64
SdTotalSize (
  VOID
  );

VOID
SdPowerOff (
  VOID
  );

VOID
SdConvertPointers (
  VOID
  );

EFI_STATUS
SdWrite (
  IN UINT64  adr,
  IN VOID   *src_,
  IN UINT64  size
  );

EFI_STATUS
SdRead (
  IN UINT64  adr,
  IN VOID   *dst_,
  IN UINT64  size
  );

#endif // SD_LIB_H_
