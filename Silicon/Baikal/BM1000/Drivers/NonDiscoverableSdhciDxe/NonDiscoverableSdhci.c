/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>

EFI_STATUS
EFIAPI
NonDiscoverableSdhciEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  FDT_CLIENT_PROTOCOL  *FdtClient;
  INT32                 Node = 0;
  CONST VOID           *Prop;
  UINT32                PropSize;
  EFI_STATUS            Status;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);

  while (TRUE) {
    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,dwcmshc-sdhci", Node, &Node);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == 16) {
      CONST EFI_PHYSICAL_ADDRESS  SdhciBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + 0));
      CONST UINTN                 SdhciSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + 1));

      Status = RegisterNonDiscoverableMmioDevice (
                 NonDiscoverableDeviceTypeSdhci,
                 NonDiscoverableDeviceDmaTypeCoherent,
                 NULL,
                 NULL,
                 1,
                 SdhciBase,
                 SdhciSize
                 );

      if (EFI_ERROR (Status)) {
        DEBUG ((
          EFI_D_ERROR,
          "%a: unable to register @ 0x%lx, Status: %r\n",
          __func__,
          SdhciBase,
          Status
          ));
      }
    }
  }

  return EFI_SUCCESS;
}
