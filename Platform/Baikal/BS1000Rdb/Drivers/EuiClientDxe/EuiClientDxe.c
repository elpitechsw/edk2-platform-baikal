/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/EuiClient.h>
#include <Protocol/UidClient.h>

STATIC
VOID
EFIAPI
EuiClientGetEui48 (
  IN      CONST EFI_PHYSICAL_ADDRESS     Base,
  IN      CONST UINTN                    DevIdx,
  IN OUT  EFI_MAC_ADDRESS * CONST        MacAddr
  );

STATIC
BOOLEAN
EFIAPI
EuiClientIsValidEui48 (
  IN      CONST EFI_MAC_ADDRESS * CONST  MacAddr
  );

STATIC UID_CLIENT_PROTOCOL  *UidClient;

EFI_STATUS
EFIAPI
EuiClientDxeInitialize (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  STATIC EUI_CLIENT_PROTOCOL  mEuiClientProtocol = {
    EuiClientGetEui48,
    EuiClientIsValidEui48
  };

  Status = gBS->LocateProtocol (&gUidClientProtocolGuid, NULL, (VOID **) &UidClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to locate UidClientProtocol, Status: %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEuiClientProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mEuiClientProtocol
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to install EuiClientProtocol, Status: %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  return EFI_SUCCESS;
}

STATIC
VOID
EFIAPI
EuiClientGetEui48 (
  IN      CONST EFI_PHYSICAL_ADDRESS  Base,
  IN      CONST UINTN                 DevIdx,
  IN OUT  EFI_MAC_ADDRESS * CONST     MacAddr
  )
{
  ASSERT (MacAddr != NULL);

  if (!EuiClientIsValidEui48 (MacAddr)) {
    MacAddr->Addr[0] = 0x4C;
    MacAddr->Addr[1] = 0xA5;
    MacAddr->Addr[2] = 0x15;
    MacAddr->Addr[3] = (UidClient->Get32 () >> 16) & 0xFF;
    MacAddr->Addr[4] = (UidClient->Get32 () >>  8) & 0xFF;
    MacAddr->Addr[5] = (UidClient->Get32 () & 0xF8) | (DevIdx & 0x7);
  }
}

STATIC
BOOLEAN
EFIAPI
EuiClientIsValidEui48 (
  IN  CONST EFI_MAC_ADDRESS * CONST  MacAddr
  )
{
  // Check if it is zero address
  if ((MacAddr->Addr[0] |
       MacAddr->Addr[1] |
       MacAddr->Addr[2] |
       MacAddr->Addr[3] |
       MacAddr->Addr[4] |
       MacAddr->Addr[5]) == 0) {
    return FALSE;
  }

  // Check if it is a multicast address
  if (MacAddr->Addr[0] & 0x01) {
    return FALSE;
  }

  return TRUE;
}
