/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Pci22.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/GpioLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PciHostBridgeLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Platform/ConfigVars.h>
#include <Protocol/FdtClient.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <Protocol/PciIo.h>

#include <BM1000.h>

#define BM1000_PCIE_GPR_RESET_REG(PcieIdx)       (BM1000_PCIE_GPR_BASE + (PcieIdx) * 0x20 + 0x00)
#define BM1000_PCIE_GPR_RESET_PHY_RESET          BIT0
#define BM1000_PCIE_GPR_RESET_PIPE0_RESET        BIT4
#define BM1000_PCIE_GPR_RESET_PIPE1_RESET        BIT5
#define BM1000_PCIE_GPR_RESET_CORE_RST           BIT8
#define BM1000_PCIE_GPR_RESET_PWR_RST            BIT9
#define BM1000_PCIE_GPR_RESET_STICKY_RST         BIT10
#define BM1000_PCIE_GPR_RESET_NONSTICKY_RST      BIT11
#define BM1000_PCIE_GPR_RESET_HOT_RESET          BIT12
#define BM1000_PCIE_GPR_RESET_ADB_PWRDWN         BIT13

#define BM1000_PCIE_GPR_GENCTL_REG(PcieIdx)      (BM1000_PCIE_GPR_BASE + (PcieIdx) * 0x20 + 0x08)
#define BM1000_PCIE_GPR_GENCTL_LTSSM_EN          BIT1

#define BM1000_PCIE_GPR_POWERCTL_REG(PcieIdx)    (BM1000_PCIE_GPR_BASE + (PcieIdx) * 0x20 + 0x10)

#define BM1000_PCIE_GPR_MSI_TRANS_CTL2_REG                   (BM1000_PCIE_GPR_BASE + 0xF8)
#define BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE0_MSI_TRANS_EN    BIT9
#define BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE1_MSI_TRANS_EN    BIT10
#define BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE2_MSI_TRANS_EN    BIT11
#define BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE0_MASK  (3 << 0)
#define BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE1_MASK  (3 << 2)
#define BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE2_MASK  (3 << 4)

#define BM1000_PCIE_PF0_TYPE1_HDR_TYPE1_CLASS_CODE_REV_ID_REG                        0x008
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG                               0x07C
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_BITS                (0x3F << 4)
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_SHIFT               4

#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG                        0x080
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_RETRAIN_LINK           BIT5
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_BITS        (0xF << 16)
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_SHIFT       16
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_BITS   (0x3F << 20)
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_SHIFT  20

#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG                      0x0A0
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG_TRGT_LNK_SPEED_BITS  0xF

#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_FORCE_OFF                                    0x708
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_FORCE_OFF_FORCE_EN                           BIT15

#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF                                0x710
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_BITS              (0x3F << 16)
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X1                (0x01 << 16)
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X2                (0x03 << 16)
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X4                (0x07 << 16)
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X8                (0x0F << 16)
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X16               (0x1F << 16)

#define BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF                                     0x80C
#define BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_BITS                   (0x1F << 8)
#define BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_SHIFT                  8

#define BM1000_PCIE_PF0_PORT_LOGIC_GEN3_RELATED_OFF                                  0x890
#define BM1000_PCIE_PF0_PORT_LOGIC_GEN3_RELATED_OFF_GEN3_EQUALIZATION_DISABLE        BIT16

#define BM1000_PCIE_PF0_PORT_LOGIC_PIPE_LOOPBACK_CONTROL_OFF                         0x8B8
#define BM1000_PCIE_PF0_PORT_LOGIC_PIPE_LOOPBACK_CONTROL_OFF_PIPE_LOOPBACK           BIT31

#define BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF                                0x8BC
#define BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN                   BIT0

#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_VIEWPORT_OFF                                 0x900
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_VIEWPORT_OFF_REGION_DIR_INBOUND              BIT31
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_VIEWPORT_OFF_REGION_DIR_OUTBOUND             0

#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0                 0x904
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM        0
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_IO         2
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG0       4
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG1       5

#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0                 0x908
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE  BIT28
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_REGION_EN       BIT31

#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0                 0x90C
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0               0x910
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_LIMIT_ADDR_OFF_OUTBOUND_0                    0x914
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0               0x918
#define BM1000_PCIE_PF0_PORT_LOGIC_IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0             0x91C

#define RANGES_FLAG_IO   0x01000000
#define RANGES_FLAG_MEM  0x02000000

#pragma pack(1)
typedef struct {
  ACPI_HID_DEVICE_PATH      AcpiDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  EndDevicePath;
} EFI_PCI_ROOT_BRIDGE_DEVICE_PATH;
#pragma pack ()

