/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>

#include <Library/DebugLib.h>
#include <Library/HobLib.h>

#include <Library/BaikalSpdLib.h>

#pragma pack (push, 1)
typedef struct {
  CONST UINT8  Buf[BAIKAL_SPD_MAX_SIZE * BAIKAL_SPD_PORT_COUNT];
  struct {
    CONST UINT32  Serial;
    CONST UINT8   Part[20];
  } Extra[BAIKAL_SPD_PORT_COUNT];
  CONST UINT8  IsDualChannel[BAIKAL_SPD_PORT_COUNT];
  CONST UINT32 ConfiguredSpeed[BAIKAL_SPD_PORT_COUNT];
} BAIKAL_SPD_INFO;
#pragma pack (pop)

STATIC VOID  *SpdAddr;

STATIC
VOID *
SpdGetAddr (
  VOID
  )
{
  if (SpdAddr == NULL) {
    VOID  *Hob;

    Hob = GetFirstGuidHob (&gBaikalSpdHobGuid);
    ASSERT (Hob != NULL);
    ASSERT (GET_GUID_HOB_DATA_SIZE (Hob) == sizeof (UINT64));
    SpdAddr = (VOID *)(UINTN)*(UINT64 *)GET_GUID_HOB_DATA (Hob);
    ASSERT (SpdAddr != NULL);
  }

  return SpdAddr;
}

CONST UINT8 *
SpdGetBuf (
  IN  CONST UINTN  Port
  )
{
  BAIKAL_SPD_INFO  *Spd = SpdGetAddr ();

  if (Port >= BAIKAL_SPD_PORT_COUNT) {
    return NULL;
  }

  return &Spd->Buf[Port * BAIKAL_SPD_MAX_SIZE];
}

UINTN
SpdGetConfiguredSpeed (
  IN  CONST UINTN  Port
  )
{
  BAIKAL_SPD_INFO  *Spd = SpdGetAddr ();

  if (Port >= BAIKAL_SPD_PORT_COUNT) {
    return 0;
  }

  return Spd->ConfiguredSpeed[Port];
}

CONST UINT8 *
SpdGetPart (
  IN  CONST UINTN  Port
  )
{
  BAIKAL_SPD_INFO  *Spd = SpdGetAddr ();

  if (Port >= BAIKAL_SPD_PORT_COUNT) {
    return NULL;
  }

  return Spd->Extra[Port].Part;
}

UINT32
SpdGetSerial (
  IN  CONST UINTN  Port
  )
{
  BAIKAL_SPD_INFO  *Spd = SpdGetAddr ();

  if (Port >= BAIKAL_SPD_PORT_COUNT) {
    return 0;
  }

  return Spd->Extra[Port].Serial;
}

INTN
SpdGetSize (
  IN  CONST UINTN  Port
  )
{
  BAIKAL_SPD_INFO      *Spd = SpdGetAddr ();
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

INTN
SpdIsDualChannel (
  IN  CONST UINTN  Port
  )
{
  BAIKAL_SPD_INFO  *Spd = SpdGetAddr ();

  if (Port >= BAIKAL_SPD_PORT_COUNT) {
    return 0;
  }

  return Spd->IsDualChannel[Port] == 'y';
}
