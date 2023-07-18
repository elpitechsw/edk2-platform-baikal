/** @file
  Copyright (c) 2022 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/ArmSmcLib.h>
#include <Library/CmuLib.h>

#define BAIKAL_SMC_CMU_CMD             0xC2000000
#define BAIKAL_SMC_CMU_CLKCH_SET_RATE  6
#define BAIKAL_SMC_CMU_CLKCH_GET_RATE  7

UINTN
EFIAPI
CmuClkChGetRate (
  IN CONST EFI_PHYSICAL_ADDRESS  ClkChCtlAddr
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_CMU_CMD;
  ArmSmcArgs.Arg1 = ((ClkChCtlAddr & 0xFFFF) - 0x20) / 0x10; // Clock channel num
  ArmSmcArgs.Arg2 = BAIKAL_SMC_CMU_CLKCH_GET_RATE;
  ArmSmcArgs.Arg4 = ClkChCtlAddr & 0xFFFF0000; // CMU base
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

  ArmSmcArgs.Arg0 = BAIKAL_SMC_CMU_CMD;
  ArmSmcArgs.Arg1 = ((ClkChCtlAddr & 0xFFFF) - 0x20) / 0x10; // Clock channel num
  ArmSmcArgs.Arg2 = BAIKAL_SMC_CMU_CLKCH_SET_RATE;
  ArmSmcArgs.Arg3 = ClkChRate;
  ArmSmcArgs.Arg4 = ClkChCtlAddr & 0xFFFF0000; // CMU base
  ArmCallSmc (&ArmSmcArgs);
}
