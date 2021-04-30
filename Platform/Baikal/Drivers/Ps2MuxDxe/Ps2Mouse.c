/** @file
  PS/2 Mouse Communication Interface.

Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Ps2Mux.h"
#include "Ps2Mouse.h"

UINT8 SampleRateTbl[MaxSampleRate]  = { 0xa, 0x14, 0x28, 0x3c, 0x50, 0x64, 0xc8 };

UINT8 ResolutionTbl[MaxResolution]  = { 0, 1, 2, 3 };

/**
  I/O work flow to wait input buffer empty in given time.

  @param Dev       Pointer to instance of PS2MUX_DEV
  @param Timeout   Wating time.

  @retval EFI_TIMEOUT if input is still not empty in given time.
  @retval EFI_SUCCESS input is empty.
**/
STATIC
EFI_STATUS
MouseWaitInputEmpty (
  IN PS2MUX_DEV                           *Dev,
  IN UINTN                                Timeout
  )
{
  UINTN Delay;
  UINT8 Data;

  Delay = Timeout / 50;

  do {

    //
    // Check if there's any data left to process
    //
    if (!MouseReadData (Dev, &Data)) {
      break;
    }

    gBS->Stall (50);
    Delay--;
  } while (Delay != 0);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  wait for a specific value to be presented in
  input data queue and then read it,
  used in mouse commands ack

  @param ConsoleIn Pointer to instance of PS2MUX_DEV
  @param Value     the value wanted to be waited.

  @retval EFI_TIMEOUT Fail to get specific value in given time
  @retval EFI_SUCCESS Success to get specific value in given time.

**/
STATIC
EFI_STATUS
MouseWaitForValue (
  IN PS2MUX_DEV              *Dev,
  IN UINT8                   Value,
  UINT32                     WaitTimeOut
  )
{
  UINT8   Data;
  UINT32  TimeOut;
  UINT32  SumTimeOut;
  UINT32  GotIt;

  GotIt       = 0;
  TimeOut     = 0;
  SumTimeOut  = 0;

  //
  // Make sure the initial value of 'Data' is different from 'Value'
  //
  Data = 0;
  if (Data == Value) {
    Data = 1;
  }
  //
  // Read from input data queue (multiple times if needed)
  // until the expected value appears
  // use SumTimeOut to control the iteration
  //
  while (1) {
    //
    // Perform a read
    //
    for (TimeOut = 0; TimeOut < MOUSE_TIMEOUT; TimeOut += PS2MUX_RX_DELAY) {
      if (MouseReadData (Dev, &Data)) {
        break;
      }

      MicroSecondDelay (PS2MUX_RX_DELAY);
    }

    SumTimeOut += TimeOut;

    if (Data == Value) {
      GotIt = 1;
      break;
    }

    if (SumTimeOut >= WaitTimeOut) {
      break;
    }
  }
  //
  // Check results
  //
  if (GotIt == 1) {
    return EFI_SUCCESS;
  } else {
    return EFI_TIMEOUT;
  }

}

/**
  I/O work flow of transmitting mouse command.

  @param Dev       Pointer to instance of PS2MUX_DEV
  @param Command   Mouse I/O command

  @retval EFI_SUCCESS Success to execute I/O work flow
  @retval EFI_TIMEOUT operation timed out
**/
STATIC
EFI_STATUS
MouseWriteCommand (
  IN PS2MUX_DEV                           *Dev,
  IN UINT8                                Command
  )
{
  EFI_STATUS  Status;

  //
  // Wait input data buffer empty
  //
  Status = MouseWaitInputEmpty (Dev, MOUSE_TIMEOUT);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Send auxiliary device command
  //
  MouseWriteData (Dev, Command);
  MicroSecondDelay (PS2MUX_TX_DELAY);

  //
  // Wait for device acknowledge
  //
  Status = MouseWaitForValue (Dev, PS2_ACK, MOUSE_TIMEOUT);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Issue command to reset mouse.

  @param Dev       Pointer to instance of PS2MUX_DEV

  @return Status of command issuing.
**/
STATIC
EFI_STATUS
Ps2MouseReset (
  IN PS2MUX_DEV                           *Dev
  )
{
  EFI_STATUS  Status;

  Status = MouseWriteCommand (Dev, RESET_CMD);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = MouseWaitForValue (Dev, PS2MOUSE_BAT1, MOUSE_BAT_TIMEOUT);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }
  Status = MouseWaitForValue (Dev, PS2MOUSE_BAT2, MOUSE_BAT_TIMEOUT);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Issue command to set mouse's sample rate

  @param Dev        Pointer to instance of PS2MUX_DEV
  @param SampleRate Value of sample rate

  @return Status of command issuing.
**/
STATIC
EFI_STATUS
Ps2MouseSetSampleRate (
  IN PS2MUX_DEV                           *Dev,
  IN MOUSE_SR                             SampleRate
  )
{
  EFI_STATUS  Status;

  //
  // Send command to set mouse sample rate
  //
  Status = MouseWriteCommand (Dev, SETSR_CMD);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MouseWriteData (Dev, SampleRateTbl[SampleRate]);
  MicroSecondDelay (PS2MUX_TX_DELAY);

  return Status;
}

/**
  Issue command to set mouse's resolution.

  @param Dev        Pointer to instance of PS2MUX_DEV
  @param Resolution Value of resolution

  @return Status of command issuing.
**/
STATIC
EFI_STATUS
Ps2MouseSetResolution (
  IN PS2MUX_DEV                           *Dev,
  IN MOUSE_RE                             Resolution
  )
{
  EFI_STATUS  Status;

  //
  // Send command to set mouse resolution
  //
  Status = MouseWriteCommand (Dev, SETRE_CMD);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MouseWriteData (Dev, ResolutionTbl[Resolution]);
  MicroSecondDelay (PS2MUX_TX_DELAY);

  return Status;
}

/**
  Issue command to set mouse's scaling.

  @param Dev       Pointer to instance of PS2MUX_DEV
  @param Scaling   Value of scaling

  @return Status of command issuing.
**/
STATIC
EFI_STATUS
Ps2MouseSetScaling (
  IN PS2MUX_DEV                           *Dev,
  IN MOUSE_SF                             Scaling
  )
{
  //
  // Send command to set mouse scaling data
  //
  return MouseWriteCommand (Dev, Scaling == Scaling1 ?
                                 SETSF1_CMD : SETSF2_CMD);
}

/**
  Issue command to enable Ps2 mouse.

  @param Dev       Pointer to instance of PS2MUX_DEV

  @return Status of command issuing.
**/
STATIC
EFI_STATUS
Ps2MouseEnable (
  IN PS2MUX_DEV                           *Dev
  )
{
  //
  // Send command to enable mouse
  //
  return MouseWriteCommand (Dev, ENABLE_CMD);
}

/**
  Read data via I2cIo protocol with given number.

  @param Dev       Pointer to instance of PS2MUX_DEV
  @param Buffer    Buffer receive data of mouse
  @param BufSize   The size of buffer
  @param State     Check input or read data

  @return status of reading mouse data.
**/
STATIC
EFI_STATUS
Ps2MouseRead (
  IN PS2MUX_DEV                           *Dev,
  OUT UINT8                               *Buffer,
  IN OUT UINTN                            *BufSize,
  IN  UINTN                               State
  )
{
  BOOLEAN     Result;
  EFI_STATUS  Status;
  UINTN       BytesRead;

  Result    = TRUE;
  Status    = EFI_SUCCESS;
  BytesRead = 0;

  if (State == PS2_READ_BYTE_ONE) {
    Result = MouseReadData (Dev, Buffer);
    if (Result) {
      BytesRead = 1;
      MicroSecondDelay (PS2MUX_RX_DELAY);
    }
  } else {
    for (BytesRead = 0; (BytesRead < *BufSize) && Result; BytesRead++) {
      Result = MouseReadData (Dev, Buffer + BytesRead);
      MicroSecondDelay (PS2MUX_RX_DELAY);
    }
  }

  if (!Result) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Verify the correct number of bytes read
  //
  if (BytesRead == 0 || BytesRead != *BufSize) {
    Status = EFI_NOT_FOUND;
  } else {
    Status = EFI_SUCCESS;
  }

  *BufSize = BytesRead;
  return Status;
}

/**
  Get mouse packet. Only care first 3 bytes

  @param Dev       Pointer to the PS/2 Multiplexer Private Data Structure

  @retval EFI_NOT_READY  Mouse Device not ready to input data packet, or some error happened during getting the packet
  @retval EFI_SUCCESS    The data packet is gotten successfully.

**/
EFI_STATUS
Ps2MouseGetPacket (
  PS2MUX_DEV        *Dev
  )

{
  EFI_STATUS  Status;
  UINT8       Packet[PS2_PACKET_LENGTH];
  UINT8       Data;
  UINTN       Count;
  UINTN       State;
  INT16       RelativeMovementX;
  INT16       RelativeMovementY;
  BOOLEAN     LButton;
  BOOLEAN     RButton;

  State           = PS2_READ_BYTE_ONE;

  //
  // State machine to get mouse packet
  //
  while (1) {

    switch (State) {
    case PS2_READ_BYTE_ONE:
      //
      // Read mouse first byte data, if failed, immediately return
      //
      Count  = 1;
      Status = Ps2MouseRead (Dev, &Data, &Count, State);
      if (EFI_ERROR (Status)) {
        return EFI_NOT_READY;
      }

      if (Count != 1) {
        return EFI_NOT_READY;
      }

      if (IS_PS2_SYNC_BYTE (Data)) {
        Packet[0] = Data;
        State     = PS2_READ_DATA_BYTE;
      }
      break;

    case PS2_READ_DATA_BYTE:
      Count   = 2;
      Status  = Ps2MouseRead (Dev, (Packet + 1), &Count, State);
      if (EFI_ERROR (Status)) {
        return EFI_NOT_READY;
      }

      if (Count != 2) {
        return EFI_NOT_READY;
      }

      State = PS2_PROCESS_PACKET;
      break;

    case PS2_PROCESS_PACKET:
      //
      // Decode the packet
      //
      RelativeMovementX = Packet[1];
      RelativeMovementY = Packet[2];
      //
      //               Bit 7   |    Bit 6   |    Bit 5   |   Bit 4    |   Bit 3  |   Bit 2    |   Bit 1   |   Bit 0
      //  Byte 0  | Y overflow | X overflow | Y sign bit | X sign bit | Always 1 | Middle Btn | Right Btn | Left Btn
      //  Byte 1  |                                           8 bit X Movement
      //  Byte 2  |                                           8 bit Y Movement
      //
      // X sign bit + 8 bit X Movement : 9-bit signed twos complement integer that presents the relative displacement of the device in the X direction since the last data transmission.
      // Y sign bit + 8 bit Y Movement : Same as X sign bit + 8 bit X Movement.
      //
      //
      // First, Clear X and Y high 8 bits
      //
      RelativeMovementX = (INT16) (RelativeMovementX & 0xFF);
      RelativeMovementY = (INT16) (RelativeMovementY & 0xFF);
      //
      // Second, if the 9-bit signed twos complement integer is negative, set the high 8 bit 0xff
      //
      if ((Packet[0] & 0x10) != 0) {
        RelativeMovementX = (INT16) (RelativeMovementX | 0xFF00);
      }
      if ((Packet[0] & 0x20) != 0) {
        RelativeMovementY = (INT16) (RelativeMovementY | 0xFF00);
      }


      RButton           = (UINT8) (Packet[0] & 0x2);
      LButton           = (UINT8) (Packet[0] & 0x1);

      //
      // Update mouse state
      //
      Dev->MouseState.RelativeMovementX += RelativeMovementX;
      Dev->MouseState.RelativeMovementY -= RelativeMovementY;
      Dev->MouseState.RightButton = (UINT8) (RButton ? TRUE : FALSE);
      Dev->MouseState.LeftButton  = (UINT8) (LButton ? TRUE : FALSE);
      Dev->MouseStateChanged      = TRUE;

      return EFI_SUCCESS;
    }
  }
}

/**
  Reset the Mouse and do BAT test for it, if ExtendedVerification is TRUE and
  there is a mouse device connected to system.

  @param This                 - Pointer of simple pointer Protocol.
  @param ExtendedVerification - Whether configure mouse parameters. True: do; FALSE: skip.


  @retval EFI_SUCCESS         - The command byte is written successfully.
  @retval EFI_DEVICE_ERROR    - Errors occurred during resetting keyboard.

**/
EFI_STATUS
EFIAPI
MouseReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN BOOLEAN                        ExtendedVerification
  )
{
  EFI_STATUS    Status;
  PS2MUX_DEV    *Dev;
  EFI_TPL       OldTpl;
  UINT8         Data;

  Dev = PS2MUX_DEV_FROM_SIMPLE_POINTER (This);

  //
  // Report reset progress code
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_MOUSE | EFI_P_PC_RESET,
    Dev->DevicePath
    );

  //
  // Raise TPL
  //
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  ZeroMem (&Dev->MouseState, sizeof (EFI_SIMPLE_POINTER_STATE));
  Dev->MouseStateChanged = FALSE;

  //
  // Restore TPL
  //
  gBS->RestoreTPL (OldTpl);

  //
  // Exhaust input data
  //
  while (MouseReadData (Dev, &Data)) {
    MicroSecondDelay (PS2MUX_RX_DELAY);
  }

  Status = EFI_SUCCESS;
  //
  // The PS2 mouse driver reset behavior is always successfully return no matter wheater or not there is mouse connected to system.
  // This behavior is needed by performance speed. The following mouse command only succeessfully finish when mouse device is
  // connected to system, so if PS2 mouse device not connect to system or user not ask for, we skip the mouse configuration and enabling
  //
  if (ExtendedVerification && CheckMouseConnect (Dev)) {
    //
    // Send mouse reset command and set mouse default configure
    //
    Status = Ps2MouseReset (Dev);
    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = Ps2MouseSetSampleRate (Dev, Dev->MouseSampleRate);
    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = Ps2MouseSetResolution (Dev, Dev->MouseResolution);
    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = Ps2MouseSetScaling (Dev, Dev->MouseScaling);
    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Status = Ps2MouseEnable (Dev);
    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }
  }
