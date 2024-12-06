/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifdef BAIKAL_DBS
  #include "DsdtDbs.asl"
#elif defined(BAIKAL_DBS_OV)
  #include "DsdtDbsOv.asl"
#elif defined(BAIKAL_MBS_1S)
  #include "DsdtMbs1s.asl"
#elif defined(ELPITECH)
#if defined(ELP_12)
  #include "DsdtMbs2s.asl"
#else
  #include "DsdtElp.asl"
#endif
#elif defined(BAIKAL_MBS_2S)
  #include "DsdtMbs2s.asl"
#elif defined(BAIKAL_QEMU_S)
DefinitionBlock ("Dsdt.aml", "DSDT", 2, "BAIKAL", "BKLEDSDT", 1)
{
}
#endif
