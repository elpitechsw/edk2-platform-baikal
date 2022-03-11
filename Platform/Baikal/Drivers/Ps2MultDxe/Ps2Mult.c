/** @file

  PS/2 Multiplexer driver. Routines that interacts with callers,
  conforming to EFI driver model

Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DwUartLib.h>
#include <Protocol/NonDiscoverableDevice.h>
#include "Ps2Mult.h"

STATIC CONST EFI_GUID Ps2MultKbdGuid = {
          0x22f1771e, 0x0910, 0x44bf, \
          { 0xF2, 0xA1, 0x1B, 0x66, 0x99, 0xF9, 0x31, 0x5E } \
};

#define DP_NODE_LEN(Type) { (UINT8)sizeof (Type), (UINT8)(sizeof (Type) >> 8) }

#pragma pack (1)
typedef struct {
  VENDOR_DEVICE_PATH          Vendor;
  UINT64                      BaseAddress;
  UINT8                       Unused;
} NON_DISCOVERABLE_PS2_MULT;

typedef struct {
#if 0 //vvv
  UART_DEVICE_PATH            Uart;
#elif 0 //vvv se
  VENDOR_DEVICE_PATH          SerialDxe;
  UART_DEVICE_PATH            Uart;
#endif
  NON_DISCOVERABLE_PS2_MULT   Dev;
  EFI_DEVICE_PATH_PROTOCOL    End;
} PLATFORM_PS2MULT_KBD;
#pragma pack ()

#define PS2MULT_DEV_GUID { \
          0x22f1771e, 0x0910, 0x44bf, \
          { 0xF2, 0xA1, 0x1B, 0x66, 0x99, 0xF9, 0x31, 0x5E } \
          }

STATIC PLATFORM_PS2MULT_KBD mPs2MultKbd = {
#if 0 //vvv
  {
    { MESSAGING_DEVICE_PATH, MSG_UART_DP, DP_NODE_LEN (UART_DEVICE_PATH) },
    0,
    FixedPcdGet64 (PcdUartDefaultBaudRate),
    FixedPcdGet8 (PcdUartDefaultDataBits),
    FixedPcdGet8 (PcdUartDefaultParity),
    FixedPcdGet8 (PcdUartDefaultStopBits)
  },
#elif 0 //vvv se
  {
    { HARDWARE_DEVICE_PATH, HW_VENDOR_DP, DP_NODE_LEN (VENDOR_DEVICE_PATH) },
    EDKII_SERIAL_PORT_LIB_VENDOR_GUID
  },
  {
    { MESSAGING_DEVICE_PATH, MSG_UART_DP, DP_NODE_LEN (UART_DEVICE_PATH) },
    0,
    FixedPcdGet64 (PcdUartDefaultBaudRate),
    FixedPcdGet8 (PcdUartDefaultDataBits),
    FixedPcdGet8 (PcdUartDefaultParity),
    FixedPcdGet8 (PcdUartDefaultStopBits)
  },
#endif
  {
    {
      { HARDWARE_DEVICE_PATH, HW_VENDOR_DP, DP_NODE_LEN (NON_DISCOVERABLE_PS2_MULT) },
      PS2MULT_DEV_GUID
//vvv      EDKII_NON_DISCOVERABLE_DEVICE_PROTOCOL_GUID
    },
    FixedPcdGet64(PcdPs2MultUartBaseAddr),
    0
  },
  {
    END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE,
    DP_NODE_LEN (EFI_DEVICE_PATH_PROTOCOL)
  }
};

//
// Function prototypes
//
/**
  Test controller is a PS/2 Multiplexer Controller.

  @param This                 Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller           driver's controller
  @param RemainingDevicePath  children device path

  @retval EFI_UNSUPPORTED controller is not a PS/2 Multiplexer
  @retval EFI_SUCCESS     controller is a PS/2 Multiplexer
**/
EFI_STATUS
EFIAPI
Ps2MultDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

