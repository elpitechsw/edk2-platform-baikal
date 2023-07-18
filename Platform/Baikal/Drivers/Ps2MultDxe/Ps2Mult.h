/** @file
  PS/2 keyboard driver header file

  Copyright (c) 2006 - 2017, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2022, Elpitech.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _PS2MULT_H_
#define _PS2MULT_H_

#include <Uefi.h>

#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/DevicePath.h>
#include <Protocol/Ps2Policy.h>

#include <Library/IoLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>
#include <Library/PcdLib.h>

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL   gPs2MultDriver;
extern EFI_COMPONENT_NAME_PROTOCOL   gPs2MultComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL  gPs2MultComponentName2;

//
// Driver Private Data
//
#define PS2MULT_DEV_SIGNATURE         SIGNATURE_32 ('p', 's', '2', 'm')

#define PS2MULT_INPUT_DATA_MAX_COUNT 8
typedef struct {
  UINT8                               Buffer[PS2MULT_INPUT_DATA_MAX_COUNT];
  UINTN                               Head;
  UINTN                               Tail;
} INPUT_DATA_QUEUE;

typedef struct _KEYBOARD_CONSOLE_IN_EX_NOTIFY {
  UINTN                               Signature;
  EFI_KEY_DATA                        KeyData;
  EFI_KEY_NOTIFY_FUNCTION             KeyNotificationFn;
  LIST_ENTRY                          NotifyEntry;
} KEYBOARD_CONSOLE_IN_EX_NOTIFY;

#define KEYBOARD_SCAN_CODE_MAX_COUNT  32
typedef struct {
  UINT8                               Buffer[KEYBOARD_SCAN_CODE_MAX_COUNT];
  UINTN                               Head;
  UINTN                               Tail;
} SCAN_CODE_QUEUE;

#define KEYBOARD_EFI_KEY_MAX_COUNT    256
typedef struct {
  EFI_KEY_DATA                        Buffer[KEYBOARD_EFI_KEY_MAX_COUNT];
  UINTN                               Head;
  UINTN                               Tail;
} EFI_KEY_QUEUE;

typedef struct {
  UINTN                               Signature;

  EFI_HANDLE                          Handle;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL      ConIn;
  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   ConInEx;
  EFI_SIMPLE_POINTER_PROTOCOL         SimplePointerProtocol;

  EFI_EVENT                           TimerEvent;

  UINTN                               UartBase;
  UINT8                               OutPort;
  UINT8                               InPort;
  BOOLEAN                             Escape;

  BOOLEAN                             LeftCtrl;
  BOOLEAN                             RightCtrl;
  BOOLEAN                             LeftAlt;
  BOOLEAN                             RightAlt;
  BOOLEAN                             LeftShift;
  BOOLEAN                             RightShift;
  BOOLEAN                             LeftLogo;
  BOOLEAN                             RightLogo;
  BOOLEAN                             Menu;
  BOOLEAN                             SysReq;

  BOOLEAN                             CapsLock;
  BOOLEAN                             NumLock;
  BOOLEAN                             ScrollLock;

  BOOLEAN                             IsSupportPartialKey;
  BOOLEAN                             KeyboardEnabled;

  UINT8                               DataPackageSize;

  INPUT_DATA_QUEUE                    KeyboardInputData;
  //
  // Queue storing key scancodes
  //
  SCAN_CODE_QUEUE                     ScancodeQueue;
  EFI_KEY_QUEUE                       EfiKeyQueue;
  EFI_KEY_QUEUE                       EfiKeyQueueForNotify;

  //
  // Error state
  //
  BOOLEAN                             KeyboardErr;

  EFI_UNICODE_STRING_TABLE            *ControllerNameTable;

  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
  //
  // Notification Function List
  //
  LIST_ENTRY                          NotifyList;
  EFI_EVENT                           KeyNotifyProcessEvent;
} PS2MULT_DEV;

#define PS2MULT_DEV_FROM_SIMPLE_TEXT_INPUT(a) \
  CR (a, PS2MULT_DEV, ConIn, PS2MULT_DEV_SIGNATURE)
#define PS2MULT_DEV_FROM_SIMPLE_TEXT_INPUT_EX(a) \
  CR (a, PS2MULT_DEV, ConInEx, PS2MULT_DEV_SIGNATURE)
#define PS2MULT_DEV_FROM_SIMPLE_POINTER(a) \
  CR (a, PS2MULT_DEV, SimplePointerProtocol, PS2MULT_DEV_SIGNATURE)

#define TABLE_END 0x0

#define PS2MULT_BAUDRATE  115200

//
// Driver entry point
//
/**
  The user Entry Point for module Ps2Mult. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializePs2Mult (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

#define KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE SIGNATURE_32 ('k', 'c', 'e', 'n')

#define PS2MULT_RX_DELAY                2000    // 0.002s
#define PS2MULT_TX_DELAY                2000    // 0.002s
#define PS2MULT_TIMER_INTERVAL          10000   // 0.01s
#define KEYBOARD_MAX_TRY                256     // 256
#define KEYBOARD_TIMEOUT                65536   // 0.07s
#define KEYBOARD_WAITFORVALUE_TIMEOUT   100000  // 0.1s
#define KEYBOARD_BAT_TIMEOUT            400000  // 0.4s

#define KEYBOARD_8048_COMMAND_LED_SETUP                     0xED
#define KEYBOARD_8048_COMMAND_ECHO                          0xEE
#define KEYBOARD_8048_COMMAND_SELECT_SCAN_CODE_SET          0xF0
#define KEYBOARD_8048_COMMAND_ID                            0xF2
#define KEYBOARD_8048_COMMAND_REPEAT_SETUP                  0xF3
#define KEYBOARD_8048_COMMAND_SCAN_ENABLE                   0xF4
#define KEYBOARD_8048_COMMAND_SCAN_DISABLE                  0xF5
#define KEYBOARD_8048_COMMAND_RESEND                        0xFE
#define KEYBOARD_8048_COMMAND_RESET                         0xFF

#define KEYBOARD_8048_RETURN_8042_BAT_SUCCESS               0xAA
#define KEYBOARD_8048_RETURN_8042_BAT_ERROR                 0xFC
#define KEYBOARD_8048_RETURN_8042_ACK                       0xFA

#define KEYBOARD_DATA_ADDRESS          0x00
#define MOUSE_DATA_ADDRESS             0x01

//
// Other functions that are used among .c files
//
/**
  Show keyboard status lights according to
  indicators in Dev.

  @param Dev       Pointer to instance of PS2MULT_DEV

  @return status

**/
EFI_STATUS
UpdateStatusLights (
  IN PS2MULT_DEV              *Dev
  );

