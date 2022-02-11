/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>

#include <BS1000.h>

#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS  8

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

  VirtualMemoryTable[Index].PhysicalBase = BS1000_OHCI_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BS1000_OHCI_BASE;
  VirtualMemoryTable[Index].Length       = BS1000_OHCI_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BS1000_EHCI_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BS1000_EHCI_BASE;
  VirtualMemoryTable[Index].Length       = BS1000_EHCI_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BS1000_GMAC0_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BS1000_GMAC0_BASE;
  VirtualMemoryTable[Index].Length       = BS1000_GMAC0_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BS1000_GMAC1_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BS1000_GMAC1_BASE;
  VirtualMemoryTable[Index].Length       = BS1000_GMAC1_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BS1000_UART_A1_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BS1000_UART_A1_BASE;
  VirtualMemoryTable[Index].Length       = BS1000_UART_A1_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = BS1000_GIC_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BS1000_GIC_BASE;
  VirtualMemoryTable[Index].Length       = BS1000_GIC_SIZE;
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[Index].PhysicalBase = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].VirtualBase  = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].Length       = PcdGet64 (PcdSystemMemorySize);
  VirtualMemoryTable[Index++].Attributes = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  // End of Table
  ZeroMem (&VirtualMemoryTable[Index], sizeof (ARM_MEMORY_REGION_DESCRIPTOR));

  ASSERT (Index < MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
}