/**
  Create PS2MULT_DEV instance on controller.

  @param This         Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller   driver controller handle
  @param RemainingDevicePath Children's device path

  @retval whether success to create PS/2 Multiplexer driver instance.
**/
EFI_STATUS
EFIAPI
Ps2MultDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

/**
  Stop this driver on ControllerHandle. Support stopping any child handles
  created by this driver.

  @param  This              Protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
Ps2MultDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// DriverBinding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gPs2MultDriver = {
  Ps2MultDriverSupported,
  Ps2MultDriverStart,
  Ps2MultDriverStop,
  0xa,
  NULL,
  NULL
};

/**
  Free the waiting key notify list.

  @param ListHead  Pointer to list head

  @retval EFI_INVALID_PARAMETER  ListHead is NULL
  @retval EFI_SUCCESS            Sucess to free NotifyList
**/
STATIC
EFI_STATUS
KbdFreeNotifyList (
  IN OUT LIST_ENTRY           *ListHead
  )
{
  KEYBOARD_CONSOLE_IN_EX_NOTIFY *NotifyNode;

  if (ListHead == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  while (!IsListEmpty (ListHead)) {
    NotifyNode = CR (
                   ListHead->ForwardLink,
                   KEYBOARD_CONSOLE_IN_EX_NOTIFY,
                   NotifyEntry,
                   KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE
                   );
    RemoveEntryList (ListHead->ForwardLink);
    gBS->FreePool (NotifyNode);
  }

  return EFI_SUCCESS;
}

/**
  Initialise keyboard portion of the driver.

  @param Dev   - the device instance

  @return status of operation

**/
STATIC
EFI_STATUS
Ps2MultKeyboardInit (
  IN OUT PS2MULT_DEV                        *Dev
  )
{
  EFI_STATUS                                Status;
  EFI_STATUS                                ReadStatus;
  UINT8                                     Data;
  EFI_STATUS_CODE_VALUE                     StatusCode;

  StatusCode = 0;
  //
  // Report that the keyboard is being enabled
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_ENABLE,
    Dev->DevicePath
    );

  (Dev->ConIn).Reset               = KeyboardEfiReset;
  (Dev->ConIn).ReadKeyStroke       = KeyboardReadKeyStroke;
  Dev->ConInEx.Reset               = KeyboardEfiResetEx;
  Dev->ConInEx.ReadKeyStrokeEx     = KeyboardReadKeyStrokeEx;
  Dev->ConInEx.SetState            = KeyboardSetState;
  Dev->ConInEx.RegisterKeyNotify   = KeyboardRegisterKeyNotify;
  Dev->ConInEx.UnregisterKeyNotify = KeyboardUnregisterKeyNotify;

  InitializeListHead (&Dev->NotifyList);

  if (!CheckKeyboardConnect (Dev)) {
    DEBUG ((EFI_D_INFO, "Ps2Mult: no keyboard detected\n"));
    Status      = EFI_DEVICE_ERROR;
    StatusCode  = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_NOT_DETECTED;
    goto KeyboardErrorExit;
  }

  //
  // Setup the WaitForKey event
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_CALLBACK,
                  KeyboardWaitForKey,
                  Dev,
                  &((Dev->ConIn).WaitForKey)
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
    StatusCode  = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_CONTROLLER_ERROR;
    goto KeyboardErrorExit;
  }
  //
  // Setup the WaitForKeyEx event
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_CALLBACK,
                  KeyboardWaitForKeyEx,
                  Dev,
                  &(Dev->ConInEx.WaitForKeyEx)
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
    StatusCode  = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_CONTROLLER_ERROR;
    goto KeyboardErrorExit;
  }

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyNotifyProcessHandler,
                  Dev,
                  &Dev->KeyNotifyProcessEvent
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
    StatusCode  = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_CONTROLLER_ERROR;
    goto KeyboardErrorExit;
  }

  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_PRESENCE_DETECT,
    Dev->DevicePath
    );

  //
  // Reset the keyboard device
  //
  Status = Dev->ConInEx.Reset (&Dev->ConInEx,
                               FeaturePcdGet (PcdPs2KbdExtendedVerification));
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Ps2Mult: keyboard reset failed\n"));
    Status      = EFI_DEVICE_ERROR;
    StatusCode  = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_NOT_DETECTED;
    goto KeyboardErrorExit;
  }

  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_DETECTED,
    Dev->DevicePath
    );

  return Status;

