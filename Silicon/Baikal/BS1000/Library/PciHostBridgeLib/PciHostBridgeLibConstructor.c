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

#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG                      0x0A0
#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG_TRGT_LNK_SPEED_BITS  0xF

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

STATIC CONST EFI_PCI_ROOT_BRIDGE_DEVICE_PATH  mEfiPciRootBridgeDevicePathTemplate = {
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
};

STATIC CONST EFI_PHYSICAL_ADDRESS  mPcieApbBaseList[] = {
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

STATIC CONST EFI_PHYSICAL_ADDRESS  mPcieDbiBaseList[] = {
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

#if !defined(MDEPKG_NDEBUG)
STATIC CONST UINTN  mPcieDbiSizeList[] = {
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
#endif

STATIC CONST EFI_PHYSICAL_ADDRESS  mPcieMmioBaseList[] = {
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

STATIC CONST UINTN  mPcieMmioSizeList[] = {
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

STATIC_ASSERT (
  ARRAY_SIZE (mPcieApbBaseList) == ARRAY_SIZE (mPcieDbiBaseList),
  "ARRAY_SIZE (mPcieApbBaseList) != ARRAY_SIZE (mPcieDbiBaseList)"
  );
#if !defined(MDEPKG_NDEBUG)
STATIC_ASSERT (
  ARRAY_SIZE (mPcieDbiSizeList) == ARRAY_SIZE (mPcieDbiBaseList),
  "ARRAY_SIZE (mPcieDbiSizeList) != ARRAY_SIZE (mPcieDbiBaseList)"
  );
#endif
STATIC_ASSERT (
  ARRAY_SIZE (mPcieMmioBaseList) == ARRAY_SIZE (mPcieDbiBaseList),
  "ARRAY_SIZE (mPcieMmioBaseList) != ARRAY_SIZE (mPcieDbiBaseList)"
  );
STATIC_ASSERT (
  ARRAY_SIZE (mPcieMmioSizeList) == ARRAY_SIZE (mPcieDbiBaseList),
  "ARRAY_SIZE (mPcieMmioSizeList) != ARRAY_SIZE (mPcieDbiBaseList)"
  );

STATIC EFI_PHYSICAL_ADDRESS  *mPcieApbBases;
EFI_PHYSICAL_ADDRESS         *mPcieDbiBases;
EFI_PHYSICAL_ADDRESS         *mPcieCfgBases;
STATIC PCI_ROOT_BRIDGE       *mPcieRootBridges;
STATIC UINTN                  mPcieRootBridgesNum;

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

  if (PLATFORM_ADDR_IN_CHIP(PcieDbiBase) == BS1000_PCIE0_P0_DBI_BASE ||
      PLATFORM_ADDR_IN_CHIP(PcieDbiBase) == BS1000_PCIE0_P1_DBI_BASE ||
      PLATFORM_ADDR_IN_CHIP(PcieDbiBase) == BS1000_PCIE1_P0_DBI_BASE ||
      PLATFORM_ADDR_IN_CHIP(PcieDbiBase) == BS1000_PCIE1_P1_DBI_BASE ||
      PLATFORM_ADDR_IN_CHIP(PcieDbiBase) == BS1000_PCIE2_P0_DBI_BASE ||
      PLATFORM_ADDR_IN_CHIP(PcieDbiBase) == BS1000_PCIE2_P1_DBI_BASE) {
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

STATIC
VOID
PciHostBridgeLibRootBridgeLinkUp (
  IN CONST UINTN  PcieIdx,
  IN EFI_PHYSICAL_ADDRESS PerstGpioBase,
  IN INTN                 PerstGpio,
  IN INTN                 PerstGpioPolarity
  )
{
  BOOLEAN  ComponentExists = FALSE;
  UINT64   TimeStart;

  if (PerstGpioBase) {
    if (PerstGpioPolarity) {
      GpioOutRst (PerstGpioBase, PerstGpio);
    } else {
      GpioOutSet (PerstGpioBase, PerstGpio);
    }
    GpioDirSet (PerstGpioBase, PerstGpio);

    gBS->Stall (1000);

    if (PerstGpioPolarity) {
      GpioOutSet (PerstGpioBase, PerstGpio);
    } else {
      GpioOutRst (PerstGpioBase, PerstGpio);
    }
    GpioDirSet (PerstGpioBase, PerstGpio);
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
    UINT64        PerformanceCounter = GetPerformanceCounter ();

    if (!ComponentExists) {
      if ((PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_MASK) > 0x1) {
        ComponentExists = TRUE;
        DEBUG ((
          EFI_D_INFO,
          "PcieRoot(0x%x|0x%llx)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link partner detected\n",
          PcieIdx,
          mPcieDbiBases[PcieIdx],
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
          "PcieRoot(0x%x|0x%llx)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link %a GT/s, x%u",
          PcieIdx,
          mPcieDbiBases[PcieIdx],
          (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
          PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_LTSSM_STATE_MASK,
          PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_SMLH_LINKUP ? '+' : '-',
          PcieApbPeLinkDbg2 & BS1000_PCIE_APB_PE_LINK_DBG2_RDLH_LINKUP ? '+' : '-',
          LinkSpeedString,
          (PcieLnkStat & BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_BITS) >>
            BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_NEGO_LINK_WIDTH_SHIFT
          ));
#endif
        // Wait until device starts responding to cfg requests
        while (MmioRead32 (mPcieCfgBases[PcieIdx] + (1 << 20)) == 0) {
          PerformanceCounter = GetPerformanceCounter ();
          MmioWrite32(mPcieCfgBases[PcieIdx] + (1 << 20), 0xffffffff);
          gBS->Stall (1000);
          if (((GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000) > 1000) {
            break;
          }
        }

        if (MmioRead32 (mPcieCfgBases[PcieIdx] + (1 << 20)) != 0xFFFFFFFF &&
            MmioRead32 (mPcieCfgBases[PcieIdx] + (1 << 20) + 0x8000) == 0xFFFFFFFF) {
          //
          // Device appears to filter CFG0 requests, so the 64 KiB granule for the iATU
          // isn't a problem. We don't have to ignore fn > 0 or shift MCFG by 0x8000.
          //
#if !defined(MDEPKG_NDEBUG)
          DEBUG ((EFI_D_INFO, ", Cfg0Filter+\n"));
#endif
        } else {
#if !defined(MDEPKG_NDEBUG)
          DEBUG ((EFI_D_INFO, ", Cfg0Filter-\n"));
#endif
        }
#if !defined(MDEPKG_NDEBUG)
        DEBUG((EFI_D_INFO,
          "PcieRoot(0x%x): [%dms]: dev_id at 1:0.0 - %x, dev_id at 1:1.0 - %x\n",
          PcieIdx,
          (GetTimeInNanoSecond (PerformanceCounter) - TimeStart) / 1000000,
          MmioRead32 (mPcieCfgBases[PcieIdx] + (1 << 20)),
          MmioRead32 (mPcieCfgBases[PcieIdx] + (1 << 20) + 0x8000)));
#endif

        break;
      } else if (GetTimeInNanoSecond (PerformanceCounter) - TimeStart > 100000000) {
        // Wait up to 100 ms for link up
        DEBUG ((
          EFI_D_INFO,
          "PcieRoot(0x%x|0x%llx)[%03ums: LTSSM:0x%02x SMLH%c RDLH%c]: link is inactive\n",
          PcieIdx,
          mPcieDbiBases[PcieIdx],
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

STATIC
VOID
PciHostBridgeLibRootBrigeInit (
  IN CONST UINTN                 PcieIdx,
  IN CONST UINTN                 MaxSpeed,
  IN CONST UINTN                 NumLanes,
  IN CONST UINTN                 CfgSize,
  IN CONST EFI_PHYSICAL_ADDRESS  MemBase,
  IN CONST EFI_PHYSICAL_ADDRESS  IoBase
  )
{
  PCI_ROOT_BRIDGE  *lPcieRootBridge = &mPcieRootBridges[PcieIdx];
  UINTN    PciePortLinkCapableLanesVal;

  ASSERT (MaxSpeed == 0 ||
          MaxSpeed == 1 ||
          MaxSpeed == 2 ||
          MaxSpeed == 3 ||
          MaxSpeed == 4
    );

  ASSERT (NumLanes == 0 ||
          NumLanes == 1 ||
          NumLanes == 2 ||
          NumLanes == 4 ||
          NumLanes == 8 ||
          NumLanes == 16
    );

  MmioAnd32 (
     mPcieApbBases[PcieIdx] +
     BS1000_PCIE_APB_PE_GEN_CTRL3,
    ~BS1000_PCIE_APB_PE_GEN_CTRL3_LTSSM_EN
    );

  if (MaxSpeed > 0) {
    MmioAndThenOr32 (
       mPcieDbiBases[PcieIdx] +
       BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG,
      ~BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG_TRGT_LNK_SPEED_BITS,
       MaxSpeed
      );
  }

  if (NumLanes == 1) {
    PciePortLinkCapableLanesVal = BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X1;
  } else if (NumLanes == 2) {
    PciePortLinkCapableLanesVal = BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X2;
  } else if (NumLanes == 4) {
    PciePortLinkCapableLanesVal = BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X4;
  } else if (NumLanes == 8) {
    PciePortLinkCapableLanesVal = BS1000_PCIE_PF0_PORT_LOGIC_PORT_LINK_CTRL_OFF_LINK_CAPABLE_X8;
  } else if (NumLanes == 16) {
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
       NumLanes << BS1000_PCIE_PF0_PORT_LOGIC_GEN2_CTRL_OFF_NUM_OF_LANES_SHIFT
      );
  }

  // Enable writing read-only registers using DBI
  MmioOr32 (
    mPcieDbiBases[PcieIdx] +
    BS1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF,
    BS1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN
    );

  MmioAndThenOr32 (
    mPcieDbiBases[PcieIdx] + BS1000_PCIE_PF0_TYPE1_HDR_TYPE1_CLASS_CODE_REV_ID_REG,
    0x000000FF,
    0x06040100
    );

  if (PciePortLinkCapableLanesVal) {
    MmioAndThenOr32 (
       mPcieDbiBases[PcieIdx] +
       BS1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG,
      ~BS1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_BITS,
       NumLanes << BS1000_PCIE_PF0_PCIE_CAP_LINK_CAPABILITIES_REG_MAX_WIDTH_SHIFT
      );
  }

  //
  // Disable the Data Link Feature Exchange.
  // Some legacy controllers (e.g., ASM1166) do not correctly ignore the DLLP.
  //
  if (PLATFORM_ADDR_IN_CHIP(mPcieDbiBases[PcieIdx]) == BS1000_PCIE0_P0_DBI_BASE ||
      PLATFORM_ADDR_IN_CHIP(mPcieDbiBases[PcieIdx]) == BS1000_PCIE1_P0_DBI_BASE ||
      PLATFORM_ADDR_IN_CHIP(mPcieDbiBases[PcieIdx]) == BS1000_PCIE2_P0_DBI_BASE ||
      PLATFORM_ADDR_IN_CHIP(mPcieDbiBases[PcieIdx]) == BS1000_PCIE3_P0_DBI_BASE ||
      PLATFORM_ADDR_IN_CHIP(mPcieDbiBases[PcieIdx]) == BS1000_PCIE4_P0_DBI_BASE) {
    MmioAnd32 (mPcieDbiBases[PcieIdx] + 0x2F4, ~BIT31);
  } else if (PLATFORM_ADDR_IN_CHIP(mPcieDbiBases[PcieIdx]) == BS1000_PCIE0_P1_DBI_BASE ||
             PLATFORM_ADDR_IN_CHIP(mPcieDbiBases[PcieIdx]) == BS1000_PCIE1_P1_DBI_BASE ||
             PLATFORM_ADDR_IN_CHIP(mPcieDbiBases[PcieIdx]) == BS1000_PCIE2_P1_DBI_BASE ||
             PLATFORM_ADDR_IN_CHIP(mPcieDbiBases[PcieIdx]) == BS1000_PCIE3_P2_DBI_BASE ||
             PLATFORM_ADDR_IN_CHIP(mPcieDbiBases[PcieIdx]) == BS1000_PCIE4_P2_DBI_BASE) {
    MmioAnd32 (mPcieDbiBases[PcieIdx] + 0x2BC, ~BIT31);
  } else {
    MmioAnd32 (mPcieDbiBases[PcieIdx] + 0x2A8, ~BIT31);
  }

  // Disable writing read-only registers using DBI
  MmioAnd32 (
     mPcieDbiBases[PcieIdx] +
     BS1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF,
    ~BS1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN
    );

  // Region 0: CFG0
  PciHostBridgeLibCfgWindow (
    mPcieDbiBases[PcieIdx],
    0,
    mPcieCfgBases[PcieIdx] + SIZE_1MB,
    0, // See AcpiPlatformDxe/Iort.c for implications of using 0 here instead of encoding the bus
    SIZE_64KB,
    BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG0,
    BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE
    );

  // Region 1: CFG1
  PciHostBridgeLibCfgWindow (
    mPcieDbiBases[PcieIdx],
    1,
    mPcieCfgBases[PcieIdx] + SIZE_2MB,
    0,
    CfgSize - SIZE_2MB,
    BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_CFG1,
    BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_2_OFF_OUTBOUND_0_CFG_SHIFT_MODE
    );

  // Region 2: MEM
  if (lPcieRootBridge->Mem.Limit >= lPcieRootBridge->Mem.Base) {
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      2,
      MemBase,
      lPcieRootBridge->Mem.Base,
      lPcieRootBridge->Mem.Limit - lPcieRootBridge->Mem.Base + 1,
      BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_MEM,
      0
      );
  }

  // Region 3: I/O
  if (lPcieRootBridge->Io.Limit >= lPcieRootBridge->Io.Base) {
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      3,
      IoBase,
      lPcieRootBridge->Io.Base,
      lPcieRootBridge->Io.Limit - lPcieRootBridge->Io.Base + 1,
      BS1000_PCIE_PF0_ATU_CAP_IATU_REGION_CTRL_1_OFF_OUTBOUND_0_TYPE_IO,
      0
      );
  }
}

EFI_STATUS
EFIAPI
PciHostBridgeLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  FDT_CLIENT_PROTOCOL  *FdtClient;
  INT32                 Node = 0;
  EFI_STATUS            Status;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);

  // Acquire PCIe RC related data from FDT and initialize RC's
  while (TRUE) {
    EFI_PHYSICAL_ADDRESS              ApbBase;
    EFI_PHYSICAL_ADDRESS              DbiBase;
    EFI_PHYSICAL_ADDRESS              CfgBase;
    UINTN                             CfgSize;
    UINTN                             MaxSpeed;
    UINTN                             NumLanes;
    EFI_PHYSICAL_ADDRESS              IoBase;
    EFI_PHYSICAL_ADDRESS              IoMin;
    UINTN                             IoSize;
    EFI_PHYSICAL_ADDRESS              MemBase;
    EFI_PHYSICAL_ADDRESS              MemMin;
    UINTN                             MemSize;
    EFI_PHYSICAL_ADDRESS              MemAbove4GBase;
    EFI_PHYSICAL_ADDRESS              MemAbove4GMin;
    UINTN                             MemAbove4GSize;
    INTN                              ApbRegIdx = -1, CfgRegIdx = -1, DbiRegIdx = -1;
    CONST VOID                       *Prop;
    UINT32                            PropSize;
    EFI_PCI_ROOT_BRIDGE_DEVICE_PATH  *DevicePath;
    UINTN                             PerstGpio;
    UINTN                             PerstGpioPolarity;
    EFI_PHYSICAL_ADDRESS              PerstGpioBase;

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
#if !defined(MDEPKG_NDEBUG)
      CONST UINTN  ApbSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + ApbRegIdx * 2 + 1));
      CONST UINTN  DbiSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + DbiRegIdx * 2 + 1));
#endif
      UINTN        ListIdx;

      ApbBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + ApbRegIdx * 2));
      DbiBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + DbiRegIdx * 2));
      CfgBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + CfgRegIdx * 2));
      CfgSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + CfgRegIdx * 2 + 1));

      for (ListIdx = 0; ListIdx < ARRAY_SIZE (mPcieDbiBaseList); ++ListIdx) {
        if (PLATFORM_ADDR_IN_CHIP(DbiBase) == mPcieDbiBaseList[ListIdx]) {
          ASSERT (PLATFORM_ADDR_IN_CHIP(ApbBase) == mPcieApbBaseList[ListIdx]);
#if !defined(MDEPKG_NDEBUG)
          ASSERT (ApbSize == 0x100);
          ASSERT (DbiSize == mPcieDbiSizeList[ListIdx]);
#endif
          ASSERT (PLATFORM_ADDR_IN_CHIP(CfgBase) >= mPcieMmioBaseList[ListIdx]);
          ASSERT (PLATFORM_ADDR_IN_CHIP(CfgBase) <  mPcieMmioBaseList[ListIdx] + mPcieMmioSizeList[ListIdx]);
          ASSERT (CfgSize >  SIZE_2MB);
          ASSERT (CfgSize <= mPcieMmioSizeList[ListIdx]);
          ASSERT ((CfgSize & (SIZE_1MB - 1)) == 0);
          break;
        }
      }
    } else {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "ranges", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize > 0 && (PropSize % (sizeof (UINT32) + 3 * sizeof (UINT64))) == 0) {
      UINTN  Entry;

      IoSize         = 0;
      MemSize        = 0;
      MemAbove4GSize = 0;

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
          if (IoSize < Size) {
            IoMin  = PciBase;
            IoBase = CpuBase;
            IoSize = Size;
          }
        } else if (Flags & RANGES_FLAG_MEM) {
          if (PciBase <= MAX_UINT32) {
            if (MemSize < Size) {
              MemMin  = PciBase;
              MemBase = CpuBase;
              MemSize = Size;
            }
          } else if (PciBase == CpuBase) {
            if (MemAbove4GSize < Size) {
              MemAbove4GMin  = PciBase;
              MemAbove4GBase = CpuBase;
              MemAbove4GSize = Size;
            }
          }
        }

        Prop = &(((CONST UINT32 *) Prop)[7]);
      }
    } else {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "max-link-speed", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == sizeof (UINT32)) {
      MaxSpeed = SwapBytes32 (*(CONST UINT32 *) Prop);
    } else {
      MaxSpeed = 0;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "num-lanes", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == sizeof (UINT32)) {
      NumLanes = SwapBytes32 (*(CONST UINT32 *) Prop);
    } else {
      NumLanes = 0;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reset-gpios", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize >= (3 * sizeof (UINT32))) {
      INT32        PerstGpioPhandle;
      INT32        PerstGpioNode;
      PerstGpioPhandle  = SwapBytes32 (*(CONST UINT32 *) Prop);
      PerstGpio         = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
      PerstGpioPolarity = SwapBytes32 (((CONST UINT32 *) Prop)[2]);
      if ((FdtClient->FindNodeByPhandle (FdtClient, PerstGpioPhandle, &PerstGpioNode) == EFI_SUCCESS) &&
        (FdtClient->FindParentNode (FdtClient, PerstGpioNode, &PerstGpioNode) == EFI_SUCCESS)) {
        if (FdtClient->GetNodeProperty (FdtClient, PerstGpioNode, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
            PropSize == 2 * sizeof (UINT64)) {
          PerstGpioBase = SwapBytes64 (*(CONST UINT64 *) Prop);
        } else {
          PerstGpioBase = 0;
        }
      } else {
        PerstGpioBase = 0;
      }
    } else {
      PerstGpioBase = 0;
    }

    mPcieRootBridges = ReallocatePool (
                         mPcieRootBridgesNum       * sizeof (PCI_ROOT_BRIDGE),
                         (mPcieRootBridgesNum + 1) * sizeof (PCI_ROOT_BRIDGE),
                         mPcieRootBridges
                         );
    ASSERT (mPcieRootBridges != NULL);
    mPcieApbBases = ReallocatePool (
                      mPcieRootBridgesNum       * sizeof (EFI_PHYSICAL_ADDRESS),
                      (mPcieRootBridgesNum + 1) * sizeof (EFI_PHYSICAL_ADDRESS),
                      mPcieApbBases
                      );
    ASSERT (mPcieApbBases != NULL);
    mPcieDbiBases = ReallocatePool (
                      mPcieRootBridgesNum       * sizeof (EFI_PHYSICAL_ADDRESS),
                      (mPcieRootBridgesNum + 1) * sizeof (EFI_PHYSICAL_ADDRESS),
                      mPcieDbiBases
                      );
    ASSERT (mPcieDbiBases != NULL);
    mPcieCfgBases = ReallocatePool (
                      mPcieRootBridgesNum       * sizeof (EFI_PHYSICAL_ADDRESS),
                      (mPcieRootBridgesNum + 1) * sizeof (EFI_PHYSICAL_ADDRESS),
                      mPcieCfgBases
                      );
    ASSERT (mPcieCfgBases != NULL);
    DevicePath = AllocateCopyPool (
                   sizeof (mEfiPciRootBridgeDevicePathTemplate),
                   &mEfiPciRootBridgeDevicePathTemplate
                   );
    ASSERT (DevicePath != NULL);
    DevicePath->AcpiDevicePath.UID = mPcieRootBridgesNum;

    mPcieRootBridges[mPcieRootBridgesNum].Segment    = mPcieRootBridgesNum;
    mPcieRootBridges[mPcieRootBridgesNum].Supports   = 0;
    mPcieRootBridges[mPcieRootBridgesNum].Attributes = mPcieRootBridges[mPcieRootBridgesNum].Supports;
    mPcieRootBridges[mPcieRootBridgesNum].DmaAbove4G = TRUE;
    mPcieRootBridges[mPcieRootBridgesNum].NoExtendedConfigSpace = FALSE;
    mPcieRootBridges[mPcieRootBridgesNum].ResourceAssigned      = FALSE;
    mPcieRootBridges[mPcieRootBridgesNum].AllocationAttributes  = EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM;

    mPcieRootBridges[mPcieRootBridgesNum].Bus.Base           = 0;
    mPcieRootBridges[mPcieRootBridgesNum].Bus.Limit          = CfgSize / SIZE_1MB - 1;

    if (IoSize > 0) {
      mPcieRootBridges[mPcieRootBridgesNum].Io.Base          = IoMin;
      mPcieRootBridges[mPcieRootBridgesNum].Io.Limit         = IoMin + IoSize - 1;
      mPcieRootBridges[mPcieRootBridgesNum].Io.Translation   = IoMin - IoBase;
    } else {
      mPcieRootBridges[mPcieRootBridgesNum].Io.Base          = MAX_UINT64;
      mPcieRootBridges[mPcieRootBridgesNum].Io.Limit         = 0;
    }

    if (MemSize > 0) {
      mPcieRootBridges[mPcieRootBridgesNum].Mem.Base         = MemMin;
      mPcieRootBridges[mPcieRootBridgesNum].Mem.Limit        = MemMin + MemSize - 1;
      mPcieRootBridges[mPcieRootBridgesNum].Mem.Translation  = MemMin - MemBase;
    } else {
      mPcieRootBridges[mPcieRootBridgesNum].Mem.Base         = MAX_UINT64;
      mPcieRootBridges[mPcieRootBridgesNum].Mem.Limit        = 0;
    }

    if (MemAbove4GSize > 0) {
      mPcieRootBridges[mPcieRootBridgesNum].AllocationAttributes  |= EFI_PCI_HOST_BRIDGE_MEM64_DECODE;
      mPcieRootBridges[mPcieRootBridgesNum].MemAbove4G.Base  = MemAbove4GBase;
      mPcieRootBridges[mPcieRootBridgesNum].MemAbove4G.Limit = MemAbove4GBase + MemAbove4GSize - 1;
    } else {
      mPcieRootBridges[mPcieRootBridgesNum].MemAbove4G.Base  = MAX_UINT64;
      mPcieRootBridges[mPcieRootBridgesNum].MemAbove4G.Limit = 0;
    }

    mPcieRootBridges[mPcieRootBridgesNum].PMem.Base          = MAX_UINT64;
    mPcieRootBridges[mPcieRootBridgesNum].PMem.Limit         = 0;
    mPcieRootBridges[mPcieRootBridgesNum].PMemAbove4G.Base   = MAX_UINT64;
    mPcieRootBridges[mPcieRootBridgesNum].PMemAbove4G.Limit  = 0;
    mPcieRootBridges[mPcieRootBridgesNum].DevicePath         = (EFI_DEVICE_PATH_PROTOCOL *) DevicePath;

    mPcieApbBases[mPcieRootBridgesNum] = ApbBase;
    mPcieDbiBases[mPcieRootBridgesNum] = DbiBase;
    mPcieCfgBases[mPcieRootBridgesNum] = CfgBase;

    PciHostBridgeLibRootBrigeInit (mPcieRootBridgesNum, MaxSpeed, NumLanes, CfgSize, MemBase, IoBase);
    PciHostBridgeLibRootBridgeLinkUp (mPcieRootBridgesNum, PerstGpioBase, PerstGpio, PerstGpioPolarity);

    ++mPcieRootBridgesNum;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PciHostBridgeLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (mPcieApbBases != NULL) {
    FreePool (mPcieApbBases);
  }
  if (mPcieDbiBases != NULL) {
    FreePool (mPcieDbiBases);
  }
  if (mPcieCfgBases != NULL) {
    FreePool (mPcieCfgBases);
  }

  return EFI_SUCCESS;
}

BOOLEAN
PciHostBridgeLibGetLink (
  IN CONST UINTN  PcieIdx
  )
{
  ASSERT (PcieIdx < mPcieRootBridgesNum);

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

VOID
PciHostBridgeLibFreeRootBridges (
  PCI_ROOT_BRIDGE  *Bridges,
  UINTN             Count
  )
{
  if (Bridges == NULL || Count == 0) {
    return;
  }

  do {
    --Count;
    FreePool (Bridges[Count].DevicePath);
  } while (Count > 0);

  FreePool (Bridges);
}