/**
  write key to keyboard.

  @param Dev       Pointer to instance of PS2MULT_DEV
  @param Data      value wanted to be written

  @retval EFI_TIMEOUT - GC_TODO: Add description for return value
  @retval EFI_SUCCESS - GC_TODO: Add description for return value

**/
EFI_STATUS
KeyboardRead (
  IN PS2MULT_DEV              *Dev,
  OUT UINT8                   *Data
  );

/**
  Process key notify.

  @param  Event                 Indicates the event that invoke this function.
  @param  Context               Indicates the calling context.
**/
VOID
EFIAPI
KeyNotifyProcessHandler (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  );

/**
  Perform keyboard Initialization.
  If ExtendedVerification is TRUE, do additional test for
  the keyboard interface

  @param Dev       - PS2MULT_DEV instance pointer
  @param ExtendedVerification - indicates a thorough initialization

  @retval EFI_DEVICE_ERROR Fail to init keyboard
  @retval EFI_SUCCESS      Success to init keyboard
**/
EFI_STATUS
InitKeyboard (
  IN OUT PS2MULT_DEV             *Dev,
  IN BOOLEAN                     ExtendedVerification
  );

/**
  Read data from the keyboard input data queue.

  @param Dev       Pointer to instance of PS2MULT_DEV
  @param Data      Pointer to variable to store value in

  @return true if operation has succeeded, false otherwise

**/
BOOLEAN
KeyReadData (
  IN PS2MULT_DEV             *Dev,
  OUT UINT8                  *Data
  );

/**
  Write data to the keyboard.

  @param Dev       Pointer to instance of PS2MULT_DEV
  @param Data      Value to be written

**/
VOID
KeyWriteData (
  IN PS2MULT_DEV             *Dev,
  IN UINT8                   Data
  );