KeyboardErrorExit:
  //
  // Report error code
  //
  if (StatusCode != 0) {
    REPORT_STATUS_CODE_WITH_DEVICE_PATH (
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      StatusCode,
      Dev->DevicePath
      );
  }

  if ((Dev != NULL) && (Dev->ConIn.WaitForKey != NULL)) {
    gBS->CloseEvent (Dev->ConIn.WaitForKey);
  }
  if ((Dev != NULL) && (Dev->ConInEx.WaitForKeyEx != NULL)) {
    gBS->CloseEvent (Dev->ConInEx.WaitForKeyEx);
  }
  if ((Dev != NULL) && (Dev->KeyNotifyProcessEvent != NULL)) {
    gBS->CloseEvent (Dev->KeyNotifyProcessEvent);
  }
  KbdFreeNotifyList (&Dev->NotifyList);
  //
  // Since there will be no timer handler for keyboard input any more,
  // exhaust input data just in case there is still keyboard data left
  //
  if ((Dev != NULL) && (Status != EFI_DEVICE_ERROR)) {
    ReadStatus = EFI_SUCCESS;
    while (!EFI_ERROR (ReadStatus)) {
      ReadStatus = KeyboardRead (Dev, &Data);
    }
  }

  return Status;
}

/**
  De-initialise keyboard portion of the driver.

  @param Dev   - the device instance

  @return status of operation

**/
STATIC
EFI_STATUS
Ps2MultKeyboardDeinit (
  IN OUT PS2MULT_DEV          *Dev,
  IN EFI_HANDLE               Controller
)
{
  EFI_STATUS                  Status;
  UINT8                       Data;

  //
  // Report that the keyboard is being disabled
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_DISABLE,
    Dev->DevicePath
    );

  //
  // Since there will be no timer handler for keyboard input any more,
  // exhaust input data just in case there is still keyboard data left
  //
    Status = EFI_SUCCESS;
    while (!EFI_ERROR (Status)) {
      Status = KeyboardRead (Dev, &Data);
    }
  //
  // Uninstall the SimpleTextIn and SimpleTextInEx protocols
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  &Dev->ConIn,
                  &gEfiSimpleTextInputExProtocolGuid,
                  &Dev->ConInEx,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Free other resources
  //
  if ((Dev->ConIn).WaitForKey != NULL) {
    gBS->CloseEvent ((Dev->ConIn).WaitForKey);
    (Dev->ConIn).WaitForKey = NULL;
  }
  if (Dev->ConInEx.WaitForKeyEx != NULL) {
    gBS->CloseEvent (Dev->ConInEx.WaitForKeyEx);
    Dev->ConInEx.WaitForKeyEx = NULL;
  }
  if (Dev->KeyNotifyProcessEvent != NULL) {
    gBS->CloseEvent (Dev->KeyNotifyProcessEvent);
    Dev->KeyNotifyProcessEvent = NULL;
  }
  KbdFreeNotifyList (&Dev->NotifyList);

  return Status;
}

