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
#include <Platform/Pcie.h>
#include <Protocol/FdtClient.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <Protocol/PciIo.h>

#define BM1000_GPIO32_BASE    0x20200000

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

CONST EFI_PHYSICAL_ADDRESS          mPcieDbiBases[]    = BM1000_PCIE_DBI_BASES;
UINT32                              mPcieCfg0FilteringWorks;
EFI_PHYSICAL_ADDRESS                mPcieCfgBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                        mPcieCfgSizes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                        mPcieIdxs[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC CONST EFI_PHYSICAL_ADDRESS   mPcieMmio32Bases[] = BM1000_PCIE_MMIO32_BASES;
STATIC CONST UINTN                  mPcieMmio32Sizes[] = BM1000_PCIE_MMIO32_SIZES;
STATIC CONST EFI_PHYSICAL_ADDRESS   mPciePortIoBases[] = BM1000_PCIE_PORTIO_BASES;
STATIC CONST UINTN                  mPciePortIoSizes[] = BM1000_PCIE_PORTIO_SIZES;
STATIC CONST UINTN                  mPciePortIoMins[]  = BM1000_PCIE_PORTIO_MINS;
STATIC CONST UINTN                  mPciePortIoMaxs[]  = BM1000_PCIE_PORTIO_MAXS;
STATIC PCI_ROOT_BRIDGE             *mPcieRootBridges;
STATIC UINTN                        mPcieRootBridgesNum;

STATIC_ASSERT (
  ARRAY_SIZE (mPcieDbiBases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieDbiBases) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieCfgBases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieCfgBases) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieCfgSizes) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieCfgSizes) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieMmio32Bases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieMmio32Bases) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieMmio32Sizes) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieMmio32Sizes) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPciePortIoBases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPciePortIoBases) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPciePortIoSizes) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPciePortIoSizes) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPciePortIoMins) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPciePortIoMins) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPciePortIoMaxs) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPciePortIoMaxs) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
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

  ArmDataMemoryBarrier ();
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
  INTN                  PcieX8PrsntGpio;
  INTN                  PcieX8PrsntPolarity;
  EFI_STATUS            Status;
  PCI_ROOT_BRIDGE      *lPcieRootBridge;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);

  // Acquire PCIe RC related data from FDT
  for (Node = 0, Iter = 0; Iter < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths);) {
    CONST VOID  *Prop;
    UINT32       PropSize;

    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bm1000-pcie", Node, &Node);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == 32) {
      CONST EFI_PHYSICAL_ADDRESS  DbiBase = SwapBytes64 (((CONST UINT64 *) Prop)[0]);
      CONST EFI_PHYSICAL_ADDRESS  CfgBase = SwapBytes64 (((CONST UINT64 *) Prop)[2]);
      CONST UINTN                 CfgSize = SwapBytes64 (((CONST UINT64 *) Prop)[3]);

      for (PcieIdx = 0; PcieIdx < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths); ++PcieIdx) {
        if (DbiBase == mPcieDbiBases[PcieIdx]) {
          mPcieIdxs[Iter] = PcieIdx;
          mPcieCfgBases[PcieIdx] = CfgBase;
          mPcieCfgSizes[PcieIdx] = CfgSize;
          break;
        }
      }
    } else {
      continue;
    }

    PcieIdx = mPcieIdxs[Iter];

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

    if (PcieIdx == BM1000_PCIE2_IDX) {
      INT32  SubNode;

      SubNode = 0;
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

  // Initialise PCIe RCs
  for (Iter = 0; Iter < mPcieRootBridgesNum; ++Iter, ++lPcieRootBridge) {
    BOOLEAN  ComponentExists = FALSE;
    UINTN    PciePortLinkCapableLanesVal;
    UINT64   TimeStart;

    PcieIdx = mPcieIdxs[Iter];

    lPcieRootBridge->Segment    = PcieIdx;
    lPcieRootBridge->Supports   = 0;
    lPcieRootBridge->Attributes = lPcieRootBridge->Supports;
    lPcieRootBridge->DmaAbove4G = TRUE;
    lPcieRootBridge->NoExtendedConfigSpace = FALSE;
    lPcieRootBridge->ResourceAssigned      = FALSE;
    lPcieRootBridge->AllocationAttributes  = EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM;

    lPcieRootBridge->Bus.Base          = 0;
    lPcieRootBridge->Bus.Limit         = mPcieCfgSizes[PcieIdx] / SIZE_1MB - 1;
    lPcieRootBridge->Io.Base           = mPciePortIoMins[PcieIdx];
    lPcieRootBridge->Io.Limit          = mPciePortIoMaxs[PcieIdx];
    lPcieRootBridge->Io.Translation    = mPciePortIoMins[PcieIdx] - mPciePortIoBases[PcieIdx];
    lPcieRootBridge->Mem.Base          = mPcieMmio32Bases[PcieIdx];
    lPcieRootBridge->Mem.Limit         = mPcieMmio32Sizes[PcieIdx] < SIZE_4GB ?
                                           mPcieMmio32Bases[PcieIdx] + mPcieMmio32Sizes[PcieIdx] - 1 :
                                           mPcieMmio32Bases[PcieIdx] + SIZE_4GB - 1;

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
    if (PcieIdx == BM1000_PCIE2_IDX) {
      MmioOr32 (
        BM1000_PCIE_GPR_RESET_REG (PcieIdx),
        BM1000_PCIE_GPR_RESET_NONSTICKY_RST | BM1000_PCIE_GPR_RESET_STICKY_RST  |
        BM1000_PCIE_GPR_RESET_PWR_RST       | BM1000_PCIE_GPR_RESET_CORE_RST    |
        BM1000_PCIE_GPR_RESET_PIPE1_RESET   | BM1000_PCIE_GPR_RESET_PIPE0_RESET |
        BM1000_PCIE_GPR_RESET_PHY_RESET
        );

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
    } else {
      MmioOr32 (
        BM1000_PCIE_GPR_RESET_REG (PcieIdx),
        BM1000_PCIE_GPR_RESET_NONSTICKY_RST | BM1000_PCIE_GPR_RESET_STICKY_RST  |
        BM1000_PCIE_GPR_RESET_PWR_RST       | BM1000_PCIE_GPR_RESET_CORE_RST    |
        BM1000_PCIE_GPR_RESET_PIPE0_RESET   | BM1000_PCIE_GPR_RESET_PHY_RESET
        );
    }

    gBS->Stall (1); // Delay at least 100 ns

    // Deassert PCIe RC resets
    if (PcieIdx == BM1000_PCIE2_IDX) {
      MmioAnd32 (
          BM1000_PCIE_GPR_RESET_REG (PcieIdx),
        ~(BM1000_PCIE_GPR_RESET_ADB_PWRDWN    | BM1000_PCIE_GPR_RESET_HOT_RESET   |
          BM1000_PCIE_GPR_RESET_NONSTICKY_RST | BM1000_PCIE_GPR_RESET_STICKY_RST  |
          BM1000_PCIE_GPR_RESET_PWR_RST       | BM1000_PCIE_GPR_RESET_CORE_RST    |
          BM1000_PCIE_GPR_RESET_PIPE1_RESET   | BM1000_PCIE_GPR_RESET_PIPE0_RESET |
          BM1000_PCIE_GPR_RESET_PHY_RESET)
        );
    } else {
      MmioAnd32 (
          BM1000_PCIE_GPR_RESET_REG (PcieIdx),
        ~(BM1000_PCIE_GPR_RESET_ADB_PWRDWN    | BM1000_PCIE_GPR_RESET_HOT_RESET   |
          BM1000_PCIE_GPR_RESET_NONSTICKY_RST | BM1000_PCIE_GPR_RESET_STICKY_RST  |
          BM1000_PCIE_GPR_RESET_PWR_RST       | BM1000_PCIE_GPR_RESET_CORE_RST    |
          BM1000_PCIE_GPR_RESET_PIPE0_RESET   | BM1000_PCIE_GPR_RESET_PHY_RESET)
        );
    }

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
      // so that devices on different segments don't map to same DeviceID. E.g.,
      // - for DBM:
      // PCIe0 -> 0x1000x
      // PCIe1 -> 0x2000x
      // PCIe2 -> 0x3000x
      // - for MBM:
      // PCIe0 -> 0x1000x
      // PCIe2 -> 0x2000x
      //
      // where 'x' is:
      // - 8 when CFG0 filtering does not work
      // - 0 when CFG0 filtering works or ACPI_PCIE_CUSTOM mode is set
      if (PcieIdx == BM1000_PCIE0_IDX) {
        MmioAndThenOr32 (
           BM1000_PCIE_GPR_MSI_TRANS_CTL2_REG,
          ~BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE0_MASK,
           BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE0_MSI_TRANS_EN | (1 << 0)
          );
      } else if (PcieIdx == BM1000_PCIE1_IDX) {
        MmioAndThenOr32 (
           BM1000_PCIE_GPR_MSI_TRANS_CTL2_REG,
          ~BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE1_MASK,
           BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE1_MSI_TRANS_EN | (2 << 2)
          );
      } else if (PcieIdx == BM1000_PCIE2_IDX) {
        if (FdtClient->FindNextCompatibleNode (FdtClient, "baikal,dbm10", -1, &Node) == EFI_SUCCESS ||
            FdtClient->FindNextCompatibleNode (FdtClient, "baikal,dbm20", -1, &Node) == EFI_SUCCESS) {
          MmioAndThenOr32 (
             BM1000_PCIE_GPR_MSI_TRANS_CTL2_REG,
            ~BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE2_MASK,
             BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE2_MSI_TRANS_EN | (3 << 4)
            );
        } else {
          MmioAndThenOr32 (
             BM1000_PCIE_GPR_MSI_TRANS_CTL2_REG,
            ~BM1000_PCIE_GPR_MSI_TRANS_CTL2_MSI_RCNUM_PCIE2_MASK,
             BM1000_PCIE_GPR_MSI_TRANS_CTL2_PCIE2_MSI_TRANS_EN | (2 << 4)
            );
        }
      }
    }

    // Region 0: CFG0
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      0,
      mPcieCfgBases[PcieIdx],
      // See AcpiPlatformDxe/Iort.c for implications of using 0 here instead of encoding the bus
      0,
      mPcieCfgSizes[PcieIdx] >= SIZE_2MB ? SIZE_2MB : mPcieCfgSizes[PcieIdx],
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG0,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE
      );

    // Region 1: CFG1
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      1,
      mPcieCfgBases[PcieIdx],
      0,
      mPcieCfgSizes[PcieIdx] > SIZE_2MB ? mPcieCfgSizes[PcieIdx] - SIZE_2MB : 0,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE
      );

    // Region 2: MMIO32 range
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      2,
      lPcieRootBridge->Mem.Base,
      lPcieRootBridge->Mem.Base,
      lPcieRootBridge->Mem.Limit - lPcieRootBridge->Mem.Base + 1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM,
      0
      );

    // Region 3: port I/O range
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      3,
      mPciePortIoBases[PcieIdx],
      lPcieRootBridge->Io.Base,
      lPcieRootBridge->Io.Limit - lPcieRootBridge->Io.Base + 1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_IO,
      0
      );

    // Force PCIE_CAP_TARGET_LINK_SPEED to Gen1
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

    // Wait for link
    TimeStart = GetTimeInNanoSecond (GetPerformanceCounter ());
    for (;;) {
      CONST UINT32  PcieGprStat = MmioRead32 (BM1000_PCIE_GPR_STATUS_REG (PcieIdx));
      CONST UINT64  PerformanceCounter = GetPerformanceCounter ();

      if (!ComponentExists) {
        if ((PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) > 0x01) {
          ComponentExists = TRUE;
          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x)[%3u ms, LTSSM:0x%02x]: link partner detected\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK
            ));
        } else if (GetTimeInNanoSecond (PerformanceCounter) - TimeStart > 50000000) {
          // According to PCI Express Base Specification device must enter LTSSM detect state within 20 ms of reset
          break;
        }
      }

      if (ComponentExists) {
        if ((PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) ==
                           BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
#if !defined(MDEPKG_NDEBUG)
          CONST UINT32  PcieLnkStat = MmioRead32 (mPcieDbiBases[PcieIdx] +
                                                   BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG);
          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x)[%3u ms, LTSSM:0x%02x]: LnkSpeed:%u LnkWidth:%u\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK,
            (PcieLnkStat & BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_BITS) >>
              BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_SHIFT,
            (PcieLnkStat & BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_BITS) >>
              BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_SHIFT
            ));
