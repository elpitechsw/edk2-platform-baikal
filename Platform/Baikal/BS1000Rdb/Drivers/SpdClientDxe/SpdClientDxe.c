/** @file
  Copyright (c) 2022 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseLib.h>
#include <Library/CrcLib.h>
#include <Library/DebugLib.h>
#include <Library/DwI2cLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>
#include <Protocol/SpdClient.h>

#include <BS1000.h>

#define DIMM_NUM     12
#define SPD_MAXSIZE  512

#define SPD_SPA0     0x36
#define SPD_SPA1     0x37

STATIC
CONST VOID *
EFIAPI
SpdClientGetData (
  IN  CONST UINTN  DimmIdx
  );

STATIC
UINTN
EFIAPI
SpdClientGetMaxSize (
  VOID
  );

STATIC UINT8  SpdData[DIMM_NUM][SPD_MAXSIZE];

EFI_STATUS
EFIAPI
SpdClientDxeInitialize (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN                 DimmIdx;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  EFI_STATUS            Status;

  STATIC SPD_CLIENT_PROTOCOL  mSpdClientProtocol = {
    SpdClientGetData,
    SpdClientGetMaxSize
  };

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to locate FdtClientProtocol, Status: %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gSpdClientProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSpdClientProtocol
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to install SpdClientProtocol, Status: %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  for (DimmIdx = 0; DimmIdx < ARRAY_SIZE (SpdData); ++DimmIdx) {
    UINT8                        *Buf  = SpdData[DimmIdx];
    CONST  EFI_PHYSICAL_ADDRESS   I2cBase = DimmIdx < 6 ? BS1000_I2C2_BASE : BS1000_I2C3_BASE;
    UINTN                         I2cIclk;
    INT32                         Node = 0;
    INTN                          RxSize;
    CONST  UINTN                  SpdAddr = 0x50 + DimmIdx % 6;
    UINT8                         StartAddr = 0;

    for (;;) {
      CONST VOID  *Prop;
      UINT32       PropSize;

      if (FdtClient->FindNextCompatibleNode (FdtClient, "snps,designware-i2c", Node, &Node) != EFI_SUCCESS) {
        return EFI_DEVICE_ERROR;
      }

      if (FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS && PropSize == 2 * sizeof (UINT64) &&
          SwapBytes64 (((CONST UINT64 *) Prop)[0]) == I2cBase &&
          FdtClient->GetNodeProperty (FdtClient, Node, "clocks", &Prop, &PropSize) == EFI_SUCCESS && PropSize == sizeof (UINT32) &&
          FdtClient->FindNodeByPhandle (FdtClient, SwapBytes32 (*(CONST UINT32 *) Prop), &Node) == EFI_SUCCESS &&
          FdtClient->GetNodeProperty (FdtClient, Node, "clock-frequency", &Prop, &PropSize) == EFI_SUCCESS && PropSize == sizeof (UINT32)) {
        I2cIclk = SwapBytes32 (*(CONST UINT32 *) Prop);
        break;
      }
    }

    gBS->SetMem (Buf, sizeof (SpdData[0]), 0xFF);

    I2cTxRx (I2cBase, I2cIclk, SPD_SPA0, &StartAddr, sizeof (StartAddr), NULL, 0);
    RxSize = I2cTxRx (I2cBase, I2cIclk, SpdAddr, &StartAddr, sizeof (StartAddr), Buf, 128);

    if (RxSize == 128 &&
        Crc16 (Buf, 126, 0) == ((Buf[127] << 8) | Buf[126])) {
      CONST UINTN  BytesUsed = Buf[0] & 0xF;

      if (BytesUsed > 1 && BytesUsed < 5) {
        Buf += RxSize;
        StartAddr += RxSize;
        RxSize = I2cTxRx (I2cBase, I2cIclk, SpdAddr, &StartAddr, sizeof (StartAddr), Buf, 128);

        if (RxSize == 128 && BytesUsed > 2) {
          Buf += RxSize;
          StartAddr = 0;
          I2cTxRx (I2cBase, I2cIclk, SPD_SPA1, &StartAddr, sizeof (StartAddr), NULL, 0);
          I2cTxRx (I2cBase, I2cIclk, SpdAddr,  &StartAddr, sizeof (StartAddr), Buf, BytesUsed == 3 ? 128 : 256);
          I2cTxRx (I2cBase, I2cIclk, SPD_SPA0, &StartAddr, sizeof (StartAddr), NULL, 0);
        }
      }
    }
  }

  return EFI_SUCCESS;
}

STATIC
CONST VOID *
EFIAPI
SpdClientGetData (
  IN  CONST UINTN  DimmIdx
  )
{
  if (DimmIdx >= ARRAY_SIZE (SpdData)) {
    return NULL;
  }

  return SpdData[DimmIdx];
}

STATIC
UINTN
EFIAPI
SpdClientGetMaxSize (
  VOID
  )
{
  return sizeof (SpdData[0]);
}
