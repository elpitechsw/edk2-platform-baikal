/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_SMBUS_LIB_H_
#define BAIKAL_SMBUS_LIB_H_

INTN
SmbusTxRx (
  IN   CONST UINTN        Bus,
  IN   CONST UINTN        TargetAddr,
  IN   CONST VOID *CONST  TxBuf,
  IN   CONST UINTN        TxBufSize,
  OUT  VOID *CONST        RxBuf,
  IN   CONST UINTN        RxBufSize
  );

#endif // BAIKAL_SMBUS_LIB_H_
