/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaikalSmbiosLib.h>
#include <Library/BaseMemoryLib.h>

STATIC
UINT64
SmbiosGetSpdCapacity (
  IN  CONST UINT8 * CONST  SpdBuf
  )
{
  UINT64  SdramCapacityPerDie;
  UINT64  Total;

  if (!SpdBuf) {
    return 0;
  }

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

  CONST CHAR8  DieCount                = ((SpdBuf[6] >> 4) & 0x7) + 1;
  CONST CHAR8  LogicalRanksPerDimm     = ((SpdBuf[12] >> 3) & 0x7) + 1;
  CONST INT16  PrimaryBusWidth         = 8 << (SpdBuf[13] & 0x7);
  CONST CHAR8  PrimarySdramPackageType = (SpdBuf[6] >> 7) & 0x1;
  CONST CHAR8  SdramDeviceWidth        = 4 << (SpdBuf[12] & 0x3);
  CONST CHAR8  SignalLoading           = SpdBuf[6] & 0x3;

  if (SdramDeviceWidth > 32 || PrimaryBusWidth > 64) {
    return 0;
  }

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
SmbiosSetDdrInfo (
  IN      CONST UINT8 * CONST             SpdBuf,
  IN      CONST UINTN                     Size,
  IN OUT  BAIKAL_SMBIOS_DDR_INFO * CONST  Info,
  IN      CONST UINT32 * CONST            Serial,
  IN      CONST UINT8 * CONST             PartNumber
  )
{
  Info->Size = SmbiosGetSpdCapacity (SpdBuf);
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

  if (Size >= 349) {
    Info->ManufacturerId = *(UINT16 *) &SpdBuf[320];

    if (Serial) {
      Info->SerialNumber = *Serial;
    } else {
      Info->SerialNumber = *(UINT32 *) &SpdBuf[325];
    }

    if (PartNumber) {
      CopyMem (&Info->PartNumber, PartNumber, sizeof (Info->PartNumber));
    } else {
      CopyMem (&Info->PartNumber, &SpdBuf[329], sizeof (Info->PartNumber));
    }
  } else {
    Info->ManufacturerId = 0;
    Info->SerialNumber = 0;
    ZeroMem (&Info->PartNumber, sizeof (Info->PartNumber));
  }

  return 0;
}
