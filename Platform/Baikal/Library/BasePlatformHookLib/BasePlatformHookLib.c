/** @file
  Copyright (c) 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#define R_UART_LSR          5
#define   B_UART_LSR_TXRDY  BIT5
#define   B_UART_LSR_TEMT   BIT6

STATIC
UINT8
SerialPortReadRegister (
  UINTN  Base,
  UINTN  Offset
  )
{
  if (FixedPcdGet8 (PcdSerialRegisterAccessWidth) == 32) {
    return (UINT8)MmioRead32 (Base + Offset * FixedPcdGet32 (PcdSerialRegisterStride));
  }

  return MmioRead8 (Base + Offset * FixedPcdGet32 (PcdSerialRegisterStride));
}

RETURN_STATUS
EFIAPI
PlatformHookSerialPortInitialize (
  VOID
  )
{
  UINTN  SerialRegisterBase;

  SerialRegisterBase = FixedPcdGet64 (PcdSerialRegisterBase);
  if (SerialRegisterBase == 0) {
    return RETURN_SUCCESS;
  }

  //
  // BaseSerialPortLib16550 writes DLAB bit during SerialPortInitialize ().
  // Verify that both the transmit FIFO and the shift register are empty.
  //
  while ((SerialPortReadRegister (SerialRegisterBase, R_UART_LSR) & (B_UART_LSR_TEMT | B_UART_LSR_TXRDY)) != (B_UART_LSR_TEMT | B_UART_LSR_TXRDY)) {
  }

  return RETURN_SUCCESS;
}
