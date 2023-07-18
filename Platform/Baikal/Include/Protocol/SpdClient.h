/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef SPD_CLIENT_H_
#define SPD_CLIENT_H_

#define SPD_CLIENT_PROTOCOL_GUID { \
  0xBD3E356A, 0xC664, 0x473C, { 0x97, 0xAB, 0x6C, 0x09, 0xD8, 0x9C, 0xF4, 0xC5 }}

typedef struct _SPD_CLIENT_PROTOCOL SPD_CLIENT_PROTOCOL;

typedef
CONST VOID *
(EFIAPI *SPD_CLIENT_GET_DATA) (
  IN  CONST UINTN  DimmIdx
  );

typedef
UINTN
(EFIAPI *SPD_CLIENT_GET_MAXSIZE) (
  VOID
  );

struct _SPD_CLIENT_PROTOCOL {
  SPD_CLIENT_GET_DATA     GetData;
  SPD_CLIENT_GET_MAXSIZE  GetMaxSize;
};

extern EFI_GUID gSpdClientProtocolGuid;

#endif // SPD_CLIENT_H_
