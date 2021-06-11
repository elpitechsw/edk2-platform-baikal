/** @file
  Copyright (c) 2019 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaikalSpdLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/NetLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>
#include <Protocol/FruClient.h>
#include "GmacSnp.h"

typedef struct {
  MAC_ADDR_DEVICE_PATH      MacAddrDevPath;
  EFI_DEVICE_PATH_PROTOCOL  End;
} GMAC_ETH_DEVPATH;

STATIC
BOOLEAN
IsValidMacAddr (
  IN  EFI_MAC_ADDRESS  *MacAddr
  )
{
  // Check if it is a multicast address
  if (MacAddr->Addr[0] & 0x01) {
    return FALSE;
  }

  // Check if it is zero address
  if ((MacAddr->Addr[0] |
       MacAddr->Addr[1] |
       MacAddr->Addr[2] |
       MacAddr->Addr[3] |
       MacAddr->Addr[4] |
       MacAddr->Addr[5]) == 0) {
    return FALSE;
  }

  return TRUE;
}

EFI_STATUS
EFIAPI
GmacDxeDriverEntry (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT32                Crc32 = 0;
  UINTN                 DevIdx = 0;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  EFI_STATUS            FdtStatus;
  FRU_CLIENT_PROTOCOL  *FruClient;
  BOOLEAN               GmacFound = FALSE;
  UINTN                 Idx;
  INT32                 Node;
  EFI_STATUS            Status;
  CONST VOID           *Prop;
  UINT32                PropSize;

  if (SystemTable == NULL) {
    DEBUG ((EFI_D_ERROR, "%a: failed, Status: %r\n", __FUNCTION__, EFI_INVALID_PARAMETER));
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FdtClientProtocol, Status: %r\n", __FUNCTION__, Status));
    return Status;
  }

  Status = gBS->LocateProtocol (&gFruClientProtocolGuid, NULL, (VOID **) &FruClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FruClientProtocol, Status: %r\n", __FUNCTION__, Status));
    return Status;
  }

  FdtStatus = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,baikal-m", -1, &Node);
  if (!EFI_ERROR (FdtStatus)) {
    FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "model", &Prop, NULL);
    if (!EFI_ERROR (FdtStatus)) {
      INTN  BoardType = -1;

      if (!AsciiStrCmp (Prop, "Baikal Electronics Baikal-M evaluation board")) {
        BoardType = 0;
      } else if (!AsciiStrCmp (Prop, "Baikal Electronics Baikal-M mitx board")) {
        BoardType = 1;
      }

      if (BoardType != -1) {
        UINTN  SpdAddr;
        UINTN  SpdSize = 0;

        // Get unique data from SPD
        if (!BoardType) {
          for (Idx = 0; Idx < 4 && !SpdSize; ++Idx) {
            SpdAddr = 0x50 + Idx;
            SpdSize = SpdGetSize (0, SpdAddr);
          }
        } else {
          for (Idx = 0; Idx < 2 && !SpdSize; ++Idx) {
            SpdAddr = 0x50 + Idx * 2;
            SpdSize = SpdGetSize (1, SpdAddr);
          }
        }

        if (SpdSize) {
          UINT8  *Spd;

          Status = gBS->AllocatePool (EfiBootServicesData, SpdSize, (VOID **) &Spd);
          if (!EFI_ERROR (Status)) {
            UINTN  TotalSize = 0;

            if (SpdSize <= 256) {
              TotalSize = SpdGetBuf (BoardType, SpdAddr, Spd, SpdSize);
            } else {
              TotalSize = SpdGetBuf (BoardType, SpdAddr, Spd, 256);
              SpdSwitchPage (BoardType, 1);
              TotalSize += SpdGetBuf (BoardType, SpdAddr, Spd + 256, SpdSize - 256);
              SpdSwitchPage (BoardType, 0);
            }

            if (TotalSize == SpdSize && SpdIsValid (Spd, SpdSize)) {
              gBS->CalculateCrc32 (Spd, SpdSize, &Crc32);
            }

            gBS->FreePool (Spd);
          }
        }
      }
    }
  }

  for (Node = 0;;) {
    FdtStatus = FdtClient->FindNextCompatibleNode (FdtClient, "be,dwmac", Node, &Node);
    if (EFI_ERROR (FdtStatus)) {
      FdtStatus = FdtClient->FindNextCompatibleNode (FdtClient, "snps,dwmac", Node, &Node);
    }

    if (EFI_ERROR (FdtStatus)) {
      FdtStatus = FdtClient->FindNextCompatibleNode (FdtClient, "snps,dwmac-3.710", Node, &Node);
    }

    if (EFI_ERROR (FdtStatus)) {
      break;
    }

    FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "status", &Prop, &PropSize);
    if (EFI_ERROR (Status) || AsciiStrCmp ((CONST CHAR8 *) Prop, "okay") != 0) {
      ++DevIdx;
      continue;
    }

    FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize);
    if (FdtStatus == EFI_SUCCESS && (PropSize % 16) == 0) {
      BOOLEAN            DtMacAddrSetRequest = FALSE;
      GMAC_ETH_DEVPATH  *EthDevPath;
      VOID *CONST        GmacRegs = (VOID *) SwapBytes64 (((CONST UINT64 *) Prop)[0]);
      EFI_HANDLE        *Handle;
      EFI_MAC_ADDRESS    MacAddr;
      VOID              *Snp;
      INTN               SnpsResetGpioPin;
      INTN               SnpsResetPolarity;

      Status = gBS->AllocatePool (EfiBootServicesData, sizeof (GMAC_ETH_DEVPATH), (VOID **) &EthDevPath);
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "%a: unable to allocate EthDevPath, Status: %r\n", __FUNCTION__, Status));
        return Status;
      }

      FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "mac-address", &Prop, &PropSize);
      if (FdtStatus == EFI_SUCCESS && PropSize == 6) {
        for (Idx = 0; Idx < PropSize; ++Idx) {
          MacAddr.Addr[Idx] = ((CONST UINT8 *) Prop)[Idx];
        }

        FdtStatus = IsValidMacAddr (&MacAddr) ? EFI_SUCCESS : EFI_INVALID_PARAMETER;
      }

      if (EFI_ERROR (FdtStatus)) {
        FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "local-mac-address", &Prop, &PropSize);
        if (FdtStatus == EFI_SUCCESS && PropSize == 6) {
          for (Idx = 0; Idx < PropSize; ++Idx) {
            MacAddr.Addr[Idx] = ((CONST UINT8 *) Prop)[Idx];
          }

          FdtStatus = IsValidMacAddr (&MacAddr) ? EFI_SUCCESS : EFI_INVALID_PARAMETER;
          if (EFI_ERROR (FdtStatus)) {
            DtMacAddrSetRequest = TRUE;
          }
        }
      }

      if (EFI_ERROR (FdtStatus)) {
        EFI_MAC_ADDRESS  FruMacAddr;

        gBS->SetMem (&FruMacAddr, sizeof (FruMacAddr), 0);
        Status = FruClient->GetMultirecordMacAddr (GmacRegs == (VOID *) 0x30240000 ? 0 : 1, &FruMacAddr);
        if (Status == EFI_SUCCESS && IsValidMacAddr (&FruMacAddr)) {
            gBS->CopyMem (&MacAddr, &FruMacAddr, sizeof (EFI_MAC_ADDRESS));
        } else {
          UINT64  MacAddrRegVal;

          MacAddrRegVal   = (*(UINT32 *)((EFI_PHYSICAL_ADDRESS) GmacRegs + 0x40) & 0xFFFF);
          MacAddrRegVal <<= 32;
          MacAddrRegVal  |= (*(UINT32 *)((EFI_PHYSICAL_ADDRESS) GmacRegs + 0x44));

          MacAddr.Addr[0] = (MacAddrRegVal >>  0) & 0xFF;
          MacAddr.Addr[1] = (MacAddrRegVal >>  8) & 0xFF;
          MacAddr.Addr[2] = (MacAddrRegVal >> 16) & 0xFF;
          MacAddr.Addr[3] = (MacAddrRegVal >> 24) & 0xFF;
          MacAddr.Addr[4] = (MacAddrRegVal >> 32) & 0xFF;
          MacAddr.Addr[5] = (MacAddrRegVal >> 40) & 0xFF;

          if (!IsValidMacAddr (&MacAddr)) {
            MacAddr.Addr[0] = 0x4C;
            MacAddr.Addr[1] = 0xA5;
            MacAddr.Addr[2] = 0x15;
            MacAddr.Addr[3] = (Crc32 >> 16) & 0xFF;
            MacAddr.Addr[4] = (Crc32 >>  8) & 0xFF;
            MacAddr.Addr[5] = (Crc32 & 0xFE) | DevIdx;
          }
        }

        if (DtMacAddrSetRequest) {
          FdtStatus = FdtClient->SetNodeProperty (FdtClient, Node, "local-mac-address", MacAddr.Addr, 6);
          if (EFI_ERROR (FdtStatus)) {
            DEBUG ((EFI_D_ERROR, "%a: unable to set 'local-mac-address' FDT node property, FdtStatus: %r\n", __FUNCTION__, FdtStatus));
          }

          FdtStatus = FdtClient->SetNodeProperty (FdtClient, Node, "mac-address", MacAddr.Addr, 6);
          if (EFI_ERROR (FdtStatus)) {
            DEBUG ((EFI_D_ERROR, "%a: unable to set 'mac-address' FDT node property, FdtStatus: %r\n", __FUNCTION__, FdtStatus));
          }
        }
      }

      SnpsResetGpioPin  = -1;
      SnpsResetPolarity = -1;
      if (FdtClient->GetNodeProperty (FdtClient, Node, "snps,reset-gpios", &Prop, &PropSize) == EFI_SUCCESS) {
        if (PropSize == 12) {
          SnpsResetGpioPin  = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
          SnpsResetPolarity = SwapBytes32 (((CONST UINT32 *) Prop)[2]);
          if (SnpsResetGpioPin > 31 || SnpsResetPolarity > 1) {
            SnpsResetGpioPin  = -1;
            SnpsResetPolarity = -1;
          }
        }
      } else if (FdtClient->GetNodeProperty (FdtClient, Node, "snps,reset-gp-out", &Prop, &PropSize) == EFI_SUCCESS) {
          SnpsResetGpioPin  = 0xE0;
          SnpsResetPolarity = 0;
      }

      EthDevPath->MacAddrDevPath.Header.Type    = MESSAGING_DEVICE_PATH;
      EthDevPath->MacAddrDevPath.Header.SubType = MSG_MAC_ADDR_DP;
      EthDevPath->MacAddrDevPath.IfType         = NET_IFTYPE_ETHERNET;
      gBS->CopyMem (&EthDevPath->MacAddrDevPath.MacAddress, &MacAddr, sizeof (EFI_MAC_ADDRESS));
      SetDevicePathNodeLength (&EthDevPath->MacAddrDevPath, sizeof (MAC_ADDR_DEVICE_PATH));
      SetDevicePathEndNode (&EthDevPath->End);

      DEBUG ((
        EFI_D_NET,
        "%a: GMAC@%p MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
        __FUNCTION__,
        GmacRegs,
        EthDevPath->MacAddrDevPath.MacAddress.Addr[0],
        EthDevPath->MacAddrDevPath.MacAddress.Addr[1],
        EthDevPath->MacAddrDevPath.MacAddress.Addr[2],
        EthDevPath->MacAddrDevPath.MacAddress.Addr[3],
        EthDevPath->MacAddrDevPath.MacAddress.Addr[4],
        EthDevPath->MacAddrDevPath.MacAddress.Addr[5]
        ));

      Status = GmacSnpInstanceCtor (
                 GmacRegs,
                 SnpsResetGpioPin,
                 SnpsResetPolarity,
                 &EthDevPath->MacAddrDevPath.MacAddress,
                 &Snp,
                 &Handle
                 );

      if (EFI_ERROR (Status)) {
        gBS->FreePool (EthDevPath);
        return Status;
      }

      Status = gBS->InstallMultipleProtocolInterfaces (
                      Handle,
                      &gEfiSimpleNetworkProtocolGuid,
                      Snp,
                      &gEfiDevicePathProtocolGuid,
                      &EthDevPath->MacAddrDevPath,
                      NULL
                      );

      if (EFI_ERROR (Status)) {
        DEBUG ((
          EFI_D_ERROR,
          "%a: unable to InstallMultipleProtocolInterfaces, Status: %r\n",
          __FUNCTION__,
          Status
          ));
        GmacSnpInstanceDtor (Snp);
        gBS->FreePool (EthDevPath);
        return Status;
      }

      GmacFound = TRUE;
      ++DevIdx;
    }
  }

  if (!GmacFound) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}