Exit:

  return Status;
}

/**
  Check whether there is PS/2 mouse device in system

  @param Dev        - PS/2 Multiplexer Private Data Structure

  @retval TRUE      - Keyboard in System.
  @retval FALSE     - Keyboard not in System.

**/
BOOLEAN
CheckMouseConnect (
  IN  PS2MUX_DEV        *Dev
  )

{
  EFI_STATUS     Status;

  Status = Ps2MouseEnable (Dev);
  if (!EFI_ERROR (Status)) {
    return TRUE;
  }

  return FALSE;
}

/**
  Get and Clear mouse status.

  @param This                 - Pointer of simple pointer Protocol.
  @param State                - Output buffer holding status.

  @retval EFI_INVALID_PARAMETER Output buffer is invalid.
  @retval EFI_NOT_READY         Mouse is not changed status yet.
  @retval EFI_SUCCESS           Mouse status is changed and get successful.
**/
EFI_STATUS
EFIAPI
MouseGetState (
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN OUT EFI_SIMPLE_POINTER_STATE   *State
  )
{
  PS2MUX_DEV    *Dev;
  EFI_TPL       OldTpl;

  Dev = PS2MUX_DEV_FROM_SIMPLE_POINTER (This);

  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!Dev->MouseStateChanged) {
    return EFI_NOT_READY;
  }

  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);
  CopyMem (State, &(Dev->MouseState), sizeof (EFI_SIMPLE_POINTER_STATE));

  //
  // clear mouse state
  //
  Dev->MouseState.RelativeMovementX = 0;
  Dev->MouseState.RelativeMovementY = 0;
  Dev->MouseState.RelativeMovementZ = 0;
  Dev->MouseStateChanged            = FALSE;
  gBS->RestoreTPL (OldTpl);

  return EFI_SUCCESS;
}

/**

  Event notification function for SIMPLE_POINTER.WaitForInput event.
  Signal the event if there is input from mouse.

  @param Event    event object
  @param Context  event context

**/
VOID
EFIAPI
MouseWaitForInput (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  )
{
  PS2MUX_DEV    *Dev;

  Dev = (PS2MUX_DEV *) Context;

  //
  // Someone is waiting on the mouse event, if there's
  // input from mouse, signal the event
  //
  if (Dev->MouseStateChanged) {
    gBS->SignalEvent (Event);
  }

}