#endif
          // Some PCIe cards (e.g. MegaRAID 9460-16i) require extra time to start
          gBS->Stall (1000000);

          if (MmioRead32 (mPcieCfgBases[PcieIdx]) != 0xFFFFFFFF &&
              MmioRead32 (mPcieCfgBases[PcieIdx] + 0x8000) == 0xFFFFFFFF) {
            //
            // Device appears to filter CFG0 requests, so the 64 KiB granule for the iATU
            // isn't a problem. We don't have to ignore fn > 0 or shift MCFG by 0x8000.
            //
            DEBUG ((EFI_D_INFO, "PcieRoot(0x%x): CFG0 filtering seems functional\n", PcieIdx));
            mPcieCfg0FilteringWorks |= 1 << PcieIdx;
            Status = PcdSet32S (PcdPcieCfg0FilteringWorks, mPcieCfg0FilteringWorks);
            ASSERT_EFI_ERROR (Status);
          }

          break;
        } else if (GetTimeInNanoSecond (PerformanceCounter) - TimeStart > 100000000) {
          // Wait up to 100 ms for link up
          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x)[%3u ms, LTSSM:0x%02x]: device is not entered L0 state\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK
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
            "PcieRoot(0x%x)[%3u ms, LTSSM:0x%02x]: %a\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK,
            (PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) == 0x03 ?
              "polling compliance" : "pre detect quiet"
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
  return (PcieGprStat & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) ==
                        BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0;
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
