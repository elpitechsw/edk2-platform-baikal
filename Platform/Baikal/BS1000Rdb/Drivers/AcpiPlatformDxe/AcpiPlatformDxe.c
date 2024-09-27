/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/AcpiTable.h>

#include "AcpiPlatform.h"

extern unsigned char  dsdt_aml_code[];

STATIC
EFI_STATUS
DsdtInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) dsdt_aml_code;
  return EFI_SUCCESS;
}

typedef
EFI_STATUS
(*BAIKAL_ACPI_INIT_FUNCTION) (
  EFI_ACPI_DESCRIPTION_HEADER **
  );

extern EFI_STATUS Dbg2Init (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS FadtInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS GtdtInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS IortInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS MadtInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS McfgInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS PmttInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS PpttInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS SlitInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS SpcrInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS SratInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);

typedef
VOID
(*BAIKAL_ACPI_DESTROY_FUNCTION) (
  EFI_ACPI_DESCRIPTION_HEADER *
  );

extern VOID SratDestroy (EFI_ACPI_DESCRIPTION_HEADER  *Table);

STATIC struct {
  CONST CHAR16                 *Name;
  BAIKAL_ACPI_INIT_FUNCTION     Init;
  BAIKAL_ACPI_DESTROY_FUNCTION  Destroy;
} AcpiTableInit[] = {
  {L"DBG2", &Dbg2Init,     NULL},
  {L"DSDT", &DsdtInit,     NULL},
  {L"FADT", &FadtInit,     NULL},
  {L"GTDT", &GtdtInit,     NULL},
  {L"IORT", &IortInit,     NULL},
  {L"MADT", &MadtInit,     NULL},
  {L"MCFG", &McfgInit,     NULL},
  {L"PMTT", &PmttInit,     NULL},
  {L"PPTT", &PpttInit,     NULL},
  {L"SLIT", &SlitInit,     NULL},
  {L"SPCR", &SpcrInit,     NULL},
  {L"SRAT", &SratInit,     &SratDestroy}
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
                  (VOID **) &AcpiTableProtocol
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
    Status = (*AcpiTableInit[Idx].Init) (&AcpiTable);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = AcpiTableProtocol->InstallAcpiTable (
                                  AcpiTableProtocol,
                                  AcpiTable,
                                  AcpiTable->Length,
                                  &TableHandle
                                  );

    if (AcpiTableInit[Idx].Destroy) {
      (*AcpiTableInit[Idx].Destroy) (AcpiTable);
    }

    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: Failed to Install %s ACPI Table. Status = %r\n",
        AcpiTableInit[Idx].Name,
        Status
        ));
        continue;
    }

    DEBUG ((
      DEBUG_INFO,
      "INFO: %s ACPI Table installed. Status = %r\n",
      AcpiTableInit[Idx].Name,
      Status
      ));
  }

  return EFI_SUCCESS;
}
