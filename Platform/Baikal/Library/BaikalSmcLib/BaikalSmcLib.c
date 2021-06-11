/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/ArmSmcLib.h>
#include <Library/UefiRuntimeLib.h>

#define BAIKAL_SMC_FLASH_DATA_SIZE  1024
#define BAIKAL_SMC_FLASH            0x82000002
#define BAIKAL_SMC_FLASH_WRITE      (BAIKAL_SMC_FLASH + 0)
#define BAIKAL_SMC_FLASH_READ       (BAIKAL_SMC_FLASH + 1)
#define BAIKAL_SMC_FLASH_ERASE      (BAIKAL_SMC_FLASH + 2)
#define BAIKAL_SMC_FLASH_PUSH       (BAIKAL_SMC_FLASH + 3)
#define BAIKAL_SMC_FLASH_PULL       (BAIKAL_SMC_FLASH + 4)
#define BAIKAL_SMC_FLASH_POSITION   (BAIKAL_SMC_FLASH + 5)
#define BAIKAL_SMC_FLASH_INFO       (BAIKAL_SMC_FLASH + 6)

STATIC
INTN
BaikalSmcFlashPosition (
  IN  CONST UINT32  Position
  )
{
  if (Position >= BAIKAL_SMC_FLASH_DATA_SIZE) {
    return -1;
  }

  ARM_SMC_ARGS  Arg = {BAIKAL_SMC_FLASH_POSITION, Position, 0, 0, 0};
  ArmCallSmc (&Arg);
  return Arg.Arg0;
}

STATIC
INTN
BaikalSmcFlashWriteBuf (
  IN  CONST UINT32  Adr,
  IN  CONST UINT32  Size
  )
{
  if (!Size) {
    return -1;
  }

  ARM_SMC_ARGS  Arg = {BAIKAL_SMC_FLASH_WRITE, Adr, Size, 0, 0};
  ArmCallSmc (&Arg);
  return Arg.Arg0;
}

STATIC
INTN
BaikalSmcFlashReadBuf (
  IN  CONST UINT32  Adr,
  IN  CONST UINT32  Size
  )
{
  if (!Size) {
    return -1;
  }

  ARM_SMC_ARGS  Arg = {BAIKAL_SMC_FLASH_READ, Adr, Size, 0, 0};
  ArmCallSmc (&Arg);
  return Arg.Arg0;
}

/* Push 4xUINT64 words to ARM-TF */
STATIC
INTN
BaikalSmcFlashPush4Words (
  IN  CONST VOID  *Data
  )
{
  if (Data == NULL) {
    return -1;
  }

  CONST UINT64  *Ptr = Data;
  ARM_SMC_ARGS  Arg = {BAIKAL_SMC_FLASH_PUSH, Ptr[0], Ptr[1], Ptr[2], Ptr[3]};
  ArmCallSmc (&Arg);
  return Arg.Arg0;
}

/* Pull 4xUINT64 words from ARM-TF */
STATIC
INTN
BaikalSmcFlashPull4Words (
  IN  VOID  *Data
  )
{
  if (Data == NULL) {
    return -1;
  }

  ARM_SMC_ARGS  Arg = {BAIKAL_SMC_FLASH_PULL, 0, 0, 0, 0};
  ArmCallSmc (&Arg);

  UINT64  *Ptr = Data;
  Ptr[0] = Arg.Arg0;
  Ptr[1] = Arg.Arg1;
  Ptr[2] = Arg.Arg2;
  Ptr[3] = Arg.Arg3;

  return 0;
}

/* Push buffer to ATM-TF */
STATIC
INTN
BaikalSmcFlashPushBuf (
  IN  CONST VOID  *Data,
  IN  UINT32       Size
  )
{
  INTN ret;
  if (Data == NULL || !Size || (Size > BAIKAL_SMC_FLASH_DATA_SIZE)) {
    return -1;
  }

  CONST UINT8  *a = Data;
  UINT8 *pb;
  UINT8  part;
  UINT64  b[4];

  ret = BaikalSmcFlashPosition (0);
  if (ret) {
    return ret;
  }

  while (Size) {
    pb = (VOID *) b;
    part = MIN (Size, sizeof (b));
    Size -= part;

    while (part--)
      *pb++ = *a++;

    ret = BaikalSmcFlashPush4Words (b);
    if (ret) {
      return ret;
    }
  }

  return 0;
}

