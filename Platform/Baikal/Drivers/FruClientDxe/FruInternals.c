/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include "FruInternals.h"

#define COMMON_HEADER_FORMAT_VERSION  0x01
#define BOARD_AREA_FORMAT_VERSION     0x01
#define PRODUCT_AREA_FORMAT_VERSION   0x01
#define MULTIRECORD_FORMAT_VERSION    0x02

#pragma pack(1)
typedef struct {
  UINT8  FormatVersion:4;
  UINT8  Reserved:4;
  UINT8  InternalUseAreaOffset;
  UINT8  ChassisAreaOffset;
  UINT8  BoardAreaOffset;
  UINT8  ProductAreaOffset;
  UINT8  MultirecordAreaOffset;
  UINT8  Pad;
  UINT8  Checksum;
} COMMON_HEADER;
#pragma pack()

STATIC
CHAR8
Ascii6ToAscii8 (
  IN  CONST UINT8  Ascii6
  )
{
  return Ascii6 + 0x20;
}

STATIC
UINTN
Ascii6BufToAscii8Buf (
  IN   CONST UINT8 * CONST  Buf6,
  IN   CONST UINTN          Buf6Size,
  OUT        CHAR8 * CONST  Buf8,
  IN   CONST UINTN          Buf8Size
  )
{
  UINTN  Idx6, Idx8;

  ASSERT (Buf6 != NULL);
  ASSERT (Buf8 != NULL);

  for (Idx6 = 0, Idx8 = 0; Idx6 < Buf6Size && Idx8 < Buf8Size; ++Idx8) {
    if (!(Idx8 % 4)) {
      Buf8[Idx8] = Ascii6ToAscii8 (Buf6[Idx6++] & 0x3F);
    } else if ((Idx8 % 4) == 1) {
      Buf8[Idx8] = Ascii6ToAscii8 ((Buf6[Idx6 - 1] >> 6) | ((Buf6[Idx6] & 0xF) << 2));
      ++Idx6;
    } else if ((Idx8 % 4) == 2) {
      Buf8[Idx8] = Ascii6ToAscii8 ((Buf6[Idx6 - 1] >> 4) | ((Buf6[Idx6] & 3) << 4));
    } else {
      Buf8[Idx8] = Ascii6ToAscii8 (Buf6[Idx6++] >> 2);
    }
  }

  return Idx8;
}

STATIC
CHAR8
BcdPlusToAscii8 (
  IN  CONST UINT8  BcdPlus
  )
{
  if (BcdPlus < 0xA) {
    return '0' + BcdPlus;
  } else if (BcdPlus == 0xA) {
    return ' ';
  } else if (BcdPlus == 0xB) {
    return '-';
  } else if (BcdPlus == 0xC) {
    return '.';
  }

  return 0;
}

STATIC
UINTN
BcdPlusBufToAscii8Buf (
  IN   CONST UINT8 * CONST  BufBcd,
  IN   CONST UINTN          BufBcdSize,
  OUT        CHAR8 * CONST  Buf8,
  IN   CONST UINTN          Buf8Size
  )
{
  UINTN  Idx8, IdxBcd;

  for (Idx8 = 0, IdxBcd = 0; IdxBcd < BufBcdSize && Idx8 < Buf8Size; ++Idx8) {
    if (!(Idx8 % 2)) {
      Buf8[Idx8] = BcdPlusToAscii8 ((BufBcd[IdxBcd] >> 0) & 0xF);
    } else {
      Buf8[Idx8] = BcdPlusToAscii8 ((BufBcd[IdxBcd] >> 4) & 0xF);
      ++IdxBcd;
    }
  }

  return Idx8;
}

STATIC
UINT8
CalcChecksum (
  IN  CONST UINT8  *Buf,
  IN        UINTN   BufSize
  )
{
  UINT8  Checksum = 0;

  ASSERT (Buf != NULL);

  while (BufSize--) {
    Checksum += *Buf++;
  }

  return Checksum;
}

