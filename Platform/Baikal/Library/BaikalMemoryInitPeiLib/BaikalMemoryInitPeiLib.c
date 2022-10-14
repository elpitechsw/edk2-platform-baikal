/** @file
*
*  Copyright (c) 2011-2014, ARM Limited. All rights reserved.
*  Copyright (c) 2014, Linaro Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <PiPei.h>

#include <Library/ArmMmuLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

VOID
BuildMemoryTypeInformationHob (
  VOID
  );

STATIC
VOID
InitMmu (
  VOID
  )
{
  ARM_MEMORY_REGION_DESCRIPTOR  *MemoryTable;
  VOID                          *TranslationTableBase;
  UINTN                          TranslationTableSize;
  RETURN_STATUS                  Status;

  // Get Virtual Memory Map from the Platform Library
  ArmPlatformGetVirtualMemoryMap (&MemoryTable);

  // Note: Because we called PeiServicesInstallPeiMemory() before to call InitMmu() the MMU Page Table resides in
  //       DRAM (even at the top of DRAM as it is the first permanent memory allocation).
  Status = ArmConfigureMmu (MemoryTable, &TranslationTableBase, &TranslationTableSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Error: Failed to enable MMU\n"));
  }
}

EFI_STATUS
EFIAPI
MemoryPeim (
  IN  EFI_PHYSICAL_ADDRESS  UefiMemoryBase,
  IN  UINT64                UefiMemorySize
  )
{
  EFI_RESOURCE_ATTRIBUTE_TYPE  ResourceAttributes;

  // Ensure PcdSystemMemorySize has been set
  ASSERT (PcdGet64 (PcdSystemMemorySize));

  // Now, the permanent memory has been installed, we can call AllocatePages()
  ResourceAttributes =
    EFI_RESOURCE_ATTRIBUTE_PRESENT |
    EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
    EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
    EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
    EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
    EFI_RESOURCE_ATTRIBUTE_TESTED;

  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    ResourceAttributes,
    PcdGet64 (PcdSystemMemoryBase),
    0x10000000
    );

  // Reserve secure memory region
  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    ResourceAttributes,
    0x90000000,
    0x1000000
    );

  BuildMemoryAllocationHob (
    0x90000000,
    0x1000000,
    EfiReservedMemoryType
    );

  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    ResourceAttributes,
    0x91000000,
    PcdGet64 (PcdSystemMemorySize) - 0x11000000
    );

  // Build Memory Allocation Hob
  InitMmu ();

  if (FeaturePcdGet (PcdPrePiProduceMemoryTypeInformationHob)) {
    // Optional feature that helps prevent EFI memory map fragmentation
    BuildMemoryTypeInformationHob ();
  }

  return EFI_SUCCESS;
}
