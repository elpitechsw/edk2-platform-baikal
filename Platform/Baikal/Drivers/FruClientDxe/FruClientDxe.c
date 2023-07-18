/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DwI2cLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>
#include <Protocol/FruClient.h>
#include "FruInternals.h"

#define EEPROM_SIZE  4096

#define MULTIRECORD_TYPEID_MAC0  0xC0
#define MULTIRECORD_TYPEID_MAC1  0xC6

STATIC
UINTN
EFIAPI
FruClientGetBoardMfgDateTime (
  VOID
  );

STATIC
UINTN
EFIAPI
FruClientReadBoardManufacturer (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadBoardName (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadBoardSerialNumber (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadBoardPartNumber (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadBoardFileId (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadProductManufacturer (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadProductName (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadProductPartNumber (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadProductVersion (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadProductSerialNumber (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadProductAssetTag (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
UINTN
EFIAPI
FruClientReadProductFileId (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC
EFI_STATUS
EFIAPI
FruClientGetMultirecordMacAddr (
  IN   CONST UINTN              MacAddrIdx,
  OUT  EFI_MAC_ADDRESS * CONST  MacAddr
  );

STATIC
UINTN
FruReadTypLenEncBoardData (
  IN   CONST UINTN    FieldIdx,
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN   DstBufSize
  );

STATIC
UINTN
FruReadTypLenEncProductData (
  IN   CONST UINTN    FieldIdx,
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  );

STATIC UINTN   mFruAddr;
STATIC UINT8  *mFruBuf;
STATIC UINTN   mFruBufSize;

EFI_STATUS
EFIAPI
FruClientDxeInitialize (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  FDT_CLIENT_PROTOCOL   *FdtClient;
  CONST UINT16           FruMemAddr = 0;
  EFI_PHYSICAL_ADDRESS   I2cBase = 0;
  UINTN                  I2cIclk = 0;
  INTN                   I2cRxedSize;
  INT32                  Node;
  CONST VOID            *Prop;
  UINT32                 PropSize;
  EFI_STATUS             Status;

  STATIC FRU_CLIENT_PROTOCOL  mFruClientProtocol = {
    FruClientGetBoardMfgDateTime,
    FruClientReadBoardManufacturer,
    FruClientReadBoardName,
    FruClientReadBoardSerialNumber,
    FruClientReadBoardPartNumber,
    FruClientReadBoardFileId,
    FruClientReadProductManufacturer,
    FruClientReadProductName,
    FruClientReadProductPartNumber,
    FruClientReadProductVersion,
    FruClientReadProductSerialNumber,
    FruClientReadProductAssetTag,
    FruClientReadProductFileId,
    FruClientGetMultirecordMacAddr
  };

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to locate FdtClientProtocol, Status: %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  if (FdtClient->FindCompatibleNode (FdtClient, "atmel,24c32", &Node) == EFI_SUCCESS &&
      FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
      PropSize == sizeof (UINT32)) {
    mFruAddr = SwapBytes32 (*(CONST UINT32 *) Prop);
    if (mFruAddr >= 0x50 && mFruAddr <= 0x57 &&
        FdtClient->FindParentNode  (FdtClient, Node, &Node) == EFI_SUCCESS &&
        FdtClient->IsNodeEnabled   (FdtClient, Node) &&
        FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == 2 * sizeof (UINT64)) {
      I2cBase = SwapBytes64 (((CONST UINT64 *) Prop)[0]);

      if (FdtClient->GetNodeProperty (FdtClient, Node, "clocks", &Prop, &PropSize) == EFI_SUCCESS && PropSize == sizeof (UINT32) &&
          FdtClient->FindNodeByPhandle (FdtClient, SwapBytes32 (*(CONST UINT32 *) Prop), &Node) == EFI_SUCCESS &&
          FdtClient->GetNodeProperty (FdtClient, Node, "clock-frequency", &Prop, &PropSize) == EFI_SUCCESS && PropSize == sizeof (UINT32)) {
        I2cIclk = SwapBytes32 (*(CONST UINT32 *) Prop);
      }
    } else {
      mFruAddr = 0;
    }
  }

  Status = gBS->AllocatePool (EfiBootServicesData, EEPROM_SIZE, (VOID **) &mFruBuf);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to allocate mFruBuf, Status: %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  if (I2cBase && I2cIclk && mFruAddr) {
    I2cRxedSize = I2cTxRx (
                    I2cBase,
                    I2cIclk,
                    mFruAddr,
                    (UINT8 *) &FruMemAddr,
                    sizeof (FruMemAddr),
                    mFruBuf,
                    EEPROM_SIZE
                    );

    if (I2cRxedSize == EEPROM_SIZE) {
      mFruBufSize = I2cRxedSize;
    }
  }

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gFruClientProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mFruClientProtocol
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to install FruClientProtocol, Status: %r\n",
      __func__,
      Status
      ));
    return Status;
  }

#if !defined(MDEPKG_NDEBUG)
  if (mFruBufSize == EEPROM_SIZE) {
    UINTN            Idx;
    EFI_MAC_ADDRESS  MacAddr;
    UINTN            RetVal;
    CHAR8            Str[FRU_TYPLENSTR_MAX_SIZE];

    RetVal = mFruClientProtocol.GetBoardMfgDateTime ();
    if (RetVal > 0) {
      EFI_TIME  EfiTime;
      UINT32    EpochSeconds;

      EfiTime.Year   = 1996;
      EfiTime.Month  = 1;
      EfiTime.Day    = 1;
      EfiTime.Hour   = 0;
      EfiTime.Minute = 0;
      EfiTime.Second = 0;

      EpochSeconds = EfiTimeToEpoch (&EfiTime);
      EpochSeconds += RetVal * 60;
      EpochToEfiTime (EpochSeconds, &EfiTime);

      DEBUG ((
        EFI_D_INFO,
        "FRU board mfg.date/time:   %04u-%02u-%02u %02u:%02u\n",
        EfiTime.Year,
        EfiTime.Month,
        EfiTime.Day,
        EfiTime.Hour,
        EfiTime.Minute
        ));
    }

    RetVal = mFruClientProtocol.ReadBoardManufacturer (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU board manufacturer:    %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadBoardName (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU board name:            %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadBoardSerialNumber (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU board serial number:   %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadBoardPartNumber (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU board part number:     %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadBoardFileId (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU board file ID:         %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadProductManufacturer (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU product manufacturer:  %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadProductName (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU product name:          %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadProductPartNumber (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU product part number:   %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadProductVersion (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU product version:       %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadProductSerialNumber (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU product serial number: %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadProductAssetTag (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU product asset tag:     %a\n", Str));
    }

    RetVal = mFruClientProtocol.ReadProductFileId (Str, sizeof (Str));
    if (RetVal > 0) {
      DEBUG ((EFI_D_INFO, "FRU product file ID:       %a\n", Str));
    }

    for (Idx = 0; Idx < 2; ++Idx) {
      gBS->SetMem (&MacAddr, sizeof (MacAddr), 0);
      Status = mFruClientProtocol.GetMultirecordMacAddr (Idx, &MacAddr);
      if (Status == EFI_SUCCESS) {
        DEBUG ((
          EFI_D_INFO,
          "FRU MAC address #%u:        %02x:%02x:%02x:%02x:%02x:%02x\n",
          Idx,
          MacAddr.Addr[0],
          MacAddr.Addr[1],
          MacAddr.Addr[2],
          MacAddr.Addr[3],
          MacAddr.Addr[4],
          MacAddr.Addr[5]
          ));
      }
    }
  }
#endif

  return EFI_SUCCESS;
}

STATIC
UINTN
EFIAPI
FruClientGetBoardMfgDateTime (
  VOID
  )
{
  CONST UINT8  *BoardArea;
  UINTN         BoardAreaSize;
  EFI_STATUS    Status;

  if (mFruBufSize != EEPROM_SIZE) {
    return 0;
  }

  Status = FruInternalsBoardAreaLocate (
             mFruBuf,
             mFruBufSize,
             &BoardArea,
             &BoardAreaSize
             );

  if (EFI_ERROR (Status) || BoardAreaSize < 6) {
    return 0;
  }

  return (BoardArea[3] <<  0) |
         (BoardArea[4] <<  8) |
         (BoardArea[5] << 16);
}

STATIC
UINTN
EFIAPI
FruClientReadBoardManufacturer (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncBoardData (0, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadBoardName (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncBoardData (1, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadBoardSerialNumber (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncBoardData (2, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadBoardPartNumber (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncBoardData (3, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadBoardFileId (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncBoardData (4, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadProductManufacturer (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncProductData (0, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadProductName (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncProductData (1, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadProductPartNumber (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncProductData (2, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadProductVersion (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncProductData (3, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadProductSerialNumber (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncProductData (4, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadProductAssetTag (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncProductData (5, DstBuf, DstBufSize);
}

STATIC
UINTN
EFIAPI
FruClientReadProductFileId (
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  return FruReadTypLenEncProductData (6, DstBuf, DstBufSize);
}

STATIC
EFI_STATUS
EFIAPI
FruClientGetMultirecordMacAddr (
  IN   CONST UINTN              MacAddrIdx,
  OUT  EFI_MAC_ADDRESS * CONST  MacAddr
  )
{
  CONST UINT8         *MrecArea;
  MULTIRECORD_HEADER   MrecHdr;
  EFI_STATUS           Status;

  ASSERT (MacAddrIdx < 2);
  ASSERT (MacAddr != NULL);

  if (mFruBufSize != EEPROM_SIZE) {
    return EFI_DEVICE_ERROR;
  }

  Status = FruInternalsMultirecordAreaLocate (mFruBuf, mFruBufSize, &MrecArea);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (FruInternalsMultirecordParseHeader (
           MrecArea,
           (mFruBuf + mFruBufSize) - MrecArea,
           &MrecHdr
           ) == EFI_SUCCESS) {
    if ((MacAddrIdx == 0 && MrecHdr.TypeId == MULTIRECORD_TYPEID_MAC0) ||
        (MacAddrIdx == 1 && MrecHdr.TypeId == MULTIRECORD_TYPEID_MAC1)) {
      if (FruInternalsMultirecordCheckData (MrecArea,
                                            (mFruBuf + mFruBufSize) - MrecArea,
                                            &MrecHdr
                                            ) == EFI_SUCCESS) {
        UINTN  Idx;

        if (MrecHdr.Length == 9) {
          // OEM Record must have 3-byte Manufacturer ID field according to IPMI FRU Spec
          for (Idx = 0; Idx < 6; ++Idx) {
            MacAddr->Addr[Idx] = MrecArea[sizeof (MULTIRECORD_HEADER) + 3 + Idx];
          }

          return EFI_SUCCESS;
        } else if (MrecHdr.Length == 6) {
          STATIC BOOLEAN  WarnOnceMac[2];

          if ((MrecHdr.TypeId == MULTIRECORD_TYPEID_MAC0 && !WarnOnceMac[0]) ||
              (MrecHdr.TypeId == MULTIRECORD_TYPEID_MAC1 && !WarnOnceMac[1])) {
            // Legacy BMC FW generates incorrect OEM Records without 3-byte Manufacturer ID field
            DEBUG ((
              EFI_D_WARN,
              "%a: MrecHdr.Length:%u is deprecated for MrecHdr.TypeId:0x%02x\n",
              __func__,
              MrecHdr.Length,
              MrecHdr.TypeId
              ));

            if (MrecHdr.TypeId == MULTIRECORD_TYPEID_MAC0) {
              WarnOnceMac[0] = TRUE;
            } else if (MrecHdr.TypeId == MULTIRECORD_TYPEID_MAC1) {
              WarnOnceMac[1] = TRUE;
            }
          }

          for (Idx = 0; Idx < 6; ++Idx) {
            MacAddr->Addr[Idx] = MrecArea[sizeof (MULTIRECORD_HEADER) + Idx];
          }

          return EFI_SUCCESS;
        } else {
          DEBUG ((
            EFI_D_ERROR,
            "%a: MrecHdr.Length:%u does not match MrecHdr.TypeId:0x%02x\n",
            __func__,
            MrecHdr.Length,
            MrecHdr.TypeId
            ));
        }
      }
    }

    if (MrecHdr.Format & 0x80) {
      break;
    }

    MrecArea += sizeof (MULTIRECORD_HEADER) + MrecHdr.Length;
  }

  return EFI_INVALID_PARAMETER;
}

STATIC
UINTN
FruReadTypLenEncBoardData (
  IN   CONST UINTN    FieldIdx,
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  CONST UINT8  *BoardArea;
  UINTN         BoardAreaSize;
  UINTN         DstStrLen = DstBufSize;
  CONST UINT8  *EncData;
  UINTN         Idx;
  EFI_STATUS    Status;

  ASSERT (FieldIdx < 5);
  ASSERT (DstBuf != NULL);
  ASSERT (DstBufSize > 0);

  if (mFruBufSize != EEPROM_SIZE) {
    return 0;
  }

  Status = FruInternalsBoardAreaLocate (
             mFruBuf,
             mFruBufSize,
             &BoardArea,
             &BoardAreaSize
             );
  if (EFI_ERROR (Status) || BoardAreaSize <= 6) {
    return 0;
  }

  EncData = BoardArea + 6;
  for (Idx = 0; Idx < FieldIdx; ++Idx) {
    EncData = FruInternalsTypLenEncReadData (EncData, NULL, NULL);
    if (EncData == NULL || EncData >= BoardArea + BoardAreaSize) {
      return 0;
    }
  }

  FruInternalsTypLenEncReadData (EncData, DstBuf, &DstStrLen);
  return DstStrLen;
}

STATIC
UINTN
FruReadTypLenEncProductData (
  IN   CONST UINTN    FieldIdx,
  OUT  CHAR8 * CONST  DstBuf,
  IN   CONST UINTN    DstBufSize
  )
{
  UINTN         DstStrLen = DstBufSize;
  CONST UINT8  *EncData;
  UINTN         Idx;
  CONST UINT8  *ProductArea;
  UINTN         ProductAreaSize;
  EFI_STATUS    Status;

  ASSERT (FieldIdx < 7);
  ASSERT (DstBuf != NULL);
  ASSERT (DstBufSize > 0);

  if (mFruBufSize != EEPROM_SIZE) {
    return 0;
  }

  Status = FruInternalsProductAreaLocate (
             mFruBuf,
             mFruBufSize,
             &ProductArea,
             &ProductAreaSize
             );

  if (EFI_ERROR (Status) || ProductAreaSize <= 3) {
    return 0;
  }

  EncData = ProductArea + 3;
  for (Idx = 0; Idx < FieldIdx; ++Idx) {
    EncData = FruInternalsTypLenEncReadData (EncData, NULL, NULL);
    if (EncData == NULL || EncData >= ProductArea + ProductAreaSize) {
      return 0;
    }
  }

  FruInternalsTypLenEncReadData (EncData, DstBuf, &DstStrLen);
  return DstStrLen;
}
