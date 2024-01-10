/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_MEMORY_RANGE_LIB_H_
#define BAIKAL_MEMORY_RANGE_LIB_H_

#include <Uefi/UefiBaseType.h>

EFI_STATUS
GetMemoryRanges (
  IN OUT  INT32 * CONST       Node,
  OUT  CONST UINT32 ** CONST  Reg,
  OUT  UINTN * CONST          Amount,
  OUT  UINTN * CONST          AddressCells,
  OUT  UINTN * CONST          SizeCells
  );

#endif // BAIKAL_MEMORY_RANGE_LIB_H_