/**
  Test if controller is a PS/2 Multiplexer Controller.

  @param This                 Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller           driver's controller
  @param RemainingDevicePath  children device path

  @retval EFI_UNSUPPORTED controller is not a PS/2 Mutiplexer controller
  @retval EFI_SUCCESS     controller is a PS/2 Mutiplexer controller
**/
EFI_STATUS
EFIAPI
Ps2MultDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                        Status;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  EFI_DEV_PATH                      *TmpDevPath;

  //
  // Check whether the controller is I2C IO Controller.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CHAR16 *DevPath = ConvertDevicePathToText(DevicePath, FALSE, FALSE);
  DEBUG((EFI_D_VERBOSE, "Ps2Mult: Controller %p - checking %s\n", Controller, DevPath));
  FreePool(DevPath);
  do {
    TmpDevPath = (EFI_DEV_PATH *)DevicePath;
    DevicePath = NextDevicePathNode (DevicePath);
  } while (!IsDevicePathEnd (DevicePath));

  if (CompareGuid(&TmpDevPath->Vendor.Guid, &Ps2MultKbdGuid)) {
    DEBUG((EFI_D_VERBOSE, "Ps2Mult: DevicePath match\n"));
    return EFI_SUCCESS;
  }
  return EFI_UNSUPPORTED;
}

/**
  Create PS2MULT_DEV instance on controller.

  @param This         Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller   driver controller handle
  @param RemainingDevicePath Children's device path

  @retval whether success to create PS/2 Multiplexer driver instance.
**/
EFI_STATUS
EFIAPI
Ps2MultDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                                Status;
  PS2MULT_DEV                                *Dev;
  EFI_STATUS_CODE_VALUE                     StatusCode;
  EFI_DEVICE_PATH_PROTOCOL                  *DevicePath;
  UINT64                                    BaudRate = PS2MULT_BAUDRATE;
  UINT32                                    FifoDepth = 0;
  EFI_PARITY_TYPE                           Parity = DefaultParity;
  UINT8                                     DataBits = 8;
  EFI_STOP_BITS_TYPE                        StopBits = DefaultStopBits;

  StatusCode = 0;

  DEBUG((EFI_D_VERBOSE, "Ps2MultDriverStart (Controller %p)\n", Controller));
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Allocate private data
  //
  Dev = AllocateZeroPool (sizeof (PS2MULT_DEV));
  if (Dev == NULL) {
    Status      = EFI_OUT_OF_RESOURCES;
    StatusCode  = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_CONTROLLER_ERROR;
    goto ErrorExit;
  }
  //
  // Setup the device instance
  //
  Dev->Signature              = PS2MULT_DEV_SIGNATURE;
  Dev->Handle                 = Controller;
  Dev->DevicePath             = DevicePath;
  Dev->KeyboardEnabled        = FALSE;
  Dev->UartBase               = (UINTN)FixedPcdGet64(PcdPs2MultUartBaseAddr);

  Status = DwUartInitializePort (
             Dev->UartBase,
             &BaudRate,
             &FifoDepth,
             &Parity,
             &DataBits,
             &StopBits
             );

  Status = Ps2MultKeyboardInit (Dev);
  Dev->KeyboardEnabled = !EFI_ERROR (Status);

  if (Dev->KeyboardEnabled) {
    //
    // Setup a periodic timer, used for reading multiplexer data at a fixed interval
    //
    Status = gBS->CreateEvent (
                    EVT_TIMER | EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    Ps2MultTimerHandler,
                    Dev,
                    &Dev->TimerEvent
                    );
    if (EFI_ERROR (Status)) {
      Status      = EFI_OUT_OF_RESOURCES;
      StatusCode  = EFI_P_EC_CONTROLLER_ERROR;
      StatusCode |= EFI_PERIPHERAL_KEYBOARD;
      goto ErrorExit;
    }
    Ps2MultPollStart (Dev);
  } else {
    Status      = EFI_DEVICE_ERROR;
    goto ErrorExit;
  }

  Dev->ControllerNameTable = NULL;
  AddUnicodeString2 (
    "eng",
    gPs2MultComponentName.SupportedLanguages,
    &Dev->ControllerNameTable,
    L"PS/2 Multiplexer Device",
    TRUE
    );
  AddUnicodeString2 (
    "en",
    gPs2MultComponentName2.SupportedLanguages,
    &Dev->ControllerNameTable,
    L"PS/2 Multiplexer Device",
    FALSE
    );

  if (Dev->KeyboardEnabled) {
    //
    // Install protocol interfaces for the keyboard device.
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Controller,
                    &gEfiSimpleTextInProtocolGuid,
                    &Dev->ConIn,
                    &gEfiSimpleTextInputExProtocolGuid,
                    &Dev->ConInEx,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      StatusCode = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_CONTROLLER_ERROR;
      goto ErrorExit;
    }
  }

  DEBUG ((EFI_D_VERBOSE, "Ps2Mult: initialization complete\n"));

  return Status;

