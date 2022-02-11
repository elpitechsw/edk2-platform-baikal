/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaikalSmcLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/ShellParameters.h>
#include "SpiFlashImageRaw.h"

STATIC CONST UINT8  BoardFlashImage[] = BOARD_FLASH_IMAGE;

EFI_STATUS
EFIAPI
SpiFlashImageMain (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  INTN                            Err;
  VOID                           *FlashBuf;
  EFI_STATUS                      Status;
  EFI_SHELL_PARAMETERS_PROTOCOL  *ShellParameters;

  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiShellParametersProtocolGuid,
                  (VOID **) &ShellParameters
                  );
  if (EFI_ERROR (Status)) {
    Print (L"Please use UEFI Shell to run this application.\n");
    return Status;
  }

  if (ShellParameters->Argc > 1) {
    Print (L"Flash image to SPI Flash. The image is built-in in the application.\n");
    Print (L"\n");
    Print (L"SPIFLASHIMAGE\n");
    Print (L"\n");
    Print (L"NOTES:\n");
    Print (L"  1. The application has been successfully tested on FW v4.4 and higher.\n");
    Print (L"  2. DO NOT USE the application on FW v4.3 and lower!\n");
    Print (L"\n");
    Print (L"EXAMPLES:\n");
    Print (L"  * To write the built-in image to SPI Flash:\n");
    Print (L"    fs0:\\> spiflashimage\n");
    return EFI_SUCCESS;
  }

  Print (L"SpiFlashImage: erasing...\n");
  Err = BaikalSmcFlashErase (0, sizeof (BoardFlashImage));
  if (Err) {
    Print (L"SpiFlashImage: error %d\n", Err);
    return EFI_DEVICE_ERROR;
  }

  Print (L"SpiFlashImage: writing...\n");
  Err = BaikalSmcFlashWrite (0, BoardFlashImage, sizeof (BoardFlashImage));
  if (Err) {
    Print (L"SpiFlashImage: error %d\n", Err);
    return EFI_DEVICE_ERROR;
  }

  Print (L"SpiFlashImage: verifying...\n");
  FlashBuf = AllocateZeroPool (sizeof (BoardFlashImage));
  if (FlashBuf == NULL) {
    Print (L"SpiFlashImage: failed allocate %u bytes.\n", sizeof (BoardFlashImage));
    return EFI_OUT_OF_RESOURCES;
  }

  Err = BaikalSmcFlashRead (0, FlashBuf, sizeof (BoardFlashImage));
  if (Err) {
    Print (L"SpiFlashImage: error %d\n", Err);
    return EFI_DEVICE_ERROR;
  }

  if (CompareMem (BoardFlashImage, FlashBuf, sizeof (BoardFlashImage)) == 0) {
    UINT32  Crc32 = 0;
    gBS->CalculateCrc32 (FlashBuf, sizeof (BoardFlashImage), &Crc32);
    Print (L"SpiFlashImage: success, CRC32 0x%x\n", Crc32);
  } else {
    Print (L"SpiFlashImage: error.\n");
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