/* Pull buffer from ARM-TF */
STATIC
INTN
BaikalSmcFlashPullBuf (
  IN  VOID    *Data,
  IN  UINT32   Size
  )
{
  INTN ret;
  if (Data == NULL || !Size || (Size > BAIKAL_SMC_FLASH_DATA_SIZE)) {
    return -1;
  }

  UINT8 *a = Data;
  UINT8 *pb;
  UINT8  part;
  UINT64  b[4];

  ret = BaikalSmcFlashPosition (0);
  if (ret) {
    return ret;
  }

  while (Size) {
    ret = BaikalSmcFlashPull4Words (b);
    if (ret) {
      return ret;
    }

    pb = (VOID *) b;
    part = MIN (Size, sizeof (b));
    Size -= part;
    while (part--)
      *a++ = *pb++;
  }
  return 0;
}

INTN
BaikalSmcFlashErase (
  IN  CONST UINT32  Adr,
  IN  CONST UINT32  Size
  )
{
  if (!Size) {
    return -1;
  }

  ARM_SMC_ARGS Arg = {BAIKAL_SMC_FLASH_ERASE, Adr, Size, 0, 0};
  ArmCallSmc (&Arg);
  return Arg.Arg0;
}

INTN
BaikalSmcFlashWrite (
  IN  UINT32       Adr,
  IN  CONST VOID  *Data,
  IN  UINT32       Size
  )
{
  INTN ret;

  if (Data == NULL || !Size) {
    return -1;
  }

  UINT32 part;
  CONST UINT8  *pdata = Data;

  while (Size) {
    part = MIN (Size, BAIKAL_SMC_FLASH_DATA_SIZE);

    ret = BaikalSmcFlashPushBuf (pdata, part);
    if (ret) {
      return ret;
    }

    ret = BaikalSmcFlashWriteBuf (Adr, part);
    if (ret) {
      return ret;
    }

    Adr   += part;
    pdata += part;
    Size  -= part;
  }

  return 0;
}

INTN
BaikalSmcFlashRead (
  IN  UINT32   Adr,
  IN  VOID    *Data,
  IN  UINT32   Size
  )
{
  INTN ret;

  if (Data == NULL || !Size) {
    return -1;
  }

  UINT32 part;
  UINT8 *pdata = Data;

  while (Size) {
    part = MIN (Size, BAIKAL_SMC_FLASH_DATA_SIZE);

    ret = BaikalSmcFlashReadBuf (Adr, part);
    if (ret) {
      return ret;
    }

    ret = BaikalSmcFlashPullBuf (pdata, part);
    if (ret) {
      return ret;
    }

    Adr   += part;
    pdata += part;
    Size  -= part;
  }

  return 0;
}

INTN
BaikalSmcFlashInfo (
  IN  UINT32 *CONST  SectorSize,
  IN  UINT32 *CONST  SectorCount
  )
{
  ARM_SMC_ARGS  Arg;
  Arg.Arg0 = BAIKAL_SMC_FLASH_INFO;
  ArmCallSmc (&Arg);

  if (SectorSize != NULL) {
    *SectorSize = Arg.Arg1;
  }

  if (SectorCount != NULL) {
    *SectorCount = Arg.Arg2;
  }

  return Arg.Arg0;
}

VOID
BaikalSmcFlashConvertPointers (
  VOID
  )
{
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashInfo);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashErase);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashWrite);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashRead);

  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashPosition);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashWriteBuf);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashReadBuf);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashPush4Words);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashPull4Words);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashPushBuf);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashPullBuf);
}