ErrorExit:
  //
  // Report error code
  //
  if (StatusCode != 0) {
    REPORT_STATUS_CODE_WITH_DEVICE_PATH (
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      StatusCode,
      DevicePath
      );
  }

  if ((Dev != NULL) && (Dev->TimerEvent != NULL)) {
    Ps2MultPollStop (Dev);
    gBS->CloseEvent (Dev->TimerEvent);
  }
  if ((Dev != NULL) && Dev->KeyboardEnabled) {
    Ps2MultKeyboardDeinit (Dev, Controller);
  }
  if ((Dev != NULL) && (Dev->ControllerNameTable != NULL)) {
    FreeUnicodeStringTable (Dev->ControllerNameTable);
  }
  if (Dev != NULL) {
    gBS->FreePool (Dev);
  }

  return Status;
}

/**
  Stop this driver on ControllerHandle. Support stopping any child handles
  created by this driver.

  @param  This              Protocol instance pointer.
  @param  Controller        Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
Ps2MultDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
{
  EFI_STATUS                     Status;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
  EFI_SIMPLE_POINTER_PROTOCOL    *SimplePointerProtocol;
  PS2MULT_DEV                     *Dev;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  (VOID **) &ConIn,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiSimplePointerProtocolGuid,
                    (VOID **) &SimplePointerProtocol,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Dev = PS2MULT_DEV_FROM_SIMPLE_POINTER (SimplePointerProtocol);
  } else {
    Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiSimpleTextInputExProtocolGuid,
                    NULL,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Dev = PS2MULT_DEV_FROM_SIMPLE_TEXT_INPUT (ConIn);
  }

  if (Dev->TimerEvent != NULL) {
    Ps2MultPollStop (Dev);
    gBS->CloseEvent (Dev->TimerEvent);
    Dev->TimerEvent = NULL;
  }
  //
  // Disable Keyboard
  //
  if (Dev->KeyboardEnabled) {
    Ps2MultKeyboardDeinit (Dev, Controller);
  }
  FreeUnicodeStringTable (Dev->ControllerNameTable);
  gBS->FreePool (Dev);

  return EFI_SUCCESS;
}

/**
  The module Entry Point for module Ps2Mult.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializePs2Mult(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  //
  // Install driver model protocol(s).
  //
  gPs2MultDriver.ImageHandle = ImageHandle;
  gPs2MultDriver.DriverBindingHandle = ImageHandle;
  Status = gBS->InstallMultipleProtocolInterfaces (
		&gPs2MultDriver.DriverBindingHandle,
		&gEfiDriverBindingProtocolGuid, &gPs2MultDriver,
		&gEfiComponentNameProtocolGuid, &gPs2MultComponentName,
		&gEfiComponentName2ProtocolGuid, &gPs2MultComponentName2,
		&gEfiDevicePathProtocolGuid, &mPs2MultKbd,
		NULL);

  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Unload function for module Ps2Mult.

  @param  ImageHandle[in]        The allocated handle for the EFI image

  @retval EFI_SUCCESS            The driver was unloaded successfully
  @retval EFI_INVALID_PARAMETER  ImageHandle is not a valid image handle.

**/
EFI_STATUS
EFIAPI
UnloadPs2Mult (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS          Status;

  //
  // Uninstall protocols installed by the driver in its entrypoint
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (ImageHandle,
                  &gEfiDriverBindingProtocolGuid,
                  &gPs2MultDriver,
                  NULL
                  );

  return EFI_SUCCESS;
}
