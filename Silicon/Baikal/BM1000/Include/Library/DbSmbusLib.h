/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef DB_SMBUS_LIB_H_
#define DB_SMBUS_LIB_H_

INTN
SmbusTxRx (
  IN   CONST EFI_PHYSICAL_ADDRESS  Base,
  IN   CONST UINTN                 TargetAddr,
  IN   CONST VOID * CONST          TxBuf,
  IN   CONST UINTN                 TxBufSize,
  OUT  VOID * CONST                RxBuf,
  IN   CONST UINTN                 RxBufSize
  );

#endif // DB_SMBUS_LIB_H_
