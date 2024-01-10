/** @file
  Copyright (c) 2020 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
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
  BM1000_PCIE1_DBI_SIZE,
  BM1000_PCIE2_DBI_SIZE
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
  EFI_STATUS            Status;
  PCI_ROOT_BRIDGE      *lPcieRootBridge;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);

  // Acquire PCIe RC related data from FDT
  for (Iter = 0; Iter < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths);) {
    INTN         CfgRegIdx = -1, DbiRegIdx = -1;
    CONST VOID  *Prop;
    UINT32       PropSize;

    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bm1000-pcie", Node, &Node);
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
      CONST EFI_PHYSICAL_ADDRESS  DbiBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + DbiRegIdx * 2));
#if !defined(MDEPKG_NDEBUG)
      CONST UINTN                 DbiSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + DbiRegIdx * 2 + 1));
#endif
      CONST EFI_PHYSICAL_ADDRESS  CfgBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + CfgRegIdx * 2));
      CONST UINTN                 CfgSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + CfgRegIdx * 2 + 1));

      for (PcieIdx = 0; PcieIdx < ARRAY_SIZE (mEfiPciRootBridgeDevicePaths); ++PcieIdx) {
        if (DbiBase == mPcieDbiBases[PcieIdx]) {
          ASSERT (DbiSize == mPcieDbiSizes[PcieIdx]);
          ASSERT (CfgSize > SIZE_2MB);
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

    if (mPcieDbiBases[PcieIdx] == BM1000_PCIE2_DBI_BASE) {
      INT32  SubNode;

      SubNode = 0;
      Pcie2PrsntGpio     = -1;
      Pcie2PrsntPolarity = -1;

      Status = FdtClient->FindNextCompatibleNode (FdtClient, "snps,dw-apb-gpio-port", SubNode, &SubNode);
      if (Status == EFI_SUCCESS) {
        Status = FdtClient->FindNextSubnode (FdtClient, "pcie2-prsnt", SubNode, &SubNode);
        if (Status == EFI_SUCCESS) {
          Status = FdtClient->GetNodeProperty (FdtClient, SubNode, "gpios", &Prop, &PropSize);
          if (Status == EFI_SUCCESS && PropSize == 2 * sizeof (UINT32)) {
            Pcie2PrsntGpio     = SwapBytes32 (((CONST UINT32 *) Prop)[0]);
            Pcie2PrsntPolarity = SwapBytes32 (((CONST UINT32 *) Prop)[1]);
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
    if (mPcieDbiBases[PcieIdx] == BM1000_PCIE2_DBI_BASE) {
      MmioOr32 (
        BM1000_PCIE_GPR_RST (PcieIdx),
        BM1000_PCIE_GPR_RST_NONSTICKY_RST | BM1000_PCIE_GPR_RST_STICKY_RST |
        BM1000_PCIE_GPR_RST_PWR_RST       | BM1000_PCIE_GPR_RST_CORE_RST   |
        BM1000_PCIE_GPR_RST_PIPE1_RST     | BM1000_PCIE_GPR_RST_PIPE0_RST  |
        BM1000_PCIE_GPR_RST_PHY_RST
        );

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
    } else {
      MmioOr32 (
        BM1000_PCIE_GPR_RST (PcieIdx),
        BM1000_PCIE_GPR_RST_NONSTICKY_RST | BM1000_PCIE_GPR_RST_STICKY_RST |
        BM1000_PCIE_GPR_RST_PWR_RST       | BM1000_PCIE_GPR_RST_CORE_RST   |
        BM1000_PCIE_GPR_RST_PIPE0_RST     | BM1000_PCIE_GPR_RST_PHY_RST
        );
    }

    gBS->Stall (1); // Delay at least 100 ns

    // Deassert PCIe RC resets
    if (mPcieDbiBases[PcieIdx] == BM1000_PCIE2_DBI_BASE) {
      MmioAnd32 (
          BM1000_PCIE_GPR_RST (PcieIdx),
        ~(BM1000_PCIE_GPR_RST_ADB_PWRDWN    | BM1000_PCIE_GPR_RST_HOT_RST    |
          BM1000_PCIE_GPR_RST_NONSTICKY_RST | BM1000_PCIE_GPR_RST_STICKY_RST |
          BM1000_PCIE_GPR_RST_PWR_RST       | BM1000_PCIE_GPR_RST_CORE_RST   |
          BM1000_PCIE_GPR_RST_PIPE1_RST     | BM1000_PCIE_GPR_RST_PIPE0_RST  |
          BM1000_PCIE_GPR_RST_PHY_RST)
        );
    } else {
      MmioAnd32 (
          BM1000_PCIE_GPR_RST (PcieIdx),
        ~(BM1000_PCIE_GPR_RST_ADB_PWRDWN    | BM1000_PCIE_GPR_RST_HOT_RST    |
          BM1000_PCIE_GPR_RST_NONSTICKY_RST | BM1000_PCIE_GPR_RST_STICKY_RST |
          BM1000_PCIE_GPR_RST_PWR_RST       | BM1000_PCIE_GPR_RST_CORE_RST   |
          BM1000_PCIE_GPR_RST_PIPE0_RST     | BM1000_PCIE_GPR_RST_PHY_RST)
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

    // Enable writing read-only registers using DBI
    MmioOr32 (
      mPcieDbiBases[PcieIdx] +
      BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF,
      BM1000_PCIE_PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_DBI_RO_WR_EN
      );

    MmioAndThenOr32 (
      mPcieDbiBases[PcieIdx] + BM1000_PCIE_PF0_TYPE1_HDR_TYPE1_CLASS_CODE_REV_ID_REG,
      0x000000FF,
      0x06040000
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
      // so that devices on different segments don't map to same DeviceID. E.g.,
      // - for DBM:
      // PCIe0 -> 0x0000x
      // PCIe1 -> 0x1000x
      // PCIe2 -> 0x2000x
      // - for MBM:
      // PCIe0 -> 0x0000x
      // PCIe2 -> 0x2000x
      //
      // where 'x' is:
      // - 8 when CFG0 filtering does not work
      // - 0 when CFG0 filtering works or ACPI_PCIE_CUSTOM mode is set
      //
      if (mPcieDbiBases[PcieIdx] == BM1000_PCIE0_DBI_BASE) {
        MmioAndThenOr32 (
           BM1000_PCIE_GPR_MSI_TRANS2,
          ~BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE0_MASK,
           BM1000_PCIE_GPR_MSI_TRANS2_PCIE0_MSI_TRANS_EN
          );
      } else if (mPcieDbiBases[PcieIdx] == BM1000_PCIE1_DBI_BASE) {
        MmioAndThenOr32 (
           BM1000_PCIE_GPR_MSI_TRANS2,
          ~BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE1_MASK,
           BM1000_PCIE_GPR_MSI_TRANS2_PCIE1_MSI_TRANS_EN | (1 << 2)
          );
      } else if (mPcieDbiBases[PcieIdx] == BM1000_PCIE2_DBI_BASE) {
        MmioAndThenOr32 (
           BM1000_PCIE_GPR_MSI_TRANS2,
          ~BM1000_PCIE_GPR_MSI_TRANS2_MSI_RCNUM_PCIE2_MASK,
           BM1000_PCIE_GPR_MSI_TRANS2_PCIE2_MSI_TRANS_EN | (2 << 4)
          );
      }
    }

    // Region 0: CFG0
    PciHostBridgeLibCfgWindow (
      mPcieDbiBases[PcieIdx],
      0,
      mPcieCfgBases[PcieIdx],
      0, // See AcpiPlatformDxe/Iort.c for implications of using 0 here instead of encoding the bus
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
      mPcieCfgSizes[PcieIdx], // 0..0x1FFFFF is masked by CFG0
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
      CONST UINT64  PerformanceCounter = GetPerformanceCounter ();

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
          // Some PCIe cards (e.g. LSI MegaRAID) require extra time to start
          while (GetTimeInNanoSecond (GetPerformanceCounter () - PerformanceCounter) < 1300000000);

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
