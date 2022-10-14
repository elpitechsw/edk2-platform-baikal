/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Pci22.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Platform/ConfigVars.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/PciIo.h>

#include <BM1000.h>
#include "AcpiPlatform.h"

// DeviceOp PkgLength NameString
#define MAX_DEV_MATCH_LEN   (2 + 4 + 4)

#define NAMEOP_PATTERN_LEN  5
#define AML_NAMEOP_8        0x0A
#define AML_NAMEOP_16       0x0B
#define AML_NAMEOP_32       0x0C
#define AML_NAMEOP_64       0x0E

#define PPB_IO_RANGE        2
#define PPB_MEM32_RANGE     3
#define PPB_PMEM32_RANGE    4
#define PPB_PMEM64_RANGE    5

extern unsigned char  dsdt_aml_code[];
extern unsigned char  ssdtpciecustom_aml_code[];
extern unsigned char  ssdtpcieecam_aml_code[];

STATIC EFI_ACPI_TABLE_PROTOCOL  *mAcpiTableProtocol;
STATIC VOID                     *mPioRegistration;

STATIC
EFI_STATUS
DsdtInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) dsdt_aml_code;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SsdtPcieInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  if (PcdGet32 (PcdAcpiPcieMode) == ACPI_PCIE_CUSTOM) {
     *Table = (EFI_ACPI_DESCRIPTION_HEADER *) ssdtpciecustom_aml_code;
     return EFI_SUCCESS;
  }

  //
  // For other ACPI_PCIE_ types (beyond NONE, obviously),
  // the SSDTs are custom generated.
  //
  return EFI_NOT_FOUND;
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
extern EFI_STATUS PmttInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS PpttInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);
extern EFI_STATUS SpcrInit (EFI_ACPI_DESCRIPTION_HEADER  **Table);

STATIC BAIKAL_ACPI_INIT_FUNCTION  AcpiTableInit[] = {
  &CsrtInit,
  &Dbg2Init,
  &DsdtInit,
  &FacsInit,
  &FadtInit,
  &GtdtInit,
  &IortInit,
  &MadtInit,
  &McfgInit,
  &PmttInit,
  &PpttInit,
  &SpcrInit,
  &SsdtPcieInit
};

