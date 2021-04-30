/** @file
  Routines that access virtual PS/2 interface emulated by EC

Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include "Ps2Mux.h"

struct KeyCodeData {
  UINT8   ScanCode;
  UINT16  EfiScanCode;
  CHAR16  UnicodeChar;
  CHAR16  ShiftUnicodeChar;
};

#define KEYBOARD_USE_SET2

#if defined(KEYBOARD_USE_SET2)
#include "ScanCodeSet2.h"
#else
#include "ScanCodeSet1.h"
#endif

/**
  Get scancode from scancode buffer and translate into EFI-scancode and unicode defined by EFI spec.

  The function is always called in TPL_NOTIFY.

  @param Dev            PS2MUX_DEV instance pointer
  @param UpdateLights   pointer to a variable to store a flag which indicates
                        that an update of status lights is required

**/
STATIC
VOID
KeyGetchar (
  IN OUT PS2MUX_DEV              *Dev,
  OUT BOOLEAN                    *UpdateLights
  );

/**
  Return the count of scancode in the queue.

  @param Queue     Pointer to instance of SCAN_CODE_QUEUE.

  @return          Count of the scancode.
**/
UINTN
GetScancodeBufCount (
  IN SCAN_CODE_QUEUE       *Queue
  )
{
  if (Queue->Head <= Queue->Tail) {
    return Queue->Tail - Queue->Head;
  } else {
    return Queue->Tail + KEYBOARD_SCAN_CODE_MAX_COUNT - Queue->Head;
  }
}

/**
  Read several bytes from the scancode buffer without removing them.
  This function is called to see if there are enough bytes of scancode
  representing a single key.

  @param Queue     Pointer to instance of SCAN_CODE_QUEUE.
  @param Count     Number of bytes to be read
  @param Buf       Store the results

  @retval EFI_SUCCESS   success to scan the keyboard code
  @retval EFI_NOT_READY invalid parameter
**/
EFI_STATUS
GetScancodeBufHead (
  IN  SCAN_CODE_QUEUE        *Queue,
  IN  UINTN                  Count,
  OUT UINT8                  *Buf
  )
{
  UINTN                      Index;
  UINTN                      Pos;

  //
  // check the valid range of parameter 'Count'
  //
  if (GetScancodeBufCount (Queue) < Count) {
    return EFI_NOT_READY;
  }
  //
  // retrieve the values
  //
  for (Index = 0, Pos = Queue->Head; Index < Count; Index++, Pos = (Pos + 1) % KEYBOARD_SCAN_CODE_MAX_COUNT) {
    Buf[Index] = Queue->Buffer[Pos];
  }

  return EFI_SUCCESS;
}

/**

  Read & remove several bytes from the scancode buffer.
  This function is usually called after GetScancodeBufHead()

  @param Queue     Pointer to instance of SCAN_CODE_QUEUE.
  @param Count     Number of bytes to be read
  @param Buf       Store the results

  @retval EFI_SUCCESS success to scan the keyboard code
  @retval EFI_NOT_READY invalid parameter
**/
EFI_STATUS
PopScancodeBufHead (
  IN  SCAN_CODE_QUEUE       *Queue,
  IN  UINTN                 Count,
  OUT UINT8                 *Buf OPTIONAL
  )
{
  UINTN                     Index;

  //
  // Check the valid range of parameter 'Count'
  //
  if (GetScancodeBufCount (Queue) < Count) {
    return EFI_NOT_READY;
  }
  //
  // Retrieve and remove the values
  //
  for (Index = 0; Index < Count; Index++, Queue->Head = (Queue->Head + 1) % KEYBOARD_SCAN_CODE_MAX_COUNT) {
    if (Buf != NULL) {
      Buf[Index] = Queue->Buffer[Queue->Head];
    }
  }

  return EFI_SUCCESS;
}

/**
  Push one byte to the scancode buffer.

  @param Queue     Pointer to instance of SCAN_CODE_QUEUE.
  @param Scancode  The byte to push.
**/
VOID
PushScancodeBufTail (
  IN  SCAN_CODE_QUEUE       *Queue,
  IN  UINT8                 Scancode
  )
{
  if (GetScancodeBufCount (Queue) == KEYBOARD_SCAN_CODE_MAX_COUNT - 1) {
    PopScancodeBufHead (Queue, 1, NULL);
  }

  Queue->Buffer[Queue->Tail] = Scancode;
  Queue->Tail = (Queue->Tail + 1) % KEYBOARD_SCAN_CODE_MAX_COUNT;
}

