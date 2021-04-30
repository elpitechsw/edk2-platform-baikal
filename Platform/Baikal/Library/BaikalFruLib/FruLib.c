// Copyright (c) 2020 Baikal Electronics JSC
// Author: Mikhail Ivanov <michail.ivanov@baikalelectronics.ru>

#include <string.h>
#include <Uefi.h>
#include <Library/BaikalFruLib.h>
#include <Library/DebugLib.h>

#define FRU_COMMON_HEADER_FORMAT_VERSION  0x01

#define FRU_STR_MAX         32
#define BOARD_AREA_VERSION  0x01
#define MR_MAC_REC          0xc0
#define MR_MAC2_REC         0xc6
#define MR_MAC3_REC         0xc7
#define MR_MACN_REC         0xc8
#define N_MULTIREC          16

typedef struct {
  UINT8  CommonHeaderFormatVersion:4;
  UINT8  Reserved:4;
  UINT8  InternalUseAreaOffset;
  UINT8  ChassisInfoAreaOffset;
  UINT8  BoardAreaOffset;
  UINT8  ProductInfoAreaOffset;
  UINT8  MultiRecordAreaOffset;
  UINT8  Pad;
  UINT8  CommonHeaderChecksum;
} FRU_COMMON_HEADER;

// This data structure is for the library routines.
// It does not represent FRU MREC structure.
typedef struct {
  UINT8   Type;
  UINT8   Format;
  UINT8  *Data;
  UINT8   DataSize;
} MULTIRECORD;

STATIC
UINT8
EFIAPI
FruCalcChecksum (
  IN  CONST UINT8  *Buf,
  IN        UINTN   Size
  );

STATIC
EFI_STATUS
EFIAPI
BaikalFruParseMultirecord (
  IN   UINT8        *Buf,
  IN   UINTN         BufSize,
  OUT  MULTIRECORD  *Multirecord
  );

STATIC
UINTN
EFIAPI
ReadFruStr (
  IN   UINT8  *Buf,
  IN   UINTN   BufOffset,
  OUT  UINT8  *Str,
  OUT  UINTN  *StrLen
  );

STATIC
UINT8
EFIAPI
BcdToAscii (
  IN  UINT8  Bcd
  )
{
  if (Bcd < 0xa) {
    return '0' + Bcd;
  } else if (Bcd == 0xa) {
    return ' ';
  } else if (Bcd == 0xb) {
    return '-';
  } else if (Bcd == 0xc) {
    return '.';
  }
  return 0;
}

STATIC
UINTN
EFIAPI
BcdToStr (
  IN   UINT8  *Buf,
  IN   UINTN   Len,
  OUT  UINT8  *Str
  )
{
  UINTN  Idx;

  for (Idx = 0; Idx < Len; ++Idx) {
    *Str++ = BcdToAscii ((Buf[Idx] >> 0) & 0xf);
    *Str++ = BcdToAscii ((Buf[Idx] >> 4) & 0xf);
  }

  return Len * 2;
}

STATIC
UINT8
EFIAPI
Ascii6ToAscii (
  IN  UINT8  Ascii6
  )
{
  return Ascii6 + 0x20;
}

STATIC
UINTN
EFIAPI
Ascii6ToStr (
  IN   UINT8  *Buf,
  IN   UINTN   Len,
  OUT  UINT8  *Str
  )
{
  UINTN  Idx;

  for (Idx = 0; Idx < Len; Idx += 3) {
    *Str++ = Ascii6ToAscii (Buf[Idx] & 0x3f);
    *Str++ = Ascii6ToAscii ((Buf[Idx] >> 6) | ((Buf[Idx + 1] & 0xf) << 2));
    *Str++ = Ascii6ToAscii ((Buf[Idx + 1] >> 4) | ((Buf[Idx + 2] & 3) << 4));
    *Str++ = Ascii6ToAscii (Buf[Idx + 2] >> 2);
  }

  return Len / 3 * 4;
}

STATIC
UINT8
EFIAPI
FruCalcChecksum (
  IN  CONST UINT8  *Buf,
  IN        UINTN   Size
  )
{
  UINT8  Checksum = 0;

  while (Size--) {
    Checksum += *Buf++;
  }

  return Checksum;
}

