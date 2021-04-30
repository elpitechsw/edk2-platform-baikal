/** @file
  Copyright (c) 2019 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaikalFruLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/NetLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>
#include <Protocol/I2cIo.h>
#include "BaikalEthSnp.h"

typedef struct {
  MAC_ADDR_DEVICE_PATH      MacAddrDevPath;
  EFI_DEVICE_PATH_PROTOCOL  End;
} BAIKAL_ETH_DEVPATH;

typedef struct {
  UINTN                           OperationCount;
  EFI_I2C_OPERATION               Address;
  EFI_I2C_OPERATION               Data;
} EEPROM_I2C_READ_REQUEST;

STATIC
EFI_STATUS
LocateI2cEepromDevice (
  OUT EFI_I2C_IO_PROTOCOL     **Instance
)
{
  EFI_I2C_IO_PROTOCOL *Dev;
  EFI_STATUS          Status;
  EFI_HANDLE          *HandleBuffer;
  UINTN               HandleCount;
  UINTN               Index;
  BOOLEAN             Found;

  if (Instance == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Retrieve all I2C device I/O handles in the handle database
  //
  Status = gBS->LocateHandleBuffer (ByProtocol,
                                    &gEfiI2cIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Locate protocol instance matching specific device
  //
  Found = FALSE;
  for (Index = 0; (Index < HandleCount) && !Found; Index++) {
    Status = gBS->OpenProtocol (
                    HandleBuffer[Index],
                    &gEfiI2cIoProtocolGuid,
                    (VOID **) &Dev,
                    gImageHandle,
                    HandleBuffer[Index],
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status) &&
        CompareGuid (Dev->DeviceGuid, &gEepromI2cDeviceGuid)) {
      Found = TRUE;
      *Instance = Dev;
    }
  }

  //
  // Free the handle array
  //
  gBS->FreePool (HandleBuffer);

  return Found ? EFI_SUCCESS : EFI_NOT_FOUND;
}

/**
  Read data from the EEPROM.

  @param  I2cIo                 Pointer to instance of I2C_IO_PROTOCOL.
  @param  Address               Memory address.
  @param  Size                  Number of bytes to read.
  @param  Data                  Pointer to the buffer to store the data in.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_DEVICE_ERROR      The data could not be retrieved from memory
                                due to hardware error.
**/
STATIC
EFI_STATUS
I2cEepromReadData (
  IN  EFI_I2C_IO_PROTOCOL         *I2cIo,
  IN  UINT16                      Address,
  IN  UINT16                      Size,
  OUT UINT8                       *Data
  )
{
  EEPROM_I2C_READ_REQUEST  Request;
  EFI_STATUS               Status;
  UINT8                    AddressBuffer[2];

  Status = EFI_DEVICE_ERROR;
  AddressBuffer[0] = (Address >> 8) & 0xFF;
  AddressBuffer[1] = Address & 0xFF;

  if (I2cIo != NULL) {
    Request.OperationCount = 2;
    Request.Address.Flags = 0;
    Request.Address.LengthInBytes = sizeof (AddressBuffer);
    Request.Address.Buffer = AddressBuffer;
    Request.Data.Flags = I2C_FLAG_READ;
    Request.Data.LengthInBytes = Size;
    Request.Data.Buffer = Data;
    Status = I2cIo->QueueRequest (I2cIo, 0,
                                  NULL,
                                  (EFI_I2C_REQUEST_PACKET *)&Request,
                                  NULL);
    if (EFI_ERROR (Status)) {
      if (Status != EFI_TIMEOUT) {
        DEBUG ((EFI_D_ERROR, "I2cEeprom: read @%04X failed: %r\r\n",
                Address, Status));
      }
      Status = EFI_DEVICE_ERROR;
    }
  }

  return Status;
}

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
  EFI_I2C_IO_PROTOCOL  *I2cIo;
  UINT8                *FruBuf;
  EFI_MAC_ADDRESS       FruMacAddrs[4];
  CONST UINT16          FruMemAddr = 0;
  BOOLEAN               GmacFound = FALSE;
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
    DEBUG ((EFI_D_ERROR, "%a: unable to allocate FruBuf, Status: %r\n", __FUNCTION__, Status));
    return Status;
  }

  gBS->SetMem (FruMacAddrs, sizeof (FruMacAddrs), 0);

  Status = LocateI2cEepromDevice (&I2cIo);
  if (!EFI_ERROR (Status)) {
    Status = I2cEepromReadData (I2cIo, FruMemAddr, FRU_SIZE, FruBuf);
    if (Status == EFI_SUCCESS) {
      for (Idx = 0; Idx < sizeof FruMacAddrs / sizeof FruMacAddrs[0]; ++Idx) {
        BaikalFruGetMacAddr(FruBuf, FRU_SIZE, Idx, &FruMacAddrs[Idx]);
      }
    }
  } else {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FRU EEPROM device, Status: %r\n", __FUNCTION__, Status));
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

    FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "status", (CONST VOID **) &Reg, &RegSize);
    if (EFI_ERROR (FdtStatus) || AsciiStrCmp ((CONST CHAR8 *)Reg, "okay") != 0) {
      continue;
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

  /* Do the same for xgmac */
  for (Node = 0;;) {
    EFI_STATUS     FdtStatus;
    CONST UINT64  *Reg;
    UINT32         RegSize;

    FdtStatus = FdtClient->FindNextCompatibleNode (FdtClient, "amd,xgbe-seattle-v1a", Node, &Node);

    if (EFI_ERROR (FdtStatus)) {
      break;
    }

    FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "status", (CONST VOID **) &Reg, &RegSize);
    if (EFI_ERROR (FdtStatus) || AsciiStrCmp ((CONST CHAR8 *)Reg, "okay") != 0) {
      continue;
    }

    FdtStatus = FdtClient->GetNodeProperty (FdtClient, Node, "reg", (CONST VOID **) &Reg, &RegSize);
    if (FdtStatus == EFI_SUCCESS && (RegSize % 16) == 0) {
      BOOLEAN              DtMacAddrSetRequest = FALSE;
      VOID * CONST         XGmacRegs = (VOID *) SwapBytes64 (Reg[0]);
      EFI_MAC_ADDRESS      MacAddr;
      CONST UINT8         *RegByte;

      DEBUG ((EFI_D_NET, "%a: XGMAC@%p found\n", __FUNCTION__, XGmacRegs));

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

          MacAddrRegVal   = (*(UINT32 *)((EFI_PHYSICAL_ADDRESS)XGmacRegs + 0x300) & 0xffff);
          MacAddrRegVal <<= 32;
          MacAddrRegVal  |= (*(UINT32 *)((EFI_PHYSICAL_ADDRESS)XGmacRegs + 0x304));

          MacAddr.Addr[0] = (MacAddrRegVal >>  0) & 0xff;
          MacAddr.Addr[1] = (MacAddrRegVal >>  8) & 0xff;
          MacAddr.Addr[2] = (MacAddrRegVal >> 16) & 0xff;
          MacAddr.Addr[3] = (MacAddrRegVal >> 24) & 0xff;
          MacAddr.Addr[4] = (MacAddrRegVal >> 32) & 0xff;
          MacAddr.Addr[5] = (MacAddrRegVal >> 40) & 0xff;

          Status = IsValidMacAddr(&MacAddr);
          if (EFI_ERROR (Status)) {
            MacAddr.Addr[0] = 0x4c;
            MacAddr.Addr[1] = 0xa5;
            MacAddr.Addr[2] = 0x15;
            MacAddr.Addr[3] = 0x01;
            MacAddr.Addr[4] = 0x02;
            MacAddr.Addr[5] = DevIdx;
          }
        }

        if (DtMacAddrSetRequest) {
          UINT32  MacAddrHi, MacAddrLo;
          FdtStatus = FdtClient->SetNodeProperty (FdtClient, Node, "local-mac-address", MacAddr.Addr, 6);
          if (EFI_ERROR (FdtStatus)) {
            DEBUG ((EFI_D_ERROR, "%a: unable to set 'local-mac-address' FDT node property, Status: %r\n", __FUNCTION__, Status));
          }

          FdtStatus = FdtClient->SetNodeProperty (FdtClient, Node, "mac-address", MacAddr.Addr, 6);
          if (EFI_ERROR (FdtStatus)) {
            DEBUG ((EFI_D_ERROR, "%a: unable to set 'mac-address' FDT node property, Status: %r\n", __FUNCTION__, Status));
          }
          MacAddrHi = MacAddr.Addr[4] | (MacAddr.Addr[5] << 8);
          MacAddrLo = MacAddr.Addr[0] | (MacAddr.Addr[1] << 8) | (MacAddr.Addr[2] << 16) | (MacAddr.Addr[3] << 24);
          *(UINT32 *)((EFI_PHYSICAL_ADDRESS)XGmacRegs + 0x300) = MacAddrHi;
          *(UINT32 *)((EFI_PHYSICAL_ADDRESS)XGmacRegs + 0x304) = MacAddrLo;
        }
      }

      DEBUG ((
        EFI_D_NET,
        "%a: XGMAC@%p MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
        __FUNCTION__,
        XGmacRegs,
        MacAddr.Addr[0],
        MacAddr.Addr[1],
        MacAddr.Addr[2],
        MacAddr.Addr[3],
        MacAddr.Addr[4],
        MacAddr.Addr[5]
        ));

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
