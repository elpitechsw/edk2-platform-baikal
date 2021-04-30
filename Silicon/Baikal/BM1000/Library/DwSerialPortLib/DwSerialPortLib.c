/** @file
  Serial I/O Port library functions with no library constructor/destructor

  Copyright (c) 2015 - 2020, Baikal Electronics. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

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

  BaudRate = (UINTN)PcdGet64 (PcdUartBaudRate);
  ReceiveFifoDepth = 0; // Use the default value for Fifo depth
  Parity = (EFI_PARITY_TYPE)PcdGet8 (PcdUartParity);
  DataBits = PcdGet8 (PcdUartDataBits);
  StopBits = (EFI_STOP_BITS_TYPE) PcdGet8 (PcdUartStopBits);

  return DwUartInitializePort (
      (UINTN)PcdGet64 (PcdUartRegisterBase),
      &BaudRate, &ReceiveFifoDepth, &Parity, &DataBits, &StopBits);
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
  return DwUartWrite ((UINTN)PcdGet64 (PcdUartRegisterBase), Buffer, NumberOfBytes);
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
  return DwUartRead ((UINTN)PcdGet64 (PcdUartRegisterBase), Buffer, NumberOfBytes);
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
  return DwUartPoll ((UINTN)PcdGet64 (PcdUartRegisterBase));
}
