/** @file
  PS/2 Mouse Communication Interface

Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PS2MOUSE_H_
#define _PS2MOUSE_H_

#define PS2_PACKET_LENGTH       3
#define PS2_SYNC_MASK           0xc
#define PS2_SYNC_BYTE           0x8

#define IS_PS2_SYNC_BYTE(byte)  ((byte & PS2_SYNC_MASK) == PS2_SYNC_BYTE)

#define PS2_READ_BYTE_ONE       0
#define PS2_READ_DATA_BYTE      1
#define PS2_PROCESS_PACKET      2

//
// PS/2 Device Command
//
#define SETSF1_CMD  0xe6
#define SETSF2_CMD  0xe7
#define SETRE_CMD   0xe8
#define READ_CMD    0xeb
#define SETRM_CMD   0xf0
#define SETSR_CMD   0xf3
#define ENABLE_CMD  0xf4
#define DISABLE_CMD 0xf5
#define RESET_CMD   0xff

//
// return code
//
#define PS2_ACK       0xfa
#define PS2_RESEND    0xfe
#define PS2MOUSE_BAT1 0xaa
#define PS2MOUSE_BAT2 0x00

/**
  Get mouse packet. Only care first 3 bytes

  @param Dev       Pointer to the PS/2 Multiplexer Private Data Structure

  @retval EFI_NOT_READY  Mouse Device not ready to input data packet, or some error happened during getting the packet
  @retval EFI_SUCCESS    The data packet is gotten successfully.

**/
EFI_STATUS
Ps2MouseGetPacket (
  PS2MUX_DEV        *Dev
  );

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
  );

/**
  Check whether there is PS/2 mouse device in system

  @param Dev        - PS/2 Multiplexer Private Data Structure

  @retval TRUE      - Keyboard in System.
  @retval FALSE     - Keyboard not in System.

**/
BOOLEAN
CheckMouseConnect (
  IN  PS2MUX_DEV        *Dev
  );

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
  );

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
  );

#endif

