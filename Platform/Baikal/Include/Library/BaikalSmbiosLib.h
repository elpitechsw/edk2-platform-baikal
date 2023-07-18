/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_SMBIOS_LIB_H_
#define BAIKAL_SMBIOS_LIB_H_

typedef struct {
  UINT32  SerialNumber;
  UINT8   PartNumber[20];
  UINT64  Size;
  UINT8   Rank;
  UINT16  Speed;
  UINT16  Voltage;
  UINT16  DataWidth;
  UINT8   ExtensionWidth;
  UINT16  ManufacturerId;
} BAIKAL_SMBIOS_DDR_INFO;

INTN
SmbiosSetDdrInfo (
  IN      CONST UINT8 * CONST             SpdBuf,
  IN      CONST UINTN                     Size,
  IN OUT  BAIKAL_SMBIOS_DDR_INFO * CONST  Info,
  IN      CONST UINT32 * CONST            Serial,
  IN      CONST UINT8 * CONST             PartNumber
  );

#endif // BAIKAL_SMBIOS_LIB_H_
