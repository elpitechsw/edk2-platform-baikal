/** @file
  Copyright (c) 2019 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/NetLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/EuiClient.h>
#include <Protocol/FdtClient.h>
#include "GmacRegs.h"
#include "GmacSnp.h"

typedef struct {
  MAC_ADDR_DEVICE_PATH      MacAddrDevPath;
  EFI_DEVICE_PATH_PROTOCOL  End;
} GMAC_ETH_DEVPATH;

EFI_STATUS
EFIAPI
GmacDxeDriverEntry (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN                 DevIdx = 0;
  EUI_CLIENT_PROTOCOL  *EuiClient;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  EFI_STATUS            FdtStatus;
  INT32                 GmacNode = 0;
  UINTN                 Idx;
  EFI_STATUS            Status;
  CONST VOID           *Prop;
  UINT32                PropSize;

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
    if (FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bm1000-gmac", GmacNode, &GmacNode) != EFI_SUCCESS &&
        FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bs1000-gmac", GmacNode, &GmacNode) != EFI_SUCCESS) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, GmacNode)) {
      continue;
    }

    FdtStatus = FdtClient->GetNodeProperty (FdtClient, GmacNode, "reg", &Prop, &PropSize);
    if (FdtStatus == EFI_SUCCESS && PropSize == 2 * sizeof (UINT64)) {
      BOOLEAN                      DmaCoherent = FALSE;
      GMAC_ETH_DEVPATH            *EthDevPath;
      volatile GMAC_REGS * CONST   GmacRegs = (VOID *) SwapBytes64 (ReadUnaligned64 (Prop));
      EFI_HANDLE                  *Handle;
      EFI_MAC_ADDRESS              MacAddr;
      INT32                        Node;
      EFI_PHYSICAL_ADDRESS         ResetGpioBase;
      INTN                         ResetGpioPin;
      INTN                         ResetPolarity;
      VOID                        *Snp;
      BOOLEAN                      Tx2AddDiv2 = FALSE;
      EFI_PHYSICAL_ADDRESS         Tx2ClkChCtlAddr = 0;
      INT32                        PhyAddr = -1;
      INT32                        ClkCsr = 4;
      BOOLEAN                      RgmiiRxId = FALSE;
      BOOLEAN                      RgmiiTxId = FALSE;

      Status = gBS->AllocatePool (EfiBootServicesData, sizeof (GMAC_ETH_DEVPATH), (VOID **) &EthDevPath);
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "%a: unable to allocate EthDevPath, Status: %r\n", __func__, Status));
        return Status;
      }

      Node = GmacNode;
      do {
        if (FdtClient->GetNodeProperty (FdtClient, Node, "dma-coherent", &Prop, &PropSize) == EFI_SUCCESS) {
          DmaCoherent = TRUE;
          break;
        }
      } while (FdtClient->FindParentNode (FdtClient, Node, &Node) == EFI_SUCCESS);

      if (FdtClient->GetNodeProperty (FdtClient, GmacNode, "clock-names", &Prop, &PropSize) == EFI_SUCCESS) {
        UINTN         ClkNameIdx = 0;
        CONST CHAR8  *ClkNamePtr = Prop;

        while (ClkNamePtr < (CHAR8 *) Prop + PropSize) {
          if (AsciiStrCmp (ClkNamePtr, "tx2_clk") == 0) {
            if (FdtClient->GetNodeProperty (FdtClient, GmacNode, "clocks", &Prop, &PropSize) == EFI_SUCCESS && PropSize > 0 && (PropSize % sizeof (UINT32)) == 0) {
              UINT32        ClkPhandleIdx = 0;
              CONST UINT32 *ClkPhandlePtr = Prop;

              for (;;) {
                if (FdtClient->FindNodeByPhandle (FdtClient, SwapBytes32 (*ClkPhandlePtr), &Node) == EFI_SUCCESS) {
                  if (ClkPhandleIdx == ClkNameIdx) {
                    CONST UINT32  Tx2ClkChNum = SwapBytes32 (*(ClkPhandlePtr + 1));

                    if (FdtClient->GetNodeProperty (FdtClient, Node, "compatible", &Prop, &PropSize) == EFI_SUCCESS) {
                      if (AsciiStrCmp (Prop, "baikal,bm1000-cmu") == 0) {
                        if (FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS && PropSize == 2 * sizeof (UINT64)) {
                          Tx2ClkChCtlAddr = SwapBytes64 (ReadUnaligned64 (Prop)) + 0x20 + Tx2ClkChNum * 0x10;
                        }
                      } else if (AsciiStrCmp (Prop, "baikal,bs1000-cmu") == 0) {
                        if (FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS && PropSize == sizeof (UINT32)) {
                          Tx2ClkChCtlAddr = SwapBytes32 (*(CONST UINT32 *) Prop) + Tx2ClkChNum * 0x10;
                        }
                      }
                    }

                    break;
                  } else if (FdtClient->GetNodeProperty (FdtClient, Node, "#clock-cells", &Prop, &PropSize) == EFI_SUCCESS && PropSize == sizeof (UINT32)) {
                    ClkPhandleIdx++;
                    ClkPhandlePtr += 1 + SwapBytes32 (*(CONST UINT32 *) Prop);
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

      if (FdtClient->GetNodeProperty (FdtClient, GmacNode, "compatible", &Prop, &PropSize) == EFI_SUCCESS) {
        CONST CHAR8  *CompatiblePtr = Prop;
        while (CompatiblePtr < (CHAR8 *) Prop + PropSize) {
          if (AsciiStrCmp (CompatiblePtr, "baikal,bs1000-gmac") == 0) {
            Tx2AddDiv2 = TRUE;
            break;
          } else {
            CompatiblePtr += AsciiStrLen (CompatiblePtr) + 1;
          }
        }
      }

      SetMem (&MacAddr, sizeof (MacAddr), 0);
      FdtStatus = FdtClient->GetNodeProperty (FdtClient, GmacNode, "mac-address", &Prop, &PropSize);
      if (FdtStatus == EFI_SUCCESS) {
        if (PropSize == 6) {
          for (Idx = 0; Idx < PropSize; ++Idx) {
            MacAddr.Addr[Idx] = ((CONST UINT8 *) Prop)[Idx];
          }

          FdtStatus = EuiClient->IsValidEui48 (&MacAddr) ? EFI_SUCCESS : EFI_INVALID_PARAMETER;
        } else {
          FdtStatus = EFI_INVALID_PARAMETER;
        }
      }

      if (EFI_ERROR (FdtStatus)) {
        FdtStatus = FdtClient->GetNodeProperty (FdtClient, GmacNode, "local-mac-address", &Prop, &PropSize);
        if (FdtStatus == EFI_SUCCESS) {
          if (PropSize == 6) {
            for (Idx = 0; Idx < PropSize; ++Idx) {
              MacAddr.Addr[Idx] = ((CONST UINT8 *) Prop)[Idx];
            }

            FdtStatus = EuiClient->IsValidEui48 (&MacAddr) ? EFI_SUCCESS : EFI_INVALID_PARAMETER;
          } else {
            FdtStatus = EFI_INVALID_PARAMETER;
          }
        }
      }

      if (EFI_ERROR (FdtStatus)) {
        UINT64  MacAddrRegVal;

        MacAddrRegVal   = GmacRegs->MacAddr0Hi & 0xFFFF;
        MacAddrRegVal <<= 32;
        MacAddrRegVal  |= GmacRegs->MacAddr0Lo;

        for (Idx = 0; Idx < 6; ++Idx) {
          MacAddr.Addr[Idx] = (MacAddrRegVal >> (Idx * 8)) & 0xFF;
        }

        EuiClient->GetEui48 ((EFI_PHYSICAL_ADDRESS) GmacRegs, DevIdx, &MacAddr);
        FdtStatus = FdtClient->SetNodeProperty (FdtClient, GmacNode, "local-mac-address", MacAddr.Addr, 6);
        if (EFI_ERROR (FdtStatus)) {
          DEBUG ((
            EFI_D_ERROR,
            "%a: unable to set 'local-mac-address' FDT node property, FdtStatus: %r\n",
            __func__,
            FdtStatus
            ));
        }

        FdtStatus = FdtClient->SetNodeProperty (FdtClient, GmacNode, "mac-address", MacAddr.Addr, 6);
        if (EFI_ERROR (FdtStatus)) {
          DEBUG ((
            EFI_D_ERROR,
            "%a: unable to set 'mac-address' FDT node property, FdtStatus: %r\n",
            __func__,
            FdtStatus
            ));
        }
      }

      ResetGpioBase = 0;
      ResetGpioPin  = -1;
      ResetPolarity = -1;
      if (FdtClient->GetNodeProperty (FdtClient, GmacNode, "snps,reset-gpios", &Prop, &PropSize) == EFI_SUCCESS) {
        if (PropSize == 12) {
          INT32  ResetGpioNode;

          ResetGpioPin  = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
          ResetPolarity = SwapBytes32 (((CONST UINT32 *) Prop)[2]);
          if (FdtClient->FindNodeByPhandle (FdtClient, SwapBytes32 (((CONST UINT32 *) Prop)[0]), &ResetGpioNode) == EFI_SUCCESS &&
              FdtClient->FindParentNode    (FdtClient, ResetGpioNode, &ResetGpioNode) == EFI_SUCCESS &&
              FdtClient->GetNodeProperty   (FdtClient, ResetGpioNode, "reg", &Prop, &PropSize) == EFI_SUCCESS && PropSize == 2 * sizeof (UINT64)) {
            ResetGpioBase = SwapBytes64 (ReadUnaligned64 (Prop));
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

      if (FdtClient->GetNodeProperty (FdtClient, GmacNode, "phy-handle", &Prop, &PropSize) == EFI_SUCCESS && PropSize == sizeof (UINT32)) {
        INT32  PhyNode;
        if (FdtClient->FindNodeByPhandle (FdtClient, SwapBytes32 (*(CONST UINT32 *) Prop), &PhyNode) == EFI_SUCCESS &&
            FdtClient->GetNodeProperty   (FdtClient, PhyNode, "reg", &Prop, &PropSize) == EFI_SUCCESS && PropSize == sizeof (UINT32)) {
          PhyAddr = SwapBytes32 (*(CONST UINT32 *) Prop);
        }
      }

      if (FdtClient->GetNodeProperty (FdtClient, GmacNode, "phy-mode", &Prop, &PropSize) == EFI_SUCCESS) {
        if (AsciiStrCmp (Prop, "rgmii-id") == 0) {
          RgmiiRxId = TRUE;
          RgmiiTxId = TRUE;
        } else if (AsciiStrCmp (Prop, "rgmii-rxid") == 0) {
          RgmiiRxId = TRUE;
          RgmiiTxId = FALSE;
        } else if (AsciiStrCmp (Prop, "rgmii-txid") == 0) {
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
        DEBUG ((EFI_D_ERROR, "%a: unable to InstallMultipleProtocolInterfaces, Status: %r\n", __func__, Status));
        GmacSnpInstanceDestructor (Snp);
        gBS->FreePool (EthDevPath);
        return Status;
      }

      ++DevIdx;
    }
  }

  return EFI_SUCCESS;
}
