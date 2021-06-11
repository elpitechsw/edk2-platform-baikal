/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_SPD_LIB_H_
#define BAIKAL_SPD_LIB_H_

#define BAIKAL_SPD_DDR_ADDR_LENGTH 4

extern CONST UINT8 SpdDdrAddr[BAIKAL_SPD_DDR_ADDR_LENGTH];

INTN
SpdGetSize (
  IN  CONST UINTN  IsSmbus,
  IN  CONST UINTN  TargetAddr
  );

VOID
SpdSwitchPage (
  IN  CONST UINTN  IsSmbus,
  IN  CONST UINTN  Page
  );

INTN
SpdGetBuf (
  IN   CONST UINTN  IsSmbus,
  IN   CONST UINTN  TargetAddr,
  OUT  VOID *CONST  RxBuf,
  IN   CONST UINTN  RxBufSize
  );

INTN
SpdIsValid (
  IN  CONST VOID *CONST  Buf,
  IN  CONST UINTN        Size
  );

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
  UINT16  ProductId;
  UINT16  SubsystemManufacturerId;
  UINT16  SubsystemProductId;
} BAIKAL_SPD_SMBIOS_INFO;

INTN
SpdSetSmbiosInfo (
  IN      CONST VOID *CONST              Buf,
  IN      CONST UINT16                   Size,
  IN      CONST UINT8                    IsHybrid,
  IN OUT  BAIKAL_SPD_SMBIOS_INFO *CONST  Info
  );

#endif // BAIKAL_SPD_LIB_H_
