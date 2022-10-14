/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
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
#define BAIKAL_SMC_FLASH_LOCK       (BAIKAL_SMC_FLASH + 7)

INTN
BaikalSmcFlashErase (
  IN  CONST UINTN  Addr,
  IN  CONST UINTN  Size
  )
{
  ARM_SMC_ARGS  Arg;

  if (!Size) {
    return -1;
  }

  Arg.Arg0 = BAIKAL_SMC_FLASH_ERASE;
  Arg.Arg1 = Addr;
  Arg.Arg2 = Size;
  ArmCallSmc (&Arg);
  return Arg.Arg0;
}

INTN
BaikalSmcFlashInfo (
  IN  UINT32 * CONST  SectorSize,
  IN  UINT32 * CONST  SectorCount
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

INTN
BaikalSmcFlashLock (
  IN  CONST UINTN  Lock
  )
{
  ARM_SMC_ARGS  Arg;

  Arg.Arg0 = BAIKAL_SMC_FLASH_LOCK;
  Arg.Arg1 = Lock;
  ArmCallSmc (&Arg);
  return Arg.Arg0;
}

STATIC
INTN
BaikalSmcFlashPosition (
  IN  CONST UINTN  Position
  )
{
  ARM_SMC_ARGS  Arg;

  if (Position >= BAIKAL_SMC_FLASH_DATA_SIZE) {
    return -1;
  }

  Arg.Arg0 = BAIKAL_SMC_FLASH_POSITION;
  Arg.Arg1 = Position;
  ArmCallSmc (&Arg);
  return Arg.Arg0;
}

STATIC
INTN
BaikalSmcFlashPull4Words (
  IN  VOID  *Data
  )
{
  ARM_SMC_ARGS   Arg;
  UINT64        *DataPtr = Data;

  if (Data == NULL) {
    return -1;
  }

  Arg.Arg0 = BAIKAL_SMC_FLASH_PULL;
  ArmCallSmc (&Arg); // Pull 4xUINT64 words from TF-A
  DataPtr[0] = Arg.Arg0;
  DataPtr[1] = Arg.Arg1;
  DataPtr[2] = Arg.Arg2;
  DataPtr[3] = Arg.Arg3;
  return 0;
}

STATIC
INTN
BaikalSmcFlashPullBuf (
  IN  VOID   *Data,
  IN  UINTN   Size
  )
{
  UINT8  *DataPtr = Data;
  INTN    Err;

  if (Data == NULL || !Size || (Size > BAIKAL_SMC_FLASH_DATA_SIZE)) {
    return -1;
  }

  Err = BaikalSmcFlashPosition (0);
  if (Err) {
    return Err;
  }

  while (Size) {
    UINT64   Buf[4];
    UINT8   *BufPtr = (VOID *) Buf;
    UINTN    Part = MIN (Size, sizeof (Buf));

    Err = BaikalSmcFlashPull4Words (Buf); // Pull buffer from TF-A
    if (Err) {
      return Err;
    }

    Size -= Part;
    while (Part--) {
      *DataPtr++ = *BufPtr++;
    }
  }

  return 0;
}

STATIC
INTN
BaikalSmcFlashPush4Words (
  IN  CONST VOID  *Data
  )
{
  ARM_SMC_ARGS   Arg;
  CONST UINT64  *DataPtr = Data;

  if (Data == NULL) {
    return -1;
  }

  Arg.Arg0 = BAIKAL_SMC_FLASH_PUSH;
  Arg.Arg1 = DataPtr[0];
  Arg.Arg2 = DataPtr[1];
  Arg.Arg3 = DataPtr[2];
  Arg.Arg4 = DataPtr[3];
  ArmCallSmc (&Arg); // Push 4xUINT64 words to TF-A
  return Arg.Arg0;
}

STATIC
INTN
BaikalSmcFlashPushBuf (
  IN  CONST VOID  *Data,
  IN  UINTN        Size
  )
{
  CONST UINT8  *DataPtr = Data;
  INTN          Err;

  if (Data == NULL || !Size || (Size > BAIKAL_SMC_FLASH_DATA_SIZE)) {
    return -1;
  }

  Err = BaikalSmcFlashPosition (0);
  if (Err) {
    return Err;
  }

  while (Size) {
    UINT64   Buf[4];
    UINT8   *BufPtr = (VOID *) Buf;
    UINTN    Part = MIN (Size, sizeof (Buf));

    Size -= Part;
    while (Part--) {
      *BufPtr++ = *DataPtr++;
    }

    Err = BaikalSmcFlashPush4Words (Buf); // Push buffer to TF-A
    if (Err) {
      return Err;
    }
  }

  return 0;
}

STATIC
INTN
BaikalSmcFlashReadBuf (
  IN  CONST UINTN  Addr,
  IN  CONST UINTN  Size
  )
{
  ARM_SMC_ARGS  Arg;

  if (!Size) {
    return -1;
  }

  Arg.Arg0 = BAIKAL_SMC_FLASH_READ;
  Arg.Arg1 = Addr;
  Arg.Arg2 = Size;
  ArmCallSmc (&Arg);
  return Arg.Arg0;
}

INTN
BaikalSmcFlashRead (
  IN  UINTN   Addr,
  IN  VOID   *Data,
  IN  UINTN   Size
  )
{
  UINT8  *DataPtr = Data;
  INTN    Err;

  if (Data == NULL || !Size) {
    return -1;
  }

  while (Size) {
    CONST UINTN  Part = MIN (Size, BAIKAL_SMC_FLASH_DATA_SIZE);

    Err = BaikalSmcFlashReadBuf (Addr, Part);
    if (Err) {
      return Err;
    }

    Err = BaikalSmcFlashPullBuf (DataPtr, Part);
    if (Err) {
      return Err;
    }

    Addr    += Part;
    DataPtr += Part;
    Size    -= Part;
  }

  return 0;
}

STATIC
INTN
BaikalSmcFlashWriteBuf (
  IN  CONST UINTN  Addr,
  IN  CONST UINTN  Size
  )
{
  ARM_SMC_ARGS  Arg;

  if (!Size) {
    return -1;
  }

  Arg.Arg0 = BAIKAL_SMC_FLASH_WRITE;
  Arg.Arg1 = Addr;
  Arg.Arg2 = Size;
  ArmCallSmc (&Arg);
  return Arg.Arg0;
}

INTN
BaikalSmcFlashWrite (
  IN  UINTN        Addr,
  IN  CONST VOID  *Data,
  IN  UINTN        Size
  )
{
  CONST UINT8  *DataPtr = Data;
  INTN          Err;

  if (Data == NULL || !Size) {
    return -1;
  }

  while (Size) {
    CONST UINTN  Part = MIN (Size, BAIKAL_SMC_FLASH_DATA_SIZE);

    Err = BaikalSmcFlashPushBuf (DataPtr, Part);
    if (Err) {
      return Err;
    }

    Err = BaikalSmcFlashWriteBuf (Addr, Part);
    if (Err) {
      return Err;
    }

    Addr    += Part;
    DataPtr += Part;
    Size    -= Part;
  }

  return 0;
}

VOID
BaikalSmcFlashConvertPointers (
  VOID
  )
{
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashErase);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashInfo);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashLock);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashPosition);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashPull4Words);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashPullBuf);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashPush4Words);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashPushBuf);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashRead);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashReadBuf);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashWrite);
  EfiConvertPointer (0x0, (VOID **) &BaikalSmcFlashWriteBuf);
}
