/** @file
*
*  Copyright (c) 2011-2014, ARM Limited. All rights reserved.
*  Copyright (c) 2014, Linaro Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <PiPei.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <libfdt.h>

#include <Guid/FdtHob.h>

EFI_STATUS
EFIAPI
PlatformPeim (
  VOID
  )
{
  VOID    *Base;
  UINTN    FdtSize;
  UINTN    FdtPages;
  UINT64  *HobData;
  VOID    *NewBase;

  Base = (VOID *) FixedPcdGet64 (PcdDeviceTreeInitialBaseAddress);
  ASSERT (Base != NULL);
  ASSERT (fdt_check_header (Base) == 0);

  FdtSize = fdt_totalsize (Base) + PcdGet32 (PcdDeviceTreeAllocationPadding);
  FdtPages = EFI_SIZE_TO_PAGES (FdtSize);
  NewBase = AllocatePages (FdtPages);
  ASSERT (NewBase != NULL);
  fdt_open_into (Base, NewBase, EFI_PAGES_TO_SIZE (FdtPages));

  HobData = BuildGuidHob (&gFdtHobGuid, sizeof *HobData);
  ASSERT (HobData != NULL);
  *HobData = (UINT64) NewBase;

  BuildFvHob (FixedPcdGet64 (PcdFvBaseAddress), FixedPcdGet32 (PcdFvSize));

  Base = (VOID *) FixedPcdGet64 (PcdSpdInitialBase);
  if (Base) {
    NewBase = AllocatePages (1);
    ASSERT (NewBase != NULL);
    CopyMem (NewBase, Base, 1536);

    HobData = BuildGuidHob (&gBaikalSpdHobGuid, sizeof *HobData);
    ASSERT (HobData != NULL);
    *HobData = (UINT64) NewBase;
  }

  return EFI_SUCCESS;
}
