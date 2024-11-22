/** @file
  Copyright (c) 2020 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
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
#include <Guid/EventGroup.h>

#include <BM1000.h>

#define BM1000_PCIE_GPR_RST(PcieIdx)                     (BM1000_PCIE_GPR_BASE + (PcieIdx) * 0x20 + 0x00)
#define BM1000_PCIE_GPR_RST_PHY_RST                      BIT0
#define BM1000_PCIE_GPR_RST_PIPE0_RST                    BIT4
#define BM1000_PCIE_GPR_RST_PIPE1_RST                    BIT5
#define BM1000_PCIE_GPR_RST_CORE_RST                     BIT8
#define BM1000_PCIE_GPR_RST_PWR_RST                      BIT9
#define BM1000_PCIE_GPR_RST_STICKY_RST                   BIT10
#define BM1000_PCIE_GPR_RST_NONSTICKY_RST                BIT11
#define BM1000_PCIE_GPR_RST_HOT_RST                      BIT12
#define BM1000_PCIE_GPR_RST_ADB_PWRDWN                   BIT13

#define BM1000_PCIE_GPR_GEN(PcieIdx)                     (BM1000_PCIE_GPR_BASE + (PcieIdx) * 0x20 + 0x08)
#define BM1000_PCIE_GPR_GEN_LTSSM_EN                     BIT1
#define BM1000_PCIE_GPR_GEN_DBI2_EN                      BIT2
#define BM1000_PCIE_GPR_GEN_PHY_EN                       BIT3

#define BM1000_PCIE_GPR_MSI_TRANS2                       (BM1000_PCIE_GPR_BASE + 0xF8)
#define BM1000_PCIE_GPR_MSI_TRANS2_PCIE0_MSI_TRANS_EN    BIT9
#define BM1000_PCIE_GPR_MSI_TRANS2_PCIE1_MSI_TRANS_EN    BIT10
#define BM1000_PCIE_GPR_MSI_TRANS2_PCIE2_MSI_TRANS_EN    BIT11
#define BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE0_MASK  (3 << 0)
#define BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE1_MASK  (3 << 2)
#define BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE2_MASK  (3 << 4)

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

#define BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF                               0x8A8
#define BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF_FB_MODE_BITS                  (0xF << 0)
#define BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF_FB_MODE_SHIFT                 0
#define BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF_PSET_REQ_VEC_BITS             (0xFFFF << 8)
#define BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF_PSET_REQ_VEC_SHIFT            8

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

#define BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_LANENUM                                    0xD04

#define BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL                                    0xD08
#define BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL_PHY_READ_WRITE_FLAG                BIT29
#define BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL_PHY_DONE                           BIT30
#define BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL_PHY_BUSY                           BIT31

#define BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_WRITEDATA                                  0xD0C
#define BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_READDATA                                   0xD10

#define BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL                                            0x18009
#define BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL_CDR_EN                                     BIT0
#define BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL_DFE_EN                                     BIT1
#define BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL_AGC_EN                                     BIT2
#define BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL_LCTRL_MEN                                  BIT8

#define BM1000_PCIE_PHY_LANE_RX_CTLE_CTRL                                            0x1800B
#define BM1000_PCIE_PHY_LANE_RX_CTLE_CTRL_POLE_OVRRD_EN                              BIT9
#define BM1000_PCIE_PHY_LANE_RX_CTLE_CTRL_PCS_SDS_ZERO_BITS                          (0xF << 10)
#define BM1000_PCIE_PHY_LANE_RX_CTLE_CTRL_PCS_SDS_ZERO_SHIFT                         10

#define BM1000_PCIE_PHY_LANE_TX_CFG_1                                                0x18016
#define BM1000_PCIE_PHY_LANE_TX_CFG_1_TURBO_EN_OVRRD_EN                              BIT10
#define BM1000_PCIE_PHY_LANE_TX_CFG_1_VBOOST_EN_OVRRD_EN                             BIT11

#define BM1000_PCIE_PHY_LANE_TX_CFG_3                                                0x18018
#define BM1000_PCIE_PHY_LANE_TX_CFG_3_TURBO_EN                                       BIT0
#define BM1000_PCIE_PHY_LANE_TX_CFG_3_PCS_SDS_GAIN_BITS                              (0x7 << 4)
#define BM1000_PCIE_PHY_LANE_TX_CFG_3_PCS_SDS_GAIN_SHIFT                             4
#define BM1000_PCIE_PHY_LANE_TX_CFG_3_VBOOST_EN                                      BIT14

#define BM1000_PCIE_PHY_LANE_PCS_CTLIFC_CTRL_0                                       0x3800C
#define BM1000_PCIE_PHY_LANE_PCS_CTLIFC_CTRL_0_VBOOST_EN_REQ_OVRRD_VAL               BIT13

#define BM1000_PCIE_PHY_LANE_PCS_CTLIFC_CTRL_2                                       0x3800E
#define BM1000_PCIE_PHY_LANE_PCS_CTLIFC_CTRL_2_VBOOST_EN_REQ_OVRRD_EN                BIT8

#define RANGES_FLAG_IO   0x01000000
#define RANGES_FLAG_MEM  0x02000000

BOOLEAN PciHostBridgeLibGetLink (UINTN  PcieIdx);

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
  BM1000_PCIE1_DBI_SIZE,
  BM1000_PCIE2_DBI_SIZE
};

UINT32                        mPcieCfg0Quirk;
EFI_PHYSICAL_ADDRESS          mPcieCfgBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                  mPcieCfgSizes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                  mPcieIdxs[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                  mPcieMaxLinkSpeed[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC EFI_PHYSICAL_ADDRESS   mPcieIoBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC EFI_PHYSICAL_ADDRESS   mPcieIoMins[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                  mPcieIoSizes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC EFI_PHYSICAL_ADDRESS   mPcieMemBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC EFI_PHYSICAL_ADDRESS   mPcieMemMins[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                  mPcieMemSizes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
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
STATIC_ASSERT (
  ARRAY_SIZE (mPcieCfgBases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieCfgBases) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieCfgSizes) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieCfgSizes) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );

STATIC
BOOLEAN
PciHostBridgeLibPhyWaitForDone (
  IN  EFI_PHYSICAL_ADDRESS  PcieDbiBase
  )
{
  UINT32  Val;
  UINTN   Limit;

  for (Limit = 1000; Limit > 0; --Limit) {
    Val = MmioRead32 (PcieDbiBase + BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL);
    Val &= BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL_PHY_DONE |
           BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL_PHY_BUSY;

    if (Val == BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL_PHY_DONE) {
      return TRUE;
    }

    gBS->Stall (1 * 1000);
  }

  return FALSE;
}

STATIC
BOOLEAN
PciHostBridgeLibPhyRead (
  IN  EFI_PHYSICAL_ADDRESS  PcieDbiBase,
  IN  UINT32                PhyAddr,
  OUT UINT32               *PhyData
  )
{
  BOOLEAN         Ret;

  MmioWrite32 (PcieDbiBase + BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_LANENUM, 1);
  MmioWrite32 (PcieDbiBase + BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL, PhyAddr);

  Ret = PciHostBridgeLibPhyWaitForDone (PcieDbiBase);
  if (Ret) {
    *PhyData = MmioRead32 (PcieDbiBase + BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_READDATA);
  }

  return Ret;
}

STATIC
BOOLEAN
PciHostBridgeLibPhyWrite (
  IN  EFI_PHYSICAL_ADDRESS  PcieDbiBase,
  IN  UINT32                PhyAddr,
  IN  UINT32                PhyMask,
  IN  UINT16                PhyData
  )
{
  BOOLEAN                     Ret;

  MmioWrite32 (PcieDbiBase + BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_LANENUM, PhyMask);
  MmioWrite32 (PcieDbiBase + BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_WRITEDATA, PhyData);
  MmioWrite32 (
    PcieDbiBase + BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL,
    PhyAddr | BM1000_PCIE_BK_PHY_ACCESS_AXI2MGM_ADDRCTL_PHY_READ_WRITE_FLAG
    );

  Ret = PciHostBridgeLibPhyWaitForDone (PcieDbiBase);

  return Ret;
}

STATIC
BOOLEAN
PciHostBridgeLibSetupPhy (
  IN  UINTN   PcieIdx,
  IN  UINT32  PhyMask
  )
{
  EFI_PHYSICAL_ADDRESS  PcieDbiBase;
  UINT32  PhyData;
  UINT32  OldGenCtl;

  PcieDbiBase = mPcieDbiBases[PcieIdx];

  // Enable access to PHY registers and DBI2 mode
  OldGenCtl = MmioRead32 (BM1000_PCIE_GPR_GEN (PcieIdx));
  MmioOr32 (
    BM1000_PCIE_GPR_GEN (PcieIdx),
    BM1000_PCIE_GPR_GEN_PHY_EN | BM1000_PCIE_GPR_GEN_DBI2_EN
    );

  //
  // Slice RX
  //

  // Set RX CTLE Boost to 10.2 dB
  // Set RX CTLE Peak Value to 5 dB
  // Enable RX CDR, RX DFE, RX AGC
  if (!PciHostBridgeLibPhyRead (PcieDbiBase, BM1000_PCIE_PHY_LANE_RX_CTLE_CTRL, &PhyData)) {
    return FALSE;
  }

  PhyData |= BM1000_PCIE_PHY_LANE_RX_CTLE_CTRL_POLE_OVRRD_EN;

#if 0 //vvv???
  PhyData &= ~BM1000_PCIE_PHY_LANE_RX_CTLE_CTRL_PCS_SDS_ZERO_BITS;
  PhyData |= 8 << BM1000_PCIE_PHY_LANE_RX_CTLE_CTRL_PCS_SDS_ZERO_SHIFT;
#endif

  if (!PciHostBridgeLibPhyWrite (PcieDbiBase, BM1000_PCIE_PHY_LANE_RX_CTLE_CTRL, PhyMask, PhyData)) {
    return FALSE;
  }

  if (!PciHostBridgeLibPhyRead (PcieDbiBase, BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL, &PhyData)) {
    return FALSE;
  }

  PhyData |= BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL_CDR_EN |
             BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL_DFE_EN |
             BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL_AGC_EN |
             BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL_LCTRL_MEN;

  if (!PciHostBridgeLibPhyWrite (PcieDbiBase, BM1000_PCIE_PHY_LANE_RX_LOOP_CTRL, PhyMask, PhyData)) {
    return FALSE;
  }

  //
  // Slice TX
  //

  // Set TX Gain (PCS to SerDes lane TX) TX Launch Amplitude to 1000 mVppd
  if (!PciHostBridgeLibPhyRead (PcieDbiBase, BM1000_PCIE_PHY_LANE_TX_CFG_3, &PhyData)) {
    return FALSE;
  }

  PhyData &= ~BM1000_PCIE_PHY_LANE_TX_CFG_3_PCS_SDS_GAIN_BITS;
  PhyData |= 3 << BM1000_PCIE_PHY_LANE_TX_CFG_3_PCS_SDS_GAIN_SHIFT;

  if (!PciHostBridgeLibPhyWrite (PcieDbiBase, BM1000_PCIE_PHY_LANE_TX_CFG_3, PhyMask, PhyData)) {
    return FALSE;
  }

  // Enable TX Boost (PCS to SerDes lane TX)
  if (!PciHostBridgeLibPhyRead (PcieDbiBase, BM1000_PCIE_PHY_LANE_TX_CFG_3, &PhyData)) {
    return FALSE;
  }

  PhyData |= BM1000_PCIE_PHY_LANE_TX_CFG_3_VBOOST_EN;

  if (!PciHostBridgeLibPhyWrite (PcieDbiBase, BM1000_PCIE_PHY_LANE_TX_CFG_3, PhyMask, PhyData)) {
    return FALSE;
  }

  if (!PciHostBridgeLibPhyRead (PcieDbiBase, BM1000_PCIE_PHY_LANE_TX_CFG_1, &PhyData)) {
    return FALSE;
  }

  PhyData |= BM1000_PCIE_PHY_LANE_TX_CFG_1_VBOOST_EN_OVRRD_EN;

  if (!PciHostBridgeLibPhyWrite (PcieDbiBase, BM1000_PCIE_PHY_LANE_TX_CFG_1, PhyMask, PhyData)) {
    return FALSE;
  }

  // Enable TX Boost (MAC to PCS lane TX)
  if (!PciHostBridgeLibPhyRead (PcieDbiBase, BM1000_PCIE_PHY_LANE_PCS_CTLIFC_CTRL_0, &PhyData)) {
    return FALSE;
  }

  PhyData |= BM1000_PCIE_PHY_LANE_PCS_CTLIFC_CTRL_0_VBOOST_EN_REQ_OVRRD_VAL;

  if (!PciHostBridgeLibPhyWrite (PcieDbiBase, BM1000_PCIE_PHY_LANE_PCS_CTLIFC_CTRL_0, PhyMask, PhyData)) {
    return FALSE;
  }

  if (!PciHostBridgeLibPhyRead (PcieDbiBase, BM1000_PCIE_PHY_LANE_PCS_CTLIFC_CTRL_2, &PhyData)) {
    return FALSE;
  }

  PhyData |= BM1000_PCIE_PHY_LANE_PCS_CTLIFC_CTRL_2_VBOOST_EN_REQ_OVRRD_EN;

  if (!PciHostBridgeLibPhyWrite (PcieDbiBase, BM1000_PCIE_PHY_LANE_PCS_CTLIFC_CTRL_2, PhyMask, PhyData)) {
    return FALSE;
  }

  // Disable TX Turbo (PCS to SerDes lane TX)
  if (!PciHostBridgeLibPhyRead (PcieDbiBase, BM1000_PCIE_PHY_LANE_TX_CFG_3, &PhyData)) {
    return FALSE;
  }

  PhyData &= ~BM1000_PCIE_PHY_LANE_TX_CFG_3_TURBO_EN;

  if (!PciHostBridgeLibPhyWrite (PcieDbiBase, BM1000_PCIE_PHY_LANE_TX_CFG_3, PhyMask, PhyData)) {
    return FALSE;
  }

  if (!PciHostBridgeLibPhyRead (PcieDbiBase, BM1000_PCIE_PHY_LANE_TX_CFG_1, &PhyData)) {
    return FALSE;
  }

  PhyData |= BM1000_PCIE_PHY_LANE_TX_CFG_1_TURBO_EN_OVRRD_EN;

  if (!PciHostBridgeLibPhyWrite (PcieDbiBase, BM1000_PCIE_PHY_LANE_TX_CFG_1, PhyMask, PhyData)) {
    return FALSE;
  }

  // Restore access to PHY registers and DBI2 mode
  MmioWrite32 (BM1000_PCIE_GPR_GEN (PcieIdx), OldGenCtl);

  return TRUE;
}

STATIC
VOID
PciHostBridgeLibCfgWindow (
  IN CONST EFI_PHYSICAL_ADDRESS  PcieDbiBase,
  IN CONST UINTN                 RegionIdx,
  IN CONST UINT64                CpuBase,
  IN CONST UINT64                PciBase,
  IN CONST UINT64                Size,
  IN CONST UINTN                 Type,
  IN CONST UINTN                 EnableFlags
  )
{
  ASSERT ((RegionIdx <= 3) || (PcieDbiBase == BM1000_PCIE2_DBI_BASE && RegionIdx <= 15));
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
PciHostBridgeLinkRetrain (
  IN  EFI_EVENT   Event,
  IN  VOID       *Context
  )
{
  UINTN                 PcieIdx, Iter;
  UINT64                TimeStart;
  UINT32                DevCapSpeed, TargetSpeed, Reg, CapOff;
  EFI_PHYSICAL_ADDRESS  PcieCfgBase;

  DEBUG((EFI_D_INFO, "LinkRetrain called\n"));
  for (Iter = 0; Iter < mPcieRootBridgesNum; Iter++) {
    PcieIdx = mPcieIdxs[Iter];

    if (!PciHostBridgeLibGetLink(PcieIdx))
      continue;

    /* Retrain link to 'max-link-speed' */
    PcieCfgBase = mPcieCfgBases[PcieIdx] + SIZE_1MB;

    /* Find device PCIe capability */
    for (CapOff = 0x40; CapOff; ) {
      Reg = MmioRead32(PcieCfgBase + CapOff);
      if ((Reg & 0xff) == 0x10)
        break; //PCIe capability found
      CapOff = (Reg >> 8) & 0xff; //next capability
    }

    if (CapOff) {
      DevCapSpeed = MmioRead32(PcieCfgBase + CapOff + 0xc) & 0x7;
      if (DevCapSpeed > mPcieMaxLinkSpeed[PcieIdx])
        TargetSpeed = mPcieMaxLinkSpeed[PcieIdx];
      else
        TargetSpeed = DevCapSpeed;
      if (TargetSpeed == ((MmioRead32(mPcieDbiBases[PcieIdx] +
		BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG) >> 16) & 0x7)) {
        DEBUG((EFI_D_INFO, "PcieRoot(0x%x): Speed %d is OK\n", PcieIdx, TargetSpeed));
	continue;
      }

      MmioAndThenOr32 (
         mPcieDbiBases[PcieIdx] +
         BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG,
        ~BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG_TRGT_LNK_SPEED_BITS,
         TargetSpeed
        );
      MmioOr32 (
        mPcieDbiBases[PcieIdx] +
        BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG,
        BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_RETRAIN_LINK
        );
      DEBUG((EFI_D_INFO, "PcieRoot(0x%x): Retraining link to Gen%d\n", PcieIdx, TargetSpeed));
      TimeStart = GetTimeInNanoSecond (GetPerformanceCounter());
      while (((Reg = MmioRead32 (BM1000_PCIE_GPR_STS (PcieIdx))) & 0xff) != 0xd1) {
        if (Reg & 0x1000 /*0x3000*/) {
          DEBUG((EFI_D_INFO, "Link is lost\n"));
          MmioAnd32 (BM1000_PCIE_GPR_GEN (PcieIdx), ~BM1000_PCIE_GPR_GEN_LTSSM_EN);
          goto failedretrain;
        }
        if (GetTimeInNanoSecond (GetPerformanceCounter()) - TimeStart > 1000000000) {
          DEBUG((EFI_D_ERROR, "Timeout! (LinkStatus %08x-%08x)\n",
                MmioRead32(mPcieDbiBases[PcieIdx] +
                BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG),
                MmioRead32 (BM1000_PCIE_GPR_STS (PcieIdx))));
          goto failedretrain;
        }
        gBS->Stall(10000);
      }
      Reg = MmioRead32 (
              mPcieDbiBases[PcieIdx] +
              BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG
            );
      DEBUG((EFI_D_INFO, "PcieRoot(0x%x)[%05ums]: retrained to Gen%u (status %x)\n",
            PcieIdx,
            (GetTimeInNanoSecond (GetPerformanceCounter()) - TimeStart) / 1000000,
            (Reg >> 16) & 0xf,
            Reg));
      Reg = (Reg >> 16) & 0xf;

      if (Reg < TargetSpeed) {
        gBS->Stall(200000);
        MmioOr32 (
          mPcieDbiBases[PcieIdx] +
          BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG,
          BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_RETRAIN_LINK
          );
        DEBUG((EFI_D_INFO, "PcieRoot(0x%x): Retraining link to Gen%d\n", PcieIdx, TargetSpeed));
        TimeStart = GetTimeInNanoSecond (GetPerformanceCounter());
        while (MmioRead32(mPcieDbiBases[PcieIdx] +
               BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG) &
                 0x8000000) {
          if (GetTimeInNanoSecond (GetPerformanceCounter()) - TimeStart > 1000000000) {
            DEBUG((EFI_D_ERROR, "Timeout! (LinkStatus %08x)\n",
                  MmioRead32(mPcieDbiBases[PcieIdx] +
                  BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG)));
            goto failedretrain;
          }
          gBS->Stall(10000);
        }
        Reg = MmioRead32 (
                mPcieDbiBases[PcieIdx] +
                BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG
                );
        Reg = (Reg >> 16) & 0xf;
        DEBUG((EFI_D_INFO, "PcieRoot(0x%x)[%03ums]: retrained(2) to Gen%u\n",
              PcieIdx,
              (GetTimeInNanoSecond (GetPerformanceCounter()) - TimeStart) / 1000000,
              Reg));
      }
    }
