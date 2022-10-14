/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaikalSpdLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/UidClient.h>

STATIC UINT32  Uid32;

STATIC
UINT32
EFIAPI
UidClientGet32 (
  VOID
  );

EFI_STATUS
EFIAPI
UidClientDxeInitialize (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN       SpdPort;
  EFI_STATUS  Status;

  STATIC UID_CLIENT_PROTOCOL  mUidClientProtocol = {
    UidClientGet32
  };

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gUidClientProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mUidClientProtocol
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to install UidClientProtocol, Status: %r\n",
      __FUNCTION__,
      Status
      ));
    return Status;
  }

  Uid32 = 0;

  for (SpdPort = 0; SpdPort < BAIKAL_SPD_PORT_COUNT; ++SpdPort) {
    CONST UINT8  *Spd = SpdGetBuf (SpdPort);
    UINTN         SpdSize = SpdGetSize (SpdPort);

    if (Spd && SpdSize) {
      gBS->CalculateCrc32 ((VOID *) Spd, SpdSize, &Uid32);
      break;
    }
  }

  return EFI_SUCCESS;
}

STATIC
UINT32
EFIAPI
UidClientGet32 (
  VOID
  )
{
  return Uid32;
}
