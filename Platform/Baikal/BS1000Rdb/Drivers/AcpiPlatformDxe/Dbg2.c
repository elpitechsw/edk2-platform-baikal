/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/DebugPort2Table.h>
#include <Library/BaseMemoryLib.h>
#include "AcpiPlatform.h"

#include <BS1000.h>

#define BAIKAL_DBG2_NUMBER_OF_DEBUG_PORTS                2
#define BAIKAL_DBG2_NUMBER_OF_GENERIC_ADDRESS_REGISTERS  1
#define BAIKAL_DBG2_NAME_PORT                            "\\_SB_.COM0"
#define BAIKAL_DBG2_NAMESPACE_STRING_SIZE                sizeof (BAIKAL_DBG2_NAME_PORT)
#define BAIKAL_DBG2_UART_ADDRESS_LENGTH                  0x1000

#pragma pack(1)

typedef struct {
  EFI_ACPI_DBG2_DEBUG_DEVICE_INFORMATION_STRUCT  Device;
  EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE         BaseAddressRegister;
  UINT32                                         AddressSize;
  UINT8                                          NamespaceString[BAIKAL_DBG2_NAMESPACE_STRING_SIZE];
} BAIKAL_DBG2_DEVICE_INFORMATION;

typedef struct {
  EFI_ACPI_DEBUG_PORT_2_DESCRIPTION_TABLE  Table;
  BAIKAL_DBG2_DEVICE_INFORMATION           Port[PLATFORM_CHIP_COUNT * BAIKAL_DBG2_NUMBER_OF_DEBUG_PORTS];
} BAIKAL_ACPI_DBG2;

#pragma pack()

STATIC BAIKAL_DBG2_DEVICE_INFORMATION  Dbg2PortTemplate = {
  {
    /* UINT8   Revision                        */
    EFI_ACPI_DBG2_DEBUG_DEVICE_INFORMATION_STRUCT_REVISION,
    /* UINT16  Length                          */
    sizeof (BAIKAL_DBG2_DEVICE_INFORMATION),
    /* UINT8   NumberofGenericAddressRegisters */
    BAIKAL_DBG2_NUMBER_OF_GENERIC_ADDRESS_REGISTERS,
    /* UINT16  NameSpaceStringLength           */
    BAIKAL_DBG2_NAMESPACE_STRING_SIZE,
    /* UINT16  NameSpaceStringOffset           */
    OFFSET_OF (BAIKAL_DBG2_DEVICE_INFORMATION, NamespaceString),
    /* UINT16  OemDataLength                   */
    0,
    /* UINT16  OemDataOffset                   */
    0,
    /* UINT16  PortType                        */
    EFI_ACPI_DBG2_PORT_TYPE_SERIAL,
    /* UINT16  PortSubtype                     */
    EFI_ACPI_DBG2_PORT_SUBTYPE_SERIAL_ARM_PL011_UART,
    /* UINT8   Reserved[2]                     */
    { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE },
    /* UINT16  BaseAddressRegisterOffset       */
    OFFSET_OF (BAIKAL_DBG2_DEVICE_INFORMATION, BaseAddressRegister),
    /* UINT16  AddressSizeOffset               */
    OFFSET_OF (BAIKAL_DBG2_DEVICE_INFORMATION, AddressSize)
  },
  /* EFI_ACPI_6_4_GENERIC_ADDRESS_STRUCTURE  BaseAddressRegister */
  { EFI_ACPI_6_4_SYSTEM_MEMORY, 64, 0, EFI_ACPI_6_4_QWORD, 0 },
  /* UINT32                                  AddressSize */
  BAIKAL_DBG2_UART_ADDRESS_LENGTH,
  /* UINT8                                   NamespaceString[BAIKAL_DBG2_NAMESPACE_STRING_SIZE] */
  BAIKAL_DBG2_NAME_PORT
};

STATIC BAIKAL_ACPI_DBG2  Dbg2 = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_4_DEBUG_PORT_2_TABLE_SIGNATURE,
      BAIKAL_ACPI_DBG2,
      EFI_ACPI_DEBUG_PORT_2_TABLE_REVISION,
      0x32474244
      ),
    /* UINT32  OffsetDbgDeviceInfo */
    OFFSET_OF (BAIKAL_ACPI_DBG2, Port),
    /* UINT32  NumberDbgDeviceInfo */
    PLATFORM_CHIP_COUNT * BAIKAL_DBG2_NUMBER_OF_DEBUG_PORTS
  }
};

EFI_STATUS
Dbg2Init (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINTN  ChipIdx;
  UINTN  Idx;
  UINTN  Num;

  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BAIKAL_DBG2_NUMBER_OF_DEBUG_PORTS; ++Idx, ++Num) {
      UINT64  Address = (Idx == 0) ? BS1000_UART_A1_BASE : BS1000_UART_A2_BASE;

      CopyMem (&Dbg2.Port[Num], &Dbg2PortTemplate, sizeof (Dbg2PortTemplate));
      Dbg2.Port[Num].BaseAddressRegister.Address = PLATFORM_ADDR_OUT_CHIP (ChipIdx, Address);
      Dbg2.Port[Num].NamespaceString[9] += Num;
    }
  }

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Dbg2;
  return EFI_SUCCESS;
}
