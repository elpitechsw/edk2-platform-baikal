/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef SMC_FLASH_LIB_H_
#define SMC_FLASH_LIB_H_

VOID
SmcFlashConvertPointers (
  VOID
  );

INTN
SmcFlashErase (
  IN UINTN  Addr,
  IN UINTN  Size
  );

INTN
SmcFlashInfo (
  IN UINT32  *SectorSize,
  IN UINT32  *SectorCount
  );

INTN
SmcFlashLock (
  IN UINTN  Lock
  );

INTN
SmcFlashRead (
  IN UINTN       Addr,
  IN VOID        *Data,
  IN UINTN       Size
  );

INTN
SmcFlashWrite (
  IN UINTN       Addr,
  IN CONST VOID  *Data,
  IN UINTN       Size
  );

#endif // SMC_FLASH_LIB_H_
