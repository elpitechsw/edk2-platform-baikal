/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef CMU_LIB_H_
#define CMU_LIB_H_

UINTN
EFIAPI
CmuClkChGetRate (
  IN EFI_PHYSICAL_ADDRESS  ClkChCtlAddr
  );

VOID
EFIAPI
CmuClkChSetRate (
  IN EFI_PHYSICAL_ADDRESS  ClkChCtlAddr,
  IN UINTN                 ClkChRate
  );

#endif // CMU_LIB_H_
