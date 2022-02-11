/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>

#include <Library/GpioLib.h>

#define SWPORTA_DR   0x00
#define SWPORTA_DDR  0x04

VOID
GpioDirClr (
  IN  CONST EFI_PHYSICAL_ADDRESS  Base,
  IN  CONST UINTN                 Pin
  )
{
  ASSERT (Pin < 32);

  MmioAnd32 (Base + SWPORTA_DDR, ~(1 << Pin));
}

VOID
GpioDirSet (
  IN  CONST EFI_PHYSICAL_ADDRESS  Base,
  IN  CONST UINTN                 Pin
  )
{
  ASSERT (Pin < 32);

  MmioOr32 (Base + SWPORTA_DDR, 1 << Pin);
}

VOID
GpioOutRst (
  IN  CONST EFI_PHYSICAL_ADDRESS  Base,
  IN  CONST UINTN                 Pin
  )
{
  ASSERT (Pin < 32);

  MmioAnd32 (Base + SWPORTA_DR, ~(1 << Pin));
}

VOID
GpioOutSet (
  IN  CONST EFI_PHYSICAL_ADDRESS  Base,
  IN  CONST UINTN                 Pin
  )
{
  ASSERT (Pin < 32);

  MmioOr32 (Base + SWPORTA_DR, 1 << Pin);
}
