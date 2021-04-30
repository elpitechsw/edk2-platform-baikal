/** @file
  Copyright (c) 2020, Baikal Electronics JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Pci22.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciHostBridgeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Platform/Pcie.h>
#include <Protocol/FdtClient.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>

#define BM1000_GPIO32_BASE           0x20200000
#define BM1000_GPIO32_DATA           (BM1000_GPIO32_BASE + 0x00)
#define BM1000_GPIO32_DIR            (BM1000_GPIO32_BASE + 0x04)

#define BM1000_PCIE_LCRU_GPR_BASE                     0x02050000

#define BM1000_PCIE_LCRU_GPR_RESET_REG(PcieIdx)       (BM1000_PCIE_LCRU_GPR_BASE + (PcieIdx) * 0x20 + 0x00)
#define BM1000_PCIE_LCRU_GPR_RESET_PHY_RESET          BIT0
#define BM1000_PCIE_LCRU_GPR_RESET_PIPE0_RESET        BIT4
#define BM1000_PCIE_LCRU_GPR_RESET_PIPE1_RESET        BIT5
#define BM1000_PCIE_LCRU_GPR_RESET_CORE_RST           BIT8
#define BM1000_PCIE_LCRU_GPR_RESET_PWR_RST            BIT9
#define BM1000_PCIE_LCRU_GPR_RESET_STICKY_RST         BIT10
#define BM1000_PCIE_LCRU_GPR_RESET_NONSTICKY_RST      BIT11
#define BM1000_PCIE_LCRU_GPR_RESET_HOT_RESET          BIT12
#define BM1000_PCIE_LCRU_GPR_RESET_ADB_PWRDWN         BIT13

#define BM1000_PCIE_LCRU_GPR_STATUS_REG(PcieIdx)      (BM1000_PCIE_LCRU_GPR_BASE + (PcieIdx) * 0x20 + 0x04)
#define BM1000_PCIE_LCRU_GPR_STATUS_SMLH_LINKUP       BIT6
#define BM1000_PCIE_LCRU_GPR_STATUS_RDLH_LINKUP       BIT7
#define BM1000_PCIE_LCRU_GPR_STATUS_LTSSM_STATE_MASK  0x3F
#define BM1000_PCIE_LCRU_GPR_STATUS_LTSSM_STATE_L0    0x11

#define BM1000_PCIE_LCRU_GPR_GENCTL_REG(PcieIdx)      (BM1000_PCIE_LCRU_GPR_BASE + (PcieIdx) * 0x20 + 0x08)
#define BM1000_PCIE_LCRU_GPR_GENCTL_LTSSM_ENABLE      BIT1

#define BM1000_PCIE_LCRU_GPR_POWERCTL_REG(PcieIdx)    (BM1000_PCIE_LCRU_GPR_BASE + (PcieIdx) * 0x20 + 0x10)

#define BM1000_PCIE_PF0_TYPE1_HDR_TYPE1_CLASS_CODE_REV_ID_REG                        0x008
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG                               0x07C
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_BITS                (0x3F << 4)
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_SHIFT               4

#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG                        0x080
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_BITS   (0x3F << 20)
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_SHIFT  20
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_BITS        (0xF << 16)
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_SHIFT       16

#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG                      0x0A0
#define BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG_TRGT_LNK_SPEED_BITS  0xF

#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_FORCE_OFF                                    0x708
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_FORCE_OFF_FORCE_EN                           BIT15

#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF                                0x710
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LOOPBACK_ENABLE                BIT2
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_BITS              (0x3F << 16)
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X1                (0x01 << 16)
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X2                (0x03 << 16)
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X4                (0x07 << 16)
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X8                (0x0F << 16)
#define BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X16               (0x1F << 16)

#define BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF                                     0x80C
#define BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_DIRECT_SPEED_CHANGE                 BIT17
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

CONST EFI_PHYSICAL_ADDRESS         mPcieCdmBases[]    = BM1000_PCIE_CDM_BASES;
CONST EFI_PHYSICAL_ADDRESS         mPcieCfgBases[]    = BM1000_PCIE_CFG_BASES;
STATIC CONST UINTN                 mPcieBusMins[]     = BM1000_PCIE_BUS_MINS;
STATIC CONST UINTN                 mPcieBusMaxs[]     = BM1000_PCIE_BUS_MAXS;
STATIC CONST EFI_PHYSICAL_ADDRESS  mPcieMmio32Bases[] = BM1000_PCIE_MMIO32_BASES;
STATIC CONST UINTN                 mPcieMmio32Sizes[] = BM1000_PCIE_MMIO32_SIZES;
STATIC CONST EFI_PHYSICAL_ADDRESS  mPcieMmio64Bases[] = BM1000_PCIE_MMIO64_BASES;
STATIC CONST UINTN                 mPcieMmio64Sizes[] = BM1000_PCIE_MMIO64_SIZES;
STATIC CONST EFI_PHYSICAL_ADDRESS  mPciePortIoBases[] = BM1000_PCIE_PORTIO_BASES;
STATIC CONST UINTN                 mPciePortIoSizes[] = BM1000_PCIE_PORTIO_SIZES;
STATIC CONST UINTN                 mPciePortIoMins[]  = BM1000_PCIE_PORTIO_MINS;
STATIC CONST UINTN                 mPciePortIoMaxs[]  = BM1000_PCIE_PORTIO_MAXS;

STATIC PCI_ROOT_BRIDGE  *mPcieRootBridges;
STATIC UINTN             mPcieRootBridgesNum;

PCI_ROOT_BRIDGE *
EFIAPI
BaikalPciHostBridgeLibGetRootBridges (
  OUT UINTN  *Count
  )
{
  *Count = mPcieRootBridgesNum;
  return mPcieRootBridges;
}

BOOLEAN
BaikalPciHostBridgeLibLink (
  IN  CONST UINTN  PcieIdx
  )
{
  ASSERT (PcieIdx < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths));

  UINT32 RegPcieLcruGpr = MmioRead32 (BM1000_PCIE_LCRU_GPR_STATUS_REG (PcieIdx));
  return ( (RegPcieLcruGpr & BM1000_PCIE_LCRU_GPR_STATUS_SMLH_LINKUP) &&
           (RegPcieLcruGpr & BM1000_PCIE_LCRU_GPR_STATUS_RDLH_LINKUP));
}

VOID
BaikalPciHostBridgeLibCfgWindow (
  IN  EFI_PHYSICAL_ADDRESS  PcieCdmBase,
  IN  UINTN                 RegionIdx,
  IN  UINT64                CpuBase,
  IN  UINT64                PciBase,
  IN  UINT64                Size,
  IN  UINTN                 Type,
  IN  UINTN                 EnableFlags
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
    PcieCdmBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_VIEWPORT_OFF,
    BM1000_PCIE_PF0_PORT_LOGIC_IATU_VIEWPORT_OFF_REGION_DIR_OUTBOUND | RegionIdx
    );

  ArmDataMemoryBarrier ();
  MmioWrite32 (
    PcieCdmBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0,
    (UINT32)(CpuBase & 0xFFFFFFFF)
    );

  MmioWrite32 (PcieCdmBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0, (UINT32)(CpuBase >> 32));
  MmioWrite32 (PcieCdmBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_LIMIT_ADDR_OFF_OUTBOUND_0, (UINT32)(CpuBase + Size - 1));
  MmioWrite32 (
    PcieCdmBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0,
    (UINT32)(PciBase & 0xFFFFFFFF)
    );

  MmioWrite32 (
    PcieCdmBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0,
    (UINT32)(PciBase >> 32)
    );

  MmioWrite32 (PcieCdmBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0, Type);
  MmioWrite32 (
    PcieCdmBase + BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0,
    BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_REGION_EN | EnableFlags
    );
}

EFI_STATUS
EFIAPI
BaikalPciHostBridgeLibCtor (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  ASSERT (
    ARRAY_SIZE (mPcieBusMaxs)     == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPcieBusMins)     == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPcieCdmBases)    == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPcieCfgBases)    == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPcieMmio32Bases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPcieMmio32Sizes) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPcieMmio64Bases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPcieMmio64Sizes) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPciePortIoBases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPciePortIoSizes) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPciePortIoMaxs)  == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths) &&
    ARRAY_SIZE (mPciePortIoMins)  == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)
    );

  FDT_CLIENT_PROTOCOL  *FdtClient;
  INT32                 FdtNode;
  UINTN                 Iter;
  UINTN                 PcieIdx;
  UINTN                 PcieIdxs[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  UINTN                 PcieNumLanes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  INTN                  PciePerstGpios[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  INTN                  PciePerstPolarity[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  INTN                  PcieX8PrsntGpio;
  INTN                  PcieX8PrsntPolarity;
  EFI_STATUS            Status;
  PCI_ROOT_BRIDGE      *lPcieRootBridge;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **)&FdtClient);
  ASSERT_EFI_ERROR (Status);

  // Acquire PCIe RC related data from FDT
  for (FdtNode = 0, Iter = 0; Iter < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths);) {
    CONST VOID  *Prop;
    UINT32       PropSize;

    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,pcie-m", FdtNode, &FdtNode);
    if (EFI_ERROR (Status)) {
      break;
    }

    Status = FdtClient->GetNodeProperty (FdtClient, FdtNode, "status", &Prop, &PropSize);
    if (EFI_ERROR (Status) || AsciiStrCmp ((CONST CHAR8 *)Prop, "okay") != 0) {
      continue;
    }

    Status = FdtClient->GetNodeProperty (FdtClient, FdtNode, "baikal,pcie-lcru", &Prop, &PropSize);
    if (EFI_ERROR (Status) || PropSize != 8) {
      continue;
    }

    PcieIdx = SwapBytes64 (((CONST UINT64 *)Prop)[0]) & 0xFFFFFFFF;
    if (PcieIdx >= ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)) {
      PcieIdxs[Iter] = 0;
      continue;
    }

    PcieIdxs[Iter] = PcieIdx;

    Status = FdtClient->GetNodeProperty (FdtClient, FdtNode, "num-lanes", &Prop, &PropSize);
    if (Status == EFI_SUCCESS && PropSize == 4) {
      PcieNumLanes[PcieIdx] = SwapBytes32 (((CONST UINT32 *)Prop)[0]);
    } else {
      PcieNumLanes[PcieIdx] = 0;
    }

    Status = FdtClient->GetNodeProperty (FdtClient, FdtNode, "reset-gpios", &Prop, &PropSize);
    if (Status == EFI_SUCCESS && PropSize == 12) {
      PciePerstGpios[PcieIdx]    = SwapBytes32 (((CONST UINT32 *)Prop)[1]);
      PciePerstPolarity[PcieIdx] = SwapBytes32 (((CONST UINT32 *)Prop)[2]);

      if (PciePerstPolarity[PcieIdx] > 1) {
        PciePerstPolarity[PcieIdx] = -1;
      }
    } else {
      PciePerstGpios[PcieIdx] = -1;
    }

    if (PcieIdx == BM1000_PCIE_X8_IDX) {
      INT32  FdtSubnode;

      FdtSubnode = 0;
      PcieX8PrsntGpio     = -1;
      PcieX8PrsntPolarity = -1;

      Status = FdtClient->FindNextCompatibleNode (FdtClient, "snps,dw-apb-gpio-port", FdtSubnode, &FdtSubnode);
      if (Status == EFI_SUCCESS) {
        Status = FdtClient->FindNextSubnode (FdtClient, "pcieclk", FdtSubnode, &FdtSubnode);
        if (Status == EFI_SUCCESS) {
          Status = FdtClient->GetNodeProperty (FdtClient, FdtSubnode, "line_name", &Prop, &PropSize);
          if (Status == EFI_SUCCESS && AsciiStrCmp ((CONST CHAR8 *)Prop, "pcie-x8-clock") == 0) {
            Status = FdtClient->GetNodeProperty (FdtClient, FdtSubnode, "gpios", &Prop, &PropSize);
            if (Status == EFI_SUCCESS && PropSize == 8) {
              PcieX8PrsntGpio     = SwapBytes32 (((CONST UINT32 *)Prop)[0]);
              PcieX8PrsntPolarity = SwapBytes32 (((CONST UINT32 *)Prop)[1]);
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

  mPcieRootBridges = AllocateZeroPool (mPcieRootBridgesNum * sizeof (PCI_ROOT_BRIDGE));
  lPcieRootBridge  = &mPcieRootBridges[0];

  // Initialise PCIe RCs
  for (Iter = 0; Iter < mPcieRootBridgesNum; ++Iter, ++lPcieRootBridge) {
    UINTN   Cnt;
    UINTN   PciePortLinkCapableLanesVal;
    UINT32  RegPcieLcruGpr;
    UINT32  RegPcieLnkStat;

    PcieIdx = PcieIdxs[Iter];
    lPcieRootBridge->Segment    = PcieIdx;
    lPcieRootBridge->Supports   = 0;
    lPcieRootBridge->Attributes = lPcieRootBridge->Supports;
    lPcieRootBridge->DmaAbove4G = TRUE;
    lPcieRootBridge->NoExtendedConfigSpace = FALSE;
    lPcieRootBridge->ResourceAssigned      = FALSE;
    lPcieRootBridge->AllocationAttributes  = EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM |
                                             EFI_PCI_HOST_BRIDGE_MEM64_DECODE;

    lPcieRootBridge->Bus.Base          = mPcieBusMins[PcieIdx];
    lPcieRootBridge->Bus.Limit         = mPcieBusMaxs[PcieIdx];
    lPcieRootBridge->Io.Base           = mPciePortIoMins[PcieIdx];
    lPcieRootBridge->Io.Limit          = mPciePortIoMaxs[PcieIdx];
    lPcieRootBridge->Mem.Base          = mPcieMmio32Bases[PcieIdx];
    lPcieRootBridge->Mem.Limit         = mPcieMmio32Sizes[PcieIdx] < SIZE_4GB ?
                                           mPcieMmio32Bases[PcieIdx] + mPcieMmio32Sizes[PcieIdx] - 1 :
                                           mPcieMmio32Bases[PcieIdx] + SIZE_4GB - 1;

    lPcieRootBridge->MemAbove4G.Base   = mPcieMmio64Bases[PcieIdx];
    lPcieRootBridge->MemAbove4G.Limit  = mPcieMmio64Sizes[PcieIdx] < SIZE_4GB ?
                                           mPcieMmio64Bases[PcieIdx] + mPcieMmio64Sizes[PcieIdx] - 1 :
                                           mPcieMmio64Bases[PcieIdx] + SIZE_4GB - 1;

    lPcieRootBridge->PMem.Base         = MAX_UINT64;
    lPcieRootBridge->PMem.Limit        = 0;
    lPcieRootBridge->PMemAbove4G.Base  = MAX_UINT64;
    lPcieRootBridge->PMemAbove4G.Limit = 0;
    lPcieRootBridge->DevicePath        = (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePaths[PcieIdx];

    // Assert PERST pin
    if (PciePerstGpios[PcieIdx] >= 0 && PciePerstPolarity[PcieIdx] >= 0) {
      MmioOr32 (BM1000_GPIO32_DIR, 1 << PciePerstGpios[PcieIdx]);
      if (PciePerstPolarity[PcieIdx]) {
        MmioAnd32 (BM1000_GPIO32_DATA, ~(1 << PciePerstGpios[PcieIdx]));
      } else {
        MmioOr32 (BM1000_GPIO32_DATA, 1 << PciePerstGpios[PcieIdx]);
      }
    }

    MmioAnd32 (BM1000_PCIE_LCRU_GPR_GENCTL_REG (PcieIdx), ~BM1000_PCIE_LCRU_GPR_GENCTL_LTSSM_ENABLE);

    // Assert PCIe RC resets
    if (PcieIdx == BM1000_PCIE_X8_IDX) {
      MmioOr32 (
        BM1000_PCIE_LCRU_GPR_RESET_REG (PcieIdx),
        BM1000_PCIE_LCRU_GPR_RESET_NONSTICKY_RST | BM1000_PCIE_LCRU_GPR_RESET_STICKY_RST  |
        BM1000_PCIE_LCRU_GPR_RESET_PWR_RST       | BM1000_PCIE_LCRU_GPR_RESET_CORE_RST    |
        BM1000_PCIE_LCRU_GPR_RESET_PIPE1_RESET   | BM1000_PCIE_LCRU_GPR_RESET_PIPE0_RESET |
        BM1000_PCIE_LCRU_GPR_RESET_PHY_RESET
        );

      // Assert PRSNT pin
      if (PcieX8PrsntGpio >= 0 && PcieX8PrsntPolarity >= 0) {
        MmioOr32 (BM1000_GPIO32_DIR, 1 << PcieX8PrsntGpio);
        if (PcieX8PrsntPolarity) {
          MmioAnd32 (BM1000_GPIO32_DATA, ~(1 << PcieX8PrsntGpio));
        } else {
          MmioOr32 (BM1000_GPIO32_DATA, 1 << PcieX8PrsntGpio);
        }
      }
    } else {
      MmioOr32 (
        BM1000_PCIE_LCRU_GPR_RESET_REG (PcieIdx),
        BM1000_PCIE_LCRU_GPR_RESET_NONSTICKY_RST | BM1000_PCIE_LCRU_GPR_RESET_STICKY_RST  |
        BM1000_PCIE_LCRU_GPR_RESET_PWR_RST       | BM1000_PCIE_LCRU_GPR_RESET_CORE_RST    |
        BM1000_PCIE_LCRU_GPR_RESET_PIPE0_RESET   | BM1000_PCIE_LCRU_GPR_RESET_PHY_RESET
        );
    }

    gBS->Stall (150 * 1000);

    // Deassert PCIe RC resets
    if (PcieIdx == BM1000_PCIE_X8_IDX) {
      MmioAnd32 (
        BM1000_PCIE_LCRU_GPR_RESET_REG (PcieIdx),
        ~(BM1000_PCIE_LCRU_GPR_RESET_ADB_PWRDWN    | BM1000_PCIE_LCRU_GPR_RESET_HOT_RESET   |
          BM1000_PCIE_LCRU_GPR_RESET_NONSTICKY_RST | BM1000_PCIE_LCRU_GPR_RESET_STICKY_RST  |
          BM1000_PCIE_LCRU_GPR_RESET_PWR_RST       | BM1000_PCIE_LCRU_GPR_RESET_CORE_RST    |
          BM1000_PCIE_LCRU_GPR_RESET_PIPE1_RESET   | BM1000_PCIE_LCRU_GPR_RESET_PIPE0_RESET |
          BM1000_PCIE_LCRU_GPR_RESET_PHY_RESET)
        );
    } else {
      MmioAnd32 (
        BM1000_PCIE_LCRU_GPR_RESET_REG (PcieIdx),
        ~(BM1000_PCIE_LCRU_GPR_RESET_ADB_PWRDWN    | BM1000_PCIE_LCRU_GPR_RESET_HOT_RESET   |
          BM1000_PCIE_LCRU_GPR_RESET_NONSTICKY_RST | BM1000_PCIE_LCRU_GPR_RESET_STICKY_RST  |
          BM1000_PCIE_LCRU_GPR_RESET_PWR_RST       | BM1000_PCIE_LCRU_GPR_RESET_CORE_RST    |
          BM1000_PCIE_LCRU_GPR_RESET_PIPE0_RESET   | BM1000_PCIE_LCRU_GPR_RESET_PHY_RESET)
        );
    }

    gBS->Stall (1000);

    // Deassert PERST pin
    if (PciePerstGpios[PcieIdx] >= 0 && PciePerstPolarity[PcieIdx] >= 0) {
      if (PciePerstPolarity[PcieIdx]) {
        MmioOr32 (BM1000_GPIO32_DATA, 1 << PciePerstGpios[PcieIdx]);
      } else {
        MmioAnd32 (BM1000_GPIO32_DATA, ~(1 << PciePerstGpios[PcieIdx]));
      }
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
         mPcieCdmBases[PcieIdx] +
         BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF,
        ~BM1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_BITS,
         PciePortLinkCapableLanesVal
        );

      MmioAndThenOr32 (
         mPcieCdmBases[PcieIdx] +
         BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF,
        ~BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_BITS,
         PcieNumLanes[PcieIdx] << BM1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_SHIFT
        );
    }

    MmioOr32 (
      mPcieCdmBases[PcieIdx] +
      BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF,
      BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN
      );

    MmioAndThenOr32 (
      mPcieCdmBases[PcieIdx] + BM1000_PCIE_PF0_TYPE1_HDR_TYPE1_CLASS_CODE_REV_ID_REG,
      0x000000FF,
      0x06040100
      );

    if (PciePortLinkCapableLanesVal) {
      MmioAndThenOr32 (
         mPcieCdmBases[PcieIdx] +
         BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG,
        ~BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_BITS,
         PcieNumLanes[PcieIdx] << BM1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_SHIFT
        );
    }

    MmioAnd32 (
       mPcieCdmBases[PcieIdx] +
       BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF,
      ~BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN
      );

    // Region 0: MMIO32 range
    BaikalPciHostBridgeLibCfgWindow (
      mPcieCdmBases[PcieIdx],
      0,
      lPcieRootBridge->Mem.Base,
      lPcieRootBridge->Mem.Base,
      lPcieRootBridge->Mem.Limit - lPcieRootBridge->Mem.Base + 1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM,
      0
      );

    // Region 2: MMIO64 range
    BaikalPciHostBridgeLibCfgWindow (
      mPcieCdmBases[PcieIdx],
      2,
      lPcieRootBridge->MemAbove4G.Base,
      lPcieRootBridge->MemAbove4G.Base,
      lPcieRootBridge->MemAbove4G.Limit - lPcieRootBridge->MemAbove4G.Base + 1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM,
      0
      );

    // Region 3: port I/O range
    BaikalPciHostBridgeLibCfgWindow (
      mPcieCdmBases[PcieIdx],
      3,
      mPciePortIoBases[PcieIdx],
      lPcieRootBridge->Io.Base,
      lPcieRootBridge->Io.Limit - lPcieRootBridge->Io.Base + 1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_IO,
      0
      );

    // Force PCIE_CAP_TARGET_LINK_SPEED to Gen1
    MmioAndThenOr32 (
       mPcieCdmBases[PcieIdx] +
       BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG,
      ~BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG_TRGT_LNK_SPEED_BITS,
       1
      );

    MmioOr32 (BM1000_PCIE_LCRU_GPR_GENCTL_REG (PcieIdx), BM1000_PCIE_LCRU_GPR_GENCTL_LTSSM_ENABLE);

    // Wait for link
    for (Cnt = 0; Cnt < 3000; ++Cnt) {
      RegPcieLcruGpr = MmioRead32 (BM1000_PCIE_LCRU_GPR_STATUS_REG (PcieIdx));
      if ((RegPcieLcruGpr & BM1000_PCIE_LCRU_GPR_STATUS_SMLH_LINKUP) &&
          (RegPcieLcruGpr & BM1000_PCIE_LCRU_GPR_STATUS_RDLH_LINKUP)) {
        break;
      }

      gBS->Stall (100);
    }

    RegPcieLnkStat = MmioRead32 (mPcieCdmBases[PcieIdx] + BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG);

    DEBUG ((
      EFI_D_INFO,
      "PcieRoot(0x%x): LtssmSt:0x%02x Smlh:%u Rdlh:%u LnkSpeed:%u LnkWidth:%u Delay:%ums\n",
      PcieIdx,
      RegPcieLcruGpr & BM1000_PCIE_LCRU_GPR_STATUS_LTSSM_STATE_MASK,
      !!(RegPcieLcruGpr & BM1000_PCIE_LCRU_GPR_STATUS_SMLH_LINKUP),
      !!(RegPcieLcruGpr & BM1000_PCIE_LCRU_GPR_STATUS_RDLH_LINKUP),
      (RegPcieLnkStat & BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_BITS) >>
        BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_SHIFT,
      (RegPcieLnkStat & BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_BITS) >>
        BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_SHIFT,
      Cnt / 10
      ));
  }

  return EFI_SUCCESS;
}