STATIC CONST EFI_PCI_ROOT_BRIDGE_DEVICE_PATH  mEfiPciRootBridgeDevicePaths[] = {
  {
    {
      {
        ACPI_DEVICE_PATH,
        ACPI_DP,
        {
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH)),
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH) >> 8)
        }
      },
      EISA_PNP_ID (0x0A08), // PCI Express
      0
    },
    {END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, {END_DEVICE_PATH_LENGTH, 0}}
  },
  {
    {
      {
        ACPI_DEVICE_PATH,
        ACPI_DP,
        {
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH)),
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH) >> 8)
        }
      },
      EISA_PNP_ID (0x0A08), // PCI Express
      1
    },
    {END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, {END_DEVICE_PATH_LENGTH, 0}}
  },
  {
    {
      {
        ACPI_DEVICE_PATH,
        ACPI_DP,
        {
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH)),
          (UINT8)(sizeof (ACPI_HID_DEVICE_PATH) >> 8)
        }
      },
      EISA_PNP_ID (0x0A08), // PCI Express
      2
    },
    {END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, {END_DEVICE_PATH_LENGTH, 0}}
  }
};

CONST EFI_PHYSICAL_ADDRESS  mPcieDbiBases[] = {
  BM1000_PCIE0_DBI_BASE,
  BM1000_PCIE1_DBI_BASE,
  BM1000_PCIE2_DBI_BASE
};

STATIC CONST UINTN  mPcieDbiSizes[] = {
  BM1000_PCIE0_DBI_SIZE,
  BM1000_PCIE0_DBI_SIZE,
  BM1000_PCIE1_DBI_SIZE
};

UINT32                        mPcieCfg0FilteringWorks;
EFI_PHYSICAL_ADDRESS          mPcieCfgBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC PCI_ROOT_BRIDGE       *mPcieRootBridges;
STATIC UINTN                  mPcieRootBridgesNum;

STATIC_ASSERT (
  ARRAY_SIZE (mPcieDbiBases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieDbiBases) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieDbiSizes) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieDbiSizes) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );

STATIC
VOID
PciHostBridgeLibCfgWindow (
  IN  CONST EFI_PHYSICAL_ADDRESS  PcieDbiBase,
  IN  CONST UINTN                 RegionIdx,
  IN  CONST UINT64                CpuBase,
  IN  CONST UINT64                PciBase,
  IN  CONST UINT64                Size,
  IN  CONST UINTN                 Type,
  IN  CONST UINTN                 EnableFlags
  )
{
  ASSERT (RegionIdx <= 3);
  ASSERT (Type <= 0x1F);
  ASSERT (Size >= SIZE_64KB);
  ASSERT (Size <= SIZE_4GB);

  // The addresses must be aligned to 64 KiB
  ASSERT ((CpuBase & (SIZE_64KB - 1)) == 0);
  ASSERT ((PciBase & (SIZE_64KB - 1)) == 0);
  ASSERT ((Size    & (SIZE_64KB - 1)) == 0);

  ArmDataMemoryBarrier ();
  MmioWrite32 (
    PcieDbiBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_VIEWPORT_OFF,
    BM1000_PCIE_PF0_PORT_LOGIC_IATU_VIEWPORT_OFF_REGION_DIR_OUTBOUND | RegionIdx
    );

  MmioWrite32 (
    PcieDbiBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0,
    (UINT32)(CpuBase & 0xFFFFFFFF)
    );

  MmioWrite32 (
    PcieDbiBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0,
    (UINT32)(CpuBase >> 32)
    );

  MmioWrite32 (
    PcieDbiBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_LIMIT_ADDR_OFF_OUTBOUND_0,
    (UINT32)(CpuBase + Size - 1)
    );

  MmioWrite32 (
    PcieDbiBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0,
    (UINT32)(PciBase & 0xFFFFFFFF)
    );

  MmioWrite32 (
    PcieDbiBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0,
    (UINT32)(PciBase >> 32)
    );

  MmioWrite32 (
    PcieDbiBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0,
    Type
    );

  MmioWrite32 (
    PcieDbiBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0,
    BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_REGION_EN | EnableFlags
    );
}