STATIC
EFI_STATUS
FruInternalsCommonHeaderCheck (
  IN  CONST VOID * CONST  Buf,
  IN  CONST UINTN         BufSize
  )
{
  CONST COMMON_HEADER * CONST  CommonHeader = (COMMON_HEADER *) Buf;

  ASSERT (Buf != NULL);
  ASSERT (BufSize >= sizeof (COMMON_HEADER_FORMAT_VERSION));

  if (CommonHeader->FormatVersion != COMMON_HEADER_FORMAT_VERSION) {
    DEBUG ((
      EFI_D_ERROR, "%a: invalid Common Header Format Version: %d != %d\n",
      __FUNCTION__,
      CommonHeader->FormatVersion,
      COMMON_HEADER_FORMAT_VERSION
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (CalcChecksum ((UINT8 *) CommonHeader, sizeof (COMMON_HEADER))) {
    DEBUG ((EFI_D_ERROR, "%a: invalid Common Header Checksum\n", __FUNCTION__));
    return EFI_CRC_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
FruInternalsBoardAreaLocate (
  IN   CONST UINT8 * CONST    Buf,
  IN   CONST UINTN            BufSize,
  OUT  CONST UINT8          **BoardArea,
  OUT        UINTN * CONST    BoardAreaSize
  )
{
  UINTN       BoardAreaOffset;
  EFI_STATUS  Status;

  ASSERT (Buf           != NULL);
  ASSERT (BoardArea     != NULL);
  ASSERT (BoardAreaSize != NULL);

  Status = FruInternalsCommonHeaderCheck (Buf, BufSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  BoardAreaOffset = ((COMMON_HEADER *) Buf)->BoardAreaOffset * 8;
  if (!BoardAreaOffset || BoardAreaOffset >= BufSize) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid board area offset(%u)\n",
      __FUNCTION__,
      BoardAreaOffset
      ));
    return EFI_INVALID_PARAMETER;
  }

  *BoardArea = &Buf[BoardAreaOffset];
  if (**BoardArea != BOARD_AREA_FORMAT_VERSION) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid board area format version (%u != %u)\n",
      __FUNCTION__,
      **BoardArea,
      BOARD_AREA_FORMAT_VERSION
      ));
    return EFI_INVALID_PARAMETER;
  }

  *BoardAreaSize = (*BoardArea)[1] * 8;
  if (*BoardAreaSize > BufSize - BoardAreaOffset) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid board area size(%u)\n",
      __FUNCTION__,
      *BoardAreaSize
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (CalcChecksum (*BoardArea, *BoardAreaSize)) {
    DEBUG ((EFI_D_ERROR, "%a: invalid board area checksum\n", __FUNCTION__));
    return EFI_CRC_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
FruInternalsMultirecordAreaLocate (
  IN   CONST UINT8 * CONST    Buf,
  IN   CONST UINTN            BufSize,
  OUT  CONST UINT8          **MultirecordArea
  )
{
  UINTN       MultirecordAreaOffset;
  EFI_STATUS  Status;

  ASSERT (Buf             != NULL);
  ASSERT (MultirecordArea != NULL);

  Status = FruInternalsCommonHeaderCheck (Buf, BufSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MultirecordAreaOffset = ((COMMON_HEADER *) Buf)->MultirecordAreaOffset * 8;
  if (!MultirecordAreaOffset || MultirecordAreaOffset >= BufSize) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid multirecord area offset (%u)\n",
      __FUNCTION__,
      MultirecordAreaOffset
      ));
    return EFI_INVALID_PARAMETER;
  }

  *MultirecordArea = &Buf[MultirecordAreaOffset];

  return EFI_SUCCESS;
}

