/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/CrcLib.h>

UINT16
EFIAPI
Crc16 (
  IN  CONST VOID  *Data,
  IN  UINTN        Size,
  IN  UINT16       Crc
  )
{
  UINTN         Idx;
  CONST UINT8  *Ptr = Data;

  while (Size--) {
    Crc ^= *Ptr++ << 8;

    for (Idx = 0; Idx < 8; ++Idx) {
      if (Crc & 0x8000) {
        Crc = Crc << 1 ^ 0x1021;
      } else {
        Crc <<= 1;
      }
    }
  }

  return Crc;
}