/**
  Poll keyboard for data.

  @param Dev       Pointer to instance of PS2MUX_DEV
**/
VOID
KeyboardGetPacket (
  IN OUT PS2MUX_DEV              *Dev
  )
{
  UINT8                   Data;
  EFI_TPL                 OldTpl;
  BOOLEAN                 UpdateLights;

  //
  // To let KB driver support Hot plug, here should skip the 'resend' command  for the case that
  // KB is not connected to system. If KB is not connected to system, driver will find there's something
  // error in the following code and wait for the input buffer empty, this waiting time should be short enough since
  // this is a NOTIFY TPL period function, or the system performance will degrade hardly when KB is not connected.
  // Just skip the 'resend' process simply.
  //

  //
  // Read each of bytes stored in the input queue and store it into the scan code buffer
  //
  while (KeyReadData (Dev, &Data)) {
    PushScancodeBufTail (&Dev->ScancodeQueue, Data);
  }

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  UpdateLights = FALSE;
  KeyGetchar (Dev, &UpdateLights);

  //
  // Leave critical section
  //
  gBS->RestoreTPL (OldTpl);

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);
  if (UpdateLights) {
    UpdateStatusLights (Dev);
  }
  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);
}

/**
  Display error message.

  @param Dev       Pointer to instance of PS2MUX_DEV
  @param ErrMsg    Unicode string of error message

**/
VOID
KeyboardError (
  IN PS2MUX_DEV              *Dev,
  IN CHAR16                  *ErrMsg
  )
{
  Dev->KeyboardErr = TRUE;
  DEBUG ((EFI_D_ERROR, "Ps2Mux: %s", ErrMsg));
}

/**
  Read key value .

  @param Dev       - Pointer to instance of PS2MUX_DEV
  @param Data      - Pointer to outof buffer for keeping key value

  @retval EFI_TIMEOUT Status resigter time out
  @retval EFI_SUCCESS Success to read keyboard

**/
EFI_STATUS
KeyboardRead (
  IN PS2MUX_DEV               *Dev,
  OUT UINT8                   *Data
  )

{
  BOOLEAN HasData;
  UINT32  TimeOut;

  TimeOut   = 0;

  //
  // wait till output buffer full then perform the read
  //
  for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += PS2MUX_RX_DELAY) {
    HasData = KeyReadData (Dev, Data);
    if (HasData) {
      break;
    }

    MicroSecondDelay (PS2MUX_RX_DELAY);
  }

  if (!HasData) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
KeyboardInputPurge (
  IN PS2MUX_DEV               *Dev
  )
{
  BOOLEAN HasData;
  UINT8   Data;
  UINT32  TimeOut;
  TimeOut = 0;
  while ((HasData = KeyReadData (Dev, &Data)) &&
         (TimeOut < KEYBOARD_TIMEOUT)) {
    MicroSecondDelay (30);
    TimeOut += 30;
  }
  if (HasData) {
    return EFI_TIMEOUT;
  }
  return EFI_SUCCESS;
}

/**
  write key to keyboard

  @param Dev       Pointer to instance of PS2MUX_DEV
  @param Data      value wanted to be written

  @retval EFI_TIMEOUT   The input buffer register is full for putting new value util timeout
  @retval EFI_SUCCESS   The new value is sucess put into input buffer register.

**/
EFI_STATUS
KeyboardWrite (
  IN PS2MUX_DEV              *Dev,
  IN UINT8                   Data
  )
{
  //
  // wait for input buffer empty
  //
  if (KeyboardInputPurge (Dev) != EFI_SUCCESS) {
    return EFI_TIMEOUT;
  }

  //
  // Write it
  //
  KeyWriteData (Dev, Data);

  MicroSecondDelay (PS2MUX_TX_DELAY);

  return EFI_SUCCESS;
}

