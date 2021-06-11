/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaikalI2cLib.h>
#include <Library/BaikalSmbusLib.h>
#include <Library/BaikalSpdLib.h>
#include <Library/BaseMemoryLib.h>

CONST UINT8 SpdDdrAddr[BAIKAL_SPD_DDR_ADDR_LENGTH] = {0x50, 0x51, 0x52, 0x53};

STATIC
INTN
SpdGetBufInternal (
  IN   CONST UINTN        IsSmbus,
  IN   CONST UINTN        TargetAddr,
  IN   CONST VOID *CONST  TxBuf,
  IN   CONST UINTN        TxBufSize,
  OUT  VOID *CONST        RxBuf,
  IN   CONST UINTN        RxBufSize
  )
{
  if (IsSmbus) {
    return SmbusTxRx (0, TargetAddr, TxBuf, TxBufSize, RxBuf, RxBufSize);
  } else {
    return I2cTxRx (1, TargetAddr, TxBuf, TxBufSize, RxBuf, RxBufSize);
  }
}

INTN
SpdGetSize (
  IN  CONST UINTN  IsSmbus,
  IN  CONST UINTN  TargetAddr
  )
{
  UINT8        Buf;
  CONST UINT8  StartAddr = 0;

  if (SpdGetBufInternal (IsSmbus, TargetAddr, &StartAddr, sizeof (StartAddr), &Buf, 1) != 1) {
    return 0;
  }

  switch (Buf & 0x07) {
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

VOID
SpdSwitchPage (
  IN  CONST UINTN  IsSmbus,
  IN  CONST UINTN  Page
  )
{
  CONST UINT8  TxBuf = 0;
  UINT8        RxBuf = 0;
  SpdGetBufInternal (IsSmbus, Page ? 0x37 : 0x36, &TxBuf, 1, &RxBuf, 1);
}

INTN
SpdGetBuf (
  IN   CONST UINTN  IsSmbus,
  IN   CONST UINTN  TargetAddr,
  OUT  VOID *CONST  RxBuf,
  IN   CONST UINTN  RxBufSize
  )
{
  CONST UINT8  StartAddr = 0;
  return SpdGetBufInternal (IsSmbus, TargetAddr, &StartAddr, sizeof (StartAddr), RxBuf, RxBufSize);
}

STATIC
UINT16
SpdGetCrc16 (
  IN  CONST UINT8 *CONST  SpdBuf
  )
{
  return SpdBuf[0] | (SpdBuf[1] << 8);
}

STATIC
UINT16
SpdCrc16 (
  IN  CONST UINT8  *SpdBuf,
  IN  UINTN         Size
  )
{
  UINT16        Crc  = 0;
  UINTN         Idx;

  while (Size--) {
    Crc ^= *SpdBuf++ << 8;

    for (Idx = 0; Idx < 8; ++Idx) {
      if (Crc & 0x8000) {
        Crc = Crc << 1 ^ 0x1021;
      } else {
        Crc <<= 1;
      }
    }
  }

  return Crc & 0xFFFF;
}

INTN
SpdIsValid (
  IN  CONST VOID *CONST  Buf,
  IN  CONST UINTN        Size
  )
{
  INTN                Result = 0;
  CONST UINT8 *CONST  SpdBuf = Buf;

  if (Size < 128) {
    return 0;
  }

  if (Size >= 128) {
    Result = SpdGetCrc16 (SpdBuf + 126) == SpdCrc16 (SpdBuf, 126);
  }

  if (Size >= 256) {
    Result &= SpdGetCrc16 (SpdBuf + 254) == SpdCrc16 (SpdBuf + 128, 126);
  }

  if (Size >= 320) {
    Result &= SpdGetCrc16 (SpdBuf + 318) == SpdCrc16 (SpdBuf + 256, 62);
  }

  return Result;
}

STATIC
UINT64
SpdGetCapacity (
  IN  CONST UINT8 *CONST  SpdBuf
  )
{
  UINT64 SdramCapacityPerDie;

  // SDRAM capacity in MiB
  switch (SpdBuf[4] & 0xF) {
  case 0:
    SdramCapacityPerDie = 256;
    break;
  case 1:
    SdramCapacityPerDie = 512;
    break;
  case 2:
    SdramCapacityPerDie = 1024;
    break;
  case 3:
    SdramCapacityPerDie = 2 * 1024;
    break;
  case 4:
    SdramCapacityPerDie = 4 * 1024;
    break;
  case 5:
    SdramCapacityPerDie = 8 * 1024;
    break;
  case 6:
    SdramCapacityPerDie = 16 * 1024;
    break;
  case 7:
    SdramCapacityPerDie = 32 * 1024;
    break;
  case 8:
    SdramCapacityPerDie = 12 * 1024;
    break;
  case 9:
    SdramCapacityPerDie = 24 * 1024;
    break;
  default:
    return 0;
  }

  SdramCapacityPerDie *= 1024 * 1024; // SDRAM capacity in bytes

  CONST INT16 PrimaryBusWidth         = 8 << (SpdBuf[13] & 0x7);
  CONST CHAR8 SdramDeviceWidth        = 4 << (SpdBuf[12] & 0x3);
  CONST CHAR8 LogicalRanksPerDimm     = ((SpdBuf[12] >> 3) & 0x7) + 1;
  CONST CHAR8 PrimarySdramPackageType = (SpdBuf[6] >> 7) & 0x1;
  CONST CHAR8 DieCount                = ((SpdBuf[6] >> 4) & 0x7) + 1;
  CONST CHAR8 SignalLoading           = SpdBuf[6] & 0x3;

  if (SdramDeviceWidth > 32 || PrimaryBusWidth > 64) {
    return 0;
  }

  UINT64 Total;
  Total  = SdramCapacityPerDie;
  Total /= 8;
  Total *= PrimaryBusWidth;
  Total /= SdramDeviceWidth;

  if (PrimarySdramPackageType == 0) {
    if (SignalLoading == 0) {
      Total *= LogicalRanksPerDimm;
    } else {
      return 0;
    }
  } else {
    if (SignalLoading == 1) {
      Total *= LogicalRanksPerDimm;
    } else if (SignalLoading == 2) {
      Total *= LogicalRanksPerDimm * DieCount;
    } else {
      return 0;
    }
  }

  return Total;
}

INTN
SpdSetSmbiosInfo (
  IN      CONST VOID *CONST              Buf,
  IN      CONST UINT16                   Size,
  IN      CONST UINT8                    IsHybrid,
  IN OUT  BAIKAL_SPD_SMBIOS_INFO *CONST  Info
  )
{
  CONST UINT8 *CONST  SpdBuf = Buf;

  Info->Size = SpdGetCapacity (SpdBuf);
  if (!Info->Size) {
    return -1;
  }
  Info->Rank = ((SpdBuf[12] >> 3) & 0x7) + 1;

  switch (SpdBuf[18]) {
  case 5:
    Info->Speed = 3200;
    break;
  case 6:
    Info->Speed = 2666;
    break;
  case 7:
    Info->Speed = 2400;
    break;
  case 8:
    Info->Speed = 2133;
    break;
  case 9:
    Info->Speed = 1866;
    break;
  case 10:
    Info->Speed = 1600;
    break;
  default:
    Info->Speed = 0;
  }

  if ((SpdBuf[11] & 0x3) == 1 || (SpdBuf[11] & 0x3) == 3)
    Info->Voltage = 1200;
  else
    Info->Voltage = 0;

  Info->DataWidth = 8 << (SpdBuf[13] & 0x7);
  Info->ExtensionWidth = (SpdBuf[13] & 0x18) ? 8 : 0;

  if (Size >= 199 && IsHybrid) {
    Info->ProductId = *(UINT16 *) &SpdBuf[192];
    Info->SubsystemManufacturerId = *(UINT16 *) &SpdBuf[194];
    Info->SubsystemProductId = *(UINT16 *) &SpdBuf[196];
  } else {
    Info->ProductId = 0;
    Info->SubsystemManufacturerId = 0;
    Info->SubsystemProductId = 0;
  }

  if (Size >= 349) {
    Info->ManufacturerId = *(UINT16 *) &SpdBuf[320];
    Info->SerialNumber = *(UINT32 *) &SpdBuf[325];
    CopyMem (&Info->PartNumber, &SpdBuf[329], sizeof (Info->PartNumber));
  } else {
    Info->ManufacturerId = 0;
    Info->SerialNumber = 0;
    ZeroMem (&Info->PartNumber, sizeof (Info->PartNumber));
  }

  return 0;
}
