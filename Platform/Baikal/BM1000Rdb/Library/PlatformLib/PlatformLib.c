/** @file
*
*  Copyright (c) 2011 - 2013, ARM Limited. All rights reserved.<BR>
*  Copyright (c) 2020 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
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
  { 0x000, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 0, Core 0
  { 0x001, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 0, Core 1
  { 0x100, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 1, Core 0
  { 0x101, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 1, Core 1
  { 0x200, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 2, Core 0
  { 0x201, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 2, Core 1
  { 0x300, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }, // Cluster 3, Core 0
  { 0x301, 0x0, 0x0, 0x0, (UINT64)0xFFFFFFFF }  // Cluster 3, Core 1
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
