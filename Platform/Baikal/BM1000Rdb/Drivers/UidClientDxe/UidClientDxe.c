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
  UINTN       SpdAddr;
  UINTN       SpdSize;
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

  for (SpdAddr = 0x50, SpdSize = 0; SpdAddr < 0x54; ++SpdAddr) {
    SpdSize = SpdGetSize (SpdAddr);

    if (SpdSize) {
      UINT8  *Spd;

      Status = gBS->AllocatePool (EfiBootServicesData, SpdSize, (VOID **) &Spd);
      if (!EFI_ERROR (Status)) {
        SpdGetBuf (SpdAddr, Spd, SpdSize);
        gBS->CalculateCrc32 (Spd, SpdSize, &Uid32);
        gBS->FreePool (Spd);
      }

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
