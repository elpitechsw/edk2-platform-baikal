/** @file
  Copyright (c) 2020 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifdef BAIKAL_DBM10
  #include "DsdtDbm10.asl"
#elif defined(BAIKAL_DBM20)
  #include "DsdtDbm20.asl"
#elif defined(BAIKAL_MBM10)
  #include "DsdtMbm10.asl"
#elif defined(BAIKAL_MBM20)
  #include "DsdtMbm20.asl"
#elif defined(ELPITECH)
  #include "DsdtElp.asl"
#elif defined(BAIKAL_QEMU_M)
DefinitionBlock ("Dsdt.aml", "DSDT", 2, "BAIKAL", "BKLEDSDT", 1)
{
}
#endif