STATIC
VOID
EFIAPI
PciHostBridgeLibExitBootServices (
  IN  EFI_EVENT   Event,
  IN  VOID       *Context
  )
{
  EFI_HANDLE           *HandleBuffer;
  UINTN                 HandleCount;
  UINTN                 Iter;
  PCI_TYPE01            PciConfigHeader;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  EFI_STATUS            Status;

  //
  // Linux will use EFI PciIo protocol to disable bus mastering (unless
  // efi=enable_pci_busmaster is used) on all PCI bridges. However, in ECAM
  // mode, the RP isn't actually visible to Linux, and that will break DMA for
  // the visible device (symptom: cannot boot Linux for PCIe attached SSD).
  // Thus, if we are requested to boot in ECAM mode, restore BM for RPs.
  // Moreover, if we have any cards with bridges, since these are
  // "flattened out" where each device appears on a separate segment,
  // make sure these are enabled as well.
  //

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  for (Iter = 0; Iter < HandleCount; ++Iter) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Iter],
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    PciIo->Pci.Read (
                 PciIo,
                 EfiPciIoWidthUint32,
                 0,
                 sizeof (PciConfigHeader) / sizeof (UINT32),
                 &PciConfigHeader
                 );
    if (IS_PCI_P2P (&PciConfigHeader) || IS_PCI_P2P_SUB (&PciConfigHeader)) {
      continue;
    }

    Status = PciIo->Attributes (
                      PciIo,
                      EfiPciIoAttributeOperationEnable,
                      EFI_PCI_DEVICE_ENABLE,
                      NULL
                      );
  }

  gBS->FreePool (HandleBuffer);
}

