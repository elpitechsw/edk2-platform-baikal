/** @file
  Serial I/O Port library functions with no library constructor/destructor

  Copyright (c) 2015 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Base.h>
#include <Library/DwUartLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>

/**
  Programmed hardware of Serial port.

  @return    Always return RETURN_UNSUPPORTED.

**/
RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  UINT64              BaudRate;
  UINT32              ReceiveFifoDepth;
  EFI_PARITY_TYPE     Parity;
  UINT8               DataBits;
  EFI_STOP_BITS_TYPE  StopBits;

  BaudRate = (UINTN)FixedPcdGet64 (PcdSerialBaudRate);
  ReceiveFifoDepth = 0; // Use the default value for FIFO depth
  Parity   = DefaultParity;
  DataBits = 8;
  StopBits = DefaultStopBits;

  return DwUartInitializePort (
           (UINTN)FixedPcdGet64 (PcdSerialRegisterBase),
           &BaudRate,
           &ReceiveFifoDepth,
           &Parity,
           &DataBits,
           &StopBits
           );
}

/**
  Write data to serial device.

  @param  Buffer           Point of data buffer which need to be written.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Write data failed.
  @retval !0               Actual number of bytes written to serial device.

**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
  )
{
  return DwUartWrite ((UINTN)FixedPcdGet64 (PcdSerialRegisterBase), Buffer, NumberOfBytes);
}

/**
  Read data from serial device and save the data in buffer.

  @param  Buffer           Point of data buffer which need to be written.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Read data failed.
  @retval !0               Actual number of bytes read from serial device.

**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes
)
{
  return DwUartRead ((UINTN)FixedPcdGet64 (PcdSerialRegisterBase), Buffer, NumberOfBytes);
}

/**
  Check to see if any data is available to be read from the debug device.

  @retval EFI_SUCCESS       At least one byte of data is available to be read
  @retval EFI_NOT_READY     No data is available to be read
  @retval EFI_DEVICE_ERROR  The serial device is not functioning properly

**/
BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  return DwUartPoll ((UINTN)FixedPcdGet64 (PcdSerialRegisterBase));
}

RETURN_STATUS
EFIAPI
SerialPortSetControl (
  IN UINT32  Control
  )
{
  return DwUartSetControl((UINTN)FixedPcdGet64 (PcdSerialRegisterBase), Control);
}

RETURN_STATUS
EFIAPI
SerialPortGetControl (
  OUT UINT32  *Control
  )
{
  return DwUartGetControl((UINTN)FixedPcdGet64 (PcdSerialRegisterBase), Control);
}

RETURN_STATUS
EFIAPI
SerialPortSetAttributes (
  IN OUT UINT64              *BaudRate,
  IN OUT UINT32              *ReceiveFifoDepth,
  IN OUT UINT32              *Timeout,
  IN OUT EFI_PARITY_TYPE     *Parity,
  IN OUT UINT8               *DataBits,
  IN OUT EFI_STOP_BITS_TYPE  *StopBits
  )
{
  return RETURN_UNSUPPORTED;
}
