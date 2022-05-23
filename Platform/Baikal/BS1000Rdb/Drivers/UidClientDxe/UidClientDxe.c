/** @file
  Copyright (c) 2021 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/CrcLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SpdClient.h>
#include <Protocol/UidClient.h>

STATIC
UINT32
EFIAPI
UidClientGet32 (
  VOID
  );

STATIC SPD_CLIENT_PROTOCOL  *SpdClient;
STATIC UINT32                Uid32;

EFI_STATUS
EFIAPI
UidClientDxeInitialize (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN       DimmIdx;
  EFI_STATUS  Status;

  STATIC UID_CLIENT_PROTOCOL  mUidClientProtocol = {
    UidClientGet32
  };

  Status = gBS->LocateProtocol (&gSpdClientProtocolGuid, NULL, (VOID **) &SpdClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to locate SpdClientProtocol, Status: %r\n",
      __FUNCTION__,
      Status
      ));
    return Status;
  }

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

  for (DimmIdx = 0; ; ++DimmIdx) {
    CONST UINT8  *SpdData = SpdClient->GetData (DimmIdx);
    if (SpdData == NULL) {
      break;
    }

    if (Crc16 (SpdData, 126, 0) == ((SpdData[127] << 8) | SpdData[126])) {
      CONST UINTN  BytesUsed = SpdData[0] & 0xF;
      if (BytesUsed >= 3) { // Find DIMM with manufacturer's specific data
        gBS->CalculateCrc32 ((VOID *) SpdData, MIN (BytesUsed * 128, SpdClient->GetMaxSize()), &Uid32);
        return EFI_SUCCESS;
      } else if (Uid32 == 0) {
        // Use general data of any DIMM if there is no DIMM with specific data
        gBS->CalculateCrc32 ((VOID *) SpdData, MIN (BytesUsed * 128, SpdClient->GetMaxSize()), &Uid32);
      }
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
