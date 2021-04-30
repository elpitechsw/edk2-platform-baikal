/** @file
  Serial I/O Port library functions with no library constructor/destructor

  Copyright (c) 2015 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/DwUartLib.h>
#include <Library/PcdLib.h>

#define UART_FCR_FIFO_EN       0x01
#define UART_FCR_RXSR          0x02
#define UART_FCR_TXSR          0x04
#define UART_FIFO_LENGTH       16

/*
 * Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting
 *       UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
 */
#define UART_LCR_WLS_MSK       0x03
#define UART_LCR_WLS_5         0x00
#define UART_LCR_WLS_6         0x01
#define UART_LCR_WLS_7         0x02
#define UART_LCR_WLS_8         0x03
#define UART_LCR_STB           0x04
#define UART_LCR_PEN           0x08
#define UART_LCR_EPS           0x10
#define UART_LCR_STKP          0x20
#define UART_LCR_SBRK          0x40
#define UART_LCR_BKSE          0x80
#define UART_LCR_DLAB          0x80

#define UART_MCR_DTR           0x01
#define UART_MCR_RTS           0x02
#define UART_MCR_OUT1          0x04
#define UART_MCR_OUT2          0x08
#define UART_MCR_LOOP          0x10

#define UART_LSR_DR            0x01
#define UART_LSR_OE            0x02
#define UART_LSR_PE            0x04
#define UART_LSR_FE            0x08
#define UART_LSR_BI            0x10
#define UART_LSR_THRE          0x20
#define UART_LSR_TEMT          0x40
#define UART_LSR_ERR           0x80

#define UART_MSR_DCD           0x80
#define UART_MSR_RI            0x40
#define UART_MSR_DSR           0x20
#define UART_MSR_CTS           0x10
#define UART_MSR_DDCD          0x08
#define UART_MSR_TERI          0x04
#define UART_MSR_DDSR          0x02
#define UART_MSR_DCTS          0x01

#define UART_USR_BUSY          0x01
#define UART_USR_TFNF          0x02
#define UART_USR_TFE           0x04
#define UART_USR_RFNE          0x08
#define UART_USR_RFF           0x10

#define READ_UART_REG(r)       (*((volatile unsigned int *)(r)))
#define WRITE_UART_REG(r, v)   (*((volatile unsigned int *)(r)) = v)

// Pass the base address as the function argument
#define UART_BASE              UartBase
#define UPORT_OFF              0x10000
#define UART_PORT(p)           (UART_BASE + (p)*UPORT_OFF)
#define UART_PORT_ADDR(p)      ((volatile llenv32_uart_t *)UART_PORT(p))

