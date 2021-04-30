/** @file
  Copyright (c) 2019 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaikalFruLib.h>
#include <Library/BaikalI2cLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/NetLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>
#include "BaikalEthSnp.h"

#define FRU_EEPROM_I2C_BUS   0
#define FRU_EEPROM_I2C_ADDR  0x53

typedef struct {
  MAC_ADDR_DEVICE_PATH      MacAddrDevPath;
  EFI_DEVICE_PATH_PROTOCOL  End;
} BAIKAL_ETH_DEVPATH;

STATIC
EFI_STATUS
EFIAPI
IsValidMacAddr (
  IN  EFI_MAC_ADDRESS  *MacAddr
  )
{
  // Check if it is a multicast address
  if (MacAddr->Addr[0] & 0x01) {
    return EFI_INVALID_PARAMETER;
  }

  // Check if it is zero address
  if ((MacAddr->Addr[0] |
       MacAddr->Addr[1] |
       MacAddr->Addr[2] |
       MacAddr->Addr[3] |
       MacAddr->Addr[4] |
       MacAddr->Addr[5]) == 0) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BaikalEthDxeDriverEntry (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN                 DevIdx = 0;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  UINT8                *FruBuf;
  EFI_MAC_ADDRESS       FruMacAddrs[2];
  CONST UINT16          FruMemAddr = 0;
  BOOLEAN               GmacFound = FALSE;
  INTN                  I2cRxedSize;
  UINTN                 Idx;
  INT32                 Node;
  EFI_STATUS            Status;

  if (SystemTable == NULL) {
    DEBUG ((EFI_D_ERROR, "%a: failed, Status: %r\n", __FUNCTION__, EFI_INVALID_PARAMETER));
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FdtClientProtocol, Status: %r\n", __FUNCTION__, Status));
    return Status;
  }

  Status = gBS->AllocatePool (EfiBootServicesData, FRU_SIZE, (VOID **) &FruBuf);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FruBuf, Status: %r\n", __FUNCTION__, Status));
    return Status;
  }

  I2cRxedSize = I2cTxRx (FRU_EEPROM_I2C_BUS,
                         FRU_EEPROM_I2C_ADDR,
                         (UINT8 *)&FruMemAddr,
                         sizeof FruMemAddr,
                         FruBuf,
                         FRU_SIZE
                         );

  gBS->SetMem (FruMacAddrs, sizeof (FruMacAddrs), 0);

  if (I2cRxedSize == FRU_SIZE) {
    for (Idx = 0; Idx < sizeof FruMacAddrs / sizeof FruMacAddrs[0]; ++Idx) {
      BaikalFruGetMacAddr(FruBuf, FRU_SIZE, Idx, &FruMacAddrs[Idx]);
    }
  }

  gBS->FreePool (FruBuf);

  for (Node = 0;;) {
    EFI_STATUS     FdtStatus;
    CONST UINT64  *Reg;
    UINT32         RegSize;

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

    FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "reg", (CONST VOID **) &Reg, &RegSize);
    if (FdtStatus == EFI_SUCCESS && (RegSize % 16) == 0) {
      BOOLEAN              DtMacAddrSetRequest = FALSE;
      BAIKAL_ETH_DEVPATH  *EthDevPath;
      VOID * CONST         GmacRegs = (VOID *) SwapBytes64 (Reg[0]);
      EFI_HANDLE          *Handle;
      EFI_MAC_ADDRESS      MacAddr;
      CONST UINT8         *RegByte;
      VOID                *Snp;

      DEBUG ((EFI_D_NET, "%a: GMAC@%p found\n", __FUNCTION__, GmacRegs));

      Status = gBS->AllocatePool (EfiBootServicesData, sizeof (BAIKAL_ETH_DEVPATH), (VOID **) &EthDevPath);
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "%a: unable to allocate EthDevPath, Status: %r\n", __FUNCTION__, Status));
        return Status;
      }

      FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "mac-address", (CONST VOID **) &Reg, &RegSize);
      if (FdtStatus == EFI_SUCCESS && RegSize == 6) {
        RegByte = (UINT8 *) Reg;
        for (Idx = 0; Idx < RegSize; ++Idx) {
          MacAddr.Addr[Idx] = RegByte[Idx];
        }

        FdtStatus = IsValidMacAddr(&MacAddr);
      }

      if (EFI_ERROR (FdtStatus)) {
        FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "local-mac-address", (CONST VOID **) &Reg, &RegSize);
        if (FdtStatus == EFI_SUCCESS && RegSize == 6) {
          RegByte = (UINT8 *) Reg;
          for (Idx = 0; Idx < RegSize; ++Idx) {
            MacAddr.Addr[Idx] = RegByte[Idx];
          }

          FdtStatus = IsValidMacAddr(&MacAddr);
          if (EFI_ERROR (FdtStatus)) {
            DtMacAddrSetRequest = TRUE;
          }
        }
      }

      if (EFI_ERROR (FdtStatus)) {
        Status = IsValidMacAddr(&FruMacAddrs[DevIdx]);
        if (Status == EFI_SUCCESS) {
            gBS->CopyMem (&MacAddr, &FruMacAddrs[DevIdx], sizeof (EFI_MAC_ADDRESS));
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

          Status = IsValidMacAddr(&MacAddr);
          if (EFI_ERROR (Status)) {
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
            DEBUG ((EFI_D_ERROR, "%a: unable to set 'local-mac-address' FDT node property, Status: %r\n", __FUNCTION__, Status));
          }

          FdtStatus = FdtClient->SetNodeProperty (FdtClient, Node, "mac-address", MacAddr.Addr, 6);
          if (EFI_ERROR (FdtStatus)) {
            DEBUG ((EFI_D_ERROR, "%a: unable to set 'mac-address' FDT node property, Status: %r\n", __FUNCTION__, Status));
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

      Status = BaikalEthSnpInstanceCtor (GmacRegs, &EthDevPath->MacAddrDevPath.MacAddress, &Snp, &Handle);

      if (EFI_ERROR (Status)) {
        gBS->FreePool (EthDevPath);
        return Status;
      }

      Status = gBS->InstallMultipleProtocolInterfaces (
                      Handle,
                      &gEfiSimpleNetworkProtocolGuid, Snp,
                      &gEfiDevicePathProtocolGuid, &EthDevPath->MacAddrDevPath,
                      NULL
                      );

      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "%a: unable to InstallMultipleProtocolInterfaces, Status: %r\n", __FUNCTION__, Status));
        BaikalEthSnpInstanceDtor (Snp);
        gBS->FreePool (EthDevPath);
        return Status;
      }

      GmacFound = TRUE;
      ++DevIdx;
    }
  }

  if (!GmacFound) {
    DEBUG ((EFI_D_NET, "%a: GMAC is not found\n", __FUNCTION__));
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}
