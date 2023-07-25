/** @file
  Copyright (c) 2022 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/GpioLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciHostBridgeLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>

#include <BS1000.h>

#define BS1000_PCIE_APB_PE_GEN_CTRL3                   0x58
#define BS1000_PCIE_APB_PE_GEN_CTRL3_LTSSM_EN          BIT0

#define BS1000_PCIE_APB_PE_LINK_DBG2                   0xB4
#define BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_L0    0x11
#define BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_MASK  0x3F
#define BS1000_PCIE_APB_PE_LINK_DBG2_SMLH_LINKUP       BIT6
#define BS1000_PCIE_APB_PE_LINK_DBG2_RDLH_LINKUP       BIT7

#define BS1000_PCIE_PF0_TYPE1_HDR_TYPE1_CLASS_CODE_REV_ID_REG                        0x008
#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG                               0x07C
#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_BITS                (0x3F << 4)
#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_SHIFT               4

#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG                        0x080
#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_RETRAIN_LINK           BIT5
#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_BITS        (0xF << 16)
#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_SHIFT       16
#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_BITS   (0x3F << 20)
#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_SHIFT  20

#define BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF                                0x710
#define BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_BITS              (0x3F << 16)
#define BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X1                (0x01 << 16)
#define BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X2                (0x03 << 16)
#define BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X4                (0x07 << 16)
#define BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X8                (0x0F << 16)
#define BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X16               (0x1F << 16)

#define BS1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF                                     0x80C
#define BS1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_BITS                   (0x1F << 8)
#define BS1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_SHIFT                  8

#define BS1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF                                0x8BC
#define BS1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN                   BIT0

#define BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0                    0x300000
#define BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM           0
#define BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_IO            2
#define BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG0          4
#define BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG1          5

#define BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_2_OFF_OUTBOUND_0                    0x300004
#define BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE     BIT28
#define BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_REGION_EN          BIT31

#define BS1000_PCIE_PF0_ATU_CAP_IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0                    0x300008
#define BS1000_PCIE_PF0_ATU_CAP_IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0                  0x30000C
#define BS1000_PCIE_PF0_ATU_CAP_IATU_LIMIT_ADDR_OFF_OUTBOUND_0                       0x300010
#define BS1000_PCIE_PF0_ATU_CAP_IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0                  0x300014
#define BS1000_PCIE_PF0_ATU_CAP_IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0                0x300018
#define BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_SIZE                                     0x200

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
      3
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
      4
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
      5
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
      6
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
      7
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
      8
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
      9
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
      10
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
      11
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
      12
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
      13
    },
    {END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, {END_DEVICE_PATH_LENGTH, 0}}
  }
};

STATIC CONST EFI_PHYSICAL_ADDRESS  mPcieApbBases[] = {
  BS1000_PCIE0_APB_P0_CTRL_BASE,
  BS1000_PCIE0_APB_P1_CTRL_BASE,
  BS1000_PCIE1_APB_P0_CTRL_BASE,
  BS1000_PCIE1_APB_P1_CTRL_BASE,
  BS1000_PCIE2_APB_P0_CTRL_BASE,
  BS1000_PCIE2_APB_P1_CTRL_BASE,
  BS1000_PCIE3_APB_P0_CTRL_BASE,
  BS1000_PCIE3_APB_P1_CTRL_BASE,
  BS1000_PCIE3_APB_P2_CTRL_BASE,
  BS1000_PCIE3_APB_P3_CTRL_BASE,
  BS1000_PCIE4_APB_P0_CTRL_BASE,
  BS1000_PCIE4_APB_P1_CTRL_BASE,
  BS1000_PCIE4_APB_P2_CTRL_BASE,
  BS1000_PCIE4_APB_P3_CTRL_BASE
};

CONST EFI_PHYSICAL_ADDRESS  mPcieDbiBases[] = {
  BS1000_PCIE0_P0_DBI_BASE,
  BS1000_PCIE0_P1_DBI_BASE,
  BS1000_PCIE1_P0_DBI_BASE,
  BS1000_PCIE1_P1_DBI_BASE,
  BS1000_PCIE2_P0_DBI_BASE,
  BS1000_PCIE2_P1_DBI_BASE,
  BS1000_PCIE3_P0_DBI_BASE,
  BS1000_PCIE3_P1_DBI_BASE,
  BS1000_PCIE3_P2_DBI_BASE,
  BS1000_PCIE3_P3_DBI_BASE,
  BS1000_PCIE4_P0_DBI_BASE,
  BS1000_PCIE4_P1_DBI_BASE,
  BS1000_PCIE4_P2_DBI_BASE,
  BS1000_PCIE4_P3_DBI_BASE
};

STATIC CONST UINTN  mPcieDbiSizes[] = {
  BS1000_PCIE0_P0_DBI_SIZE,
  BS1000_PCIE0_P1_DBI_SIZE,
  BS1000_PCIE1_P0_DBI_SIZE,
  BS1000_PCIE1_P1_DBI_SIZE,
  BS1000_PCIE2_P0_DBI_SIZE,
  BS1000_PCIE2_P1_DBI_SIZE,
  BS1000_PCIE3_P0_DBI_SIZE,
  BS1000_PCIE3_P1_DBI_SIZE,
  BS1000_PCIE3_P2_DBI_SIZE,
  BS1000_PCIE3_P3_DBI_SIZE,
  BS1000_PCIE4_P0_DBI_SIZE,
  BS1000_PCIE4_P1_DBI_SIZE,
  BS1000_PCIE4_P2_DBI_SIZE,
  BS1000_PCIE4_P3_DBI_SIZE
};

STATIC CONST EFI_PHYSICAL_ADDRESS  mPcieMmioBases[] = {
  BS1000_PCIE0_P0_MMIO_BASE,
  BS1000_PCIE0_P1_MMIO_BASE,
  BS1000_PCIE1_P0_MMIO_BASE,
  BS1000_PCIE1_P1_MMIO_BASE,
  BS1000_PCIE2_P0_MMIO_BASE,
  BS1000_PCIE2_P1_MMIO_BASE,
  BS1000_PCIE3_P0_MMIO_BASE,
  BS1000_PCIE3_P1_MMIO_BASE,
  BS1000_PCIE3_P2_MMIO_BASE,
  BS1000_PCIE3_P3_MMIO_BASE,
  BS1000_PCIE4_P0_MMIO_BASE,
  BS1000_PCIE4_P1_MMIO_BASE,
  BS1000_PCIE4_P2_MMIO_BASE,
  BS1000_PCIE4_P3_MMIO_BASE
};

STATIC CONST UINTN  mPcieMmioSizes[] = {
  BS1000_PCIE0_P0_MMIO_SIZE,
  BS1000_PCIE0_P1_MMIO_SIZE,
  BS1000_PCIE1_P0_MMIO_SIZE,
  BS1000_PCIE1_P1_MMIO_SIZE,
  BS1000_PCIE2_P0_MMIO_SIZE,
  BS1000_PCIE2_P1_MMIO_SIZE,
  BS1000_PCIE3_P0_MMIO_SIZE,
  BS1000_PCIE3_P1_MMIO_SIZE,
  BS1000_PCIE3_P2_MMIO_SIZE,
  BS1000_PCIE3_P3_MMIO_SIZE,
  BS1000_PCIE4_P0_MMIO_SIZE,
  BS1000_PCIE4_P1_MMIO_SIZE,
  BS1000_PCIE4_P2_MMIO_SIZE,
  BS1000_PCIE4_P3_MMIO_SIZE
};

STATIC CONST CHAR8  *mPcieNames[] = {
  "PCIe0.P0",
  "PCIe0.P1",
  "PCIe1.P0",
  "PCIe1.P1",
  "PCIe2.P0",
  "PCIe2.P1",
  "PCIe3.P0",
  "PCIe3.P1",
  "PCIe3.P2",
  "PCIe3.P3",
  "PCIe4.P0",
  "PCIe4.P1",
  "PCIe4.P2",
  "PCIe4.P3"
};

UINT32                        mPcieCfg0FilteringWorks;
EFI_PHYSICAL_ADDRESS          mPcieCfgBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                  mPcieCfgSizes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                  mPcieIdxs[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC EFI_PHYSICAL_ADDRESS   mPcieIoBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC EFI_PHYSICAL_ADDRESS   mPcieIoMins[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                  mPcieIoSizes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC EFI_PHYSICAL_ADDRESS   mPcieMemBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC EFI_PHYSICAL_ADDRESS   mPcieMemMins[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC UINTN                  mPcieMemSizes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
STATIC PCI_ROOT_BRIDGE       *mPcieRootBridges;
STATIC UINTN                  mPcieRootBridgesNum;

STATIC_ASSERT (
  ARRAY_SIZE (mPcieApbBases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieApbBases) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieDbiBases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieDbiBases) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieDbiSizes) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieDbiSizes) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieMmioBases) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieMmioBases) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieMmioSizes) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieMmioSizes) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieNames) == ARRAY_SIZE (mEfiPciRootBridgeDevicePaths),
  "ARRAY_SIZE (mPcieNames) != ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)"
  );

STATIC
VOID
PciHostBridgeLibCfgWindow (
  IN CONST EFI_PHYSICAL_ADDRESS  PcieDbiBase,
  IN CONST UINTN                 RegionIdx,
  IN       UINT64                CpuBase,
  IN CONST UINT64                PciBase,
  IN CONST UINT64                Size,
  IN CONST UINTN                 Type,
  IN CONST UINTN                 EnableFlags
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

  if (PcieDbiBase == BS1000_PCIE0_P0_DBI_BASE ||
      PcieDbiBase == BS1000_PCIE0_P1_DBI_BASE ||
      PcieDbiBase == BS1000_PCIE1_P0_DBI_BASE ||
      PcieDbiBase == BS1000_PCIE1_P1_DBI_BASE ||
      PcieDbiBase == BS1000_PCIE2_P0_DBI_BASE ||
      PcieDbiBase == BS1000_PCIE2_P1_DBI_BASE) {
    CpuBase &= 0x7FFFFFFFFF;
  } else {
    CpuBase &= 0xFFFFFFFFFF;
  }

  ArmDataMemoryBarrier ();
  MmioWrite32 (
    PcieDbiBase + BS1000_PCIE_PF0_ATU_CAP_IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0 +
      RegionIdx * BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_SIZE,
    (UINT32)(CpuBase & 0xFFFFFFFF)
    );

  MmioWrite32 (
    PcieDbiBase + BS1000_PCIE_PF0_ATU_CAP_IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0 +
      RegionIdx * BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_SIZE,
    (UINT32)(CpuBase >> 32)
    );

  MmioWrite32 (
    PcieDbiBase + BS1000_PCIE_PF0_ATU_CAP_IATU_LIMIT_ADDR_OFF_OUTBOUND_0 +
      RegionIdx * BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_SIZE,
    (UINT32)(CpuBase + Size - 1)
    );

  MmioWrite32 (
    PcieDbiBase + BS1000_PCIE_PF0_ATU_CAP_IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0 +
      RegionIdx * BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_SIZE,
    (UINT32)(PciBase & 0xFFFFFFFF)
    );

  MmioWrite32 (
    PcieDbiBase + BS1000_PCIE_PF0_ATU_CAP_IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0 +
      RegionIdx * BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_SIZE,
    (UINT32)(PciBase >> 32)
    );

  MmioWrite32 (
    PcieDbiBase + BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0 +
      RegionIdx * BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_SIZE,
    Type
    );

  MmioWrite32 (
    PcieDbiBase + BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_2_OFF_OUTBOUND_0 +
      RegionIdx * BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_SIZE,
    BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_REGION_EN | EnableFlags
    );
}

EFI_STATUS
EFIAPI
PciHostBridgeLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  FDT_CLIENT_PROTOCOL  *FdtClient;
  UINTN                 Iter;
  INT32                 Node;
  UINTN                 PcieIdx;
  UINTN                 PcieNumLanes[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  UINTN                 PciePerstGpios[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  UINTN                 PciePerstPolarity[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  UINTN                 PcieLinkSpeed[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  EFI_PHYSICAL_ADDRESS  PciePerstBases[ARRAY_SIZE (mEfiPciRootBridgeDevicePaths)];
  EFI_STATUS            Status;
  PCI_ROOT_BRIDGE      *lPcieRootBridge;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);

  // Acquire PCIe RC related data from FDT
  for (Node = 0, Iter = 0; Iter < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths);) {
    INTN         ApbRegIdx = -1, CfgRegIdx = -1, DbiRegIdx = -1;
    CONST VOID  *Prop;
    UINT32       PropSize;

    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bs1000-pcie", Node, &Node);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reg-names", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize > 0) {
      UINTN         RegIdx;
      CONST CHAR8  *StrPtr = Prop;

      for (RegIdx = 0; PropSize > 0; ++RegIdx) {
        CONST UINTN  StrSize = AsciiStrSize (StrPtr);

        ASSERT (StrSize <= PropSize);

        if (AsciiStrCmp (StrPtr, "dbi") == 0) {
          DbiRegIdx = RegIdx;
        } else if (AsciiStrCmp (StrPtr, "apb") == 0) {
          ApbRegIdx = RegIdx;
        } else if (AsciiStrCmp (StrPtr, "config") == 0) {
          CfgRegIdx = RegIdx;
        }

        PropSize -= StrSize;
        StrPtr   += StrSize;
      }

      if (ApbRegIdx == -1 || CfgRegIdx == -1 || DbiRegIdx == -1) {
        continue;
      }
    } else {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize >= (3 * 2 * sizeof (UINT64)) && (PropSize % (2 * sizeof (UINT64))) == 0) {
      CONST EFI_PHYSICAL_ADDRESS  DbiBase = SwapBytes64 (((CONST UINT64 *) Prop)[DbiRegIdx * 2]);
#if !defined(MDEPKG_NDEBUG)
      CONST UINTN                 DbiSize = SwapBytes64 (((CONST UINT64 *) Prop)[DbiRegIdx * 2 + 1]);
      CONST EFI_PHYSICAL_ADDRESS  ApbBase = SwapBytes64 (((CONST UINT64 *) Prop)[ApbRegIdx * 2]);
      CONST UINTN                 ApbSize = SwapBytes64 (((CONST UINT64 *) Prop)[ApbRegIdx * 2 + 1]);
#endif
      CONST EFI_PHYSICAL_ADDRESS  CfgBase = SwapBytes64 (((CONST UINT64 *) Prop)[CfgRegIdx * 2]);
      CONST UINTN                 CfgSize = SwapBytes64 (((CONST UINT64 *) Prop)[CfgRegIdx * 2 + 1]);

      for (PcieIdx = 0; PcieIdx < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths); ++PcieIdx) {
        if (DbiBase == mPcieDbiBases[PcieIdx]) {
          ASSERT (ApbBase == mPcieApbBases[PcieIdx]);
          ASSERT (ApbSize == 0x100);
          ASSERT (DbiSize == mPcieDbiSizes[PcieIdx]);
          ASSERT (CfgBase >= mPcieMmioBases[PcieIdx]);
          ASSERT (CfgBase <  mPcieMmioBases[PcieIdx] + mPcieMmioSizes[PcieIdx]);
          ASSERT (CfgSize >  SIZE_2MB);
          ASSERT (CfgSize <= mPcieMmioSizes[PcieIdx]);
          ASSERT ((CfgSize & (SIZE_1MB - 1)) == 0);

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
        PciBase = SwapBytes64 (*(CONST UINT64 *) ((EFI_PHYSICAL_ADDRESS) Prop + sizeof (UINT32)));
        CpuBase = SwapBytes64 (*(CONST UINT64 *) ((EFI_PHYSICAL_ADDRESS) Prop + sizeof (UINT32) + 1 * sizeof (UINT64)));
        Size    = SwapBytes64 (*(CONST UINT64 *) ((EFI_PHYSICAL_ADDRESS) Prop + sizeof (UINT32) + 2 * sizeof (UINT64)));

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
        PropSize == sizeof (UINT32)) {
      PcieLinkSpeed[PcieIdx] = SwapBytes32 (*(CONST UINT32 *) Prop);
    } else {
      PcieLinkSpeed[PcieIdx] = 0;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reset-gpios", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize >= (3 * sizeof (UINT32))) {
      INT32        PerstGpioPhandle;
      INT32        PerstGpioNode;
      PerstGpioPhandle           = SwapBytes32 (*(CONST UINT32 *) Prop);
      PciePerstGpios[PcieIdx]    = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
      PciePerstPolarity[PcieIdx] = SwapBytes32 (((CONST UINT32 *) Prop)[2]);
      if ((FdtClient->FindNodeByPhandle (FdtClient, PerstGpioPhandle, &PerstGpioNode) == EFI_SUCCESS) &&
        (FdtClient->FindParentNode (FdtClient, PerstGpioNode, &PerstGpioNode) == EFI_SUCCESS)) {
        if (FdtClient->GetNodeProperty (FdtClient, PerstGpioNode, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
            PropSize == 2 * sizeof (UINT64)) {
          PciePerstBases[PcieIdx] = SwapBytes64 (*(CONST UINT64 *) Prop);
        } else {
          PciePerstBases[PcieIdx] = 0;
        }
      } else {
        PciePerstBases[PcieIdx] = 0;
      }
    } else {
      PciePerstBases[PcieIdx] = 0;
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
    lPcieRootBridge->AllocationAttributes  = EFI_PCI_HOST_BRIDGE_MEM64_DECODE | EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM;

    lPcieRootBridge->Bus.Base          = 0;
    lPcieRootBridge->Bus.Limit         = mPcieCfgSizes[PcieIdx] / SIZE_1MB - 1;
    lPcieRootBridge->Io.Base           = mPcieIoMins[PcieIdx];
    lPcieRootBridge->Io.Limit          = mPcieIoMins[PcieIdx] + mPcieIoSizes[PcieIdx] - 1;
    lPcieRootBridge->Io.Translation    = mPcieIoMins[PcieIdx] - mPcieIoBases[PcieIdx];
    lPcieRootBridge->Mem.Base          = mPcieMemMins[PcieIdx];
    lPcieRootBridge->Mem.Limit         = mPcieMemMins[PcieIdx] + mPcieMemSizes[PcieIdx] - 1;
    lPcieRootBridge->Mem.Translation   = mPcieMemMins[PcieIdx] - mPcieMemBases[PcieIdx];
    lPcieRootBridge->MemAbove4G.Base   = mPcieMemBases[PcieIdx] + 0x100000000; // TODO: investigate it,
    lPcieRootBridge->MemAbove4G.Limit  = mPcieMemBases[PcieIdx] + 0x1FFFFFFFF; // TODO: try to add in DTB "ranges"
    lPcieRootBridge->PMem.Base         = MAX_UINT64;
    lPcieRootBridge->PMem.Limit        = 0;
    lPcieRootBridge->PMemAbove4G.Base  = MAX_UINT64;
    lPcieRootBridge->PMemAbove4G.Limit = 0;
    lPcieRootBridge->DevicePath        = (EFI_DEVICE_PATH_PROTOCOL *) &mEfiPciRootBridgeDevicePaths[PcieIdx];

    MmioAnd32 (
       mPcieApbBases[PcieIdx] +
       BS1000_PCIE_APB_PE_GEN_CTRL3,
      ~BS1000_PCIE_APB_PE_GEN_CTRL3_LTSSM_EN
      );

    if (PciePerstBases[PcieIdx]) { /* assert resets */
      if (PciePerstPolarity[PcieIdx]) {
        GpioOutRst (PciePerstBases[PcieIdx], PciePerstGpios[PcieIdx]);
      } else {
        GpioOutSet (PciePerstBases[PcieIdx], PciePerstGpios[PcieIdx]);
      }
      GpioDirSet (PciePerstBases[PcieIdx], PciePerstGpios[PcieIdx]);
    }

    if (PcieNumLanes[PcieIdx] == 1) {
      PciePortLinkCapableLanesVal = BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X1;
    } else if (PcieNumLanes[PcieIdx] == 2) {
      PciePortLinkCapableLanesVal = BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X2;
    } else if (PcieNumLanes[PcieIdx] == 4) {
      PciePortLinkCapableLanesVal = BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X4;
    } else if (PcieNumLanes[PcieIdx] == 8) {
      PciePortLinkCapableLanesVal = BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X8;
    } else if (PcieNumLanes[PcieIdx] == 16) {
      PciePortLinkCapableLanesVal = BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X16;
    } else {
      PciePortLinkCapableLanesVal = 0;
    }

    if (PciePortLinkCapableLanesVal) {
      MmioAndThenOr32 (
         mPcieDbiBases[PcieIdx] +
         BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF,
        ~BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_BITS,
         PciePortLinkCapableLanesVal
        );

      MmioAndThenOr32 (
         mPcieDbiBases[PcieIdx] +
         BS1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF,
        ~BS1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_BITS,
         PcieNumLanes[PcieIdx] << BS1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_SHIFT
        );
    }

    MmioOr32 (
      mPcieDbiBases[PcieIdx] +
      BS1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF,
      BS1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN
      );

    MmioAndThenOr32 (
      mPcieDbiBases[PcieIdx] + BS1000_PCIE_PF0_TYPE1_HDR_TYPE1_CLASS_CODE_REV_ID_REG,
      0x000000FF,
      0x06040000
      );

    if (PciePortLinkCapableLanesVal) {
      MmioAndThenOr32 (
         mPcieDbiBases[PcieIdx] +
         BS1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG,
        ~BS1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_BITS,
         PcieNumLanes[PcieIdx] << BS1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_SHIFT
        );
    }

    MmioAnd32 (
       mPcieDbiBases[PcieIdx] +
       BS1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF,
      ~BS1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN
      );

    // Region 0: CFG0
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      0,
      mPcieCfgBases[PcieIdx],
      0, // See AcpiPlatformDxe/Iort.c for implications of using 0 here instead of encoding the bus
      mPcieCfgSizes[PcieIdx] >= SIZE_2MB ? SIZE_2MB : mPcieCfgSizes[PcieIdx],
      BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG0,
      BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE
      );

    // Region 1: CFG1
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      1,
      mPcieCfgBases[PcieIdx],
      0,
      mPcieCfgSizes[PcieIdx], // 0..0x1FFFFF is masked by CFG0
      BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG1,
      BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE
      );

    // Region 2: MEM
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      2,
      mPcieMemBases[PcieIdx],
      lPcieRootBridge->Mem.Base,
      lPcieRootBridge->Mem.Limit - lPcieRootBridge->Mem.Base + 1,
      BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM,
      0
      );

    // Region 3: I/O
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      3,
      mPcieIoBases[PcieIdx],
      lPcieRootBridge->Io.Base,
      lPcieRootBridge->Io.Limit - lPcieRootBridge->Io.Base + 1,
      BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_IO,
      0
      );

    gBS->Stall (1000);

    if (PciePerstBases[PcieIdx]) { /* deassert resets */
      if (PciePerstPolarity[PcieIdx]) {
        GpioOutSet (PciePerstBases[PcieIdx], PciePerstGpios[PcieIdx]);
      } else {
        GpioOutRst (PciePerstBases[PcieIdx], PciePerstGpios[PcieIdx]);
      }
      GpioDirSet (PciePerstBases[PcieIdx], PciePerstGpios[PcieIdx]);
    }

    if (PcieLinkSpeed[PcieIdx] > 0 && PcieLinkSpeed[PcieIdx] <= 4) {
      /* Set link speed */
      MmioAndThenOr32 (mPcieDbiBases[PcieIdx] + 0xa0, ~0x0f, PcieLinkSpeed[PcieIdx]);
    }

    MmioOr32 (
      mPcieApbBases[PcieIdx] +
      BS1000_PCIE_APB_PE_GEN_CTRL3,
      BS1000_PCIE_APB_PE_GEN_CTRL3_LTSSM_EN
      );

    MmioOr32 (
      mPcieDbiBases[PcieIdx] +
      BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG,
      BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_RETRAIN_LINK
      );

    // Wait for link
    TimeStart = GetTimeInNanoSecond (GetPerformanceCounter ());
    for (;;) {
      CONST UINT32  PcieApbPeLinkDbg2 = MmioRead32 (mPcieApbBases[PcieIdx] + BS1000_PCIE_APB_PE_LINK_DBG2);
      CONST UINT64  PerformanceCounter = GetPerformanceCounter ();

      if (!ComponentExists) {
        if ((PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_MASK) > 0x1) {
          ComponentExists = TRUE;
          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x|%a)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link partner detected\n",
            PcieIdx,
            mPcieNames[PcieIdx],
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_MASK,
            PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_SMLH_LINKUP ? '+' : '-',
            PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_RDLH_LINKUP ? '+' : '-'
            ));
        } else if (GetTimeInNanoSecond (PerformanceCounter) - TimeStart > 50000000) {
          // According to PCI Express Base Specification device must enter LTSSM detect state within 20 ms of reset
          break;
        }
      }

      if (ComponentExists) {
        if (((PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_MASK) ==
                                  BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_L0) &&
             (PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_SMLH_LINKUP) &&
             (PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_RDLH_LINKUP)) {
#if !defined(MDEPKG_NDEBUG)
          CONST UINT32  PcieLnkStat = MmioRead32 (
                                        mPcieDbiBases[PcieIdx] +
                                        BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG
                                        );
          CONST CHAR8  *LinkSpeedString;
          CONST UINTN   LinkSpeedVector = (PcieLnkStat &
                                            BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_BITS) >>
                                            BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_LINK_SPEED_SHIFT;
          if (LinkSpeedVector == 1) {
            LinkSpeedString = "2.5";
          } else if (LinkSpeedVector == 2) {
            LinkSpeedString = "5.0";
          } else if (LinkSpeedVector == 3) {
            LinkSpeedString = "8.0";
          } else if (LinkSpeedVector == 4) {
            LinkSpeedString = "16.0";
          } else {
            LinkSpeedString = "???";
          }

          DEBUG ((
            EFI_D_INFO,
            "PcieRoot(0x%x|%a)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link %a GT/s, x%u",
            PcieIdx,
            mPcieNames[PcieIdx],
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_MASK,
            PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_SMLH_LINKUP ? '+' : '-',
            PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_RDLH_LINKUP ? '+' : '-',
            LinkSpeedString,
            (PcieLnkStat & BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_BITS) >>
              BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_SHIFT
            ));
#endif
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
            "PcieRoot(0x%x|%a)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link is inactive\n",
            PcieIdx,
            mPcieNames[PcieIdx],
            (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
            PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_MASK,
            PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_SMLH_LINKUP ? '+' : '-',
            PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_RDLH_LINKUP ? '+' : '-'
            ));

          //
          // System hangups have been observed when PCIe partner enters detect state
          // but L0 state or data link are not reached. Disabling LTSSM helps to prevent these hangups.
          //
          MmioAnd32 (
             mPcieApbBases[PcieIdx] +
             BS1000_PCIE_APB_PE_GEN_CTRL3,
            ~BS1000_PCIE_APB_PE_GEN_CTRL3_LTSSM_EN
            );

          break;
        }
      }
    }
  }

  return EFI_SUCCESS;
}

BOOLEAN
PciHostBridgeLibGetLink (
  IN CONST UINTN  PcieIdx
  )
{
  ASSERT (PcieIdx < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths));

  CONST UINT32  PcieApbPeLinkDbg2 = MmioRead32 (mPcieApbBases[PcieIdx] + BS1000_PCIE_APB_PE_LINK_DBG2);
  return ((PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_MASK) ==
                               BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_L0) &&
          (PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_SMLH_LINKUP) &&
          (PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_RDLH_LINKUP);
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