failedretrain:
    continue;
  }
}

STATIC
VOID
EFIAPI
PciHostBridgeLibExitBootServices (
  IN EFI_EVENT  Event,
  IN VOID       *Context
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
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_EVENT             Event;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  UINTN                 Iter;
  INT32                 Node = 0;
  UINTN                 PcieIdx;
  UINTN                 PcieNumLanes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  INTN                  PciePerstGpios[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  INTN                  PciePerstPolarity[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  INTN                  Pcie2PrsntGpio;
  INTN                  Pcie2PrsntPolarity;
  UINTN                 PcieMsiIds[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  EFI_STATUS            Status;
  PCI_ROOT_BRIDGE      *lPcieRootBridge;
  CONST VOID           *Prop;
  UINT32                PropSize;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);

  // Acquire PCIe RC related data from FDT
  for (Iter = 0; Iter < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths);) {
    INTN         CfgRegIdx = -1, DbiRegIdx = -1;

    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bm1000-pcie", Node, &Node);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
      continue;
    }

    Status = FdtClient->GetNodeProperty (FdtClient, Node, "baikal,pcie-lcru", &Prop, &PropSize);
    if (EFI_ERROR (Status) || PropSize != 2 * sizeof (UINT32)) {
      continue;
    }

    PcieIdx = SwapBytes32 (((CONST UINT32 *)Prop)[1]);
    if (PcieIdx >= ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)) {
      mPcieIdxs[Iter] = 0;
      continue;
    }

    mPcieIdxs[Iter] = PcieIdx;

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
        PropSize >= (4 * sizeof (UINT64)) && (PropSize % (2 * sizeof (UINT64))) == 0) {
#if !defined(MDEPKG_NDEBUG)
      CONST EFI_PHYSICAL_ADDRESS  DbiBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + DbiRegIdx * 2));
      CONST UINTN                 DbiSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + DbiRegIdx * 2 + 1));
