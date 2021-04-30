/** @file
  Copyright (c) 2015 - 2020, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __DW_UART_H__
#define __DW_UART_H__

#include <Uefi.h>
#include <Protocol/SerialIo.h>

/**
  Programmed hardware of Serial port.

  @return    Always return EFI_UNSUPPORTED.
**/
RETURN_STATUS
EFIAPI
DwUartInitializePort (
  IN OUT UINTN               UartBase,
  IN OUT UINT64              *BaudRate,
  IN OUT UINT32              *ReceiveFifoDepth,
  IN OUT EFI_PARITY_TYPE     *Parity,
  IN OUT UINT8               *DataBits,
  IN OUT EFI_STOP_BITS_TYPE  *StopBits
  );

/**
  Assert or deassert the control signals on a serial port.
  The following control signals are set according their bit settings :
  . Request to Send
  . Data Terminal Ready

  @param[in]  UartBase  UART registers base address
  @param[in]  Control   The following bits are taken into account :
                        . EFI_SERIAL_REQUEST_TO_SEND : assert/deassert the
                          "Request To Send" control signal if this bit is
                          equal to one/zero.
                        . EFI_SERIAL_DATA_TERMINAL_READY : assert/deassert
                          the "Data Terminal Ready" control signal if this
                          bit is equal to one/zero.
                        . EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE : enable/disable
                          the hardware loopback if this bit is equal to
                          one/zero.
                        . EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE : not supported.
                        . EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE : enable/
                          disable the hardware flow control based on CTS (Clear
                          To Send) and RTS (Ready To Send) control signals.

  @retval  RETURN_SUCCESS      The new control bits were set on the serial device.
  @retval  RETURN_UNSUPPORTED  The serial device does not support this operation.
**/
RETURN_STATUS
EFIAPI
DwUartSetControl (
  IN UINTN   UartBase,
  IN UINT32  Control
  );

/**
  Retrieve the status of the control bits on a serial device.

  @param[in]   UartBase  UART registers base address
  @param[out]  Control   Status of the control bits on a serial device :

                         . EFI_SERIAL_DATA_CLEAR_TO_SEND, EFI_SERIAL_DATA_SET_READY,
                           EFI_SERIAL_RING_INDICATE, EFI_SERIAL_CARRIER_DETECT,
                           EFI_SERIAL_REQUEST_TO_SEND, EFI_SERIAL_DATA_TERMINAL_READY
                           are all related to the DTE (Data Terminal Equipment) and
                           DCE (Data Communication Equipment) modes of operation of
                           the serial device.
                         . EFI_SERIAL_INPUT_BUFFER_EMPTY : equal to one if the receive
                           buffer is empty, 0 otherwise.
                         . EFI_SERIAL_OUTPUT_BUFFER_EMPTY : equal to one if the transmit
                           buffer is empty, 0 otherwise.
                         . EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE : equal to one if the
                           hardware loopback is enabled (the ouput feeds the receive
                           buffer), 0 otherwise.
                         . EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE : equal to one if a
                           loopback is accomplished by software, 0 otherwise.
                         . EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE : equal to one if the
                           hardware flow control based on CTS (Clear To Send) and RTS
                           (Ready To Send) control signals is enabled, 0 otherwise.


  @retval RETURN_SUCCESS  The control bits were read from the serial device.
**/
RETURN_STATUS
EFIAPI
DwUartGetControl (
  IN  UINTN    UartBase,
  OUT UINT32  *Control
  );

/**
  Write data to serial device.

  @param  Buffer           Point of data buffer which need to be written.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Write data failed.
  @retval !0               Actual number of bytes written to serial device.
**/
UINTN
EFIAPI
DwUartWrite (
  IN  UINTN   UartBase,
  IN  UINT8  *Buffer,
  IN  UINTN   NumberOfBytes
  );

/**
  Read data from serial device and save the data in buffer.

  @param  Buffer           Point of data buffer which need to be written.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Read data failed.
  @retval !0               Actual number of bytes read from serial device.
**/
UINTN
EFIAPI
DwUartRead (
  IN  UINTN   UartBase,
  OUT UINT8  *Buffer,
  IN  UINTN   NumberOfBytes
  );

/**
  Check to see if any data is available to be read from the debug device.

  @retval EFI_SUCCESS       At least one byte of data is available to be read
  @retval EFI_NOT_READY     No data is available to be read
  @retval EFI_DEVICE_ERROR  The serial device is not functioning properly
**/
BOOLEAN
EFIAPI
DwUartPoll (
  IN  UINTN  UartBase
  );

#endif /* __DW_UART_H__ */
