/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef CRC_LIB_H_
#define CRC_LIB_H_

UINT16
EFIAPI
Crc16 (
  IN  CONST VOID  *Data,
  IN  UINTN        Size,
  IN  UINT16       Crc
  );

#endif // CRC_LIB_H_
