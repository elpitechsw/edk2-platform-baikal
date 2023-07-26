/** @file
  Copyright (c) 2021 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ShellLib.h>
#include <Library/SmcFlashLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/ShellParameters.h>

EFI_STATUS
EFIAPI
SpiFlashMain (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  INTN                            Err;
  VOID                           *FileBuf;
  SHELL_FILE_HANDLE               FileHandle;
  CHAR16                         *FileName;
  UINTN                           FileSize;
  UINT32                          FlashAddr;
  VOID                           *FlashBuf;
  EFI_STATUS                      Status;
  EFI_SHELL_PARAMETERS_PROTOCOL  *ShellParameters;
  EFI_SHELL_PROTOCOL             *ShellProtocol;

  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiShellParametersProtocolGuid,
                  (VOID **) &ShellParameters
                  );
  if (EFI_ERROR (Status)) {
    Print (L"Please use UEFI Shell to run this application.\n");
    return Status;
  }

  if (ShellParameters->Argc != 3 ||
      !StrCmp (ShellParameters->Argv[1], L"-?") ||
      !StrCmp (ShellParameters->Argv[1], L"-h") ||
      !StrCmp (ShellParameters->Argv[1], L"-H")) {
    Print (L"Flash a file to SPI Flash.\n");
    Print (L"\n");
    Print (L"SPIFLASH offset filepath\n");
    Print (L"\n");
    Print (L"  offset      - Relative offset in SPI Flash (depends on driver).\n");
    Print (L"  filepath    - File path.\n");
    Print (L"\n");
    Print (L"NOTES:\n");
    Print (L"  1. The application has been successfully tested on FW v4.4 and higher.\n");
    Print (L"  2. DO NOT USE the application on FW v4.3 and lower!\n");
    Print (L"\n");
    Print (L"EXAMPLES:\n");
    Print (L"  * To write the 'board.flash.img' file to SPI Flash:\n");
    Print (L"    fs0:\\> spiflash 0 board.flash.img\n");
    return EFI_SUCCESS;
  }

  Status = gBS->LocateProtocol (
                  &gEfiShellProtocolGuid,
                  NULL,
                  (VOID **) &ShellProtocol
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  if (ShellParameters->Argv[1][0] == '-') {
    FlashAddr = 0 - ShellStrToUintn (&ShellParameters->Argv[1][1]);
  } else {
    FlashAddr = ShellStrToUintn (ShellParameters->Argv[1]);
    if (FlashAddr % 512) {
      Print (L"SpiFlash: incorrect start address.\n");
      return EFI_UNSUPPORTED;
    }
  }

  FileName = ShellParameters->Argv[2];
  Status = ShellProtocol->OpenFileByName (
                            FileName,
                            &FileHandle,
                            EFI_FILE_MODE_READ
                            );
  if (EFI_ERROR (Status)) {
    Print (L"SpiFlash: file %s is not found.\n", FileName);
    return Status;
  }

  Status = ShellProtocol->GetFileSize (FileHandle, &FileSize);
  if (EFI_ERROR (Status)) {
    Print (L"SpiFlash: failed to get file size.\n");
    ShellProtocol->CloseFile (FileHandle);
    return Status;
  }

  FileBuf = AllocateZeroPool (FileSize);
  if (FileBuf == NULL) {
    ShellProtocol->CloseFile (FileHandle);
    Print (L"SpiFlash: failed allocate %u bytes.\n", FileSize);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = ShellProtocol->ReadFile (
                            FileHandle,
                            &FileSize,
                            FileBuf
                            );
  if (EFI_ERROR (Status)) {
    Print (L"SpiFlash: failed to read %s.\n", FileName);
    ShellProtocol->CloseFile (FileHandle);
    goto Exit;
  }

  ShellProtocol->CloseFile (FileHandle);

  Print (L"SpiFlash: unlocking...\n");
  SmcFlashLock (0);

  Print (L"SpiFlash: erasing...\n");
  Err = SmcFlashErase (FlashAddr, (FileSize + 0xffff) & ~0xffffUL);
  if (Err) {
    Print (L"SpiFlash: error %d\n", Err);
    goto Exit;
  }

  Print (L"SpiFlash: writing...\n");
  Err = SmcFlashWrite (FlashAddr, FileBuf, FileSize);
  if (Err) {
    Print (L"SpiFlash: error %d\n", Err);
    goto Exit;
  }

  Print (L"SpiFlash: verifying...\n");
  FlashBuf = AllocateZeroPool (FileSize);
  if (FlashBuf == NULL) {
    Print (L"SpiFlash: failed allocate %u bytes.\n", FileSize);
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Err = SmcFlashRead (FlashAddr, FlashBuf, FileSize);
  if (Err) {
    Print (L"SpiFlash: error %d\n", Err);
    goto Exit;
  }

  Print (L"SpiFlash: locking...\n");
  SmcFlashLock (1);

  if (CompareMem (FileBuf, FlashBuf, FileSize) == 0) {
    Print (L"SpiFlash: success.\n");
  } else {
    Print (L"SpiFlash: error.\n");
  }

  FreePool (FlashBuf);

Exit:
  if (FileBuf != NULL) {
    FreePool (FileBuf);
  }

  return Status;
}