STATIC
EFI_STATUS
FindAndPatchNameOp (
  IN  EFI_ACPI_DESCRIPTION_HEADER  *Table,
  IN  CHAR8                        *Name,
  IN  UINT64                        Value
  )
{
  CHAR8  *Data = (VOID *) (Table + 1);
  CHAR8  *DataEnd = Data + Table->Length -
                    sizeof (EFI_ACPI_DESCRIPTION_HEADER) - NAMEOP_PATTERN_LEN;
  CHAR8   Pattern[NAMEOP_PATTERN_LEN];

  Pattern[0] = 0x08;
  Pattern[1] = Name[0];
  Pattern[2] = Name[1];
  Pattern[3] = Name[2];
  Pattern[4] = Name[3];

  while (Data < DataEnd) {
    if (CompareMem (Data, Pattern, NAMEOP_PATTERN_LEN) == 0) {
      switch (Data[NAMEOP_PATTERN_LEN]) {
      case AML_NAMEOP_64:
        Data[NAMEOP_PATTERN_LEN + 8] = (Value >> 56) & 0xFF;
        Data[NAMEOP_PATTERN_LEN + 7] = (Value >> 48) & 0xFF;
        Data[NAMEOP_PATTERN_LEN + 6] = (Value >> 40) & 0xFF;
        Data[NAMEOP_PATTERN_LEN + 5] = (Value >> 32) & 0xFF;
      case AML_NAMEOP_32:
        Data[NAMEOP_PATTERN_LEN + 4] = (Value >> 24) & 0xFF;
        Data[NAMEOP_PATTERN_LEN + 3] = (Value >> 16) & 0xFF;
      case AML_NAMEOP_16:
        Data[NAMEOP_PATTERN_LEN + 2] = (Value >> 8) & 0xFF;
      case AML_NAMEOP_8:
        Data[NAMEOP_PATTERN_LEN + 1] = Value & 0xFF;
        break;
      default:
        DEBUG ((
          EFI_D_ERROR,
          "Couldn't patch %a of kind 0x%x\n",
          Name,
          Data[NAMEOP_PATTERN_LEN]
          ));
        return EFI_UNSUPPORTED;
      }

      DEBUG ((EFI_D_ERROR, "Patched %a with 0x%lx\n", Name, Value));
      return EFI_SUCCESS;
    }

    Data++;
  };

  DEBUG ((EFI_D_ERROR, "Couldn't find %a\n", Name));
  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
FindAndPatchDevName (
  IN  EFI_ACPI_DESCRIPTION_HEADER  *Table,
  IN  UINTN                         Index
  )
{
  CHAR8  *Data = (VOID *) (Table + 1);
  CHAR8  *DataEnd = Data + Table->Length -
                    sizeof (EFI_ACPI_DESCRIPTION_HEADER) - MAX_DEV_MATCH_LEN;

  while (Data < DataEnd) {
    if (Data[0] == 0x5B && Data[1] == 0x82) {
      CHAR8  *Name = Data + ((Data[2] >> 6) + 3);
      if (CompareMem (Name, "XXXX", 4) == 0) {
        Name[0] = 'P';
        Name[1] = '0' + Index / 100;
        Name[2] = '0' + Index / 10;
        Name[3] = '0' + Index % 10;
        return EFI_SUCCESS;
      }
    }

    Data++;
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_ACPI_DESCRIPTION_HEADER *
GeneratePcieSsdt (
  IN  UINTN                               Index,
  IN  UINTN                               Segment,
  IN  UINTN                               RidBus,
  IN  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *IoDesc,
  IN  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *Mem32Desc,
  IN  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *PMemDesc,
  IN  EFI_ACPI_DESCRIPTION_HEADER        *Table
  )
{
  STATIC CONST EFI_PHYSICAL_ADDRESS  CfgBases[] = {
                                                    BAIKAL_ACPI_PCIE0_CFG_BASE,
                                                    BAIKAL_ACPI_PCIE1_CFG_BASE,
                                                    BAIKAL_ACPI_PCIE2_CFG_BASE
                                                  };

  UINT64                        Iosi = 0;
  EFI_ACPI_DESCRIPTION_HEADER  *NewTb;

  if (Index > BM1000_PCIE2_IDX) {
    return NULL;
  }

  if (Table->Length == 0) {
    return NULL;
  }

  NewTb = AllocatePool (Table->Length);
  if (NewTb == NULL) {
    return NULL;
  }

  CopyMem (NewTb, Table, Table->Length);

  if (EFI_ERROR (FindAndPatchDevName (NewTb, Segment))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "_UID", Segment))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "_SEG", Segment))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "RUID", Index))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "CFGB", CfgBases[Index] + (RidBus << 20)))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "MB32", Mem32Desc != NULL ? Mem32Desc->AddrRangeMin : 0))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "MS32", Mem32Desc != NULL ? Mem32Desc->AddrLen : 0))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "MBPF", PMemDesc != NULL ? PMemDesc->AddrRangeMin : 0))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "MSPF", PMemDesc != NULL ? PMemDesc->AddrLen : 0))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "IOCA", IoDesc != NULL ? IoDesc->AddrRangeMin : 0))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "IOBA", IoDesc != NULL ? IoDesc->AddrRangeMin + IoDesc->AddrTranslationOffset : 0))) {
    goto error;
  }

  //
  // There's an off-by-one error in the check for overlap in Linux's logic_pio_register_range,
  // which means we cannot have consecutive ranges, which is pretty likely when you have an adapter
  // with multiple PCIe EPs (thus with multiple PPBs). Just return the range - page, which is sloppy,
  // but in practice is unlikely to ever break anything (as the amount of I/O allocated for each PPB
  // greatly exceeds the EP allocations in practice).
  //
  // Have to subtract something page-aligned due to other Linux checks... joy.
  //
  if (IoDesc != NULL && IoDesc->AddrLen > EFI_PAGE_SIZE) {
    Iosi = IoDesc->AddrLen - EFI_PAGE_SIZE;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "IOSI", Iosi))) {
    goto error;
  }

  if (EFI_ERROR (FindAndPatchNameOp (NewTb, "BUSC", 1))) {
    goto error;
  }

  AsciiSPrint (
    (CHAR8 *) &NewTb->OemTableId,
    sizeof (NewTb->OemTableId),
    "PCI%u",
    Segment
    );

  return NewTb;

error:
  FreePool (NewTb);
  return NULL;
}

