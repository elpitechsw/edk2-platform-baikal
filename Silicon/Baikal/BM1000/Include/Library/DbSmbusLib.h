/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef DB_SMBUS_LIB_H_
#define DB_SMBUS_LIB_H_

enum SmbusSht {
  SmbusSht400kHz = 0,
  SmbusSht100kHz = 1,
  SmbusShtMax
};

UINTN
SmbusTxRx (
  IN   CONST EFI_PHYSICAL_ADDRESS  Base,
  IN   CONST UINTN                 Iclk,
  IN   CONST enum SmbusSht         Sht,
  IN   CONST UINTN                 SclClk,
  IN   CONST UINTN                 TargetAddr,
  IN   CONST VOID * CONST          TxBuf,
  IN   CONST UINTN                 TxBufSize,
  OUT  VOID * CONST                RxBuf,
  IN   CONST UINTN                 RxBufSize
  );

#endif // DB_SMBUS_LIB_H_
