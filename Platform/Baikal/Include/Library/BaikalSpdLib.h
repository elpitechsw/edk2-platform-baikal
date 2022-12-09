/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_SPD_LIB_H_
#define BAIKAL_SPD_LIB_H_

#define BAIKAL_SPD_DATA_BASE   0x8003FA00
#define BAIKAL_SPD_MAX_SIZE    512
#define BAIKAL_SPD_PORT_COUNT  2

#pragma pack (push, 1)
typedef struct {
  CONST UINT8  Buf[BAIKAL_SPD_MAX_SIZE * BAIKAL_SPD_PORT_COUNT];
  struct {
    CONST UINT32  Serial;
    CONST UINT8   Part[20];
  }  Extra[BAIKAL_SPD_PORT_COUNT];
  CONST UINT8  IsDualChannel[BAIKAL_SPD_PORT_COUNT];
  CONST UINT32 ConfiguredSpeed[BAIKAL_SPD_PORT_COUNT];
} BAIKAL_SPD_INFO;
#pragma pack (pop)

UINTN
SpdGetConfiguredSpeed (
  IN  CONST UINTN  Port
  );

INTN
SpdIsDualChannel (
  IN  CONST UINTN  Port
  );

INTN
SpdGetSize (
  IN  CONST UINTN  Port
  );

CONST UINT8 *
SpdGetBuf (
  IN  CONST UINTN  Port
  );
#endif // BAIKAL_SPD_LIB_H_