VOID
ProcessBridgeForSsdt (
  IN  UINTN                               UefiSeg,
  IN  UINTN                               Bus,
  IN  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *IoDesc,
  IN  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *Mem32Desc,
  IN  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *PMemDesc
  )
{
  UINTN               AcpiSeg;
  STATIC CONST UINT8  AcpiSegs[3] = {
     BAIKAL_ACPI_PCIE0_SEGMENT,
#ifdef BAIKAL_ACPI_PCIE1_SEGMENT
     BAIKAL_ACPI_PCIE1_SEGMENT,
#else
     0, /* Unused */
#endif
#ifdef BAIKAL_ACPI_PCIE2_SEGMENT
     BAIKAL_ACPI_PCIE2_SEGMENT
#else
     0, /* Unused */
#endif
  };

  UINTN                         Index;
  EFI_STATUS                    Status;
  EFI_ACPI_DESCRIPTION_HEADER  *Table;
  UINTN                         TableHandle;

  Index = UefiSeg;
  AcpiSeg = AcpiSegs[UefiSeg];
  if (Bus > 1) {
    AcpiSeg = SYNTH_SEG(AcpiSeg, Bus);
  }

  DEBUG ((
    DEBUG_ERROR,
    "Generating SSDT for seg %u (UEFI seg %u), index %u, bus %u\n",
    AcpiSeg,
    UefiSeg,
    Index,
    Bus
    ));

  Table = GeneratePcieSsdt (
            Index,
            AcpiSeg,
            Bus > 1 ? Bus : 0,
            IoDesc,
            Mem32Desc,
            PMemDesc,
            (EFI_ACPI_DESCRIPTION_HEADER *) ssdtpcieecam_aml_code
            );

  if (Table == NULL) {
    DEBUG ((DEBUG_ERROR, "ERROR: Failed to generate SSDT for PCI%u\n", Index));
    return;
  }

  Status = mAcpiTableProtocol->InstallAcpiTable (
                                 mAcpiTableProtocol,
                                 Table,
                                 Table->Length,
                                 &TableHandle
                                 );
  FreePool (Table);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: Failed to install SSDT for PCI%u. Status = %r\n",
      AcpiSeg,
      Status
      ));
  }
}

VOID
EFIAPI
OnPciIoInstall (
  IN  EFI_EVENT   Event,
  IN  VOID       *Context
  )
{
  EFI_STATUS                          Status;
  EFI_HANDLE                          Handle;
  UINTN                               HandleSize;
  PCI_TYPE01                          PciConfigHeader;
  EFI_PCI_IO_PROTOCOL                *PciIo;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *IoDesc    = NULL;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *Mem32Desc = NULL;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *PMemDesc  = NULL;
  UINT8                               SecondaryBus;
  UINTN                               Segment;
  UINTN                               Bus;
  UINTN                               Device;
  UINTN                               Function;

  while (TRUE) {
    HandleSize = sizeof (EFI_HANDLE);
    Status = gBS->LocateHandle (
                    ByRegisterNotify,
                    NULL,
                    mPioRegistration,
                    &HandleSize,
                    &Handle
                    );
    if (Status == EFI_NOT_FOUND) {
      break;
    }

    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      continue;
    }

    PciIo->Pci.Read (
                 PciIo,
                 EfiPciIoWidthUint32,
                 0,
                 sizeof (PciConfigHeader) / sizeof (UINT32),
                 &PciConfigHeader
                 );
    if (!IS_PCI_P2P (&PciConfigHeader) && !IS_PCI_P2P_SUB (&PciConfigHeader)) {
      continue;
    }

    PciIo->Pci.Read (
                 PciIo,
                 EfiPciIoWidthUint8,
                 PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET,
                 1,
                 &SecondaryBus
                 );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (SecondaryBus > SYNTH_BUS_PER_SEG) {
      continue;
    }

    PciIo->GetBarAttributes (
             PciIo,
             PPB_IO_RANGE,
             NULL,
             (VOID**) &IoDesc
             );

    PciIo->GetBarAttributes (
             PciIo,
             PPB_MEM32_RANGE,
             NULL,
             (VOID**) &Mem32Desc
             );

    if (PciIo->GetBarAttributes (
                 PciIo,
                 PPB_PMEM64_RANGE,
                 NULL,
                 (VOID**) &PMemDesc
                 ) != EFI_SUCCESS) {
      PciIo->GetBarAttributes (
               PciIo,
               PPB_PMEM32_RANGE,
               NULL,
               (VOID**) &PMemDesc
               );
    }

    if (IoDesc == NULL && Mem32Desc == NULL && PMemDesc == NULL) {
      continue;
    }

    ProcessBridgeForSsdt (Segment, SecondaryBus, IoDesc, Mem32Desc, PMemDesc);

    if (IoDesc != NULL) {
      FreePool (IoDesc);
    }

    if (Mem32Desc != NULL) {
      FreePool (Mem32Desc);
    }
  }
}

EFI_STATUS
EFIAPI
AcpiPlatformDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_ACPI_DESCRIPTION_HEADER  *AcpiTable;
  EFI_EVENT                     Event;
  UINTN                         Idx;
  EFI_STATUS                    Status;
  UINTN                         TableHandle;

  Status = gBS->LocateProtocol (
                  &gEfiAcpiTableProtocolGuid,
                  NULL,
                  (VOID **) &mAcpiTableProtocol
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

    Status = mAcpiTableProtocol->InstallAcpiTable (
                                   mAcpiTableProtocol,
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

  if (PcdGet32 (PcdAcpiPcieMode) == ACPI_PCIE_ECAM) {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    OnPciIoInstall,
                    NULL,
                    &Event
                    );
    ASSERT_EFI_ERROR (Status);

    Status = gBS->RegisterProtocolNotify (
                    &gEfiPciIoProtocolGuid,
                    Event,
                    &mPioRegistration
                    );
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}
