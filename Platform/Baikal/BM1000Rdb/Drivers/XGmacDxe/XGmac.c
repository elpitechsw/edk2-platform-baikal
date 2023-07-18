/** @file
  Copyright (c) 2022 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/EuiClient.h>
#include <Protocol/FdtClient.h>

EFI_STATUS
EFIAPI
XGmacDxeDriverEntry (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN                 DevIdx = 0;
  EUI_CLIENT_PROTOCOL  *EuiClient;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  INT32                 Node = 0;
  EFI_STATUS            Status;
  CONST VOID           *Prop;
  UINT32                PropSize;
  EFI_PHYSICAL_ADDRESS  XGmacRegs;

  Status = gBS->LocateProtocol (&gEuiClientProtocolGuid, NULL, (VOID **) &EuiClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate EuiClientProtocol, Status: %r\n", __func__, Status));
    return Status;
  }

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FdtClientProtocol, Status: %r\n", __func__, Status));
    return Status;
  }

  for (;;) {
    UINTN            Idx;
    EFI_MAC_ADDRESS  MacAddr;

    if (FdtClient->FindNextCompatibleNode (FdtClient, "amd,xgbe-seattle-v1a", Node, &Node) != EFI_SUCCESS) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
      continue;
    }

    Status = FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize);
    if (Status == EFI_SUCCESS && PropSize > 0 && (PropSize % (2 * sizeof (UINT64))) == 0) {
      XGmacRegs = SwapBytes64 (((CONST UINT64 *) Prop)[0]);
    } else {
      continue;
    }

    Status = FdtClient->GetNodeProperty (FdtClient, Node, "mac-address", &Prop, &PropSize);
    if (Status == EFI_SUCCESS) {
      if (PropSize == 6) {
        for (Idx = 0; Idx < PropSize; ++Idx) {
          MacAddr.Addr[Idx] = ((CONST UINT8 *) Prop)[Idx];
        }

        Status = EuiClient->IsValidEui48 (&MacAddr) ? EFI_SUCCESS : EFI_INVALID_PARAMETER;
      } else {
        Status = EFI_INVALID_PARAMETER;
      }
    }

    if (EFI_ERROR (Status)) {
      Status = FdtClient->GetNodeProperty (FdtClient, Node, "local-mac-address", &Prop, &PropSize);
      if (Status == EFI_SUCCESS) {
        if (PropSize == 6) {
          for (Idx = 0; Idx < PropSize; ++Idx) {
            MacAddr.Addr[Idx] = ((CONST UINT8 *) Prop)[Idx];
          }

          Status = EuiClient->IsValidEui48 (&MacAddr) ? EFI_SUCCESS : EFI_INVALID_PARAMETER;
        } else {
          Status = EFI_INVALID_PARAMETER;
        }
      }
    }

    if (EFI_ERROR (Status)) {
      UINT64  MacAddrRegVal;

      MacAddrRegVal   = MmioRead32 (XGmacRegs + 0x300) & 0xFFFF;
      MacAddrRegVal <<= 32;
      MacAddrRegVal  |= MmioRead32 (XGmacRegs + 0x304);

      MacAddr.Addr[0] = (MacAddrRegVal >>  0) & 0xFF;
      MacAddr.Addr[1] = (MacAddrRegVal >>  8) & 0xFF;
      MacAddr.Addr[2] = (MacAddrRegVal >> 16) & 0xFF;
      MacAddr.Addr[3] = (MacAddrRegVal >> 24) & 0xFF;
      MacAddr.Addr[4] = (MacAddrRegVal >> 32) & 0xFF;
      MacAddr.Addr[5] = (MacAddrRegVal >> 40) & 0xFF;

      EuiClient->GetEui48 (XGmacRegs, DevIdx, &MacAddr);
      Status = FdtClient->SetNodeProperty (FdtClient, Node, "local-mac-address", MacAddr.Addr, 6);
      if (EFI_ERROR (Status)) {
        DEBUG ((
          EFI_D_ERROR,
          "%a: unable to set 'local-mac-address' FDT node property, Status: %r\n",
          __func__,
          Status
          ));
      }

      Status = FdtClient->SetNodeProperty (FdtClient, Node, "mac-address", MacAddr.Addr, 6);
      if (EFI_ERROR (Status)) {
        DEBUG ((
          EFI_D_ERROR,
          "%a: unable to set 'mac-address' FDT node property, Status: %r\n",
          __func__,
          Status
          ));
      }
    }

    ++DevIdx;
  }

  return EFI_SUCCESS;
}
