/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef FRU_CLIENT_H_
#define FRU_CLIENT_H_

#define FRU_CLIENT_PROTOCOL_GUID { \
  0xA3296f6C, 0x9B04, 0x11ED, { 0xB1, 0x86, 0x2E, 0x09, 0x29, 0x74, 0x78, 0x49 }}

typedef struct _FRU_CLIENT_PROTOCOL FRU_CLIENT_PROTOCOL;

#define FRU_TYPLENSTR_MAX_SIZE  128

#define FRU_POWERPOLICY_OFF     0
#define FRU_POWERPOLICY_KEEP    1
#define FRU_POWERPOLICY_ON      2

typedef
UINTN
(EFIAPI *FRU_CLIENT_GET_BOARD_MFGDATETIME) (
  VOID
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_BOARD_MANUFACTURER) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_BOARDNAME) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_BOARD_SERIALNUMBER) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_BOARD_PARTNUMBER) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_BOARD_FILEID) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_PRODUCT_MANUFACTURER) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_PRODUCT_NAME) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_PRODUCT_PARTNUMBER) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_PRODUCT_VERSION) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_PRODUCT_SERIALNUMBER) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_PRODUCT_ASSETTAG) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_READ_PRODUCT_FILEID) (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

typedef
EFI_STATUS
(EFIAPI *FRU_CLIENT_GET_MULTIRECORD_MACADDR) (
  IN   CONST UINTN              MacAddrIdx,
  OUT  EFI_MAC_ADDRESS * CONST  MacAddr
  );

typedef
UINTN
(EFIAPI *FRU_CLIENT_GET_BOARD_POWERPOLICY) (
  VOID
  );

typedef
EFI_STATUS
(EFIAPI *FRU_CLIENT_SET_BOARD_POWERPOLICY) (
  IN UINTN Policy
  );

struct _FRU_CLIENT_PROTOCOL {
  FRU_CLIENT_GET_BOARD_MFGDATETIME      GetBoardMfgDateTime;
  FRU_CLIENT_READ_BOARD_MANUFACTURER    ReadBoardManufacturer;
  FRU_CLIENT_READ_BOARDNAME             ReadBoardName;
  FRU_CLIENT_READ_BOARD_SERIALNUMBER    ReadBoardSerialNumber;
  FRU_CLIENT_READ_BOARD_PARTNUMBER      ReadBoardPartNumber;
  FRU_CLIENT_READ_BOARD_FILEID          ReadBoardFileId;
  FRU_CLIENT_READ_PRODUCT_MANUFACTURER  ReadProductManufacturer;
  FRU_CLIENT_READ_PRODUCT_NAME          ReadProductName;
  FRU_CLIENT_READ_PRODUCT_PARTNUMBER    ReadProductPartNumber;
  FRU_CLIENT_READ_PRODUCT_VERSION       ReadProductVersion;
  FRU_CLIENT_READ_PRODUCT_SERIALNUMBER  ReadProductSerialNumber;
  FRU_CLIENT_READ_PRODUCT_ASSETTAG      ReadProductAssetTag;
  FRU_CLIENT_READ_PRODUCT_FILEID        ReadProductFileId;
  FRU_CLIENT_GET_MULTIRECORD_MACADDR    GetMultirecordMacAddr;
  FRU_CLIENT_GET_BOARD_POWERPOLICY      GetBoardPowerPolicy;
  FRU_CLIENT_SET_BOARD_POWERPOLICY      SetBoardPowerPolicy;
};

extern EFI_GUID gFdtClientProtocolGuid;

#endif // FRU_CLIENT_H_
