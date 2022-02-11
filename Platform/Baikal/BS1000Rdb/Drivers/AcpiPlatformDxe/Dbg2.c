/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/DebugPort2Table.h>

#include "AcpiPlatform.h"

#define BAIKAL_DBG2_NUMBER_OF_DEBUG_PORTS                2
#define BAIKAL_DBG2_NUMBER_OF_GENERIC_ADDRESS_REGISTERS  1
#define BAIKAL_DBG2_NAME_PORT0                           "COM0"
#define BAIKAL_DBG2_NAME_PORT1                           "COM1"
#define BAIKAL_DBG2_NAMESPACE_STRING_SIZE                sizeof (BAIKAL_DBG2_NAME_PORT0)
#define BAIKAL_DBG2_UART_ADDRESS_LENGTH                  0x1000

#define BAIKAL_DBG2_PORT(UartBase, UartName)  {                                                    \
  {                                                                                                \
    /* UINT8   Revision                        */                                                  \
    EFI_ACPI_DBG2_DEBUG_DEVICE_INFORMATION_STRUCT_REVISION,                                        \
    /* UINT16  Length                          */                                                  \
    sizeof (BAIKAL_DBG2_DEVICE_INFORMATION),                                                       \
    /* UINT8   NumberofGenericAddressRegisters */                                                  \
    BAIKAL_DBG2_NUMBER_OF_GENERIC_ADDRESS_REGISTERS,                                               \
    /* UINT16  NameSpaceStringLength           */                                                  \
    BAIKAL_DBG2_NAMESPACE_STRING_SIZE,                                                             \
    /* UINT16  NameSpaceStringOffset           */                                                  \
    OFFSET_OF (BAIKAL_DBG2_DEVICE_INFORMATION, NamespaceString),                                   \
    /* UINT16  OemDataLength                   */                                                  \
    0,                                                                                             \
    /* UINT16  OemDataOffset                   */                                                  \
    0,                                                                                             \
    /* UINT16  PortType                        */                                                  \
    EFI_ACPI_DBG2_PORT_TYPE_SERIAL,                                                                \
    /* UINT16  PortSubtype                     */                                                  \
    EFI_ACPI_DBG2_PORT_SUBTYPE_SERIAL_ARM_PL011_UART,                                              \
    /* UINT8   Reserved[2]                     */                                                  \
    { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE },                                            \
    /* UINT16  BaseAddressRegisterOffset       */                                                  \
    OFFSET_OF (BAIKAL_DBG2_DEVICE_INFORMATION, BaseAddressRegister),                               \
    /* UINT16  AddressSizeOffset               */                                                  \
    OFFSET_OF (BAIKAL_DBG2_DEVICE_INFORMATION, AddressSize)                                        \
  },                                                                                               \
  /* EFI_ACPI_6_3_GENERIC_ADDRESS_STRUCTURE  BaseAddressRegister */                                \
  { EFI_ACPI_6_3_SYSTEM_MEMORY, 32, 0, EFI_ACPI_6_3_BYTE, UartBase },                              \
  /* UINT32                                  AddressSize */                                        \
  BAIKAL_DBG2_UART_ADDRESS_LENGTH,                                                                 \
  /* UINT8                                   NamespaceString[BAIKAL_DBG2_NAMESPACE_STRING_SIZE] */ \
  UartName                                                                                         \
}

#pragma pack(1)
typedef struct {
  EFI_ACPI_DBG2_DEBUG_DEVICE_INFORMATION_STRUCT  Device;
  EFI_ACPI_6_3_GENERIC_ADDRESS_STRUCTURE         BaseAddressRegister;
  UINT32                                         AddressSize;
  UINT8                                          NamespaceString[BAIKAL_DBG2_NAMESPACE_STRING_SIZE];
} BAIKAL_DBG2_DEVICE_INFORMATION;

typedef struct {
  EFI_ACPI_DEBUG_PORT_2_DESCRIPTION_TABLE  Table;
  BAIKAL_DBG2_DEVICE_INFORMATION           Port[BAIKAL_DBG2_NUMBER_OF_DEBUG_PORTS];
} BAIKAL_ACPI_DBG2;

STATIC BAIKAL_ACPI_DBG2  Dbg2 = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_3_DEBUG_PORT_2_TABLE_SIGNATURE,
      BAIKAL_ACPI_DBG2,
      EFI_ACPI_DEBUG_PORT_2_TABLE_REVISION,
      0x32474244
      ),
    /* UINT32  OffsetDbgDeviceInfo */
    OFFSET_OF (BAIKAL_ACPI_DBG2, Port),
    /* UINT32  NumberDbgDeviceInfo */
    BAIKAL_DBG2_NUMBER_OF_DEBUG_PORTS
  },
  {
    BAIKAL_DBG2_PORT (0x00C00000, BAIKAL_DBG2_NAME_PORT0),
    BAIKAL_DBG2_PORT (0x00C10000, BAIKAL_DBG2_NAME_PORT1)
  }
};
#pragma pack()

EFI_STATUS
Dbg2Init (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Dbg2;
  return EFI_SUCCESS;
}
