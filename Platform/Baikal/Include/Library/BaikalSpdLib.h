/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_SPD_LIB_H_
#define BAIKAL_SPD_LIB_H_

#define BAIKAL_SPD_MAX_SIZE    512
#define BAIKAL_SPD_PORT_COUNT  2

CONST UINT8 *
SpdGetBuf (
  IN  CONST UINTN  Port
  );

UINTN
SpdGetConfiguredSpeed (
  IN  CONST UINTN  Port
  );

CONST UINT8 *
SpdGetPart (
  IN  CONST UINTN  Port
  );

UINT32
SpdGetSerial (
  IN  CONST UINTN  Port
  );

INTN
SpdGetSize (
  IN  CONST UINTN  Port
  );

INTN
SpdIsDualChannel (
  IN  CONST UINTN  Port
  );

#endif // BAIKAL_SPD_LIB_H_
