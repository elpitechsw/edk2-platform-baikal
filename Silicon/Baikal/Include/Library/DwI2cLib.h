/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef DW_I2C_LIB_H_
#define DW_I2C_LIB_H_

INTN
I2cTxRx (
  IN   CONST EFI_PHYSICAL_ADDRESS  Base,
  IN   CONST UINTN                 TargetAddr,
  IN   CONST VOID * CONST          TxBuf,
  IN   CONST UINTN                 TxBufSize,
  OUT  VOID * CONST                RxBuf,
  IN   CONST UINTN                 RxBufSize
  );

#endif // DW_I2C_LIB_H_
