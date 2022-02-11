/** @file
*
*  Copyright (c) 2011-2013, ARM Limited. All rights reserved.
*  Copyright (c) 2014, Linaro Limited. All rights reserved.
*  Copyright (c) 2014, Red Hat, Inc.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Library/IoLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Pi/PiBootMode.h>

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
  This function is called by PrePeiCore, in the SEC phase.
**/
RETURN_STATUS
ArmPlatformInitialize (
  IN  UINTN                     MpId
  )
{
  //
  // We are relying on ArmPlatformInitializeSystemMemory () being called from
  // InitializeMemory (), which only occurs if the following feature is disabled
  //
  ASSERT (!FeaturePcdGet (PcdSystemMemoryInitializeInSec));
  return RETURN_SUCCESS;
}

VOID
ArmPlatformInitializeSystemMemory (
  VOID
  )
{
}

VOID
ArmPlatformGetPlatformPpiList (
  OUT UINTN                   *PpiListSize,
  OUT EFI_PEI_PPI_DESCRIPTOR  **PpiList
  )
{
  *PpiListSize = 0;
  *PpiList = NULL;
}