#define UART_RBR(p)     (UART_PORT(p) + 0x00)
#define UART_DLL(p)     (UART_PORT(p) + 0x00)
#define UART_THR(p)     (UART_PORT(p) + 0x00)
#define UART_DLH(p)     (UART_PORT(p) + 0x04)
#define UART_IER(p)     (UART_PORT(p) + 0x04)
#define UART_IIR(p)     (UART_PORT(p) + 0x08)
#define UART_FCR(p)     (UART_PORT(p) + 0x08)
#define UART_LCR(p)     (UART_PORT(p) + 0x0C)
#define UART_MCR(p)     (UART_PORT(p) + 0x10)
#define UART_LSR(p)     (UART_PORT(p) + 0x14)
#define UART_MSR(p)     (UART_PORT(p) + 0x18)
#define UART_SCR(p)     (UART_PORT(p) + 0x1C)
#define UART_STHR0(p)   (UART_PORT(p) + 0x30)
#define UART_SRBR0(p)   (UART_PORT(p) + 0x30)
#define UART_SRBR1(p)   (UART_PORT(p) + 0x34)
#define UART_STHR1(p)   (UART_PORT(p) + 0x34)
#define UART_SRBR2(p)   (UART_PORT(p) + 0x38)
#define UART_STHR2(p)   (UART_PORT(p) + 0x38)
#define UART_STHR3(p)   (UART_PORT(p) + 0x3C)
#define UART_SRBR3(p)   (UART_PORT(p) + 0x3C)
#define UART_STHR4(p)   (UART_PORT(p) + 0x40)
#define UART_SRBR4(p)   (UART_PORT(p) + 0x40)
#define UART_SRBR5(p)   (UART_PORT(p) + 0x44)
#define UART_STHR5(p)   (UART_PORT(p) + 0x44)
#define UART_STHR6(p)   (UART_PORT(p) + 0x48)
#define UART_SRBR6(p)   (UART_PORT(p) + 0x48)
#define UART_SRBR7(p)   (UART_PORT(p) + 0x4C)
#define UART_STHR7(p)   (UART_PORT(p) + 0x4C)
#define UART_SRBR8(p)   (UART_PORT(p) + 0x50)
#define UART_STHR8(p)   (UART_PORT(p) + 0x50)
#define UART_STHR9(p)   (UART_PORT(p) + 0x54)
#define UART_SRBR9(p)   (UART_PORT(p) + 0x54)
#define UART_STHR10(p)  (UART_PORT(p) + 0x58)
#define UART_SRBR10(p)  (UART_PORT(p) + 0x58)
#define UART_STHR11(p)  (UART_PORT(p) + 0x5C)
#define UART_SRBR11(p)  (UART_PORT(p) + 0x5C)
#define UART_STHR12(p)  (UART_PORT(p) + 0x60)
#define UART_SRBR12(p)  (UART_PORT(p) + 0x60)
#define UART_SRBR13(p)  (UART_PORT(p) + 0x64)
#define UART_STHR13(p)  (UART_PORT(p) + 0x64)
#define UART_SRBR14(p)  (UART_PORT(p) + 0x68)
#define UART_STHR14(p)  (UART_PORT(p) + 0x68)
#define UART_SRBR15(p)  (UART_PORT(p) + 0x6C)
#define UART_STHR15(p)  (UART_PORT(p) + 0x6C)
#define UART_FAR(p)     (UART_PORT(p) + 0x70)
#define UART_TFR(p)     (UART_PORT(p) + 0x74)
#define UART_RFW(p)     (UART_PORT(p) + 0x78)
#define UART_USR(p)     (UART_PORT(p) + 0x7C)
#define UART_TFL(p)     (UART_PORT(p) + 0x80)
#define UART_RFL(p)     (UART_PORT(p) + 0x84)
#define UART_SRR(p)     (UART_PORT(p) + 0x88)
#define UART_SRTS(p)    (UART_PORT(p) + 0x8C)
#define UART_SBCR(p)    (UART_PORT(p) + 0x90)
#define UART_SDMAM(p)   (UART_PORT(p) + 0x94)
#define UART_SFE(p)     (UART_PORT(p) + 0x98)
#define UART_SRT(p)     (UART_PORT(p) + 0x9C)
#define UART_STET(p)    (UART_PORT(p) + 0xA0)
#define UART_HTX(p)     (UART_PORT(p) + 0xA4)
#define UART_DMASA(p)   (UART_PORT(p) + 0xA8)
#define UART_CPR(p)     (UART_PORT(p) + 0xF4)
#define UART_UCV(p)     (UART_PORT(p) + 0xF8)
#define UART_CTR(p)     (UART_PORT(p) + 0xFC)