/**
  wait for a specific value to be presented in
  input data queue and then read it,
  used in keyboard commands ack

  @param Dev       Pointer to instance of PS2MUX_DEV
  @param Value     the value wanted to be waited.

  @retval EFI_TIMEOUT Fail to get specific value in given time
  @retval EFI_SUCCESS Success to get specific value in given time.

**/
EFI_STATUS
KeyboardWaitForValue (
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
    for (TimeOut = 0; TimeOut < KEYBOARD_TIMEOUT; TimeOut += PS2MUX_RX_DELAY) {
      if (KeyReadData (Dev, &Data)) {
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
  Disable Keyboard Scan Code Send.

  @param Dev       Pointer to instance of PS2MUX_DEV

  @return status of command execution

**/
EFI_STATUS
KeyboardScanDisable (
  IN PS2MUX_DEV              *Dev
  )
{
  EFI_STATUS  Status;

  Status = KeyboardWrite (Dev, KEYBOARD_8048_COMMAND_SCAN_DISABLE);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = KeyboardWaitForValue (Dev, KEYBOARD_8048_RETURN_8042_ACK,
                                 KEYBOARD_WAITFORVALUE_TIMEOUT);

  return Status;
}

/**
  Enable Keyboard Scan Code Send.

  @param Dev       Pointer to instance of PS2MUX_DEV

  @return status of command execution

**/
EFI_STATUS
KeyboardScanEnable (
  IN PS2MUX_DEV              *Dev
  )
{
  EFI_STATUS  Status;

  Status = KeyboardWrite (Dev, KEYBOARD_8048_COMMAND_SCAN_ENABLE);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = KeyboardWaitForValue (Dev, KEYBOARD_8048_RETURN_8042_ACK,
                                 KEYBOARD_WAITFORVALUE_TIMEOUT);

  return Status;
}

/**
  Check if the keyboard is connected to a PS/2 port.

  @param Dev       Pointer to instance of PS2MUX_DEV

  @return EFI_SUCCESS if the device is connected and it's a keyboard

**/
EFI_STATUS
KeyboardDetect (
  IN PS2MUX_DEV              *Dev
  )
{
  EFI_STATUS  Status;
  UINT8       Data[2];

  //
  // Send keyboard command
  //
  Status = KeyboardWrite (Dev, KEYBOARD_8048_COMMAND_ID);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  KeyboardWaitForValue (Dev, KEYBOARD_8048_RETURN_8042_ACK,
                        KEYBOARD_WAITFORVALUE_TIMEOUT);

  Status = KeyboardRead (Dev, Data);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = KeyboardRead (Dev, Data + 1);
  if (!EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ps2Mux: ID %02X%02X\r\n", Data[0], Data[1]));
    if ((Data[0] == 0xAB) &&
        ((Data[1] == 0x41) || (Data[1] == 0xC1) || (Data[1] == 0x83))) {
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_NOT_FOUND;
    }
  } else {
    DEBUG ((EFI_D_INFO, "Ps2Mux: ID %02X\r\n", Data[0]));
    Status = EFI_NOT_FOUND;
  }

  return Status;
}

/**
  Show keyboard status lights according to
  indicators in Dev.

  @param Dev Pointer to instance of PS2MUX_DEV

  @return status of updating keyboard register

**/
EFI_STATUS
UpdateStatusLights (
  IN PS2MUX_DEV              *Dev
  )
{
  EFI_STATUS  Status;
  UINT8       Command;

  //
  // Send keyboard command
  //
  Status = KeyboardWrite (Dev, KEYBOARD_8048_COMMAND_LED_SETUP);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  KeyboardWaitForValue (Dev, KEYBOARD_8048_RETURN_8042_ACK,
                        KEYBOARD_WAITFORVALUE_TIMEOUT);

  //
  // Light configuration
  //
  Command = 0;
  if (Dev->CapsLock) {
    Command |= 4;
  }

  if (Dev->NumLock) {
    Command |= 2;
  }

  if (Dev->ScrollLock) {
    Command |= 1;
  }

  Status = KeyboardWrite (Dev, Command);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  KeyboardWaitForValue (Dev, KEYBOARD_8048_RETURN_8042_ACK,
                        KEYBOARD_WAITFORVALUE_TIMEOUT);
  return Status;
}

/**
  Get scancode from scancode buffer and translate into EFI-scancode and unicode defined by EFI spec.

  The function is always called in TPL_NOTIFY.

  @param Dev            PS2MUX_DEV instance pointer
  @param UpdateLights   pointer to a variable to store a flag which indicates
                        that an update of status lights is required

**/
STATIC
VOID
KeyGetchar (
  IN OUT PS2MUX_DEV              *Dev,
  OUT BOOLEAN                    *UpdateLights
  )
{
  EFI_STATUS                     Status;
  UINT16                         ScanCode;
  BOOLEAN                        Extend0;
  BOOLEAN                        Extend1;
#if defined(KEYBOARD_USE_SET2)
  BOOLEAN                        Break;
#endif
  UINTN                          Index;
  EFI_KEY_DATA                   KeyData;
  LIST_ENTRY                     *Link;
  KEYBOARD_CONSOLE_IN_EX_NOTIFY  *CurrentNotify;
  //
  // 3 bytes most
  //
  UINT8                          ScancodeArr[3];
  UINT32                         ScancodeArrPos;

  //
  // Check if there are enough bytes of scancode representing a single key
  // available in the buffer
  //
  while (TRUE) {
    Extend0        = FALSE;
    Extend1        = FALSE;
#if defined(KEYBOARD_USE_SET2)
    Break          = FALSE;
#endif
    ScancodeArrPos = 0;
    Status  = GetScancodeBufHead (&Dev->ScancodeQueue, ScancodeArrPos + 1, ScancodeArr);
    if (EFI_ERROR (Status)) {
      return ;
    }

    if (ScancodeArr[ScancodeArrPos] == SCANCODE_EXTENDED0) {
      //
      // E0 to look ahead 2 bytes
      //
      Extend0 = TRUE;
      ScancodeArrPos = 1;
      Status         = GetScancodeBufHead (&Dev->ScancodeQueue, ScancodeArrPos + 1, ScancodeArr);
      if (EFI_ERROR (Status)) {
        return ;
      }
    } else if (ScancodeArr[ScancodeArrPos] == SCANCODE_EXTENDED1) {
      //
      // E1 to look ahead 3 bytes
      //
      Extend1 = TRUE;
      ScancodeArrPos = 2;
      Status         = GetScancodeBufHead (&Dev->ScancodeQueue, ScancodeArrPos + 1, ScancodeArr);
      if (EFI_ERROR (Status)) {
        return ;
      }
    }

#if defined(KEYBOARD_USE_SET2)
    if (ScancodeArr[ScancodeArrPos] == SCANCODE_BREAK) {
      Break = TRUE;
      ScancodeArrPos++;
      Status         = GetScancodeBufHead (&Dev->ScancodeQueue, ScancodeArrPos + 1, ScancodeArr);
      if (EFI_ERROR (Status)) {
        return ;
      }
    }
#endif

    //
    // if we reach this position, scancodes for a key is in buffer now, pop them
    //
    Status = PopScancodeBufHead (&Dev->ScancodeQueue, ScancodeArrPos + 1, ScancodeArr);
    ASSERT_EFI_ERROR (Status);

    //
    // store the last available byte, this byte of scancode will be checked
    //
    ScanCode = ScancodeArr[ScancodeArrPos];

    if (!Extend1) {
      //
      // Check for special keys and update the driver state.
      //
#if defined(KEYBOARD_USE_SET2)
      switch (ScanCode) {

      case SCANCODE_CTRL_MAKE:
        if (Extend0) {
          Dev->RightCtrl = !Break;
        } else {
          Dev->LeftCtrl  = !Break;
        }
        break;

      case SCANCODE_ALT_MAKE:
          if (Extend0) {
            Dev->RightAlt = !Break;
          } else {
            Dev->LeftAlt  = !Break;
          }
        break;

      case SCANCODE_LEFT_SHIFT_MAKE:
        //
        // To avoid recognize PRNT_SCRN key as a L_SHIFT key
        // because PRNT_SCRN key generates E0 followed by L_SHIFT scan code.
        // If it the second byte of the PRNT_SCRN skip it.
        //
        if (!Extend0) {
          Dev->LeftShift  = !Break;
          break;
        }
        continue;

      case SCANCODE_RIGHT_SHIFT_MAKE:
        Dev->RightShift = !Break;
        break;

      case SCANCODE_LEFT_LOGO_MAKE:
        if (Extend0) {
          Dev->LeftLogo = !Break;
        }
        break;

      case SCANCODE_RIGHT_LOGO_MAKE:
        if (Extend0) {
          Dev->RightLogo = !Break;
        }
        break;

      case SCANCODE_MENU_MAKE:
        if (Extend0) {
          Dev->Menu = !Break;
        }
        break;

      case SCANCODE_SYS_REQ_MAKE:
        if (Extend0) {
          Dev->SysReq = !Break;
        }
        break;

      case SCANCODE_SYS_REQ_MAKE_WITH_ALT:
        Dev->SysReq = !Break;
        break;

      case SCANCODE_CAPS_LOCK_MAKE:
        if (!Break) {
          Dev->CapsLock = !Dev->CapsLock;
          *UpdateLights = TRUE;
        }
        break;
      case SCANCODE_NUM_LOCK_MAKE:
        if (!Break) {
          Dev->NumLock = !Dev->NumLock;
          *UpdateLights = TRUE;
        }
        break;
      case SCANCODE_SCROLL_LOCK_MAKE:
        if (!Break) {
          Dev->ScrollLock = !Dev->ScrollLock;
          *UpdateLights = TRUE;
        }
        break;
      }
#else
      switch (ScanCode) {

      case SCANCODE_CTRL_MAKE:
        if (Extend0) {
          Dev->RightCtrl = TRUE;
        } else {
          Dev->LeftCtrl  = TRUE;
        }
        break;
      case SCANCODE_CTRL_BREAK:
        if (Extend0) {
          Dev->RightCtrl = FALSE;
        } else {
          Dev->LeftCtrl  = FALSE;
        }
        break;

      case SCANCODE_ALT_MAKE:
          if (Extend0) {
            Dev->RightAlt = TRUE;
          } else {
            Dev->LeftAlt  = TRUE;
          }
        break;
      case SCANCODE_ALT_BREAK:
          if (Extend0) {
            Dev->RightAlt = FALSE;
          } else {
            Dev->LeftAlt  = FALSE;
          }
        break;

      case SCANCODE_LEFT_SHIFT_MAKE:
        //
        // To avoid recognize PRNT_SCRN key as a L_SHIFT key
        // because PRNT_SCRN key generates E0 followed by L_SHIFT scan code.
        // If it the second byte of the PRNT_ScRN skip it.
        //
        if (!Extend0) {
          Dev->LeftShift  = TRUE;
          break;
        }
        continue;

      case SCANCODE_LEFT_SHIFT_BREAK:
        if (!Extend0) {
          Dev->LeftShift = FALSE;
        }
        break;

      case SCANCODE_RIGHT_SHIFT_MAKE:
        Dev->RightShift = TRUE;
        break;
      case SCANCODE_RIGHT_SHIFT_BREAK:
        Dev->RightShift = FALSE;
        break;

      case SCANCODE_LEFT_LOGO_MAKE:
        Dev->LeftLogo = TRUE;
        break;
      case SCANCODE_LEFT_LOGO_BREAK:
        Dev->LeftLogo = FALSE;
        break;

      case SCANCODE_RIGHT_LOGO_MAKE:
        Dev->RightLogo = TRUE;
        break;
      case SCANCODE_RIGHT_LOGO_BREAK:
        Dev->RightLogo = FALSE;
        break;

      case SCANCODE_MENU_MAKE:
        Dev->Menu = TRUE;
        break;
      case SCANCODE_MENU_BREAK:
        Dev->Menu = FALSE;
        break;

      case SCANCODE_SYS_REQ_MAKE:
        if (Extend0) {
          Dev->SysReq = TRUE;
        }
        break;
      case SCANCODE_SYS_REQ_BREAK:
        if (Extend0) {
          Dev->SysReq = FALSE;
        }
        break;

      case SCANCODE_SYS_REQ_MAKE_WITH_ALT:
        Dev->SysReq = TRUE;
        break;
      case SCANCODE_SYS_REQ_BREAK_WITH_ALT:
        Dev->SysReq = FALSE;
        break;

      case SCANCODE_CAPS_LOCK_MAKE:
        Dev->CapsLock = !Dev->CapsLock;
        *UpdateLights = TRUE;
        break;
      case SCANCODE_NUM_LOCK_MAKE:
        Dev->NumLock = !Dev->NumLock;
        *UpdateLights = TRUE;
        break;
      case SCANCODE_SCROLL_LOCK_MAKE:
        if (!Extend0) {
          Dev->ScrollLock = !Dev->ScrollLock;
          *UpdateLights = TRUE;
        }
        break;
      }
#endif
    }

    //
    // If this is key break, ignore it
    //
#if defined(KEYBOARD_USE_SET2)
    if (Break) {
#else
    if (ScanCode >= SCANCODE_MAX_MAKE) {
#endif
      continue;
    } else {
      break;
    }
  }

  //
  // Handle Ctrl+Alt+Del hotkey
  //
  if ((Dev->LeftCtrl || Dev->RightCtrl) &&
      (Dev->LeftAlt  || Dev->RightAlt ) &&
      ScanCode == SCANCODE_DELETE_MAKE
     ) {
    DEBUG ((EFI_D_INFO, "Ps2Mux: resetting system\r\n"));
    gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);
  }

  //
  // Save the Shift/Toggle state
  //
  KeyData.KeyState.KeyShiftState = (UINT32) (EFI_SHIFT_STATE_VALID
                                 | (Dev->LeftCtrl   ? EFI_LEFT_CONTROL_PRESSED  : 0)
                                 | (Dev->RightCtrl  ? EFI_RIGHT_CONTROL_PRESSED : 0)
                                 | (Dev->LeftAlt    ? EFI_LEFT_ALT_PRESSED      : 0)
                                 | (Dev->RightAlt   ? EFI_RIGHT_ALT_PRESSED     : 0)
                                 | (Dev->LeftShift  ? EFI_LEFT_SHIFT_PRESSED    : 0)
                                 | (Dev->RightShift ? EFI_RIGHT_SHIFT_PRESSED   : 0)
                                 | (Dev->LeftLogo   ? EFI_LEFT_LOGO_PRESSED     : 0)
                                 | (Dev->RightLogo  ? EFI_RIGHT_LOGO_PRESSED    : 0)
                                 | (Dev->Menu       ? EFI_MENU_KEY_PRESSED      : 0)
                                 | (Dev->SysReq     ? EFI_SYS_REQ_PRESSED       : 0)
                                 );
  KeyData.KeyState.KeyToggleState = (EFI_KEY_TOGGLE_STATE) (EFI_TOGGLE_STATE_VALID
                                  | (Dev->CapsLock   ? EFI_CAPS_LOCK_ACTIVE :   0)
                                  | (Dev->NumLock    ? EFI_NUM_LOCK_ACTIVE :    0)
                                  | (Dev->ScrollLock ? EFI_SCROLL_LOCK_ACTIVE : 0)
                                  | (Dev->IsSupportPartialKey ? EFI_KEY_STATE_EXPOSED : 0)
                                  );

  KeyData.Key.ScanCode            = SCAN_NULL;
  KeyData.Key.UnicodeChar         = CHAR_NULL;

  //
  // Key Pad "/" shares the same scancode as that of "/" except Key Pad "/" has E0 prefix
  //
  if (Extend0 && ScanCode == SCANCODE_SLASH_MAKE) {
    KeyData.Key.UnicodeChar = L'/';
    KeyData.Key.ScanCode    = SCAN_NULL;

  //
  // PAUSE shares the same scancode as that of NUM except PAUSE has E1 prefix
  //
  } else if (Extend1 && ScanCode == SCANCODE_NUM_LOCK_MAKE) {
    KeyData.Key.UnicodeChar = CHAR_NULL;
    KeyData.Key.ScanCode    = SCAN_PAUSE;

  //
  // PAUSE shares the same scancode as that of SCROLL except PAUSE (CTRL pressed) has E0 prefix
  //
  } else if (Extend0 && ScanCode == SCANCODE_SCROLL_LOCK_MAKE) {
    KeyData.Key.UnicodeChar = CHAR_NULL;
    KeyData.Key.ScanCode    = SCAN_PAUSE;

  //
  // PRNT_SCRN shares the same scancode as that of Key Pad "*" except PRNT_SCRN has E0 prefix
  //
  } else if (Extend0 && ScanCode == SCANCODE_SYS_REQ_MAKE) {
    KeyData.Key.UnicodeChar = CHAR_NULL;
    KeyData.Key.ScanCode    = SCAN_NULL;

  //
  // Except the above special case, all others can be handled by convert table
  //
  } else {
    for (Index = 0; ConvertKeyboardScanCodeToEfiKey[Index].ScanCode != TABLE_END; Index++) {
      if (ScanCode == ConvertKeyboardScanCodeToEfiKey[Index].ScanCode) {
        KeyData.Key.ScanCode    = ConvertKeyboardScanCodeToEfiKey[Index].EfiScanCode;
        KeyData.Key.UnicodeChar = ConvertKeyboardScanCodeToEfiKey[Index].UnicodeChar;

        if ((Dev->LeftShift || Dev->RightShift) &&
            (ConvertKeyboardScanCodeToEfiKey[Index].UnicodeChar != ConvertKeyboardScanCodeToEfiKey[Index].ShiftUnicodeChar)) {
          KeyData.Key.UnicodeChar = ConvertKeyboardScanCodeToEfiKey[Index].ShiftUnicodeChar;
          //
          // Need not return associated shift state if a class of printable characters that
          // are normally adjusted by shift modifiers. e.g. Shift Key + 'f' key = 'F'
          //
          KeyData.KeyState.KeyShiftState &= ~(EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED);
        }
        //
        // alphabetic key is affected by CapsLock State
        //
        if (Dev->CapsLock) {
          if (KeyData.Key.UnicodeChar >= L'a' && KeyData.Key.UnicodeChar <= L'z') {
            KeyData.Key.UnicodeChar = (UINT16) (KeyData.Key.UnicodeChar - L'a' + L'A');
          } else if (KeyData.Key.UnicodeChar >= L'A' && KeyData.Key.UnicodeChar <= L'Z') {
            KeyData.Key.UnicodeChar = (UINT16) (KeyData.Key.UnicodeChar - L'A' + L'a');
          }
        }
        break;
      }
    }
  }

  //
  // process numeric key pad lock state and
  // reset character for keys from edit block
  //
#if defined(KEYBOARD_USE_SET2)
  if ((ScanCode >= 0x69 && ScanCode <= 0x75) ||
      (ScanCode == 0x7A) || (ScanCode == 0x7D)) {
    if (!Extend0 &&
        ((Dev->NumLock && !(Dev->LeftShift || Dev->RightShift)) ||
         (!Dev->NumLock && (Dev->LeftShift || Dev->RightShift)))) {
      KeyData.Key.ScanCode = SCAN_NULL;
    } else {
      KeyData.Key.UnicodeChar = CHAR_NULL;
    }
  }
#else
  if ((ScanCode >= 0x47) && (ScanCode <= 0x53)) {
    if (!Extend0 &&
        ((Dev->NumLock && !(Dev->LeftShift || Dev->RightShift)) ||
         (!Dev->NumLock && (Dev->LeftShift || Dev->RightShift)))) {
      KeyData.Key.ScanCode = SCAN_NULL;
    } else if (ScanCode != 0x4A && ScanCode != 0x4E) {
      KeyData.Key.UnicodeChar = CHAR_NULL;
    }
  }
#endif

  //
  // If the key can not be converted then just return.
  //
  if (KeyData.Key.ScanCode == SCAN_NULL && KeyData.Key.UnicodeChar == CHAR_NULL) {
    if (!Dev->IsSupportPartialKey) {
      return ;
    }
  }

  //
  // Signal KeyNotify process event if this key pressed matches any key registered.
  //
  for (Link = GetFirstNode (&Dev->NotifyList); !IsNull (&Dev->NotifyList, Link); Link = GetNextNode (&Dev->NotifyList, Link)) {
    CurrentNotify = CR (
                      Link,
                      KEYBOARD_CONSOLE_IN_EX_NOTIFY,
                      NotifyEntry,
                      KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                      );
    if (IsKeyRegistered (&CurrentNotify->KeyData, &KeyData)) {
      //
      // The key notification function needs to run at TPL_CALLBACK
      // while current TPL is TPL_NOTIFY. It will be invoked in
      // KeyNotifyProcessHandler() which runs at TPL_CALLBACK.
      //
      PushEfikeyBufTail (&Dev->EfiKeyQueueForNotify, &KeyData);
      gBS->SignalEvent (Dev->KeyNotifyProcessEvent);
    }
  }

  PushEfikeyBufTail (&Dev->EfiKeyQueue, &KeyData);
}

/**
  Perform keyboard Initialization.
  If ExtendedVerification is TRUE, do additional test for
  the keyboard interface

  @param Dev - PS2MUX_DEV instance pointer
  @param ExtendedVerification - indicates a thorough initialization

  @retval EFI_DEVICE_ERROR Fail to init keyboard
  @retval EFI_SUCCESS      Success to init keyboard
**/
EFI_STATUS
InitKeyboard (
  IN OUT PS2MUX_DEV              *Dev,
  IN BOOLEAN                     ExtendedVerification
  )
{
  EFI_STATUS              Status;
  EFI_TPL                 OldTpl;
  UINT8                   CommandByte;
  EFI_PS2_POLICY_PROTOCOL *Ps2Policy;
  UINT32                  TryTime;

  Status                 = EFI_SUCCESS;
  TryTime                = 0;

  //
  // Get PS/2 policy to set this
  //
  gBS->LocateProtocol (
        &gEfiPs2PolicyProtocolGuid,
        NULL,
        (VOID **) &Ps2Policy
        );

  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_CLEAR_BUFFER,
    Dev->DevicePath
    );

  //
  // Perform a read to cleanup the Status Register's
  // output buffer full bits within MAX TRY times
  //
  while (!EFI_ERROR (Status) && TryTime < KEYBOARD_MAX_TRY) {
    Status = KeyboardRead (Dev, &CommandByte);
    TryTime ++;
  }
  //
  // Exceed the max try times. The device may be error.
  //
  if (TryTime == KEYBOARD_MAX_TRY) {
    Status = EFI_DEVICE_ERROR;
    goto Done;
  }

  if (Ps2Policy != NULL) {
    Ps2Policy->Ps2InitHardware (Dev->Handle);
  }

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  Dev->KeyboardInputData.Head = 0;
  Dev->KeyboardInputData.Tail = 0;
  Dev->MouseInputData.Head = 0;
  Dev->MouseInputData.Tail = 0;
  //
  // Clear Memory Scancode Buffer
  //
  Dev->ScancodeQueue.Head = 0;
  Dev->ScancodeQueue.Tail = 0;
  Dev->EfiKeyQueue.Head   = 0;
  Dev->EfiKeyQueue.Tail   = 0;
  Dev->EfiKeyQueueForNotify.Head = 0;
  Dev->EfiKeyQueueForNotify.Tail = 0;

  //
  // Reset the status indicators
  //
  Dev->CapsLock   = FALSE;
  Dev->NumLock    = FALSE;
  Dev->ScrollLock = FALSE;
  Dev->LeftCtrl   = FALSE;
  Dev->RightCtrl  = FALSE;
  Dev->LeftAlt    = FALSE;
  Dev->RightAlt   = FALSE;
  Dev->LeftShift  = FALSE;
  Dev->RightShift = FALSE;
  Dev->LeftLogo   = FALSE;
  Dev->RightLogo  = FALSE;
  Dev->Menu       = FALSE;
  Dev->SysReq     = FALSE;

  Dev->IsSupportPartialKey = FALSE;

  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);

  //
  // For resetting keyboard is not mandatory before booting OS and sometimes keyboard responses very slow,
  // and to support KB hot plug, we need to let the InitKB succeed no matter whether there is a KB device connected
  // to system. So we only do the real resetting for keyboard when user asks and there is a real KB connected t system,
  // and normally during booting an OS, it's skipped.
  //
  if (ExtendedVerification && CheckKeyboardConnect (Dev)) {
    //
    // Additional verifications for keyboard interface
    //
    //
    // Keyboard reset with a BAT(Basic Assurance Test)
    //
    Status = KeyboardWrite (Dev, KEYBOARD_8048_COMMAND_RESET);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Keyboard data write error!\n\r");
      goto Done;
    }

    Status = KeyboardWaitForValue (Dev, KEYBOARD_8048_RETURN_8042_ACK,
                                   KEYBOARD_WAITFORVALUE_TIMEOUT);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Some specific value not aquired from keyboard!\n\r");
      goto Done;
    }
    //
    // wait for BAT completion code
    //
    Status = KeyboardWaitForValue (Dev,
                                   KEYBOARD_8048_RETURN_8042_BAT_SUCCESS,
                                   KEYBOARD_BAT_TIMEOUT);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Keyboard self test failed!\n\r");
      goto Done;
    }

    //
    // Disable Keyboard Scancode Send
    //
    Status = KeyboardWrite (Dev, KEYBOARD_8048_COMMAND_SCAN_DISABLE);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Keyboard data write error!\n\r");
      goto Done;
    }

    Status = KeyboardWaitForValue (Dev, KEYBOARD_8048_RETURN_8042_ACK,
                                   KEYBOARD_WAITFORVALUE_TIMEOUT);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Some specific value not aquired from keyboard!\n\r");
      goto Done;
    }

    //
    // Set Keyboard to use specific Scan Code Set
    //
    Status = KeyboardWrite (Dev, KEYBOARD_8048_COMMAND_SELECT_SCAN_CODE_SET);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Keyboard data write error!\n\r");
      goto Done;
    }

    Status = KeyboardWaitForValue (Dev, KEYBOARD_8048_RETURN_8042_ACK,
                                   KEYBOARD_WAITFORVALUE_TIMEOUT);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Some specific value not aquired from keyboard!\n\r");
      goto Done;
    }

    Status = KeyboardWrite (Dev, SCANCODE_SET_ID);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Keyboard data write error!!\n\r");
      goto Done;
    }

    Status = KeyboardWaitForValue (Dev, KEYBOARD_8048_RETURN_8042_ACK,
                                   KEYBOARD_WAITFORVALUE_TIMEOUT);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Some specific value not aquired from keyboard!\n\r");
      goto Done;
    }

    //
    // Enable Keyboard Scancode Send
    //
    Status = KeyboardWrite (Dev, KEYBOARD_8048_COMMAND_SCAN_ENABLE);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Keyboard data write error!\n\r");
      goto Done;
    }

    Status = KeyboardWaitForValue (Dev, KEYBOARD_8048_RETURN_8042_ACK,
                                   KEYBOARD_WAITFORVALUE_TIMEOUT);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Some specific value not aquired from keyboard!\n\r");
      goto Done;
    }

    //
    // Enter critical section
    //
    OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

    //
    if (Ps2Policy != NULL) {
      if ((Ps2Policy->KeyboardLight & EFI_KEYBOARD_CAPSLOCK) == EFI_KEYBOARD_CAPSLOCK) {
        Dev->CapsLock = TRUE;
      }

      if ((Ps2Policy->KeyboardLight & EFI_KEYBOARD_NUMLOCK) == EFI_KEYBOARD_NUMLOCK) {
        Dev->NumLock = TRUE;
      }

      if ((Ps2Policy->KeyboardLight & EFI_KEYBOARD_SCROLLLOCK) == EFI_KEYBOARD_SCROLLLOCK) {
        Dev->ScrollLock = TRUE;
      }
    }

    //
    // Leave critical section and return
    //
    gBS->RestoreTPL (OldTpl);

    //
    // Update Keyboard Lights
    //
    Status = UpdateStatusLights (Dev);
    if (EFI_ERROR (Status)) {
      KeyboardError (Dev, L"Update keyboard status lights error!\n\r");
      goto Done;
    }
  }
  //
  // At last, we can now enable the mouse interface if appropriate
  //
