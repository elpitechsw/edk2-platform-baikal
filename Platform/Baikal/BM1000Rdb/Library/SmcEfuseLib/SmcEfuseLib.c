/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/ArmSmcLib.h>

#define BAIKAL_SMC_EFUSE_GET_LOT     0xC2000202
#define BAIKAL_SMC_EFUSE_GET_SERIAL  0xC2000203
#define BAIKAL_SMC_EFUSE_GET_MAC     0xC2000204

INT64
SmcEfuseGetLot (
  VOID
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_EFUSE_GET_LOT;
  ArmCallSmc (&ArmSmcArgs);
  return ArmSmcArgs.Arg0;
}

INTN
SmcEfuseGetMac (
  VOID
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_EFUSE_GET_MAC;
  ArmCallSmc (&ArmSmcArgs);
  return ArmSmcArgs.Arg0;
}

INTN
SmcEfuseGetSerial (
  VOID
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_EFUSE_GET_SERIAL;
  ArmCallSmc (&ArmSmcArgs);
  return ArmSmcArgs.Arg0;
}
