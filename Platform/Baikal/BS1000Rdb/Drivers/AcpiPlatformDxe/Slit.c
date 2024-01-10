/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include "AcpiPlatform.h"

#include <BS1000.h>

#if PLATFORM_CHIP_COUNT > 1

#define SELF_CHIP_DISTANCE    10
#define REMOTE_CHIP_DISTANCE  20

#pragma pack(1)
typedef struct {
  EFI_ACPI_6_4_SYSTEM_LOCALITY_DISTANCE_INFORMATION_TABLE_HEADER  Header;
  UINT8                                                           Matrix[PLATFORM_CHIP_COUNT * PLATFORM_CHIP_COUNT];
} BAIKAL_ACPI_SLIT;
#pragma pack()

STATIC BAIKAL_ACPI_SLIT  Slit = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_4_SYSTEM_LOCALITY_INFORMATION_TABLE_SIGNATURE,
      BAIKAL_ACPI_SLIT,
      EFI_ACPI_6_4_SYSTEM_LOCALITY_DISTANCE_INFORMATION_TABLE_REVISION,
      0x54494C53
      ),
    /* UINT64                         NumberOfSystemLocalities */
    PLATFORM_CHIP_COUNT
  },
};

EFI_STATUS
SlitInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINTN  Idx1;
  UINTN  Idx2;
  UINTN  Num;

  for (Idx1 = 0, Num = 0; Idx1 < PLATFORM_CHIP_COUNT; ++Idx1) {
    for (Idx2 = 0; Idx2 < PLATFORM_CHIP_COUNT; ++Idx2, ++Num) {
      Slit.Matrix[Num] = (Idx1 == Idx2) ? SELF_CHIP_DISTANCE : REMOTE_CHIP_DISTANCE;
    }
  }

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Slit;
  return EFI_SUCCESS;
}

#else

EFI_STATUS
SlitInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  return EFI_UNSUPPORTED;
}

#endif
