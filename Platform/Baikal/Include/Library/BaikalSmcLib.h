/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_SMC_LIB_H_
#define BAIKAL_SMC_LIB_H_

INTN
BaikalSmcFlashErase (
  IN  CONST UINT32  Adr,
  IN  CONST UINT32  Size
  );

INTN
BaikalSmcFlashWrite (
  IN  UINT32       Adr,
  IN  CONST VOID  *Data,
  IN  UINT32       Size
  );

INTN
BaikalSmcFlashRead (
  IN  UINT32   Adr,
  IN  VOID    *Data,
  IN  UINT32   Size
  );

INTN
BaikalSmcFlashInfo (
  IN  UINT32 * CONST  SectorSize,
  IN  UINT32 * CONST  SectorCount
  );

VOID
BaikalSmcFlashConvertPointers (
  VOID
  );

#endif // BAIKAL_SMC_LIB_H_