Done:

  if (!EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ps2Mux: keyboard reset complete\r\n"));
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }

}

/**
  Check whether there is PS/2 Keyboard device in system by Echo Keyboard Command
  If Keyboard receives Echo command (byte 0xEE), it will respond with the same byte.
  If it doesn't respond, the device should not be in system.

  @param[in]  Dev                   Keyboard Private Data Structure

  @retval     TRUE                  Keyboard in System.
  @retval     FALSE                 Keyboard not in System.
**/
BOOLEAN
EFIAPI
CheckKeyboardConnect (
  IN PS2MUX_DEV              *Dev
  )
{
  EFI_STATUS     Status;

  //
  // enable keyboard itself and wait for its ack
  // If can't receive ack, Keyboard should not be connected.
  //
  if (!PcdGetBool (PcdFastPS2Detection)) {
    Status = KeyboardWrite (
               Dev,
               KEYBOARD_8048_COMMAND_ECHO
               );

    if (EFI_ERROR (Status)) {
      return FALSE;
    }
    //
    // wait for 1s
    //
    Status = KeyboardWaitForValue (
               Dev,
               KEYBOARD_8048_COMMAND_ECHO,
               KEYBOARD_WAITFORVALUE_TIMEOUT);

    if (EFI_ERROR (Status)) {
      return FALSE;
    }

    return TRUE;
  } else {
    return TRUE;
  }
}
