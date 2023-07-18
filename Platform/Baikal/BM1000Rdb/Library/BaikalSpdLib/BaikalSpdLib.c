/** @file
  Copyright (c) 2021 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaikalSpdLib.h>

UINTN
SpdGetConfiguredSpeed (
  IN  CONST UINTN  Port
  )
{
  BAIKAL_SPD_INFO  *Spd = (BAIKAL_SPD_INFO *) BAIKAL_SPD_DATA_BASE;

  if (Port >= BAIKAL_SPD_PORT_COUNT) {
    return 0;
  }

  return Spd->ConfiguredSpeed[Port];
}

INTN
SpdIsDualChannel (
  IN  CONST UINTN  Port
  )
{
  BAIKAL_SPD_INFO  *Spd = (BAIKAL_SPD_INFO *) BAIKAL_SPD_DATA_BASE;

  if (Port >= BAIKAL_SPD_PORT_COUNT) {
    return 0;
  }

  return Spd->IsDualChannel[Port] == 'y';
}

INTN
SpdGetSize (
  IN  CONST UINTN  Port
  )
{
  BAIKAL_SPD_INFO      *Spd = (BAIKAL_SPD_INFO *) BAIKAL_SPD_DATA_BASE;
  CONST UINT8 * CONST   SpdBuf = &Spd->Buf[Port * BAIKAL_SPD_MAX_SIZE];

  if (Port >= BAIKAL_SPD_PORT_COUNT) {
    return 0;
  }

  switch (*SpdBuf & 0x07) {
  case 1:
    return 128;
  case 2:
    return 256;
  case 3:
    return 384;
  case 4:
    return 512;
  }

  return 0;
}

CONST UINT8 *
SpdGetBuf (
  IN  CONST UINTN  Port
  )
{
  BAIKAL_SPD_INFO  *Spd = (BAIKAL_SPD_INFO *) BAIKAL_SPD_DATA_BASE;

  if (Port >= BAIKAL_SPD_PORT_COUNT) {
    return NULL;
  }

  return &Spd->Buf[Port * BAIKAL_SPD_MAX_SIZE];
}