EFI_STATUS
FruInternalsProductAreaLocate (
  IN   CONST UINT8 * CONST    Buf,
  IN   CONST UINTN            BufSize,
  OUT  CONST UINT8          **ProductArea,
  OUT        UINTN * CONST    ProductAreaSize
  )
{
  UINTN       ProductAreaOffset;
  EFI_STATUS  Status;

  ASSERT (Buf             != NULL);
  ASSERT (ProductArea     != NULL);
  ASSERT (ProductAreaSize != NULL);

  Status = FruInternalsCommonHeaderCheck (Buf, BufSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ProductAreaOffset = ((COMMON_HEADER *) Buf)->ProductAreaOffset * 8;
  if (!ProductAreaOffset || ProductAreaOffset >= BufSize) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid product area offset(%u)\n",
      __FUNCTION__,
      ProductAreaOffset
      ));
    return EFI_INVALID_PARAMETER;
  }

  *ProductArea = &Buf[ProductAreaOffset];
  if (**ProductArea != PRODUCT_AREA_FORMAT_VERSION) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid product area format version (%u != %u)\n",
      __FUNCTION__,
      **ProductArea,
      PRODUCT_AREA_FORMAT_VERSION
      ));
    return EFI_INVALID_PARAMETER;
  }

  *ProductAreaSize = (*ProductArea)[1] * 8;
  if (*ProductAreaSize > BufSize - ProductAreaOffset) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid product area size(%u)\n",
      __FUNCTION__,
      *ProductAreaSize
      ));

    return EFI_INVALID_PARAMETER;
  }

  if (CalcChecksum (*ProductArea, *ProductAreaSize)) {
    DEBUG ((EFI_D_ERROR, "%a: invalid product area checksum\n", __FUNCTION__));
    return EFI_CRC_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
FruInternalsMultirecordCheckData (
  IN  CONST UINT8 * CONST               MrecBuf,
  IN  CONST UINTN                       MrecBufSize,
  IN  CONST MULTIRECORD_HEADER *        MrecHdr
  )
{
  if ((MrecHdr->Format & 0x07) != MULTIRECORD_FORMAT_VERSION) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unknown multirecord format(%u)\n",
      __FUNCTION__,
      MrecHdr->Format & 0x07
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (MrecBufSize < sizeof (MULTIRECORD_HEADER) + MrecHdr->Length) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: MrecBufSize(%u) is smaller than multirecord header+data size(%u)\n",
      __FUNCTION__,
      MrecBufSize,
      sizeof (MULTIRECORD_HEADER) + MrecHdr->Length
      ));
    return EFI_INVALID_PARAMETER;
  }

  if ((CalcChecksum (MrecBuf + sizeof (MULTIRECORD_HEADER), MrecHdr->Length) +
       MrecHdr->RecordChecksum) & 0xFF) {
    DEBUG ((EFI_D_ERROR, "%a: invalid multirecord data checksum\n", __FUNCTION__));
    return EFI_CRC_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
FruInternalsMultirecordParseHeader (
  IN   CONST UINT8 * CONST         MrecBuf,
  IN   CONST UINTN                 MrecBufSize,
  OUT  MULTIRECORD_HEADER **       MrecHdr
  )
{
  ASSERT (MrecBuf != NULL);
  ASSERT (MrecHdr != NULL);

  if (MrecBufSize < sizeof (MULTIRECORD_HEADER)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: MrecBufSize (%u) is smaller than MULTIRECORD_HEADER size (%u)\n",
      __FUNCTION__,
      MrecBufSize,
      sizeof (MULTIRECORD_HEADER)
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (CalcChecksum (MrecBuf, sizeof (MULTIRECORD_HEADER))) {
    return EFI_CRC_ERROR;
  }

  *MrecHdr = (MULTIRECORD_HEADER *)MrecBuf;

  return EFI_SUCCESS;
}

CONST VOID *
FruInternalsTypLenEncReadData (
  IN      CONST UINT8 * CONST  EncBuf,
  OUT           CHAR8 * CONST  DecBuf,
  IN OUT        UINTN * CONST  DecLen
  )
{
  UINTN  EncLen;
  UINT8  EncType;

  ASSERT (EncBuf != NULL);
  ASSERT ((DecBuf != NULL && DecLen != NULL && *DecLen > 0) ||
          (DecBuf == NULL && DecLen == NULL));

  EncLen  = EncBuf[0] & 0x3F;
  EncType = EncBuf[0] >> 6;

  if (DecBuf && DecLen) { // If NULL then just skip to the next record
    if (!EncLen || (EncType == 3 && EncLen == 1)) {
      *DecLen = 0;
    } else if (EncType == 0 || EncType == 3) { // Binary or ASCII/Unicode
      *DecLen = MIN (EncLen, *DecLen - 1);
      gBS->CopyMem (DecBuf, (VOID *) &EncBuf[1], *DecLen);
      DecBuf[*DecLen] = '\0';
    } else if (EncType == 1) { // BCD-plus
      *DecLen = BcdPlusBufToAscii8Buf (&EncBuf[1], EncLen, DecBuf, *DecLen - 1);
      DecBuf[*DecLen] = '\0';
    } else { // 6-bit ASCII
      *DecLen = Ascii6BufToAscii8Buf (&EncBuf[1], EncLen, DecBuf, *DecLen - 1);
      DecBuf[*DecLen] = '\0';
    }
  }

  if (EncType == 3 && EncLen == 1) { // End-of-fields
    return NULL;
  }

  return EncBuf + EncLen + 1;
}

EFI_STATUS
FruInternalsGetMultirecord (
  IN   CONST UINT8 * CONST    Buf,
  IN   CONST UINTN            BufSize,
  IN         UINT8            MrecType,
  OUT  CONST MULTIRECORD_HEADER **MrecHeader
  )
{
  CONST UINT8         *MrecArea;
  MULTIRECORD_HEADER  *MrecHdr;
  EFI_STATUS           Status;

  ASSERT (MrecHeader != NULL);

  Status = FruInternalsMultirecordAreaLocate (Buf, BufSize, &MrecArea);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MrecHdr = (MULTIRECORD_HEADER *)MrecArea;
  while (FruInternalsMultirecordParseHeader (
           MrecArea,
           (Buf + BufSize) - MrecArea,
           &MrecHdr
           ) == EFI_SUCCESS) {
    if (MrecHdr->TypeId == MrecType) {
      if (FruInternalsMultirecordCheckData (MrecArea,
                                            (Buf + BufSize) - MrecArea,
                                            MrecHdr
                                            ) == EFI_SUCCESS) {
        *MrecHeader = MrecHdr;
        return EFI_SUCCESS;
      }
    }

    if (MrecHdr->Format & 0x80) {
      break;
    }

    MrecArea += sizeof (MULTIRECORD_HEADER) + MrecHdr->Length;
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
FruInternalsSetMultirecord (
  IN   CONST UINT8 * CONST    Buf,
  IN   CONST UINTN            BufSize,
  IN         UINT8            MrecType,
  IN         UINT8            MrecFmt,
  IN         UINT8            MrecLen,
  IN         UINTN            OemId,
  IN   CONST UINT8 *          MrecData
  )
{
  CONST UINT8         *MrecArea;
  MULTIRECORD_HEADER  *MrecHdr;
  EFI_STATUS           Status;
  UINT8               *NewData;

  ASSERT (MrecData != NULL);

  Status = FruInternalsMultirecordAreaLocate (Buf, BufSize, &MrecArea);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (FruInternalsMultirecordParseHeader (
           MrecArea,
           (Buf + BufSize) - MrecArea,
           &MrecHdr
           ) == EFI_SUCCESS) {
    if (MrecHdr->TypeId == MrecType &&
        (MrecHdr->Format & 0x7f) == MrecFmt &&
	(MrecHdr->Length == MrecLen || MrecHdr->Length == MrecLen + 3)) {
      NewData = (UINT8 *)MrecArea + sizeof (MULTIRECORD_HEADER);
      if (MrecHdr->Length == MrecLen + 3) {
        NewData[0] = OemId & 0xff;
	NewData[1] = (OemId >> 8) & 0xff;
	NewData[2] = (OemId >> 16) & 0xff;
        CopyMem(NewData + 3, MrecData, MrecLen);
      } else {
        CopyMem(NewData, MrecData, MrecLen);
      }
      MrecHdr->RecordChecksum = 256 - CalcChecksum(NewData, MrecHdr->Length);
      MrecHdr->HeaderChecksum = 256 - CalcChecksum((UINT8 *)MrecHdr, 4);

      return EFI_SUCCESS;
    }

    if (MrecHdr->Format & 0x80) {
      break;
    }

    MrecArea += sizeof (MULTIRECORD_HEADER) + MrecHdr->Length;
  }

  /* Mrec not found - create new one */
  if (MrecArea - Buf + 2 * sizeof (MULTIRECORD_HEADER) + MrecHdr->Length + MrecLen + 3 >= BufSize) {
    DEBUG((DEBUG_ERROR, "%a: No space in FRU buffer\n", __FUNCTION__));
    return EFI_BUFFER_TOO_SMALL;
  }
  MrecHdr->Format &= ~0x80;
  MrecHdr->HeaderChecksum = 256 - CalcChecksum((UINT8 *)MrecHdr, 4);
  MrecArea += sizeof (MULTIRECORD_HEADER) + MrecHdr->Length;
  MrecHdr = (MULTIRECORD_HEADER *)MrecArea;
  MrecHdr->TypeId = MrecType;
  MrecHdr->Format = MrecFmt | 0x80;
  NewData = (UINT8 *)MrecArea + sizeof (MULTIRECORD_HEADER);
  if (OemId) {
    NewData[0] = OemId & 0xff;
    NewData[1] = (OemId >> 8) & 0xff;
    NewData[2] = (OemId >> 16) & 0xff;
    CopyMem(NewData + 3, MrecData, MrecLen);
    MrecHdr->Length = MrecLen + 3;
  } else {
    MrecHdr->Length = MrecLen;
    CopyMem(NewData, MrecData, MrecLen);
  }
  MrecHdr->RecordChecksum = 256 - CalcChecksum(NewData, MrecHdr->Length);
  MrecHdr->HeaderChecksum = 256 - CalcChecksum((UINT8 *)MrecHdr, 4);

  return EFI_SUCCESS;
}

