/** @file
  Copyright (c) 2019 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/NetLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>
#include <Protocol/FruClient.h>
#include "GmacRegs.h"
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

  for (Node = 0;;) {
    if (FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bm1000-gmac", Node, &Node) != EFI_SUCCESS &&
        FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bs1000-gmac", Node, &Node) != EFI_SUCCESS) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
      continue;
    }

    FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize);
    if (FdtStatus == EFI_SUCCESS && PropSize == 16) {
      BOOLEAN                      DtMacAddrSetRequest = FALSE;
      BOOLEAN                      DmaCoherent;
      GMAC_ETH_DEVPATH            *EthDevPath;
      volatile GMAC_REGS * CONST   GmacRegs = (VOID *) SwapBytes64 (((CONST UINT64 *) Prop)[0]);
      EFI_HANDLE                  *Handle;
      EFI_MAC_ADDRESS              MacAddr;
      EFI_PHYSICAL_ADDRESS         ResetGpioBase;
      INTN                         ResetGpioPin;
      INTN                         ResetPolarity;
      VOID                        *Snp;
      BOOLEAN                      Tx2AddDiv2 = FALSE;
      EFI_PHYSICAL_ADDRESS         Tx2ClkChCtlAddr = 0;
      INT32                        PhyAddr = -1;
      INT32                        ClkCsr = 4;
      BOOLEAN                      RgmiiRxId;
      BOOLEAN                      RgmiiTxId;

      Status = gBS->AllocatePool (EfiBootServicesData, sizeof (GMAC_ETH_DEVPATH), (VOID **) &EthDevPath);
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "%a: unable to allocate EthDevPath, Status: %r\n", __FUNCTION__, Status));
        return Status;
      }

      if (FdtClient->GetNodeProperty (FdtClient, Node, "dma-coherent", &Prop, &PropSize) == EFI_SUCCESS) {
        DmaCoherent = TRUE;
      } else {
        DmaCoherent = FALSE;
      }

      if (FdtClient->GetNodeProperty (FdtClient, Node, "clock-names", &Prop, &PropSize) == EFI_SUCCESS) {
        UINTN         ClkNameIdx = 0;
        CONST CHAR8  *ClkNamePtr = Prop;

        while (ClkNamePtr < (CHAR8 *) Prop + PropSize) {
          if (!AsciiStrCmp (ClkNamePtr, "tx2_clk")) {
            if (FdtClient->GetNodeProperty (FdtClient, Node, "clocks", &Prop, &PropSize) == EFI_SUCCESS && PropSize > 0 && (PropSize % 4) == 0) {
              UINT32        ClkPhandleIdx = 0;
              CONST UINT32 *ClkPhandlePtr = Prop;

              for (;;) {
                INT32  ClkNode;

                if (FdtClient->FindNodeByPhandle (FdtClient, SwapBytes32 (*ClkPhandlePtr), &ClkNode) == EFI_SUCCESS) {
                  if (ClkPhandleIdx == ClkNameIdx) {
                    CONST UINT32  Tx2ClkChNum = SwapBytes32 (*(ClkPhandlePtr + 1));

                    if (FdtClient->GetNodeProperty (FdtClient, ClkNode, "cmu-id", &Prop, &PropSize) == EFI_SUCCESS && PropSize == 4) {
                      Tx2ClkChCtlAddr = SwapBytes32 (((CONST UINT32 *) Prop)[0]) + 0x20 + Tx2ClkChNum * 0x10;
                    } else if (FdtClient->GetNodeProperty (FdtClient, ClkNode, "reg", &Prop, &PropSize) == EFI_SUCCESS && PropSize == 4) {
                      Tx2ClkChCtlAddr = SwapBytes32 (((CONST UINT32 *) Prop)[0]) + Tx2ClkChNum * 0x10;
                    }

                    break;
                  } else if (FdtClient->GetNodeProperty (FdtClient, ClkNode, "#clock-cells", &Prop, &PropSize) == EFI_SUCCESS && PropSize == 4) {
                    ClkPhandleIdx++;
                    ClkPhandlePtr += 1 + SwapBytes32 (((CONST UINT32 *) Prop)[0]);
                  } else {
                    break;
                  }
                } else {
                  break;
                }
              }
            }

            break;
          } else {
            ClkNameIdx++;
            ClkNamePtr += AsciiStrLen (ClkNamePtr) + 1;
          }
        }
      }

      if (FdtClient->GetNodeProperty (FdtClient, Node, "compatible", &Prop, &PropSize) == EFI_SUCCESS) {
        CONST CHAR8  *CompatiblePtr = Prop;
        while (CompatiblePtr < (CHAR8 *) Prop + PropSize) {
          if (!AsciiStrCmp (CompatiblePtr, "baikal,bs1000-gmac")) {
            Tx2AddDiv2 = TRUE;
            break;
          } else {
            CompatiblePtr += AsciiStrLen (CompatiblePtr) + 1;
          }
        }
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
        Status = FruClient->GetMultirecordMacAddr (DevIdx, &FruMacAddr);
        if (Status == EFI_SUCCESS && IsValidMacAddr (&FruMacAddr)) {
            gBS->CopyMem (&MacAddr, &FruMacAddr, sizeof (EFI_MAC_ADDRESS));
        } else {
          UINT64  MacAddrRegVal;

          MacAddrRegVal   = GmacRegs->MacAddr0Hi & 0xFFFF;
          MacAddrRegVal <<= 32;
          MacAddrRegVal  |= GmacRegs->MacAddr0Lo;

          MacAddr.Addr[0] = (MacAddrRegVal >>  0) & 0xFF;
          MacAddr.Addr[1] = (MacAddrRegVal >>  8) & 0xFF;
          MacAddr.Addr[2] = (MacAddrRegVal >> 16) & 0xFF;
          MacAddr.Addr[3] = (MacAddrRegVal >> 24) & 0xFF;
          MacAddr.Addr[4] = (MacAddrRegVal >> 32) & 0xFF;
          MacAddr.Addr[5] = (MacAddrRegVal >> 40) & 0xFF;

          if (!IsValidMacAddr (&MacAddr)) {
            DEBUG((EFI_D_ERROR, "No valid MAC address for gmac%d!\n", DevIdx));
            MacAddr.Addr[0] = 0x4C;
            MacAddr.Addr[1] = 0xA5;
            MacAddr.Addr[2] = 0x15;
            MacAddr.Addr[3] = 0x01;
            MacAddr.Addr[4] = 0x02;
            MacAddr.Addr[5] = DevIdx;
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

      ResetGpioBase = 0;
      ResetGpioPin  = -1;
      ResetPolarity = -1;
      if (FdtClient->GetNodeProperty (FdtClient, Node, "snps,reset-gpios", &Prop, &PropSize) == EFI_SUCCESS) {
        if (PropSize == 12) {
          INT32  ResetGpioNode;

          ResetGpioPin  = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
          ResetPolarity = SwapBytes32 (((CONST UINT32 *) Prop)[2]);
          if (FdtClient->FindNodeByPhandle (FdtClient, SwapBytes32 (((CONST UINT32 *) Prop)[0]), &ResetGpioNode) == EFI_SUCCESS &&
              FdtClient->FindParentNode    (FdtClient, ResetGpioNode, &ResetGpioNode) == EFI_SUCCESS &&
              FdtClient->GetNodeProperty   (FdtClient, ResetGpioNode, "reg", &Prop, &PropSize) == EFI_SUCCESS && PropSize == 16) {
            ResetGpioBase = SwapBytes64 (((CONST UINT64 *) Prop)[0]);
            if (ResetGpioBase == 0 || ResetGpioPin > 31 || ResetPolarity > 1) {
              ResetGpioBase = 0;
              ResetGpioPin  = -1;
              ResetPolarity = -1;
            }
          }
        }
      } else {
        ResetGpioBase = (EFI_PHYSICAL_ADDRESS) &GmacRegs->MacGpio;
        ResetGpioPin  = 0;
      }

      if (FdtClient->GetNodeProperty (FdtClient, Node, "phy-handle", &Prop, &PropSize) == EFI_SUCCESS && PropSize == 4) {
        INT32  PhyNode;
        if (FdtClient->FindNodeByPhandle (FdtClient, SwapBytes32 (((CONST UINT32 *) Prop)[0]), &PhyNode) == EFI_SUCCESS &&
            FdtClient->GetNodeProperty   (FdtClient, PhyNode, "reg", &Prop, &PropSize) == EFI_SUCCESS && PropSize == 4) {
          PhyAddr = SwapBytes32 (((CONST UINT32 *) Prop)[0]);
        }
      }

      if (FdtClient->GetNodeProperty (FdtClient, Node, "phy-mode", &Prop, &PropSize) == EFI_SUCCESS) {
        if (!AsciiStrCmp (Prop, "rgmii-id")) {
          RgmiiRxId = TRUE;
          RgmiiTxId = TRUE;
        }
        else if (!AsciiStrCmp (Prop, "rgmii-rxid")) {
          RgmiiRxId = TRUE;
          RgmiiTxId = FALSE;
        }
        else if (!AsciiStrCmp (Prop, "rgmii-txid")) {
          RgmiiRxId = FALSE;
          RgmiiTxId = TRUE;
        }
      }

      EthDevPath->MacAddrDevPath.Header.Type    = MESSAGING_DEVICE_PATH;
      EthDevPath->MacAddrDevPath.Header.SubType = MSG_MAC_ADDR_DP;
      EthDevPath->MacAddrDevPath.IfType         = NET_IFTYPE_ETHERNET;
      gBS->CopyMem (&EthDevPath->MacAddrDevPath.MacAddress, &MacAddr, sizeof (EFI_MAC_ADDRESS));
      SetDevicePathNodeLength (&EthDevPath->MacAddrDevPath, sizeof (MAC_ADDR_DEVICE_PATH));
      SetDevicePathEndNode (&EthDevPath->End);

      Status = GmacSnpInstanceConstructor (
                 GmacRegs,
		 DmaCoherent,
		 Tx2ClkChCtlAddr,
		 Tx2AddDiv2,
                 ResetGpioBase,
                 ResetGpioPin,
                 ResetPolarity,
		 PhyAddr,
		 ClkCsr,
		 RgmiiRxId,
		 RgmiiTxId,
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
        GmacSnpInstanceDestructor (Snp);
        gBS->FreePool (EthDevPath);
        return Status;
      }

      GmacFound = TRUE;
      ++DevIdx;
    }
  }

  /* Do the same for xgmac (except starting driver) */
  for (Node = 0;;) {
    FdtStatus = FdtClient->FindNextCompatibleNode (FdtClient, "amd,xgbe-seattle-v1a", Node, &Node);

    if (EFI_ERROR (FdtStatus)) {
      break;
    }

    FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "status", &Prop, &PropSize);
    if (EFI_ERROR (Status) || AsciiStrCmp ((CONST CHAR8 *) Prop, "okay") != 0) {
      continue;
    }

    FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize);
    if (FdtStatus == EFI_SUCCESS && (PropSize % 16) == 0) {
      BOOLEAN            DtMacAddrSetRequest = FALSE;
      GMAC_ETH_DEVPATH  *EthDevPath;
      VOID *CONST        XGmacRegs = (VOID *) SwapBytes64 (((CONST UINT64 *) Prop)[0]);
      EFI_MAC_ADDRESS    MacAddr;

      DEBUG ((EFI_D_NET, "%a: XGMAC@%p found\n", __FUNCTION__, XGmacRegs));

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
        Status = FruClient->GetMultirecordMacAddr (DevIdx, &FruMacAddr);
        if (Status == EFI_SUCCESS && IsValidMacAddr (&FruMacAddr)) {
            gBS->CopyMem (&MacAddr, &FruMacAddr, sizeof (EFI_MAC_ADDRESS));
        } else {
          UINT64  MacAddrRegVal;

          MacAddrRegVal   = (*(UINT32 *)((EFI_PHYSICAL_ADDRESS) XGmacRegs + 0x300) & 0xFFFF);
          MacAddrRegVal <<= 32;
          MacAddrRegVal  |= (*(UINT32 *)((EFI_PHYSICAL_ADDRESS) XGmacRegs + 0x304));

          MacAddr.Addr[0] = (MacAddrRegVal >>  0) & 0xFF;
          MacAddr.Addr[1] = (MacAddrRegVal >>  8) & 0xFF;
          MacAddr.Addr[2] = (MacAddrRegVal >> 16) & 0xFF;
          MacAddr.Addr[3] = (MacAddrRegVal >> 24) & 0xFF;
          MacAddr.Addr[4] = (MacAddrRegVal >> 32) & 0xFF;
          MacAddr.Addr[5] = (MacAddrRegVal >> 40) & 0xFF;

          if (!IsValidMacAddr (&MacAddr)) {
            DEBUG((EFI_D_ERROR, "No valid MAC address for xgmac (DevIdx %d)!\n", DevIdx));
            MacAddr.Addr[0] = 0x4C;
            MacAddr.Addr[1] = 0xA5;
            MacAddr.Addr[2] = 0x15;
            MacAddr.Addr[3] = 0x01;
            MacAddr.Addr[4] = 0x02;
            MacAddr.Addr[5] = DevIdx;
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

      EthDevPath->MacAddrDevPath.Header.Type    = MESSAGING_DEVICE_PATH;
      EthDevPath->MacAddrDevPath.Header.SubType = MSG_MAC_ADDR_DP;
      EthDevPath->MacAddrDevPath.IfType         = NET_IFTYPE_ETHERNET;
      gBS->CopyMem (&EthDevPath->MacAddrDevPath.MacAddress, &MacAddr, sizeof (EFI_MAC_ADDRESS));
      SetDevicePathNodeLength (&EthDevPath->MacAddrDevPath, sizeof (MAC_ADDR_DEVICE_PATH));
      SetDevicePathEndNode (&EthDevPath->End);

      DEBUG ((
        EFI_D_NET,
        "%a: XGMAC@%p MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
        __FUNCTION__,
        XGmacRegs,
        EthDevPath->MacAddrDevPath.MacAddress.Addr[0],
        EthDevPath->MacAddrDevPath.MacAddress.Addr[1],
        EthDevPath->MacAddrDevPath.MacAddress.Addr[2],
        EthDevPath->MacAddrDevPath.MacAddress.Addr[3],
        EthDevPath->MacAddrDevPath.MacAddress.Addr[4],
        EthDevPath->MacAddrDevPath.MacAddress.Addr[5]
        ));

      GmacFound = TRUE;
      ++DevIdx;
    }
  }

  if (!GmacFound) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}