EFI_STATUS
EFIAPI
PciHostBridgeLibConstructor (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_EVENT             Event;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  UINTN                 Iter;
  INT32                 Node;
  UINTN                 PcieIdx;
  UINTN                 PcieNumLanes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  INTN                  PciePerstGpios[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  INTN                  PciePerstPolarity[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  UINTN                 PcieMsiDevs[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  UINTN                 PcieCfgSizes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  UINTN                 PcieIdxs[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  EFI_PHYSICAL_ADDRESS  PcieIoBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  EFI_PHYSICAL_ADDRESS  PcieIoMins[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  UINTN                 PcieIoSizes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  EFI_PHYSICAL_ADDRESS  PcieMemBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  EFI_PHYSICAL_ADDRESS  PcieMemMins[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  UINTN                 PcieMemSizes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  INTN                  PcieX8PrsntGpio;
  INTN                  PcieX8PrsntPolarity;
  EFI_STATUS            Status;
  PCI_ROOT_BRIDGE      *lPcieRootBridge;
  CONST VOID  *Prop;
  UINT32       PropSize;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);

  // Acquire PCIe RC related data from FDT
  for (Node = 0, Iter = 0; Iter < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths);) {
    INTN         CfgRegIdx = -1, DbiRegIdx = -1;

    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bm1000-pcie", Node, &Node);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
      continue;
    }

    Status = FdtClient->GetNodeProperty (FdtClient, Node, "baikal,pcie-lcru", &Prop, &PropSize);
    if (EFI_ERROR (Status) || PropSize != 8) {
      continue;
    }

    PcieIdx = SwapBytes32 (((CONST UINT32 *)Prop)[1]);
    if (PcieIdx >= ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)) {
      PcieIdxs[Iter] = 0;
      continue;
    }

    PcieIdxs[Iter] = PcieIdx;

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reg-names", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize > 0) {
      UINTN         RegIdx;
      CONST CHAR8  *StrPtr = Prop;

      for (RegIdx = 0; PropSize > 0; ++RegIdx) {
        CONST UINTN  StrSize = AsciiStrSize (StrPtr);

        ASSERT (StrSize <= PropSize);

        if (AsciiStrCmp (StrPtr, "dbi") == 0) {
          DbiRegIdx = RegIdx;
        } else if (AsciiStrCmp (StrPtr, "config") == 0) {
          CfgRegIdx = RegIdx;
        }

        PropSize -= StrSize;
        StrPtr   += StrSize;
      }

      if (CfgRegIdx == -1 || DbiRegIdx == -1) {
        continue;
      }
    } else {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize >= 32 && (PropSize % 16) == 0) {
#if !defined(MDEPKG_NDEBUG)
      CONST EFI_PHYSICAL_ADDRESS  DbiBase = SwapBytes64 (((CONST UINT64 *) Prop)[DbiRegIdx * 2]);
      CONST UINTN                 DbiSize = SwapBytes64 (((CONST UINT64 *) Prop)[DbiRegIdx * 2 + 1]);
#endif
      CONST EFI_PHYSICAL_ADDRESS  CfgBase = SwapBytes64 (((CONST UINT64 *) Prop)[CfgRegIdx * 2]);
      CONST UINTN                 CfgSize = SwapBytes64 (((CONST UINT64 *) Prop)[CfgRegIdx * 2 + 1]);

      ASSERT (DbiBase == mPcieDbiBases[PcieIdx]);
      ASSERT (DbiSize == mPcieDbiSizes[PcieIdx]);
      ASSERT (CfgSize >  SIZE_2MB);
      ASSERT ((CfgSize & (SIZE_1MB - 1)) == 0);

      PcieIdxs[Iter] = PcieIdx;
      mPcieCfgBases[PcieIdx] = CfgBase;
      PcieCfgSizes[PcieIdx] = CfgSize;
    } else {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "ranges", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize > 0 && (PropSize % 28) == 0) {
      UINTN  Entry;

      PcieIoMins[PcieIdx]  = MAX_UINT64;
      PcieMemMins[PcieIdx] = MAX_UINT64;

      for (Entry = 0; Entry < PropSize / 28; ++Entry) {
        CONST UINTN           Flags = SwapBytes32 (((CONST UINT32 *) Prop)[0]);
        EFI_PHYSICAL_ADDRESS  PciBase;
        EFI_PHYSICAL_ADDRESS  CpuBase;
        UINTN                 Size;

        PciBase   = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
        PciBase <<= 32;
        PciBase  |= SwapBytes32 (((CONST UINT32 *) Prop)[2]);

        CpuBase   = SwapBytes32 (((CONST UINT32 *) Prop)[3]);
        CpuBase <<= 32;
        CpuBase  |= SwapBytes32 (((CONST UINT32 *) Prop)[4]);

        Size      = SwapBytes32 (((CONST UINT32 *) Prop)[5]);
        Size    <<= 32;
        Size     |= SwapBytes32 (((CONST UINT32 *) Prop)[6]);

        if (Flags & RANGES_FLAG_IO) {
          PcieIoMins[PcieIdx]  = PciBase;
          PcieIoBases[PcieIdx] = CpuBase;
          PcieIoSizes[PcieIdx] = Size;
        } else if (Flags & RANGES_FLAG_MEM) {
          PcieMemMins[PcieIdx]  = PciBase;
          PcieMemBases[PcieIdx] = CpuBase;
          PcieMemSizes[PcieIdx] = Size;
        }

        Prop = &(((CONST UINT32 *) Prop)[7]);
      }
    } else {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "num-lanes", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == 4) {
      PcieNumLanes[PcieIdx] = SwapBytes32 (((CONST UINT32 *) Prop)[0]);
    } else {
      PcieNumLanes[PcieIdx] = 0;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reset-gpios", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == 12) {
      PciePerstGpios[PcieIdx]    = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
      PciePerstPolarity[PcieIdx] = SwapBytes32 (((CONST UINT32 *) Prop)[2]);

      if (PciePerstPolarity[PcieIdx] > 1) {
        PciePerstPolarity[PcieIdx] = -1;
      }
    } else {
      PciePerstGpios[PcieIdx] = -1;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "msi-map", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == 16) {
      PcieMsiDevs[PcieIdx]    = SwapBytes32 (((CONST UINT32 *) Prop)[2]) >> 16;
    } else {
      PcieMsiDevs[PcieIdx] = 0;
    }

    ++mPcieRootBridgesNum;
    ++Iter;
  }

  if (mPcieRootBridgesNum == 0) {
    return EFI_SUCCESS;
  }

  //
  // Disable MSI translations (they are disabled after reset).
  // This allows PCIe devices to target GICD_NS_SETSPI,
  // exposed via a V2M frame MADT descriptor for Windows on Arm.
  //
  MmioAnd32 (
      BM1000_PCIE_GPR_MSI_TRANS_CTL2_REG,
    ~(BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE0_MASK |
      BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE1_MASK |
      BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE2_MASK |
      BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE0_MSI_TRANS_EN   |
      BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE1_MSI_TRANS_EN   |
      BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE2_MSI_TRANS_EN)
    );

  mPcieRootBridges = AllocateZeroPool (mPcieRootBridgesNum * sizeof (PCI_ROOT_BRIDGE));
  lPcieRootBridge  = &mPcieRootBridges[0];

  INT32  SubNode = 0;

  PcieX8PrsntGpio     = -1;
  PcieX8PrsntPolarity = -1;

  Status = FdtClient->FindNextCompatibleNode (FdtClient, "snps,dw-apb-gpio-port", SubNode, &SubNode);
  if (Status == EFI_SUCCESS) {
    Status = FdtClient->FindNextSubnode (FdtClient, "pcieclk", SubNode, &SubNode);
    if (Status == EFI_SUCCESS) {
      Status = FdtClient->GetNodeProperty (FdtClient, SubNode, "line-name", &Prop, &PropSize);
      if (Status == EFI_SUCCESS && AsciiStrCmp ((CONST CHAR8 *) Prop, "pcie-x8-clock") == 0) {
        Status = FdtClient->GetNodeProperty (FdtClient, SubNode, "gpios", &Prop, &PropSize);
        if (Status == EFI_SUCCESS && PropSize == 8) {
          PcieX8PrsntGpio     = SwapBytes32 (((CONST UINT32 *) Prop)[0]);
          PcieX8PrsntPolarity = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
        }
      }
    }
  }

  // Assert PRSNT pin
  if (PcieX8PrsntGpio >= 0 &&
      PcieX8PrsntGpio <= 31 &&
      PcieX8PrsntPolarity >= 0) {
    if (PcieX8PrsntPolarity) {
      GpioOutRst (BM1000_GPIO32_BASE, PcieX8PrsntGpio);
    } else {
      GpioOutSet (BM1000_GPIO32_BASE, PcieX8PrsntGpio);
    }

    GpioDirSet (BM1000_GPIO32_BASE, PcieX8PrsntGpio);
  }

  // Initialise PCIe RCs
  for (Iter = 0; Iter < mPcieRootBridgesNum; ++Iter, ++lPcieRootBridge) {
    UINT32   ResetMask;
    BOOLEAN  ComponentExists = FALSE;
    UINTN    PciePortLinkCapableLanesVal;
    UINT64   TimeStart;

    PcieIdx = PcieIdxs[Iter];

    lPcieRootBridge->Segment    = PcieIdx;
    lPcieRootBridge->Supports   = 0;
    lPcieRootBridge->Attributes = lPcieRootBridge->Supports;
    lPcieRootBridge->DmaAbove4G = TRUE;
    lPcieRootBridge->NoExtendedConfigSpace = FALSE;
    lPcieRootBridge->ResourceAssigned      = FALSE;
    lPcieRootBridge->AllocationAttributes  = EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM;

    lPcieRootBridge->Bus.Base          = 0;
    lPcieRootBridge->Bus.Limit         = PcieCfgSizes[PcieIdx] / SIZE_1MB - 1;
    lPcieRootBridge->Io.Base           = PcieIoMins[PcieIdx];
    lPcieRootBridge->Io.Limit          = PcieIoMins[PcieIdx] + PcieIoSizes[PcieIdx] - 1;
    lPcieRootBridge->Io.Translation    = PcieIoMins[PcieIdx] - PcieIoBases[PcieIdx];
    lPcieRootBridge->Mem.Base          = PcieMemMins[PcieIdx];
    lPcieRootBridge->Mem.Limit         = PcieMemMins[PcieIdx] + PcieMemSizes[PcieIdx] - 1;
    lPcieRootBridge->Mem.Translation   = PcieMemMins[PcieIdx] - PcieMemBases[PcieIdx];
    lPcieRootBridge->MemAbove4G.Base   = MAX_UINT64;
    lPcieRootBridge->MemAbove4G.Limit  = 0;
    lPcieRootBridge->PMem.Base         = MAX_UINT64;
    lPcieRootBridge->PMem.Limit        = 0;
    lPcieRootBridge->PMemAbove4G.Base  = MAX_UINT64;
    lPcieRootBridge->PMemAbove4G.Limit = 0;
    lPcieRootBridge->DevicePath        = (EFI_DEVICE_PATH_PROTOCOL *) &mEfiPciRootBridgeDevicePaths[PcieIdx];

    // Assert PERST pin
    if (PciePerstGpios[PcieIdx] >= 0 &&
        PciePerstGpios[PcieIdx] <= 31 &&
        PciePerstPolarity[PcieIdx] >= 0) {
      if (PciePerstPolarity[PcieIdx]) {
        GpioOutRst (BM1000_GPIO32_BASE, PciePerstGpios[PcieIdx]);
      } else {
        GpioOutSet (BM1000_GPIO32_BASE, PciePerstGpios[PcieIdx]);
      }

      GpioDirSet (BM1000_GPIO32_BASE, PciePerstGpios[PcieIdx]);
    }

    MmioAnd32 (BM1000_PCIE_GPR_GENCTL_REG (PcieIdx), ~BM1000_PCIE_GPR_GENCTL_LTSSM_EN);

    // Assert PCIe RC resets
    ResetMask =
      BM1000_PCIE_GPR_RESET_NONSTICKY_RST | BM1000_PCIE_GPR_RESET_STICKY_RST  |
      BM1000_PCIE_GPR_RESET_PWR_RST       | BM1000_PCIE_GPR_RESET_CORE_RST    |
      BM1000_PCIE_GPR_RESET_PIPE0_RESET   | BM1000_PCIE_GPR_RESET_PHY_RESET;
    if (PcieNumLanes[PcieIdx] == 8) {
      ResetMask |= BM1000_PCIE_GPR_RESET_PIPE1_RESET;
    }
    MmioOr32 (BM1000_PCIE_GPR_RESET_REG (PcieIdx), ResetMask);

    gBS->Stall (1); // Delay at least 100 ns

    // Deassert PCIe RC resets
    MmioAnd32 (BM1000_PCIE_GPR_RESET_REG (PcieIdx),
      ~(ResetMask |
      BM1000_PCIE_GPR_RESET_ADB_PWRDWN    | BM1000_PCIE_GPR_RESET_HOT_RESET)
      );

    if (PcieNumLanes[PcieIdx] == 1) {
      PciePortLinkCapableLanesVal = BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X1;
    } else if (PcieNumLanes[PcieIdx] == 2) {
      PciePortLinkCapableLanesVal = BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X2;
    } else if (PcieNumLanes[PcieIdx] == 4) {
      PciePortLinkCapableLanesVal = BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X4;
    } else if (PcieNumLanes[PcieIdx] == 8) {
      PciePortLinkCapableLanesVal = BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X8;
    } else if (PcieNumLanes[PcieIdx] == 16) {
      PciePortLinkCapableLanesVal = BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X16;
    } else {
      PciePortLinkCapableLanesVal = 0;
    }

    if (PciePortLinkCapableLanesVal) {
      MmioAndThenOr32 (
         mPcieDbiBases[PcieIdx] +
         BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF,
        ~BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_BITS,
         PciePortLinkCapableLanesVal
        );

      MmioAndThenOr32 (
         mPcieDbiBases[PcieIdx] +
         BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF,
        ~BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_BITS,
         PcieNumLanes[PcieIdx] << BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_SHIFT
        );
    }

    MmioOr32 (
      mPcieDbiBases[PcieIdx] +
      BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF,
      BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN
      );

    MmioAndThenOr32 (
      mPcieDbiBases[PcieIdx] + BM1000_PCIE_PF0_TYPE1_HDR_TYPE1_CLASS_CODE_REV_ID_REG,
      0x000000FF,
      0x06040100
      );

    if (PciePortLinkCapableLanesVal) {
      MmioAndThenOr32 (
         mPcieDbiBases[PcieIdx] +
         BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG,
        ~BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_BITS,
         PcieNumLanes[PcieIdx] << BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_SHIFT
        );
    }

    MmioAnd32 (
       mPcieDbiBases[PcieIdx] +
       BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF,
      ~BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN
      );

    if (PcdGet32 (PcdAcpiPcieMode) != ACPI_PCIE_OFF &&
        PcdGet32 (PcdAcpiMsiMode)  == ACPI_MSI_ITS) {
      //
      // Enable improved ITS support by encoding the RCNUM for different RCs,
      // so that devices on different segments don't map to same DeviceID.
      // MSI is captured by ITS as a write request from initiator encoded as:
      // bits [17:16] - RC device id (configured below)
      // bits [15:8]  - subordinate pci bus id
      // bits [7:3]   - device id (devid)
      // bits [2:0]   - device function (devfn)
      if (PcieIdx == BM1000_PCIE0_IDX) {
        MmioAndThenOr32 (
           BM1000_PCIE_GPR_MSI_TRANS_CTL2_REG,
          ~BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE0_MASK,
           BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE0_MSI_TRANS_EN | (PcieMsiDevs[PcieIdx] << 0)
          );
      } else if (PcieIdx == BM1000_PCIE1_IDX) {
        MmioAndThenOr32 (
           BM1000_PCIE_GPR_MSI_TRANS_CTL2_REG,
          ~BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE1_MASK,
           BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE1_MSI_TRANS_EN | (PcieMsiDevs[PcieIdx] << 2)
          );
      } else if (PcieIdx == BM1000_PCIE2_IDX) {
        MmioAndThenOr32 (
           BM1000_PCIE_GPR_MSI_TRANS_CTL2_REG,
          ~BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE2_MASK,
           BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE2_MSI_TRANS_EN | (PcieMsiDevs[PcieIdx] << 4)
          );
      }
    }

    // Region 0: CFG0
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      0,
      mPcieCfgBases[PcieIdx],
      0, // See AcpiPlatformDxe/Iort.c for implications of using 0 here instead of encoding the bus
      SIZE_64KB,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG0,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE
      );

    // Region 1: CFG1
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      1,
      mPcieCfgBases[PcieIdx] + SIZE_1MB,
      0,
      PcieCfgSizes[PcieIdx] - SIZE_1MB,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE
      );

    // Region 2: MEM
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      2,
      PcieMemBases[PcieIdx],
      lPcieRootBridge->Mem.Base,
      lPcieRootBridge->Mem.Limit - lPcieRootBridge->Mem.Base + 1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM,
      0
      );

    // Region 3: I/O
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      3,
      PcieIoBases[PcieIdx],
      lPcieRootBridge->Io.Base,
      lPcieRootBridge->Io.Limit - lPcieRootBridge->Io.Base + 1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_IO,
      0
      );

    // Force PCIE_CAP_TARGET_LINK_SPEED to 2.5 GT/s
    MmioAndThenOr32 (
       mPcieDbiBases[PcieIdx] +
       BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG,
      ~BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG_TRGT_LNK_SPEED_BITS,
       1
      );

    MmioOr32 (BM1000_PCIE_GPR_GENCTL_REG (PcieIdx), BM1000_PCIE_GPR_GENCTL_LTSSM_EN);

    // Deassert PERST pin
    if (PciePerstGpios[PcieIdx] >= 0 &&
        PciePerstGpios[PcieIdx] <= 31 &&
        PciePerstPolarity[PcieIdx] >= 0) {
      if (PciePerstPolarity[PcieIdx]) {
        GpioOutSet (BM1000_GPIO32_BASE, PciePerstGpios[PcieIdx]);
      } else {
        GpioOutRst (BM1000_GPIO32_BASE, PciePerstGpios[PcieIdx]);
      }
    }

    MmioOr32 (
      mPcieDbiBases[PcieIdx] +
      BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG,
      BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_RETRAIN_LINK
      );

    // Wait for link
    UINT64  PerformanceCounter = GetPerformanceCounter ();
    TimeStart = GetTimeInNanoSecond (PerformanceCounter);
    for (;;) {
      CONST UINT32  PcieGprStat = MmioRead32 (BM1000_PCIE_GPR_STATUS_REG (PcieIdx));

      PerformanceCounter = GetPerformanceCounter ();

      if (!ComponentExists) {
        if ((PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) > 0x01) {
          ComponentExists = TRUE;
          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link partner detected\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_SMLH_LINKUP ? '+' : '-',
            PcieGprStat & BM1000_PCIE_GPR_STATUS_RDLH_LINKUP ? '+' : '-'
            ));
        } else if (GetTimeInNanoSecond (PerformanceCounter) - TimeStart > 50000000) {
          // According to PCI Express Base Specification device must enter LTSSM detect state within 20 ms of reset
          break;
        }
      }

      if (ComponentExists) {
        if (((PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) ==
                            BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) &&
             (PcieGprStat & BM1000_PCIE_GPR_STATUS_SMLH_LINKUP) &&
             (PcieGprStat & BM1000_PCIE_GPR_STATUS_RDLH_LINKUP)) {
#if !defined(MDEPKG_NDEBUG)
          CONST UINT32  PcieLnkStat = MmioRead32 (
                                        mPcieDbiBases[PcieIdx] +
                                        BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG
                                        );
          CONST CHAR8  *LinkSpeedString;
          CONST UINTN   LinkSpeedVector = (PcieLnkStat &
                                            BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_BITS) >>
                                            BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_SHIFT;
          if (LinkSpeedVector == 1) {
            LinkSpeedString = "2.5";
          } else if (LinkSpeedVector == 2) {
            LinkSpeedString = "5.0";
          } else if (LinkSpeedVector == 3) {
            LinkSpeedString = "8.0";
          } else {
            LinkSpeedString = "???";
          }

          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link %a GT/s, x%u",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_SMLH_LINKUP ? '+' : '-',
            PcieGprStat & BM1000_PCIE_GPR_STATUS_RDLH_LINKUP ? '+' : '-',
            LinkSpeedString,
            (PcieLnkStat & BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_BITS) >>
              BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_SHIFT
            ));
