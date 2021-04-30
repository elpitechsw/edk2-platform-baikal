/** @file
  Copyright (c) 2020, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_I2C_LIB_H_
#define BAIKAL_I2C_LIB_H_

INTN
I2cTxRx (
  IN   CONST UINTN        Bus,
  IN   CONST UINTN        TargetAddr,
  IN   CONST VOID *CONST  TxBuf,
  IN   CONST UINTN        TxBufSize,
  OUT  VOID *CONST        RxBuf,
  IN   CONST UINTN        RxBufSize
  );

#endif // BAIKAL_I2C_LIB_H_