/**
  Start multiplexer session.

  @param Dev   - the device instance

  @return status of operation

**/
EFI_STATUS
Ps2MultStart (
  IN OUT PS2MULT_DEV *Dev
  );

/**
  Enable multiplexer input data polling.

  @param Dev   - the device instance

  @return status of operation

**/
EFI_STATUS
Ps2MultPollStart (
  IN OUT PS2MULT_DEV *Dev
  );

/**
  Disable multiplexer input data polling.

  @param Dev   - the device instance

  @return status of operation

**/
EFI_STATUS
Ps2MultPollStop (
  IN OUT PS2MULT_DEV *Dev
  );

/**
  Timer event handler: read a series of scancodes from
  input data queue and put them into memory scancode buffer.
  it read as much scancodes to either fill
  the memory buffer or empty the keyboard buffer.
  It is registered as running under TPL_NOTIFY

  @param Event - The timer event
  @param Context - A PS2MULT_DEV pointer

**/
VOID
EFIAPI
Ps2MultTimerHandler (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  );

/**
  Poll keyboard for data.

  @param Dev       Pointer to instance of PS2MULT_DEV
**/
VOID
KeyboardGetPacket (
  IN OUT PS2MULT_DEV              *Dev
  );

/**
  logic reset keyboard
  Implement SIMPLE_TEXT_IN.Reset()
  Perform keyboard initialization

  @param This    Pointer to instance of EFI_SIMPLE_TEXT_INPUT_PROTOCOL
  @param ExtendedVerification Indicate that the driver may perform a more
                              exhaustive verification operation of the device during
                              reset, now this par is ignored in this driver

**/
EFI_STATUS
EFIAPI
KeyboardEfiReset (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  IN  BOOLEAN                         ExtendedVerification
  );

/**
  Implement SIMPLE_TEXT_IN.ReadKeyStroke().
  Retrieve key values for driver user.

  @param This    Pointer to instance of EFI_SIMPLE_TEXT_INPUT_PROTOCOL
  @param Key     The output buffer for key value

  @retval EFI_SUCCESS success to read key stroke
**/
EFI_STATUS
EFIAPI
KeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                   *Key
  );

