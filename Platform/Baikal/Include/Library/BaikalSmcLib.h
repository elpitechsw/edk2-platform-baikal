/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_SMC_LIB_H_
#define BAIKAL_SMC_LIB_H_

VOID
BaikalSmcFlashConvertPointers (
  VOID
  );

INTN
BaikalSmcFlashErase (
  IN  CONST UINTN  Addr,
  IN  CONST UINTN  Size
  );

INTN
BaikalSmcFlashInfo (
  IN  UINT32 * CONST  SectorSize,
  IN  UINT32 * CONST  SectorCount
  );

INTN
BaikalSmcFlashLock (
  IN  CONST UINTN  Lock
  );

INTN
BaikalSmcFlashRead (
  IN  UINTN   Addr,
  IN  VOID   *Data,
  IN  UINTN   Size
  );

INTN
BaikalSmcFlashWrite (
  IN  UINTN        Addr,
  IN  CONST VOID  *Data,
  IN  UINTN        Size
  );

#endif // BAIKAL_SMC_LIB_H_
