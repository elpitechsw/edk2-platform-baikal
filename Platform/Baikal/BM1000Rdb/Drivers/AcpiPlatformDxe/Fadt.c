/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>

#include "AcpiPlatform.h"

#define BAIKAL_FADT_NULL_GAS  { EFI_ACPI_6_4_SYSTEM_MEMORY, 0, 0, EFI_ACPI_6_4_UNDEFINED, 0 }

#define BAIKAL_FADT_FLAGS  (EFI_ACPI_6_4_HW_REDUCED_ACPI | \
                            EFI_ACPI_6_4_LOW_POWER_S0_IDLE_CAPABLE)

#pragma pack(1)
typedef EFI_ACPI_6_4_FIXED_ACPI_DESCRIPTION_TABLE  BAIKAL_ACPI_FADT;

STATIC BAIKAL_ACPI_FADT  Fadt = {
  BAIKAL_ACPI_HEADER (
    EFI_ACPI_6_4_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE,
    BAIKAL_ACPI_FADT,
    EFI_ACPI_6_4_FIXED_ACPI_DESCRIPTION_TABLE_REVISION,
    0x54444146
    ),
  /* UINT32                                  FirmwareCtrl             */
  0,
  /* UINT32                                  Dsdt                     */
  0,
  /* UINT8                                   Reserved0                */
  EFI_ACPI_RESERVED_BYTE,
  /* UINT8                                   PreferredPmProfile       */
  EFI_ACPI_6_4_PM_PROFILE_MOBILE,
  /* UINT16                                  SciInt                   */
  0,
  /* UINT32                                  SmiCmd                   */
  0,
  /* UINT8                                   AcpiEnable               */
  0,
  /* UINT8                                   AcpiDisable              */
  0,
  /* UINT8                                   S4BiosReq                */
  0,
  /* UINT8                                   PstateCnt                */
  0,
  /* UINT32                                  Pm1aEvtBlk               */
  0,
  /* UINT32                                  Pm1bEvtBlk               */
  0,
  /* UINT32                                  Pm1aCntBlk               */
  0,
  /* UINT32                                  Pm1bCntBlk               */
  0,
  /* UINT32                                  Pm2CntBlk                */
  0,
  /* UINT32                                  PmTmrBlk                 */
  0,
  /* UINT32                                  Gpe0Blk                  */
  0,
  /* UINT32                                  Gpe1Blk                  */
  0,
  /* UINT8                                   Pm1EvtLen                */
  0,
  /* UINT8                                   Pm1CntLen                */
  0,
  /* UINT8                                   Pm2CntLen                */
  0,
  /* UINT8                                   PmTmrLen                 */
  0,
  /* UINT8                                   Gpe0BlkLen               */
  0,
  /* UINT8                                   Gpe1BlkLen               */
  0,
  /* UINT8                                   Gpe1Base                 */
  0,
  /* UINT8                                   CstCnt                   */
  0,
  /* UINT16                                  PLvl2Lat                 */
  0,
  /* UINT16                                  PLvl3Lat                 */
  0,
  /* UINT16                                  FlushSize                */
  0,
  /* UINT16                                  FlushStride              */
  0,
  /* UINT8                                   DutyOffset               */
  0,
  /* UINT8                                   DutyWidth                */
  0,
  /* UINT8                                   DayAlrm                  */
  0,
  /* UINT8                                   MonAlrm                  */
  0,
  /* UINT8                                   Century                  */
  0,
  /* UINT16                                  IaPcBootArch             */
  0,
  /* UINT8                                   Reserved1                */
  EFI_ACPI_RESERVED_BYTE,
  /* UINT32                                  Flags                    */
  BAIKAL_FADT_FLAGS,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  ResetReg                 */
  BAIKAL_FADT_NULL_GAS,
  /* UINT8                                   ResetValue               */
  0,
  /* UINT16                                  ArmBootArch              */
  EFI_ACPI_6_4_ARM_PSCI_COMPLIANT,
  /* UINT8                                   MinorVersion             */
  EFI_ACPI_6_4_FIXED_ACPI_DESCRIPTION_TABLE_MINOR_REVISION,
  /* UINT64                                  XFirmwareCtrl            */
  0,
  /* UINT64                                  XDsdt                    */
  0,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  XPm1aEvtBlk              */
  BAIKAL_FADT_NULL_GAS,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  XPm1bEvtBlk              */
  BAIKAL_FADT_NULL_GAS,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  XPm1aCntBlk              */
  BAIKAL_FADT_NULL_GAS,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  XPm1bCntBlk              */
  BAIKAL_FADT_NULL_GAS,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  XPm2CntBlk               */
  BAIKAL_FADT_NULL_GAS,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  XPmTmrBlk                */
  BAIKAL_FADT_NULL_GAS,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  XGpe0Blk                 */
  BAIKAL_FADT_NULL_GAS,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  XGpe1Blk                 */
  BAIKAL_FADT_NULL_GAS,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  SleepControlReg          */
  BAIKAL_FADT_NULL_GAS,
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  SleepStatusReg           */
  BAIKAL_FADT_NULL_GAS,
  /* UINT64                                  HypervisorVendorIdentity */
  0
};
#pragma pack()

EFI_STATUS
FadtInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Fadt;
  return EFI_SUCCESS;
}
