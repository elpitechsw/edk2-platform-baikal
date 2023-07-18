/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
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
      __func__,
      CommonHeader->FormatVersion,
      COMMON_HEADER_FORMAT_VERSION
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (CalcChecksum ((UINT8 *) CommonHeader, sizeof (COMMON_HEADER))) {
    DEBUG ((EFI_D_ERROR, "%a: invalid Common Header Checksum\n", __func__));
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
      __func__,
      BoardAreaOffset
      ));
    return EFI_INVALID_PARAMETER;
  }

  *BoardArea = &Buf[BoardAreaOffset];
  if (**BoardArea != BOARD_AREA_FORMAT_VERSION) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid board area format version (%u != %u)\n",
      __func__,
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
      __func__,
      *BoardAreaSize
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (CalcChecksum (*BoardArea, *BoardAreaSize)) {
    DEBUG ((EFI_D_ERROR, "%a: invalid board area checksum\n", __func__));
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
      __func__,
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
      __func__,
      ProductAreaOffset
      ));
    return EFI_INVALID_PARAMETER;
  }

  *ProductArea = &Buf[ProductAreaOffset];
  if (**ProductArea != PRODUCT_AREA_FORMAT_VERSION) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid product area format version (%u != %u)\n",
      __func__,
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
      __func__,
      *ProductAreaSize
      ));

    return EFI_INVALID_PARAMETER;
  }

  if (CalcChecksum (*ProductArea, *ProductAreaSize)) {
    DEBUG ((EFI_D_ERROR, "%a: invalid product area checksum\n", __func__));
    return EFI_CRC_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
FruInternalsMultirecordCheckData (
  IN  CONST UINT8 * CONST               MrecBuf,
  IN  CONST UINTN                       MrecBufSize,
  IN  CONST MULTIRECORD_HEADER * CONST  MrecHdr
  )
{
  if ((MrecHdr->Format & 0x07) != MULTIRECORD_FORMAT_VERSION) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unknown multirecord format(%u)\n",
      __func__,
      MrecHdr->Format & 0x07
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (MrecBufSize < sizeof (MULTIRECORD_HEADER) + MrecHdr->Length) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: MrecBufSize(%u) is smaller than multirecord header+data size(%u)\n",
      __func__,
      MrecBufSize,
      sizeof (MULTIRECORD_HEADER) + MrecHdr->Length
      ));
    return EFI_INVALID_PARAMETER;
  }

  if ((CalcChecksum (MrecBuf + sizeof (MULTIRECORD_HEADER), MrecHdr->Length) +
       MrecHdr->RecordChecksum) & 0xFF) {
    DEBUG ((EFI_D_ERROR, "%a: invalid multirecord data checksum\n", __func__));
    return EFI_CRC_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
FruInternalsMultirecordParseHeader (
  IN   CONST UINT8 * CONST         MrecBuf,
  IN   CONST UINTN                 MrecBufSize,
  OUT  MULTIRECORD_HEADER * CONST  MrecHdr
  )
{
  ASSERT (MrecBuf != NULL);
  ASSERT (MrecHdr != NULL);

  if (MrecBufSize < sizeof (MULTIRECORD_HEADER)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: MrecBufSize (%u) is smaller than MULTIRECORD_HEADER size (%u)\n",
      __func__,
      MrecBufSize,
      sizeof (MULTIRECORD_HEADER)
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (CalcChecksum (MrecBuf, sizeof (MULTIRECORD_HEADER))) {
    return EFI_CRC_ERROR;
  }

  MrecHdr->TypeId         = MrecBuf[0];
  MrecHdr->Format         = MrecBuf[1];
  MrecHdr->Length         = MrecBuf[2];
  MrecHdr->RecordChecksum = MrecBuf[3];
  MrecHdr->HeaderChecksum = MrecBuf[4];

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
