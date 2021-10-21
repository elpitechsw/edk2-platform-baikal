/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>

#pragma pack(1)
typedef EFI_ACPI_6_3_FIRMWARE_ACPI_CONTROL_STRUCTURE  BAIKAL_ACPI_FACS;

STATIC BAIKAL_ACPI_FACS  Facs = {
  /* UINT32  Signature             */
  EFI_ACPI_6_3_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE,
  /* UINT32  Length                */
  sizeof (BAIKAL_ACPI_FACS),
  /* UINT32  HardwareSignature     */
  0,
  /* UINT32  FirmwareWakingVector  */
  0,
  /* UINT32  GlobalLock            */
  0,
  /* UINT32  Flags                 */
  EFI_ACPI_6_3_64BIT_WAKE_SUPPORTED_F,
  /* UINT64  XFirmwareWakingVector */
  0,
  /* UINT8   Version               */
  EFI_ACPI_6_3_FIRMWARE_ACPI_CONTROL_STRUCTURE_VERSION,
  /* UINT8   Reserved0[3]          */
  {
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE
  },
  /* UINT32  OspmFlags             */
  0,
  /* UINT8   Reserved1[24]         */
  {
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE
  }
};
#pragma pack()

EFI_STATUS
FacsInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *)&Facs;
  return EFI_SUCCESS;
}