/*
  Initialise the serial port to the specified settings.
  All unspecified settings will be set to the default values.

  @return    Always return EFI_SUCCESS or EFI_INVALID_PARAMETER.
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
  )
{
  UINT32  LineControl = 0;

  if ((*ReceiveFifoDepth == 0) || (*ReceiveFifoDepth >= UART_FIFO_LENGTH)) {
    *ReceiveFifoDepth = UART_FIFO_LENGTH;
  } else {
    ASSERT (*ReceiveFifoDepth < UART_FIFO_LENGTH);
    // Nothing else to do. 1 byte fifo is default.
    *ReceiveFifoDepth = 1;
  }

  switch (*Parity) {
  case DefaultParity:
    *Parity = NoParity;
  case NoParity:
    // Nothing to do. Parity is disabled by default.
    break;
  case EvenParity:
    LineControl |= UART_LCR_PEN | UART_LCR_EPS;
    break;
  case OddParity:
    LineControl |= UART_LCR_PEN;
    break;
  case MarkParity:
    LineControl |= UART_LCR_PEN | UART_LCR_STKP | UART_LCR_EPS;
    break;
  case SpaceParity:
    LineControl |= UART_LCR_PEN | UART_LCR_STKP;
    break;
  default:
    return RETURN_INVALID_PARAMETER;
  }

  switch (*DataBits) {
  case 0:
  case 8:
    *DataBits = 8;
    LineControl |= UART_LCR_WLS_8;
    break;
  case 7:
    LineControl |= UART_LCR_WLS_7;
    break;
  case 6:
    LineControl |= UART_LCR_WLS_6;
    break;
  case 5:
    LineControl |= UART_LCR_WLS_5;
    break;
  default:
    return RETURN_INVALID_PARAMETER;
  }

  switch (*StopBits) {
  case DefaultStopBits:
    *StopBits = OneStopBit;
  case OneStopBit:
    // Nothing to do. One stop bit is enabled by default.
    break;
  case TwoStopBits:
    LineControl |= UART_LCR_STB;
    break;
  case OneFiveStopBits:
    // Only 1 or 2 stops bits are supported
  default:
    return RETURN_INVALID_PARAMETER;
  }

  // DLAB is writeable only when UART is not busy (USR[0] is equal to 0)
  while (READ_UART_REG(UART_USR(0)) & UART_USR_BUSY) {
    while (READ_UART_REG(UART_LSR(0)) & UART_LSR_DR) {
      READ_UART_REG(UART_RBR(0));
    }
  }

  WRITE_UART_REG(UART_LCR(0), UART_LCR_DLAB); // DLAB -> 1

  // TODO: Specify baudrate
  WRITE_UART_REG(UART_DLL(0), 0x4); // Divisor 4 115200 divisor = 81, 9600, 12.5 MHz
  WRITE_UART_REG(UART_LCR(0), LineControl); // DLAB -> 0, 8 data bits

  // Enable Rx/Tx FIFOs
  WRITE_UART_REG(UART_FCR(0), UART_FCR_FIFO_EN | UART_FCR_RXSR | UART_FCR_TXSR);

  return RETURN_SUCCESS;
}

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
  )
{
  UINT32  McrReg;

  if (Control & EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE) {
    return RETURN_UNSUPPORTED;
  }

  McrReg = READ_UART_REG(UART_MCR(0));

  if (Control & EFI_SERIAL_REQUEST_TO_SEND) {
    McrReg |= UART_MCR_RTS;
  } else {
    McrReg &= ~UART_MCR_RTS;
  }

  if (Control & EFI_SERIAL_DATA_TERMINAL_READY) {
    McrReg |= UART_MCR_DTR;
  } else {
    McrReg &= ~UART_MCR_DTR;
  }

  if (Control & EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE) {
    McrReg |= UART_MCR_LOOP;
  } else {
    McrReg &= ~UART_MCR_LOOP;
  }

  WRITE_UART_REG(UART_MCR(0), McrReg);

  return RETURN_SUCCESS;
}

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
  )
{
  UINT32  LsrReg;
  UINT32  McrReg;
  UINT32  MsrReg;

  MsrReg = READ_UART_REG(UART_MSR(0));
  McrReg = READ_UART_REG(UART_MCR(0));
  LsrReg = READ_UART_REG(UART_LSR(0));

  *Control = 0;

  if ((MsrReg & UART_MSR_CTS) == UART_MSR_CTS) {
    *Control |= EFI_SERIAL_CLEAR_TO_SEND;
  }

  if ((MsrReg & UART_MSR_DSR) == UART_MSR_DSR) {
    *Control |= EFI_SERIAL_DATA_SET_READY;
  }

  if ((MsrReg & UART_MSR_RI) == UART_MSR_RI) {
    *Control |= EFI_SERIAL_RING_INDICATE;
  }

  if ((MsrReg & UART_MSR_DCD) == UART_MSR_DCD) {
    *Control |= EFI_SERIAL_CARRIER_DETECT;
  }

  if ((McrReg & UART_MCR_RTS) == UART_MCR_RTS) {
    *Control |= EFI_SERIAL_REQUEST_TO_SEND;
  }

  if ((McrReg & UART_MCR_DTR) == UART_MCR_DTR) {
    *Control |= EFI_SERIAL_DATA_TERMINAL_READY;
  }

  if ((LsrReg & UART_LSR_DR) == 0) {
    *Control |= EFI_SERIAL_INPUT_BUFFER_EMPTY;
  }

  if ((LsrReg & UART_LSR_THRE) == UART_LSR_THRE) {
    *Control |= EFI_SERIAL_OUTPUT_BUFFER_EMPTY;
  }

  if ((McrReg & UART_MCR_LOOP) == UART_MCR_LOOP) {
    *Control |= EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE;
  }

  return RETURN_SUCCESS;
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
DwUartWrite (
  IN  UINTN   UartBase,
  IN  UINT8  *Buffer,
  IN  UINTN   NumberOfBytes
  )
{
  UINTN  Count = 0;

  while (Count < NumberOfBytes) {
    // Wait until UART is able to accept another char
    while (!(READ_UART_REG(UART_LSR(0)) & UART_LSR_THRE));

    WRITE_UART_REG(UART_THR(0), Buffer[Count++]);
  }

  return Count;
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
DwUartRead (
  IN  UINTN   UartBase,
  OUT UINT8  *Buffer,
  IN  UINTN   NumberOfBytes
  )
{
  UINTN  Count = 0;

  while (Count < NumberOfBytes) {
    while (!(READ_UART_REG(UART_LSR(0)) & UART_LSR_DR));

    Buffer[Count++] = READ_UART_REG(UART_RBR(0));
  }

  return Count;
}

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
  )
{
  return READ_UART_REG(UART_LSR(0)) & UART_LSR_DR;
}
