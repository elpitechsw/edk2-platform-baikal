/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef SMC_EFUSE_LIB_H_
#define SMC_EFUSE_LIB_H_

INT64
SmcEfuseGetLot (
  VOID
  );

INTN
SmcEfuseGetMac (
  VOID
  );

INTN
SmcEfuseGetSerial (
  VOID
  );

#endif // SMC_EFUSE_LIB_H_
