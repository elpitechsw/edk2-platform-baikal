/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SerialPortConsoleRedirectionTable.h>

#include "AcpiPlatform.h"

#pragma pack(1)
STATIC EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE  Spcr = {
  BAIKAL_ACPI_HEADER (
    EFI_ACPI_2_0_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE,
    EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE,
    EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_REVISION,
    0x52435053
    ),
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_INTERFACE_TYPE_ARM_PL011_UART, // InterfaceType
  { EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE },                                                   // Reserved1[3]
  { EFI_ACPI_6_4_SYSTEM_MEMORY, 32, 0, EFI_ACPI_6_4_DWORD, 0x00C00000 },        // BaseAddress
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_INTERRUPT_TYPE_GIC,            // InterruptType
  0,                                                                            // Irq
  93,                                                                           // GlobalSystemInterrupt
  0,                                                                            // BaudRate
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_PARITY_NO_PARITY,              // Parity
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_STOP_BITS_1,                   // StopBits
  0,                                                                            // FlowControl
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_TERMINAL_TYPE_ANSI,            // TerminalType
  EFI_ACPI_RESERVED_BYTE,                                                       // Language
  0xFFFF,                                                                       // PciDeviceId
  0xFFFF,                                                                       // PciVendorId
  0,                                                                            // PciBusNumber
  0,                                                                            // PciDeviceNumber
  0,                                                                            // PciFunctionNumber
  0,                                                                            // PciFlags
  0,                                                                            // PciSegment
  EFI_ACPI_RESERVED_DWORD                                                       // Reserved2
};
#pragma pack()

EFI_STATUS
SpcrInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Spcr;
  return EFI_SUCCESS;
}