#endif
      CONST EFI_PHYSICAL_ADDRESS  CfgBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + CfgRegIdx * 2));
      CONST UINTN                 CfgSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + CfgRegIdx * 2 + 1));

      ASSERT (DbiBase == mPcieDbiBases[PcieIdx]);
      ASSERT (DbiSize == mPcieDbiSizes[PcieIdx]);
      ASSERT (CfgSize > SIZE_2MB);
      ASSERT ((CfgSize & (SIZE_1MB - 1)) == 0);

      mPcieCfgBases[PcieIdx] = CfgBase & ~0xfffffffULL;
      if (mPcieCfgBases[PcieIdx] + SIZE_1MB < CfgBase) {
        DEBUG((EFI_D_ERROR, "PcieRoot(0x%x): Invalid config region @ %llx\n",
               PcieIdx, CfgBase));
        continue;
      }
      mPcieCfgSizes[PcieIdx] = CfgSize + CfgBase - mPcieCfgBases[PcieIdx];
      if (mPcieCfgSizes[PcieIdx] < SIZE_2MB) {
        DEBUG((EFI_D_ERROR, "PcieRoot(0x%x): Invalid size for config region @ %llx\n",
               PcieIdx, CfgBase));
        continue;
      } else if (mPcieCfgSizes[PcieIdx] > (SIZE_1MB * 255)) {
        mPcieCfgSizes[PcieIdx] = SIZE_1MB * 255;
      }
    } else {
      continue;
    }

    PcieIdx = mPcieIdxs[Iter];

    if (FdtClient->GetNodeProperty (FdtClient, Node, "ranges", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize > 0 && (PropSize % (sizeof (UINT32) + 3 * sizeof (UINT64))) == 0) {
      UINTN  Entry;

      mPcieIoMins[PcieIdx]  = MAX_UINT64;
      mPcieMemMins[PcieIdx] = MAX_UINT64;

      for (Entry = 0; Entry < PropSize / (sizeof (UINT32) + 3 * sizeof (UINT64)); ++Entry) {
        UINTN                 Flags;
        EFI_PHYSICAL_ADDRESS  PciBase;
        EFI_PHYSICAL_ADDRESS  CpuBase;
        UINTN                 Size;

        Flags   = SwapBytes32 (((CONST UINT32 *) Prop)[0]);
        PciBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) ((EFI_PHYSICAL_ADDRESS) Prop + sizeof (UINT32))));
        CpuBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) ((EFI_PHYSICAL_ADDRESS) Prop + sizeof (UINT32) + 1 * sizeof (UINT64))));
        Size    = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) ((EFI_PHYSICAL_ADDRESS) Prop + sizeof (UINT32) + 2 * sizeof (UINT64))));

        if (Flags & RANGES_FLAG_IO) {
          mPcieIoMins[PcieIdx]  = PciBase;
          mPcieIoBases[PcieIdx] = CpuBase;
          mPcieIoSizes[PcieIdx] = Size;
        } else if (Flags & RANGES_FLAG_MEM) {
          mPcieMemMins[PcieIdx]  = PciBase;
          mPcieMemBases[PcieIdx] = CpuBase;
          mPcieMemSizes[PcieIdx] = Size;
        }

        Prop = &(((CONST UINT32 *) Prop)[7]);
      }
    } else {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "num-lanes", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == sizeof (UINT32)) {
      PcieNumLanes[PcieIdx] = SwapBytes32 (*(CONST UINT32 *) Prop);
    } else {
      PcieNumLanes[PcieIdx] = 0;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "max-link-speed", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == 4) {
      mPcieMaxLinkSpeed[PcieIdx] = SwapBytes32 (((CONST UINT32 *) Prop)[0]);
    } else {
      mPcieMaxLinkSpeed[PcieIdx] = 3;
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
        PropSize == 4 * sizeof (UINT32)) {
      PcieMsiIds[PcieIdx]    = SwapBytes32 (((CONST UINT32 *) Prop)[2]) >> 16;
    } else {
      PcieMsiIds[PcieIdx] = 0;
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
      BM1000_PCIE_GPR_MSI_TRANS2,
    ~(BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE0_MASK |
      BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE1_MASK |
      BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE2_MASK |
      BM1000_PCIE_GPR_MSI_TRANS2_PCIE0_MSI_TRANS_EN   |
      BM1000_PCIE_GPR_MSI_TRANS2_PCIE1_MSI_TRANS_EN   |
      BM1000_PCIE_GPR_MSI_TRANS2_PCIE2_MSI_TRANS_EN)
    );

  mPcieRootBridges = AllocateZeroPool (mPcieRootBridgesNum * sizeof (PCI_ROOT_BRIDGE));
  lPcieRootBridge  = &mPcieRootBridges[0];

  INT32  SubNode;

  SubNode = 0;
  Pcie2PrsntGpio     = -1;
  Pcie2PrsntPolarity = -1;

  Status = FdtClient->FindNextCompatibleNode (FdtClient, "snps,dw-apb-gpio-port", SubNode, &SubNode);
  if (Status == EFI_SUCCESS) {
    Status = FdtClient->FindNextSubnode (FdtClient, "pcieclk", SubNode, &SubNode);
    if (Status == EFI_SUCCESS) {
      Status = FdtClient->GetNodeProperty (FdtClient, SubNode, "line-name", &Prop, &PropSize);
      if (Status == EFI_SUCCESS && AsciiStrCmp ((CONST CHAR8 *) Prop, "pcie-x8-clock") == 0) {
        Status = FdtClient->GetNodeProperty (FdtClient, SubNode, "gpios", &Prop, &PropSize);
        if (Status == EFI_SUCCESS && PropSize == 2 * sizeof (UINT32)) {
          Pcie2PrsntGpio     = SwapBytes32 (((CONST UINT32 *) Prop)[0]);
          Pcie2PrsntPolarity = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
        }
      }
    }
  }

  // Assert PRSNT pin
  if (Pcie2PrsntGpio >= 0 &&
      Pcie2PrsntGpio <= 31 &&
      Pcie2PrsntPolarity >= 0) {
    if (Pcie2PrsntPolarity) {
      GpioOutRst (BM1000_GPIO32_BASE, Pcie2PrsntGpio);
    } else {
      GpioOutSet (BM1000_GPIO32_BASE, Pcie2PrsntGpio);
    }

    GpioDirSet (BM1000_GPIO32_BASE, Pcie2PrsntGpio);
  }

  // Initialise PCIe RCs
  for (Iter = 0; Iter < mPcieRootBridgesNum; ++Iter, ++lPcieRootBridge) {
    UINT32   ResetMask;
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
    lPcieRootBridge->Io.Base           = mPcieIoMins[PcieIdx];
    lPcieRootBridge->Io.Limit          = mPcieIoMins[PcieIdx] + mPcieIoSizes[PcieIdx] - 1;
    lPcieRootBridge->Io.Translation    = mPcieIoMins[PcieIdx] - mPcieIoBases[PcieIdx];
    lPcieRootBridge->Mem.Base          = mPcieMemMins[PcieIdx];
    lPcieRootBridge->Mem.Limit         = mPcieMemMins[PcieIdx] + mPcieMemSizes[PcieIdx] - 1;
    lPcieRootBridge->Mem.Translation   = mPcieMemMins[PcieIdx] - mPcieMemBases[PcieIdx];
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

    MmioAnd32 (BM1000_PCIE_GPR_GEN (PcieIdx), ~BM1000_PCIE_GPR_GEN_LTSSM_EN);

    // Assert PCIe RC resets
    ResetMask =
      BM1000_PCIE_GPR_RST_NONSTICKY_RST | BM1000_PCIE_GPR_RST_STICKY_RST  |
      BM1000_PCIE_GPR_RST_PWR_RST       | BM1000_PCIE_GPR_RST_CORE_RST    |
      BM1000_PCIE_GPR_RST_PIPE0_RST     | BM1000_PCIE_GPR_RST_PHY_RST;
    if (PcieNumLanes[PcieIdx] == 8) {
      ResetMask |= BM1000_PCIE_GPR_RST_PIPE1_RST;
    }
    MmioOr32 (BM1000_PCIE_GPR_RST(PcieIdx), ResetMask);

    gBS->Stall (1); // Delay at least 100 ns

    // Deassert PCIe RC resets
    MmioAnd32 (BM1000_PCIE_GPR_RST(PcieIdx),
      ~(ResetMask |
      BM1000_PCIE_GPR_RST_ADB_PWRDWN    | BM1000_PCIE_GPR_RST_HOT_RST)
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

    if (!PciHostBridgeLibSetupPhy (PcieIdx, (1 << PcieNumLanes[PcieIdx]) - 1)) {
      DEBUG ((EFI_D_ERROR, "PcieRoot(0x%x): cannot setup PHY\n", PcieIdx));
    }

    // Enable writing read-only registers using DBI
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

    // Disable writing read-only registers using DBI
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
           BM1000_PCIE_GPR_MSI_TRANS2,
          ~BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE0_MASK,
           BM1000_PCIE_GPR_MSI_TRANS2_PCIE0_MSI_TRANS_EN | (PcieMsiIds[PcieIdx] << 0)
          );
      } else if (PcieIdx == BM1000_PCIE1_IDX) {
        MmioAndThenOr32 (
           BM1000_PCIE_GPR_MSI_TRANS2,
          ~BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE1_MASK,
           BM1000_PCIE_GPR_MSI_TRANS2_PCIE1_MSI_TRANS_EN | (PcieMsiIds[PcieIdx] << 2)
          );
      } else if (PcieIdx == BM1000_PCIE2_IDX) {
        MmioAndThenOr32 (
           BM1000_PCIE_GPR_MSI_TRANS2,
          ~BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE2_MASK,
           BM1000_PCIE_GPR_MSI_TRANS2_PCIE2_MSI_TRANS_EN | (PcieMsiIds[PcieIdx] << 4)
          );
      }
    }

    // Region 0: CFG0
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      0,
      mPcieCfgBases[PcieIdx] + SIZE_1MB,
      0, // See AcpiPlatformDxe/Iort.c for implications of using 0 here instead of encoding the bus
      SIZE_64KB,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG0,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE
      );

    // Region 1: CFG1
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      1,
      mPcieCfgBases[PcieIdx] + SIZE_2MB,
      0,
      mPcieCfgSizes[PcieIdx] - SIZE_2MB,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE
      );

    // Region 2: MEM
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      2,
      mPcieMemBases[PcieIdx],
      lPcieRootBridge->Mem.Base,
      lPcieRootBridge->Mem.Limit - lPcieRootBridge->Mem.Base + 1,
      BM1000_PCIE_PF0_PORT_LOGIC_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM,
      0
      );

    // Region 3: I/O
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      3,
      mPcieIoBases[PcieIdx],
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

    // Configure Preset Request Vector
    MmioAndThenOr32 (
      mPcieDbiBases[PcieIdx] + BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF,
      ~BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF_PSET_REQ_VEC_BITS,
      0x185 << BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF_PSET_REQ_VEC_SHIFT
      );

    // Configure feedback mode
    MmioAndThenOr32 (
      mPcieDbiBases[PcieIdx] + BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF,
      ~BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF_FB_MODE_BITS,
      0 << BM1000_PCIE_PF0_PORT_LOGIC_GEN3_EQ_CONTROL_OFF_FB_MODE_SHIFT
      );

    MmioOr32 (BM1000_PCIE_GPR_GEN (PcieIdx), BM1000_PCIE_GPR_GEN_LTSSM_EN);

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
    TimeStart = GetTimeInNanoSecond (GetPerformanceCounter ());
    for (;;) {
      CONST UINT32  PcieGprSts = MmioRead32 (BM1000_PCIE_GPR_STS (PcieIdx));
      UINT64        PerformanceCounter = GetPerformanceCounter ();

      if (!ComponentExists) {
        if ((PcieGprSts & BM1000_PCIE_GPR_STS_LTSSM_STATE_MASK) > 0x01) {
          ComponentExists = TRUE;
          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link partner detected\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprSts & BM1000_PCIE_GPR_STS_LTSSM_STATE_MASK,
            PcieGprSts & BM1000_PCIE_GPR_STS_SMLH_LINKUP ? '+' : '-',
            PcieGprSts & BM1000_PCIE_GPR_STS_RDLH_LINKUP ? '+' : '-'
            ));
        } else if (GetTimeInNanoSecond (PerformanceCounter) - TimeStart > 50000000) {
          // According to PCI Express Base Specification device must enter LTSSM detect state within 20 ms of reset
          break;
        }
      }

      if (ComponentExists) {
        if (((PcieGprSts & BM1000_PCIE_GPR_STS_LTSSM_STATE_MASK) ==
                           BM1000_PCIE_GPR_STS_LTSSM_STATE_L0) &&
             (PcieGprSts & BM1000_PCIE_GPR_STS_SMLH_LINKUP) &&
             (PcieGprSts & BM1000_PCIE_GPR_STS_RDLH_LINKUP)) {
#if !defined(MDEPKG_NDEBUG)
          CONST UINT32  PcieLnkSts = MmioRead32 (
                                       mPcieDbiBases[PcieIdx] +
                                       BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG
                                       );
          CONST CHAR8  *LinkSpeedString;
          CONST UINTN   LinkSpeedVector = (PcieLnkSts &
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
            PcieGprSts & BM1000_PCIE_GPR_STS_LTSSM_STATE_MASK,
            PcieGprSts & BM1000_PCIE_GPR_STS_SMLH_LINKUP ? '+' : '-',
            PcieGprSts & BM1000_PCIE_GPR_STS_RDLH_LINKUP ? '+' : '-',
            LinkSpeedString,
            (PcieLnkSts & BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_BITS) >>
              BM1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_SHIFT
            ));
#endif
          // Wait until device starts responding to cfg requests
          while (MmioRead32 (mPcieCfgBases[PcieIdx] + SIZE_1MB) == 0) {
            PerformanceCounter = GetPerformanceCounter ();
            MmioWrite32(mPcieCfgBases[PcieIdx] + SIZE_1MB, 0xffffffff);
            gBS->Stall (1000);
            if (((GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000) > 1000) {
              break;
            }
          }
          DEBUG((EFI_D_INFO,
            "PcieRoot(0x%x): [%dms]: dev_id at 1:0.0 - %x, dev_id at 1:1.0 - %x\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            MmioRead32 (mPcieCfgBases[PcieIdx] + SIZE_1MB),
            MmioRead32 (mPcieCfgBases[PcieIdx] + SIZE_1MB + 0x8000)));

          if (MmioRead32 (mPcieCfgBases[PcieIdx] + SIZE_1MB) != 0xFFFFFFFF &&
              MmioRead32 (mPcieCfgBases[PcieIdx] + SIZE_1MB + 0x8000) == 0xFFFFFFFF) {
            //
            // Device appears to filter CFG0 requests, so the 64 KiB granule for the iATU
            // isn't a problem. We don't have to ignore fn > 0 or shift MCFG by 0x8000.
            //
#if !defined(MDEPKG_NDEBUG)
            DEBUG ((EFI_D_INFO, ", Cfg0Filter+\n"));
#endif
          } else if (((MmioRead32 (mPcieCfgBases[PcieIdx] + SIZE_1MB + 0xc) >> 16) & 0xff) == 0x1) {
            /* Type 1 config header */
#if !defined(MDEPKG_NDEBUG)
            DEBUG ((EFI_D_INFO, ", Cfg0Filter- (Type 1)\n"));
#endif
          } else {
#if !defined(MDEPKG_NDEBUG)
            DEBUG ((EFI_D_INFO, ", Cfg0Filter-\n"));
#endif
            mPcieCfg0Quirk |= 1 << PcieIdx;
            Status = PcdSet32S (PcdPcieCfg0Quirk, mPcieCfg0Quirk);
            ASSERT_EFI_ERROR (Status);
          }

          break;
        } else if (GetTimeInNanoSecond (PerformanceCounter) - TimeStart >= 200000000) {
          // Wait up to 200 ms for link up
          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link is inactive\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprSts & BM1000_PCIE_GPR_STS_LTSSM_STATE_MASK,
            PcieGprSts & BM1000_PCIE_GPR_STS_SMLH_LINKUP ? '+' : '-',
            PcieGprSts & BM1000_PCIE_GPR_STS_RDLH_LINKUP ? '+' : '-'
            ));

          //
          // System hangups have been observed when PCIe partner enters detect state
          // but does not reach L0. Disabling LTSSM helps to prevent these hangups.
          //
          MmioAnd32 (BM1000_PCIE_GPR_GEN (PcieIdx), ~BM1000_PCIE_GPR_GEN_LTSSM_EN);
          break;
        } else if ((PcieGprSts & BM1000_PCIE_GPR_STS_LTSSM_STATE_MASK) == 0x03 ||
                   (PcieGprSts & BM1000_PCIE_GPR_STS_LTSSM_STATE_MASK) == 0x05) {
          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: %a\n",
            PcieIdx,
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieGprSts & BM1000_PCIE_GPR_STS_LTSSM_STATE_MASK,
            PcieGprSts & BM1000_PCIE_GPR_STS_SMLH_LINKUP ? '+' : '-',
            PcieGprSts & BM1000_PCIE_GPR_STS_RDLH_LINKUP ? '+' : '-',
            (PcieGprSts & BM1000_PCIE_GPR_STS_LTSSM_STATE_MASK) == 0x03 ?
              "Polling.Compliance" : "Pre.Detect.Quiet"
            ));

          MmioAnd32 (BM1000_PCIE_GPR_GEN (PcieIdx), ~BM1000_PCIE_GPR_GEN_LTSSM_EN);

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
             BM1000_PCIE_GPR_RST (PcieIdx),
             BM1000_PCIE_GPR_RST_CORE_RST
             );

          gBS->Stall (1000);
          MmioAnd32 (
             BM1000_PCIE_GPR_RST (PcieIdx),
            ~BM1000_PCIE_GPR_RST_CORE_RST
            );

          MmioOr32 (BM1000_PCIE_GPR_GEN (PcieIdx), BM1000_PCIE_GPR_GEN_LTSSM_EN);

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

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  PciHostBridgeLinkRetrain,
                  NULL,
                  &gEfiEventReadyToBootGuid,
                  &Event
                  );

  return EFI_SUCCESS;
}

BOOLEAN
PciHostBridgeLibGetLink (
  IN CONST UINTN  PcieIdx
  )
{
  ASSERT (PcieIdx < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths));

  CONST UINT32  PcieGprSts = MmioRead32 (BM1000_PCIE_GPR_STS (PcieIdx));
  return ((PcieGprSts & BM1000_PCIE_GPR_STS_LTSSM_STATE_MASK) ==
                        BM1000_PCIE_GPR_STS_LTSSM_STATE_L0) &&
          (PcieGprSts & BM1000_PCIE_GPR_STS_SMLH_LINKUP) &&
          (PcieGprSts & BM1000_PCIE_GPR_STS_RDLH_LINKUP);
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
