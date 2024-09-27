/** @file
  Copyright (c) 2020 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>

#include <BM1000.h>

#define GSBUSCFG0                      0xC100
#define GSBUSCFG0_INCRBRSTENA          BIT0
#define GSBUSCFG0_INCR32BRSTENA        BIT4
#define GSBUSCFG0_DESWRREQINFO(Val)    ((Val & 0xF) << 16)
#define GSBUSCFG0_DATWRREQINFO(Val)    ((Val & 0xF) << 20)
#define GSBUSCFG0_DESRDREQINFO(Val)    ((Val & 0xF) << 24)
#define GSBUSCFG0_DATRDREQINFO(Val)    ((Val & 0xF) << 28)

#define GSBUSCFG1                      0xC104
#define GSBUSCFG1_PIPETRANSLIMIT(Val)  ((Val & 0xF) << 8)
#define GSBUSCFG1_EN1KPAGE             BIT12

#define GUSB3PIPECTL0                  0xC2C0
#define GUSB3PIPECTL1                  0xC2C4
#define GUSB3PIPECTL_DISRXDETP3        BIT28

STATIC
EFI_STATUS
EFIAPI
NonDiscoverableDeviceXhciInitializer (
  IN  NON_DISCOVERABLE_DEVICE  *This
  )
{
  // Set AHB-prot/AXI-cache/OCP-ReqInfo for data/descriptor read/write
  MmioAndThenOr32 (
    This->Resources->AddrRangeMin +
      GSBUSCFG0,
    ~(GSBUSCFG0_DESWRREQINFO(0xF) |
      GSBUSCFG0_DATWRREQINFO(0xF) |
      GSBUSCFG0_DESRDREQINFO(0xF) |
      GSBUSCFG0_DATRDREQINFO(0xF)),
      GSBUSCFG0_DESWRREQINFO(0x7) |
      GSBUSCFG0_DATWRREQINFO(0x7) |
      GSBUSCFG0_DESRDREQINFO(0xB) |
      GSBUSCFG0_DATRDREQINFO(0xB)
    );

  if (This->Resources->AddrRangeMin == BM1000_USB3_BASE) {
    MmioAndThenOr32 (
      This->Resources->AddrRangeMin +
        GSBUSCFG0,
       ~GSBUSCFG0_INCRBRSTENA,
        GSBUSCFG0_INCR32BRSTENA
      );

    MmioAnd32 (
      This->Resources->AddrRangeMin +
        GSBUSCFG1,
      ~(GSBUSCFG1_PIPETRANSLIMIT(0xF) |
        GSBUSCFG1_EN1KPAGE)
      );

    MmioOr32 (
      This->Resources->AddrRangeMin +
        GUSB3PIPECTL0,
        GUSB3PIPECTL_DISRXDETP3
      );

    MmioOr32 (
      This->Resources->AddrRangeMin +
        GUSB3PIPECTL1,
        GUSB3PIPECTL_DISRXDETP3
      );
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
NonDiscoverableXhciEntryPoint (
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
    Status = FdtClient->FindNextCompatibleNode (FdtClient, "snps,dwc3", Node, &Node);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == 16) {
      CONST EFI_PHYSICAL_ADDRESS  XhciBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + 0));
      CONST UINTN                 XhciSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + 1));

      Status = RegisterNonDiscoverableMmioDevice (
                 NonDiscoverableDeviceTypeXhci,
                 NonDiscoverableDeviceDmaTypeCoherent,
                 NonDiscoverableDeviceXhciInitializer,
                 NULL,
                 1,
                 XhciBase,
                 XhciSize
                 );

      if (EFI_ERROR (Status)) {
        DEBUG ((
          EFI_D_ERROR,
          "%a: unable to register @ 0x%lx, Status: %r\n",
          __func__,
          XhciBase,
          Status
          ));
      }
    }
  }

  return EFI_SUCCESS;
}
