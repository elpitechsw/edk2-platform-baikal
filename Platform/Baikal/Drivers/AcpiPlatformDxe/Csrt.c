/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>

#include "AcpiPlatformDxe.h"

#pragma pack(1)
typedef EFI_ACPI_DESCRIPTION_HEADER  BAIKAL_ACPI_CSRT;

STATIC BAIKAL_ACPI_CSRT  Csrt = BAIKAL_ACPI_HEADER (
  EFI_ACPI_6_3_CORE_SYSTEM_RESOURCE_TABLE_SIGNATURE,
  BAIKAL_ACPI_CSRT,
  0,
  0x54525343
  );
#pragma pack()

EFI_STATUS
CsrtInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *)&Csrt;
  return EFI_SUCCESS;
}
