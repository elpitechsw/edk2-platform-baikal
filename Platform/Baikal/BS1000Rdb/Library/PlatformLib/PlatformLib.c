/** @file
*
*  Copyright (c) 2011 - 2013, ARM Limited. All rights reserved.<BR>
*  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Library/ArmPlatformLib.h>
#include <Pi/PiBootMode.h>
#include <Ppi/ArmMpCoreInfo.h>

/**
  Return the current Boot Mode

  This function returns the boot reason on the platform

  @return   Return the current Boot Mode of the platform

**/
EFI_BOOT_MODE
ArmPlatformGetBootMode (
  VOID
  )
{
  return BOOT_WITH_FULL_CONFIGURATION;
}

/**
  Initialize controllers that must setup in the normal world

  This function is called by the ArmPlatformPkg/PrePi or ArmPlatformPkg/PlatformPei
  in the PEI phase.

**/
RETURN_STATUS
ArmPlatformInitialize (
  IN  UINTN                     MpId
  )
{
  return RETURN_SUCCESS;
}

STATIC ARM_CORE_INFO MpCoreInfoTable[] = {
  { 0x00000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 0, Core 0
  { 0x00100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 0, Core 1
  { 0x00200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 0, Core 2
  { 0x00300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 0, Core 3
  { 0x10000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 1, Core 0
  { 0x10100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 1, Core 1
  { 0x10200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 1, Core 2
  { 0x10300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 1, Core 3
  { 0x20000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 2, Core 0
  { 0x20100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 2, Core 1
  { 0x20200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 2, Core 2
  { 0x20300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 2, Core 3
  { 0x30000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 3, Core 0
  { 0x30100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 3, Core 1
  { 0x30200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 3, Core 2
  { 0x30300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 3, Core 3
  { 0x40000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 4, Core 0
  { 0x40100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 4, Core 1
  { 0x40200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 4, Core 2
  { 0x40300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 4, Core 3
  { 0x50000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 5, Core 0
  { 0x50100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 5, Core 1
  { 0x50200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 5, Core 2
  { 0x50300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 5, Core 3
  { 0x60000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 6, Core 0
  { 0x60100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 6, Core 1
  { 0x60200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 6, Core 2
  { 0x60300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 6, Core 3
  { 0x70000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 7, Core 0
  { 0x70100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 7, Core 1
  { 0x70200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 7, Core 2
  { 0x70300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 7, Core 3
  { 0x80000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 8, Core 0
  { 0x80100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 8, Core 1
  { 0x80200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 8, Core 2
  { 0x80300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 8, Core 3
  { 0x90000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 9, Core 0
  { 0x90100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 9, Core 1
  { 0x90200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 9, Core 2
  { 0x90300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 9, Core 3
  { 0xA0000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 10, Core 0
  { 0xA0100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 10, Core 1
  { 0xA0200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 10, Core 2
  { 0xA0300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 10, Core 3
  { 0xB0000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 11, Core 0
  { 0xB0100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 11, Core 1
  { 0xB0200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 11, Core 2
  { 0xB0300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }  // Cluster 11, Core 3
};

EFI_STATUS
PrePeiCoreGetMpCoreInfo (
  OUT UINTN                   *CoreCount,
  OUT ARM_CORE_INFO           **ArmCoreTable
  )
{
  if (ArmIsMpCore()) {
    *CoreCount    = sizeof(MpCoreInfoTable) / sizeof(ARM_CORE_INFO);
    *ArmCoreTable = MpCoreInfoTable;
    return EFI_SUCCESS;
  } else {
    return EFI_UNSUPPORTED;
  }
}

STATIC ARM_MP_CORE_INFO_PPI mMpCoreInfoPpi = { PrePeiCoreGetMpCoreInfo };

STATIC EFI_PEI_PPI_DESCRIPTOR mPlatformPpiTable[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gArmMpCoreInfoPpiGuid,
    &mMpCoreInfoPpi
  }
};

VOID
ArmPlatformGetPlatformPpiList (
  OUT UINTN                   *PpiListSize,
  OUT EFI_PEI_PPI_DESCRIPTOR  **PpiList
  )
{
  *PpiListSize = sizeof mPlatformPpiTable;
  *PpiList = mPlatformPpiTable;
}
