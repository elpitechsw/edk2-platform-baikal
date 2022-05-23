/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef EUI_CLIENT_H_
#define EUI_CLIENT_H_

#define EUI_CLIENT_PROTOCOL_GUID { \
  0xD49717DA, 0x6100, 0x4964, { 0x95, 0x1C, 0xF9, 0x8B, 0x18, 0x4D, 0x92, 0xD4 }}

typedef struct _EUI_CLIENT_PROTOCOL EUI_CLIENT_PROTOCOL;

typedef
VOID
(EFIAPI *EUI_CLIENT_GET_EUI48) (
  IN      CONST EFI_PHYSICAL_ADDRESS     Base,
  IN      CONST UINTN                    DevIdx,
  IN OUT  EFI_MAC_ADDRESS * CONST        MacAddr
  );

typedef
BOOLEAN
(EFIAPI *EUI_CLIENT_IS_VALID_EUI48) (
  IN      CONST EFI_MAC_ADDRESS * CONST  MacAddr
  );

struct _EUI_CLIENT_PROTOCOL {
  EUI_CLIENT_GET_EUI48       GetEui48;
  EUI_CLIENT_IS_VALID_EUI48  IsValidEui48;
};

extern EFI_GUID gEuiClientProtocolGuid;

#endif // EUI_CLIENT_H_
