/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/AcpiTable.h>

#include "AcpiPlatformDxe.h"

extern unsigned char  dsdt_aml_code[];

STATIC
EFI_STATUS
DsdtInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *)dsdt_aml_code;
  return EFI_SUCCESS;
}

typedef
EFI_STATUS
(*BAIKAL_ACPI_INIT_FUNCTION) (
  EFI_ACPI_DESCRIPTION_HEADER **
  );

extern EFI_STATUS CsrtInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS Dbg2Init (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS FacsInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS FadtInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS GtdtInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS IortInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS MadtInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS McfgInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS PpttInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);

STATIC BAIKAL_ACPI_INIT_FUNCTION  AcpiTableInit[] = {
  &FadtInit,
  &MadtInit,
  &PpttInit,
  &GtdtInit,
  &Dbg2Init,
  &CsrtInit,
  &FacsInit,
  &IortInit,
  &McfgInit,
  &DsdtInit
};

EFI_STATUS
EFIAPI
AcpiPlatformDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_ACPI_DESCRIPTION_HEADER  *AcpiTable;
  EFI_ACPI_TABLE_PROTOCOL      *AcpiTableProtocol;
  UINTN                         Idx;
  EFI_STATUS                    Status;
  UINTN                         TableHandle;

  Status = gBS->LocateProtocol (
                  &gEfiAcpiTableProtocolGuid,
                  NULL,
                  (VOID**)&AcpiTableProtocol
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: Failed to find AcpiTable protocol. Status = %r\n",
      Status
      ));
    return Status;
  }

  for (Idx = 0; Idx < ARRAY_SIZE (AcpiTableInit); ++Idx) {
    Status = (*AcpiTableInit[Idx]) (&AcpiTable);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = AcpiTableProtocol->InstallAcpiTable (
                                  AcpiTableProtocol,
                                  AcpiTable,
                                  AcpiTable->Length,
                                  &TableHandle
                                  );

    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: Failed to Install ACPI Table. Status = %r\n",
        Status
        ));
        continue;
    }

    DEBUG ((
      DEBUG_INFO,
      "INFO: ACPI Table installed. Status = %r\n",
      Status
      ));
  }

  return EFI_SUCCESS;
}