STATIC
EFI_STATUS
EFIAPI
FruCommonHeaderCheck (
  IN  CONST UINT8  *Buf,
  IN  CONST UINTN   BufSize
  )
{
  CONST FRU_COMMON_HEADER * CONST  FruCommonHeader = (FRU_COMMON_HEADER *)Buf;

  if (BufSize < sizeof (FRU_COMMON_HEADER)) {
    DEBUG ((EFI_D_ERROR, "%a: invalid size: %d\n", __FUNCTION__, BufSize));
    return EFI_INVALID_PARAMETER;
  }

  if (FruCommonHeader->CommonHeaderFormatVersion != FRU_COMMON_HEADER_FORMAT_VERSION) {
    DEBUG ((EFI_D_ERROR, "%a: invalid Common Header Format Version: %d != %d\n",
            __FUNCTION__,
            FruCommonHeader->CommonHeaderFormatVersion,
            FRU_COMMON_HEADER_FORMAT_VERSION
            ));

    return EFI_INVALID_PARAMETER;
  }

  if (FruCalcChecksum ((UINT8 *)FruCommonHeader, sizeof (FRU_COMMON_HEADER)) != 0) {
    DEBUG ((EFI_D_ERROR, "%a: invalid Common Header Checksum\n", __FUNCTION__));
    return EFI_CRC_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BaikalFruGetMacAddr (
  IN   UINT8            *Buf,
  IN   UINTN             BufSize,
  IN   UINTN             MacAddrIdx,
  OUT  EFI_MAC_ADDRESS  *MacAddr
  )
{
  MULTIRECORD  Mrec;
  UINTN        MrecAreaOffset;
  UINTN        MrecNum;
  UINTN        MrecType;
  EFI_STATUS   Status;

  Status = FruCommonHeaderCheck (Buf, BufSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (MacAddrIdx == 0) {
    MrecType = MR_MAC_REC;
  } else if (MacAddrIdx == 1) {
    MrecType = MR_MAC2_REC;
  } else if (MacAddrIdx == 2) {
    MrecType = MR_MAC3_REC;
  } else {
    MrecType = MR_MACN_REC;
  }

  MrecAreaOffset = Buf[5] * 8;
  if (MrecAreaOffset == 0 || MrecAreaOffset >= BufSize) {
    DEBUG ((EFI_D_ERROR, "FRU: invalid multirecord area offset: %d\n", MrecAreaOffset));
    return EFI_INVALID_PARAMETER;
  }

  for (MrecNum = 0; MrecNum < N_MULTIREC; ++MrecNum) {
    if (BaikalFruParseMultirecord (Buf + MrecAreaOffset, BufSize - MrecAreaOffset, &Mrec) == EFI_SUCCESS) {
      UINT8  Checksum = (UINT8)(FruCalcChecksum (Mrec.Data, Mrec.DataSize) + Buf[MrecAreaOffset + 3]);

      if (Checksum) {
        DEBUG ((EFI_D_ERROR, "FRU: invalid multirecord data checksum\n"));
      } else if (Mrec.Type == MrecType) {
        UINTN  Idx;
        UINT8  *MacData = Mrec.Data;
	if (MacAddrIdx > 2) {
           if ((MacAddrIdx - 2) > MacData[0]) {
             break;
	   } else {
             MacData = MacData + 1 + 6 * (MacAddrIdx - 3);
	   }
        }
        for (Idx = 0; Idx < 6; ++Idx) {
          MacAddr->Addr[Idx] = MacData[Idx];
        }

        return EFI_SUCCESS;
      }

      if (Mrec.Format & 0x80) {
        break;
      }

      MrecAreaOffset += 5 + Mrec.DataSize;
    } else {
      break;
    }
  }

  DEBUG ((EFI_D_ERROR, "FRU: multirecord not found\n"));
  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
EFIAPI
BaikalFruParseBoardArea (
  IN   UINT8  *Buf,
  IN   UINTN   BufSize,
  OUT  UINT8  *BoardMfg   OPTIONAL,
  OUT  UINT8  *BoardName  OPTIONAL,
  OUT  UINT8  *BoardSn    OPTIONAL
  )
{
  UINTN       BoardAreaSize;
  UINTN       BoardAreaOffset;
  EFI_STATUS  Status;
  UINT8       Str[FRU_STR_MAX];
  UINTN       StrLen;
  UINTN       StrOffset;

  Status = FruCommonHeaderCheck (Buf, BufSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  BoardAreaOffset = Buf[3] * 8;
  if (BoardAreaOffset == 0 || BoardAreaOffset >= BufSize) {
    DEBUG ((EFI_D_ERROR, "FRU: invalid board area offset: %d\n", BoardAreaOffset));
    return EFI_INVALID_PARAMETER;
  }

  if (Buf[BoardAreaOffset] != BOARD_AREA_VERSION) {
    DEBUG ((EFI_D_ERROR, "FRU: invalid board area version: %d != %d\n", Buf[BoardAreaOffset], BOARD_AREA_VERSION));
    return EFI_INVALID_PARAMETER;
  }

  BoardAreaSize = Buf[BoardAreaOffset + 1] * 8;
  if (BoardAreaSize > BufSize) {
    DEBUG ((EFI_D_ERROR, "FRU: invalid board area size: %d\n", BoardAreaSize));
    return EFI_INVALID_PARAMETER;
  }

  if (FruCalcChecksum (&Buf[BoardAreaOffset], BoardAreaSize) != 0) {
    DEBUG ((EFI_D_ERROR, "FRU: invalid board area checksum\n"));
    return EFI_CRC_ERROR;
  }

  memset (Str, 0, sizeof (Str));
  StrOffset = 5;

  StrOffset = ReadFruStr (&Buf[BoardAreaOffset], StrOffset, Str, &StrLen);
  if (BoardMfg) {
    memcpy (BoardMfg, Str, StrLen);
  }

  StrOffset = ReadFruStr (&Buf[BoardAreaOffset], StrOffset, Str, &StrLen);
  if (BoardName) {
    memcpy (BoardName, Str, StrLen);
  }

  StrOffset = ReadFruStr (&Buf[BoardAreaOffset], StrOffset, Str, &StrLen);
  if (BoardSn) {
    memcpy (BoardSn, Str, StrLen);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
BaikalFruParseMultirecord (
  IN   UINT8        *Buf,
  IN   UINTN         BufSize,
  OUT  MULTIRECORD  *Mrec
  )
{
  UINT8  MrecFormat;
  UINT8  MrecDataSize;

  if (BufSize < 5) {
    DEBUG ((EFI_D_ERROR, "FRU: buffer size is smaller than MREC header size: %d\n", BufSize));
    return EFI_INVALID_PARAMETER;
  }

  if (FruCalcChecksum (Buf, 5) != 0) {
    DEBUG ((EFI_D_ERROR, "FRU: invalid MREC header checksum\n"));
    return EFI_CRC_ERROR;
  }

  MrecFormat = Buf[1] & 0x07;
  if (MrecFormat != 0x02) {
    DEBUG ((EFI_D_ERROR, "FRU: unknown MREC format: %d\n", MrecFormat));
    return EFI_INVALID_PARAMETER;
  }

  MrecDataSize = Buf[2];
  if (BufSize < 5 + MrecDataSize) {
    DEBUG ((EFI_D_ERROR, "FRU: buffer size is smaller than MREC header+data size\n"));
    return EFI_INVALID_PARAMETER;
  }

  Mrec->Type     = Buf[0];
  Mrec->Format   = Buf[1];
  Mrec->Data     = &Buf[5];
  Mrec->DataSize = MrecDataSize;
  return EFI_SUCCESS;
}

STATIC
UINTN
EFIAPI
ReadFruStr (
  IN   UINT8  *Buf,
  IN   UINTN   BufOffset,
  OUT  UINT8  *Str,
  OUT  UINTN  *StrLen
  )
{
  UINTN  BufLen;
  UINTN  RetLen;
  UINT8  Type;

  BufLen = Buf[BufOffset] & 0x3f;
  Type   = Buf[BufOffset] >> 6;
  RetLen = BufLen < FRU_STR_MAX - 1 ? BufLen : FRU_STR_MAX - 1;

  if (Str) { // If Str is NULL then skip to next record
    if (Type == 0 || Type == 3) { // binary or ASCII/Unicode
      memcpy (Str, Buf + BufOffset + 1, RetLen);
      *StrLen = RetLen;
    } else if (Type == 1) { // BCD-plus
      *StrLen = BcdToStr (Buf + BufOffset + 1, RetLen, Str);
    } else if (Type == 2) { // ASCII-6
      *StrLen = Ascii6ToStr (Buf + BufOffset + 1, RetLen, Str);
    }
  }

  return BufOffset + BufLen + 1;
}
