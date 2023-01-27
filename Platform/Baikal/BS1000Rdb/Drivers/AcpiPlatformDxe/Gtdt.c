/** @file
  Copyright (c) 2021 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/PcdLib.h>

#include "AcpiPlatform.h"

#define BAIKAL_GTDT_TIMER_FLAGS  \
          EFI_ACPI_6_4_GTDT_TIMER_FLAG_TIMER_INTERRUPT_POLARITY | \
          EFI_ACPI_6_4_GTDT_TIMER_FLAG_ALWAYS_ON_CAPABILITY

#pragma pack(1)
typedef EFI_ACPI_6_4_GENERIC_TIMER_DESCRIPTION_TABLE  BAIKAL_ACPI_GTDT;

STATIC BAIKAL_ACPI_GTDT  Gtdt = {
  BAIKAL_ACPI_HEADER (
    EFI_ACPI_6_4_GENERIC_TIMER_DESCRIPTION_TABLE_SIGNATURE,
    BAIKAL_ACPI_GTDT,
    EFI_ACPI_6_4_GENERIC_TIMER_DESCRIPTION_TABLE_REVISION,
    0x54445447
    ),
  /* UINT64  CntControlBasePhysicalAddress */
  0xFFFFFFFFFFFFFFFF,
  /* UINT32  Reserved                      */
  EFI_ACPI_RESERVED_DWORD,
  /* UINT32  SecurePL1TimerGSIV            */
  0,
  /* UINT32  SecurePL1TimerFlags           */
  BAIKAL_GTDT_TIMER_FLAGS,
  /* UINT32  NonSecurePL1TimerGSIV         */
  0,
  /* UINT32  NonSecurePL1TimerFlags        */
  BAIKAL_GTDT_TIMER_FLAGS,
  /* UINT32  VirtualTimerGSIV              */
  0,
  /* UINT32  VirtualTimerFlags             */
  BAIKAL_GTDT_TIMER_FLAGS,
  /* UINT32  NonSecurePL2TimerGSIV         */
  0,
  /* UINT32  NonSecurePL2TimerFlags        */
  BAIKAL_GTDT_TIMER_FLAGS,
  /* UINT64  CntReadBasePhysicalAddress    */
  0xFFFFFFFFFFFFFFFF,
  /* UINT32  PlatformTimerCount            */
  0,
  /* UINT32  PlatformTimerOffset           */
  0,
  /* UINT32  VirtualPL2TimerGSIV           */
  28,
  /* UINT32  VirtualPL2TimerFlags          */
  BAIKAL_GTDT_TIMER_FLAGS
};
#pragma pack()

EFI_STATUS
GtdtInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  Gtdt.SecurePL1TimerGSIV    = FixedPcdGet32 (PcdArmArchTimerSecIntrNum);
  Gtdt.NonSecurePL1TimerGSIV = FixedPcdGet32 (PcdArmArchTimerIntrNum);
  Gtdt.VirtualTimerGSIV      = FixedPcdGet32 (PcdArmArchTimerVirtIntrNum);
  Gtdt.NonSecurePL2TimerGSIV = FixedPcdGet32 (PcdArmArchTimerHypIntrNum);

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Gtdt;
  return EFI_SUCCESS;
}
