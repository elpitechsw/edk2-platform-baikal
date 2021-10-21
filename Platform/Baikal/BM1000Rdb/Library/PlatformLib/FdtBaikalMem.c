/** @file
  Copyright (c) 2019 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>

#include <BM1000.h>

#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS  29

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU
  on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR
                                    describing a Physical-to-Virtual Memory
                                    mapping. This array must be ended by a
                                    zero-filled entry

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN  ARM_MEMORY_REGION_DESCRIPTOR  **VirtualMemoryMap
  )
{
  UINTN                          Index = 0;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;

  ASSERT (VirtualMemoryMap != NULL);

  VirtualMemoryTable = AllocatePages (
                         EFI_SIZE_TO_PAGES (sizeof (ARM_MEMORY_REGION_DESCRIPTOR) * MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS)
                         );

  if (VirtualMemoryTable == NULL) {
    return;
  }

  VirtualMemoryTable[Index].PhysicalBase = BM1000_PCIE_GPR_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_PCIE_GPR_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_PCIE_GPR_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_PCIE_X4_0_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_PCIE_X4_0_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_PCIE_X4_0_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_PCIE_X4_1_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_PCIE_X4_1_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_PCIE_X4_1_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_PCIE_X8_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_PCIE_X8_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_PCIE_X8_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_GPIO32_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_GPIO32_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_GPIO32_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_UART1_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_UART1_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_UART1_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_I2C1_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_I2C1_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_I2C1_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_I2C2_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_I2C2_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_I2C2_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_SMBUS1_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_SMBUS1_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_SMBUS1_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_SMBUS2_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_SMBUS2_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_SMBUS2_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_USB2_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_USB2_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_USB2_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_USB3_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_USB3_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_USB3_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_SATA0_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_SATA0_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_SATA0_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_SATA1_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_SATA1_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_SATA1_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_LVDS_VDU_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_LVDS_VDU_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_LVDS_VDU_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_MSHC_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_MSHC_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_MSHC_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_GIC_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_GIC_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_GIC_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_GMAC0_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_GMAC0_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_GMAC0_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_GMAC1_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_GMAC1_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_GMAC1_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_HDMI_VDU_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_HDMI_VDU_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_HDMI_VDU_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_HDMI_CTRLR_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_HDMI_CTRLR_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_HDMI_CTRLR_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_DYNIO0_PCIE_X4_0_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_DYNIO0_PCIE_X4_0_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_DYNIO0_PCIE_X4_0_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_DYNIO0_PCIE_X4_1_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_DYNIO0_PCIE_X4_1_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_DYNIO0_PCIE_X4_1_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_DYNIO0_PCIE_X8_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_DYNIO0_PCIE_X8_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_DYNIO0_PCIE_X8_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].Length       = PcdGet64 (PcdSystemMemorySize);
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_DYNIO1_PCIE_X4_0_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_DYNIO1_PCIE_X4_0_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_DYNIO1_PCIE_X4_0_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_DYNIO1_PCIE_X4_1_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_DYNIO1_PCIE_X4_1_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_DYNIO1_PCIE_X4_1_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BM1000_DYNIO1_PCIE_X8_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BM1000_DYNIO1_PCIE_X8_BASE;
  VirtualMemoryTable[Index].Length       = BM1000_DYNIO1_PCIE_X8_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // End of Table
  ZeroMem (&VirtualMemoryTable[Index], sizeof (ARM_MEMORY_REGION_DESCRIPTOR));

  ASSERT (Index < MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
}