#endif
          // Wait until device starts responding to cfg requests
          while (MmioRead32 (mPcieCfgBases[PcieIdx]) == 0) {
            PerformanceCounter = GetPerformanceCounter ();
            MmioWrite32(mPcieCfgBases[PcieIdx], 0xffffffff);
            gBS->Stall (1000000);
            if (((GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000) > 1000) {
              break;
            }
          }
          DEBUG((EFI_D_INFO,
            "PcieRoot(0x%x): [%dms]: dev_id at 1:0.0 - %x, dev_id at 1:1.0 - %x\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            MmioRead32 (mPcieCfgBases[PcieIdx]),
            MmioRead32 (mPcieCfgBases[PcieIdx] + 0x8000)));

          if (MmioRead32 (mPcieCfgBases[PcieIdx]) != 0xFFFFFFFF &&
              MmioRead32 (mPcieCfgBases[PcieIdx] + 0x8000) == 0xFFFFFFFF) {
            //
            // Device appears to filter CFG0 requests, so the 64 KiB granule for the iATU
            // isn't a problem. We don't have to ignore fn > 0 or shift MCFG by 0x8000.
            //
#if !defined(MDEPKG_NDEBUG)
            DEBUG ((EFI_D_INFO, ", Cfg0Filter+\n"));
#endif
            mPcieCfg0FilteringWorks |= 1 << PcieIdx;
            Status = PcdSet32S (PcdPcieCfg0FilteringWorks, mPcieCfg0FilteringWorks);
            ASSERT_EFI_ERROR (Status);
          } else {
#if !defined(MDEPKG_NDEBUG)
            DEBUG ((EFI_D_INFO, ", Cfg0Filter-\n"));
#endif
          }

          break;
        } else if (GetTimeInNanoSecond (PerformanceCounter) - TimeStart > 100000000) {
          // Wait up to 100 ms for link up
          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link is inactive\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_SMLH_LINKUP ? '+' : '-',
            PcieGprStat & BM1000_PCIE_GPR_STATUS_RDLH_LINKUP ? '+' : '-'
            ));

          //
          // System hangups have been observed when PCIe partner enters detect state
          // but does not reach L0. Disabling LTSSM helps to prevent these hangups.
          //
          MmioAnd32 (BM1000_PCIE_GPR_GENCTL_REG (PcieIdx), ~BM1000_PCIE_GPR_GENCTL_LTSSM_EN);
          break;
        } else if ((PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) == 0x03 ||
                   (PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) == 0x05) {
          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: %a\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_SMLH_LINKUP ? '+' : '-',
            PcieGprStat & BM1000_PCIE_GPR_STATUS_RDLH_LINKUP ? '+' : '-',
            (PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) == 0x03 ?
              "Polling.Compliance" : "Pre.Detect.Quiet"
            ));

          MmioAnd32 (BM1000_PCIE_GPR_GENCTL_REG (PcieIdx), ~BM1000_PCIE_GPR_GENCTL_LTSSM_EN);

          // Assert PERST pin
          if (PciePerstGpios[PcieIdx] >= 0 &&
              PciePerstGpios[PcieIdx] <= 31 &&
              PciePerstPolarity[PcieIdx] >= 0) {
            if (PciePerstPolarity[PcieIdx]) {
              GpioOutRst (BM1000_GPIO32_BASE, PciePerstGpios[PcieIdx]);
            } else {
              GpioOutSet (BM1000_GPIO32_BASE, PciePerstGpios[PcieIdx]);
            }
          }

          MmioOr32 (
             BM1000_PCIE_GPR_RESET_REG (PcieIdx),
             BM1000_PCIE_GPR_RESET_CORE_RST
             );

          gBS->Stall (1000);
          MmioAnd32 (
             BM1000_PCIE_GPR_RESET_REG (PcieIdx),
            ~BM1000_PCIE_GPR_RESET_CORE_RST
            );

          MmioOr32 (BM1000_PCIE_GPR_GENCTL_REG (PcieIdx), BM1000_PCIE_GPR_GENCTL_LTSSM_EN);

          // Deassert PERST pin
          if (PciePerstGpios[PcieIdx] >= 0 &&
              PciePerstGpios[PcieIdx] <= 31 &&
              PciePerstPolarity[PcieIdx] >= 0) {
            if (PciePerstPolarity[PcieIdx]) {
              GpioOutSet (BM1000_GPIO32_BASE, PciePerstGpios[PcieIdx]);
            } else {
              GpioOutRst (BM1000_GPIO32_BASE, PciePerstGpios[PcieIdx]);
            }
          }
        }
      }
    }
  }

  if (PcdGet32 (PcdAcpiPcieMode) == ACPI_PCIE_ECAM) {
    Status = gBS->CreateEvent (
                    EVT_SIGNAL_EXIT_BOOT_SERVICES,
                    TPL_NOTIFY,
                    PciHostBridgeLibExitBootServices,
                    NULL,
                    &Event
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

BOOLEAN
PciHostBridgeLibGetLink (
  IN  CONST UINTN  PcieIdx
  )
{
  ASSERT (PcieIdx < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths));

  CONST UINT32  PcieGprStat = MmioRead32 (BM1000_PCIE_GPR_STATUS_REG (PcieIdx));
  return ((PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) ==
                         BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) &&
          (PcieGprStat & BM1000_PCIE_GPR_STATUS_SMLH_LINKUP) &&
          (PcieGprStat & BM1000_PCIE_GPR_STATUS_RDLH_LINKUP);
}

PCI_ROOT_BRIDGE *
EFIAPI
PciHostBridgeLibGetRootBridges (
  OUT UINTN * CONST  Count
  )
{
  *Count = mPcieRootBridgesNum;
  return mPcieRootBridges;
}
