/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef UID_CLIENT_H_
#define UID_CLIENT_H_

#define UID_CLIENT_PROTOCOL_GUID { \
  0x304A2CC1, 0x1004, 0x4AB2, { 0xB0, 0x90, 0x7D, 0x9C, 0xB4, 0xD9, 0x0A, 0x7F }}

typedef struct _UID_CLIENT_PROTOCOL UID_CLIENT_PROTOCOL;

typedef
UINT32
(EFIAPI *UID_CLIENT_GET32) (
  VOID
  );

struct _UID_CLIENT_PROTOCOL {
  UID_CLIENT_GET32  Get32;
};

extern EFI_GUID gUidClientProtocolGuid;

#endif // UID_CLIENT_H_
