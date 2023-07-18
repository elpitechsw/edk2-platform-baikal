/** @file
  Copyright (c) 2022 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/ArmSmcLib.h>
#include <Library/CmuLib.h>

#define BAIKAL_SMC_CLK_SET  0xC2000401
#define BAIKAL_SMC_CLK_GET  0xC2000402

UINTN
EFIAPI
CmuClkChGetRate (
  IN CONST EFI_PHYSICAL_ADDRESS  ClkChCtlAddr
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_CLK_GET;
  ArmSmcArgs.Arg1 = ClkChCtlAddr;
  ArmCallSmc (&ArmSmcArgs);
  return ArmSmcArgs.Arg0;
}

VOID
EFIAPI
CmuClkChSetRate (
  IN CONST EFI_PHYSICAL_ADDRESS  ClkChCtlAddr,
  IN CONST UINTN                 ClkChRate
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_CLK_SET;
  ArmSmcArgs.Arg1 = ClkChCtlAddr;
  ArmSmcArgs.Arg2 = ClkChRate;
  ArmCallSmc (&ArmSmcArgs);
}