/**
  Event notification function for SIMPLE_TEXT_IN.WaitForKey event
  Signal the event if there is key available

  @param Event    the event object
  @param Context  waitting context

**/
VOID
EFIAPI
KeyboardWaitForKey (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

/**
  Disable Keyboard Scan Code Send.

  @param Dev       Pointer to instance of PS2MULT_DEV

  @return status of command execution

**/
EFI_STATUS
KeyboardScanDisable (
  IN PS2MULT_DEV              *Dev
  );

/**
  Enable Keyboard Scan Code Send.

  @param Dev       Pointer to instance of PS2MULT_DEV

  @return status of command execution

**/
EFI_STATUS
KeyboardScanEnable (
  IN PS2MULT_DEV              *Dev
  );

/**
  Check if the keyboard is connected to a PS/2 port.

  @param Dev       Pointer to instance of PS2MULT_DEV

  @return EFI_SUCCESS if the device is connected and it's a keyboard

**/
EFI_STATUS
KeyboardDetect (
  IN PS2MULT_DEV              *Dev
  );

/**
  Check whether there is Ps/2 Keyboard device in system by Echo Keyboard Command
  If Keyboard receives Echo command (byte 0xEE), it will respond with the same byte.
  If it doesn't respond, the device should not be in system.

  @param[in]  Dev         Pointer to instance of PS2MULT_DEV

  @retval     TRUE                  Keyboard in System.
  @retval     FALSE                 Keyboard not in System.
**/
BOOLEAN
EFIAPI
CheckKeyboardConnect (
  IN PS2MULT_DEV              *Dev
  );

/**
  Event notification function for SIMPLE_TEXT_INPUT_EX_PROTOCOL.WaitForKeyEx event
  Signal the event if there is key available

  @param Event    event object
  @param Context  waiting context

**/
VOID
EFIAPI
KeyboardWaitForKeyEx (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

//
// Simple Text Input Ex protocol function prototypes
//

/**
  Reset the input device and optionaly run diagnostics

  @param This                 - Protocol instance pointer.
  @param ExtendedVerification - Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS           - The device was reset.
  @retval EFI_DEVICE_ERROR      - The device is not functioning properly and could
                                  not be reset.

**/
EFI_STATUS
EFIAPI
KeyboardEfiResetEx (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN BOOLEAN                            ExtendedVerification
  );

/**
    Reads the next keystroke from the input device. The WaitForKey Event can
    be used to test for existance of a keystroke via WaitForEvent () call.


    @param This       - Protocol instance pointer.
    @param KeyData    - A pointer to a buffer that is filled in with the keystroke
                 state data for the key that was pressed.

    @retval EFI_SUCCESS           - The keystroke information was returned.
    @retval EFI_NOT_READY         - There was no keystroke data availiable.
    @retval EFI_DEVICE_ERROR      - The keystroke information was not returned due to
                            hardware errors.
    @retval EFI_INVALID_PARAMETER - KeyData is NULL.

**/
EFI_STATUS
EFIAPI
KeyboardReadKeyStrokeEx (
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  OUT EFI_KEY_DATA                      *KeyData
  );

/**
  Set certain state for the input device.

  @param This              - Protocol instance pointer.
  @param KeyToggleState    - A pointer to the EFI_KEY_TOGGLE_STATE to set the
                        state for the input device.

  @retval EFI_SUCCESS           - The device state was set successfully.
  @retval EFI_DEVICE_ERROR      - The device is not functioning correctly and could
                            not have the setting adjusted.
  @retval EFI_UNSUPPORTED       - The device does not have the ability to set its state.
  @retval EFI_INVALID_PARAMETER - KeyToggleState is NULL.

**/
EFI_STATUS
EFIAPI
KeyboardSetState (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_TOGGLE_STATE               *KeyToggleState
  );

/**
    Register a notification function for a particular keystroke for the input device.

    @param This                    - Protocol instance pointer.
    @param KeyData                 - A pointer to a buffer that is filled in with the keystroke
                                     information data for the key that was pressed. If KeyData.Key,
                                     KeyData.KeyState.KeyToggleState and KeyData.KeyState.KeyShiftState
                                     are 0, then any incomplete keystroke will trigger a notification of
                                     the KeyNotificationFunction.
    @param KeyNotificationFunction - Points to the function to be called when the key
                                     sequence is typed specified by KeyData. This notification function
                                     should be called at <=TPL_CALLBACK.
    @param NotifyHandle            - Points to the unique handle assigned to the registered notification.

    @retval EFI_SUCCESS             - The notification function was registered successfully.
    @retval EFI_OUT_OF_RESOURCES    - Unable to allocate resources for necesssary data structures.
    @retval EFI_INVALID_PARAMETER   - KeyData or NotifyHandle is NULL.

**/
EFI_STATUS
EFIAPI
KeyboardRegisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_DATA                       *KeyData,
  IN EFI_KEY_NOTIFY_FUNCTION            KeyNotificationFunction,
  OUT VOID                              **NotifyHandle
  );

/**
    Remove a registered notification function from a particular keystroke.

    @param This                    - Protocol instance pointer.
    @param NotificationHandle      - The handle of the notification function being unregistered.


    @retval EFI_SUCCESS             - The notification function was unregistered successfully.
    @retval EFI_INVALID_PARAMETER   - The NotificationHandle is invalid.
    @retval EFI_NOT_FOUND           - Can not find the matching entry in database.

**/
EFI_STATUS
EFIAPI
KeyboardUnregisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN VOID                               *NotificationHandle
  );

/**
  Push one key data to the EFI key buffer.

  @param Queue     Pointer to instance of EFI_KEY_QUEUE.
  @param KeyData   The key data to push.
**/
VOID
PushEfikeyBufTail (
  IN  EFI_KEY_QUEUE         *Queue,
  IN  EFI_KEY_DATA          *KeyData
  );

/**
  Judge whether is a registed key

  @param RegsiteredData       A pointer to a buffer that is filled in with the keystroke
                              state data for the key that was registered.
  @param InputData            A pointer to a buffer that is filled in with the keystroke
                              state data for the key that was pressed.

  @retval TRUE                Key be pressed matches a registered key.
  @retval FLASE               Match failed.

**/
BOOLEAN
IsKeyRegistered (
  IN EFI_KEY_DATA  *RegsiteredData,
  IN EFI_KEY_DATA  *InputData
  );

#endif
