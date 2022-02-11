/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef GPIO_LIB_H_
#define GPIO_LIB_H_

VOID
GpioDirClr (
  IN  CONST EFI_PHYSICAL_ADDRESS  Base,
  IN  CONST UINTN                 Pin
  );

VOID
GpioDirSet (
  IN  CONST EFI_PHYSICAL_ADDRESS  Base,
  IN  CONST UINTN                 Pin
  );

VOID
GpioOutRst (
  IN  CONST EFI_PHYSICAL_ADDRESS  Base,
  IN  CONST UINTN                 Pin
  );

VOID
GpioOutSet (
  IN  CONST EFI_PHYSICAL_ADDRESS  Base,
  IN  CONST UINTN                 Pin
  );

#endif // GPIO_LIB_H_
