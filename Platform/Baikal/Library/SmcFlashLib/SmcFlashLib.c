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
SmcFlashErase (
  IN CONST UINTN  Addr,
  IN CONST UINTN  Size
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  if (!Size) {
    return -1;
  }

  ArmSmcArgs.Arg0 = BAIKAL_SMC_FLASH_ERASE;
  ArmSmcArgs.Arg1 = Addr;
  ArmSmcArgs.Arg2 = Size;
  ArmCallSmc (&ArmSmcArgs);
  return ArmSmcArgs.Arg0;
}

INTN
SmcFlashInfo (
  IN UINT32 * CONST  SectorSize,
  IN UINT32 * CONST  SectorCount
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_FLASH_INFO;
  ArmCallSmc (&ArmSmcArgs);

  if (SectorSize != NULL) {
    *SectorSize = ArmSmcArgs.Arg1;
  }

  if (SectorCount != NULL) {
    *SectorCount = ArmSmcArgs.Arg2;
  }

  return ArmSmcArgs.Arg0;
}

INTN
SmcFlashLock (
  IN CONST UINTN  Lock
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_FLASH_LOCK;
  ArmSmcArgs.Arg1 = Lock;
  ArmCallSmc (&ArmSmcArgs);
  return ArmSmcArgs.Arg0;
}

STATIC
INTN
SmcFlashPosition (
  IN CONST UINTN  Position
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  if (Position >= BAIKAL_SMC_FLASH_DATA_SIZE) {
    return -1;
  }

  ArmSmcArgs.Arg0 = BAIKAL_SMC_FLASH_POSITION;
  ArmSmcArgs.Arg1 = Position;
  ArmCallSmc (&ArmSmcArgs);
  return ArmSmcArgs.Arg0;
}

STATIC
INTN
SmcFlashPull4Words (
  IN VOID  *Data
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  UINT64        *DataPtr = Data;

  if (Data == NULL) {
    return -1;
  }

  ArmSmcArgs.Arg0 = BAIKAL_SMC_FLASH_PULL;
  ArmCallSmc (&ArmSmcArgs); // Pull 4xUINT64 words from TF-A
  DataPtr[0] = ArmSmcArgs.Arg0;
  DataPtr[1] = ArmSmcArgs.Arg1;
  DataPtr[2] = ArmSmcArgs.Arg2;
  DataPtr[3] = ArmSmcArgs.Arg3;
  return 0;
}

STATIC
INTN
SmcFlashPullBuf (
  IN VOID   *Data,
  IN UINTN  Size
  )
{
  UINT8  *DataPtr = Data;
  INTN   Err;

  if (Data == NULL || !Size || (Size > BAIKAL_SMC_FLASH_DATA_SIZE)) {
    return -1;
  }

  Err = SmcFlashPosition (0);
  if (Err) {
    return Err;
  }

  while (Size) {
    UINT64   Buf[4];
    UINT8   *BufPtr = (VOID *) Buf;
    UINTN    Part = MIN (Size, sizeof (Buf));

    Err = SmcFlashPull4Words (Buf); // Pull buffer from TF-A
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
SmcFlashPush4Words (
  IN CONST VOID  *Data
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  CONST UINT64  *DataPtr = Data;

  if (Data == NULL) {
    return -1;
  }

  ArmSmcArgs.Arg0 = BAIKAL_SMC_FLASH_PUSH;
  ArmSmcArgs.Arg1 = DataPtr[0];
  ArmSmcArgs.Arg2 = DataPtr[1];
  ArmSmcArgs.Arg3 = DataPtr[2];
  ArmSmcArgs.Arg4 = DataPtr[3];
  ArmCallSmc (&ArmSmcArgs); // Push 4xUINT64 words to TF-A
  return ArmSmcArgs.Arg0;
}

STATIC
INTN
SmcFlashPushBuf (
  IN CONST VOID  *Data,
  IN UINTN       Size
  )
{
  CONST UINT8  *DataPtr = Data;
  INTN         Err;

  if (Data == NULL || !Size || (Size > BAIKAL_SMC_FLASH_DATA_SIZE)) {
    return -1;
  }

  Err = SmcFlashPosition (0);
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

    Err = SmcFlashPush4Words (Buf); // Push buffer to TF-A
    if (Err) {
      return Err;
    }
  }

  return 0;
}

STATIC
INTN
SmcFlashReadBuf (
  IN CONST UINTN  Addr,
  IN CONST UINTN  Size
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  if (!Size) {
    return -1;
  }

  ArmSmcArgs.Arg0 = BAIKAL_SMC_FLASH_READ;
  ArmSmcArgs.Arg1 = Addr;
  ArmSmcArgs.Arg2 = Size;
  ArmCallSmc (&ArmSmcArgs);
  return ArmSmcArgs.Arg0;
}

INTN
SmcFlashRead (
  IN UINTN  Addr,
  IN VOID   *Data,
  IN UINTN  Size
  )
{
  UINT8  *DataPtr = Data;
  INTN   Err;

  if (Data == NULL || !Size) {
    return -1;
  }

  while (Size) {
    CONST UINTN  Part = MIN (Size, BAIKAL_SMC_FLASH_DATA_SIZE);

    Err = SmcFlashReadBuf (Addr, Part);
    if (Err) {
      return Err;
    }

    Err = SmcFlashPullBuf (DataPtr, Part);
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
SmcFlashWriteBuf (
  IN CONST UINTN  Addr,
  IN CONST UINTN  Size
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  if (!Size) {
    return -1;
  }

  ArmSmcArgs.Arg0 = BAIKAL_SMC_FLASH_WRITE;
  ArmSmcArgs.Arg1 = Addr;
  ArmSmcArgs.Arg2 = Size;
  ArmCallSmc (&ArmSmcArgs);
  return ArmSmcArgs.Arg0;
}

INTN
SmcFlashWrite (
  IN UINTN       Addr,
  IN CONST VOID  *Data,
  IN UINTN       Size
  )
{
  CONST UINT8  *DataPtr = Data;
  INTN         Err;

  if (Data == NULL || !Size) {
    return -1;
  }

  while (Size) {
    CONST UINTN  Part = MIN (Size, BAIKAL_SMC_FLASH_DATA_SIZE);

    Err = SmcFlashPushBuf (DataPtr, Part);
    if (Err) {
      return Err;
    }

    Err = SmcFlashWriteBuf (Addr, Part);
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
SmcFlashConvertPointers (
  VOID
  )
{
  EfiConvertPointer (0x0, (VOID **) &SmcFlashErase);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashInfo);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashLock);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashPosition);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashPull4Words);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashPullBuf);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashPush4Words);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashPushBuf);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashRead);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashReadBuf);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashWrite);
  EfiConvertPointer (0x0, (VOID **) &SmcFlashWriteBuf);
}
