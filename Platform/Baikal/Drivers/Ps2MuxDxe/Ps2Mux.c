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

#include "Ps2Mux.h"
#include "Ps2Mouse.h"

#pragma pack (1)
typedef struct {
  EFI_DEVICE_PATH_PROTOCOL Header;
  EFI_GUID                 Guid;
  UINTN                    BaseAddress;
} PLATFORM_I2C_CONTROLLER;
#pragma pack ()

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
Ps2MuxDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

/**
  Create PS2MUX_DEV instance on controller.

  @param This         Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller   driver controller handle
  @param RemainingDevicePath Children's device path

  @retval whether success to create PS/2 Multiplexer driver instance.
**/
EFI_STATUS
EFIAPI
Ps2MuxDriverStart (
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
Ps2MuxDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// DriverBinding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gPs2MuxDriver = {
  Ps2MuxDriverSupported,
  Ps2MuxDriverStart,
  Ps2MuxDriverStop,
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
Ps2MuxKeyboardInit (
  IN OUT PS2MUX_DEV                         *Dev
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
    DEBUG ((EFI_D_INFO, "Ps2Mux: no keyboard detected (I2C device index %u)\r\n",
            Dev->I2cDevice->DeviceIndex));
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
    DEBUG ((EFI_D_ERROR, "Ps2Mux: keyboard reset failed (I2C device index %u)\r\n",
            Dev->I2cDevice->DeviceIndex));
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
  Initialise mouse portion of the driver.

  @param Dev   - the device instance

  @return status of operation

**/
STATIC
EFI_STATUS
Ps2MuxMouseInit (
  IN OUT PS2MUX_DEV                         *Dev
  )
{
  EFI_STATUS                                Status;
  EFI_STATUS_CODE_VALUE                     StatusCode;

  StatusCode = 0;
  //
  // Report that the mouse is being enabled
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_MOUSE | EFI_P_PC_ENABLE,
    Dev->DevicePath
    );

  //
  // Setup the device instance
  //
  Dev->MouseSampleRate      = SampleRate20;
  Dev->MouseResolution      = MouseResolution4;
  Dev->MouseScaling         = Scaling1;
  Dev->DataPackageSize      = 3;

  Dev->MouseInputData.Head  = 0;
  Dev->MouseInputData.Tail  = 0;

  //
  // Resolution = 4 counts/mm
  //
  Dev->MouseMode.ResolutionX                = 4;
  Dev->MouseMode.ResolutionY                = 4;
  Dev->MouseMode.LeftButton                 = TRUE;
  Dev->MouseMode.RightButton                = TRUE;

  Dev->SimplePointerProtocol.Reset     = MouseReset;
  Dev->SimplePointerProtocol.GetState  = MouseGetState;
  Dev->SimplePointerProtocol.Mode      = &(Dev->MouseMode);

  //
  // Initialize keyboard controller if necessary
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_MOUSE | EFI_P_MOUSE_PC_SELF_TEST,
    Dev->DevicePath
    );

  if (!CheckMouseConnect (Dev)) {
    DEBUG ((EFI_D_INFO, "Ps2Mux: no mouse detected (I2C device index %u)\r\n",
            Dev->I2cDevice->DeviceIndex));
    Status      = EFI_DEVICE_ERROR;
    StatusCode  = EFI_PERIPHERAL_MOUSE | EFI_P_EC_NOT_DETECTED;
    goto MouseErrorExit;
  }

  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_MOUSE | EFI_P_PC_PRESENCE_DETECT,
    Dev->DevicePath
    );

  //
  // Reset the mouse
  //
  Status = Dev->SimplePointerProtocol.Reset (
                     &Dev->SimplePointerProtocol,
                     FeaturePcdGet (PcdPs2MouseExtendedVerification)
                     );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Ps2Mux: mouse reset failed (I2C device index %u)\r\n",
            Dev->I2cDevice->DeviceIndex));
    Status      = EFI_DEVICE_ERROR;
    StatusCode  = EFI_PERIPHERAL_MOUSE | EFI_P_EC_NOT_DETECTED;
    goto MouseErrorExit;
  }

  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_MOUSE | EFI_P_PC_DETECTED,
    Dev->DevicePath
    );

  //
  // Setup the WaitForInput event
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_CALLBACK,
                  MouseWaitForInput,
                  Dev,
                  &((Dev->SimplePointerProtocol).WaitForInput)
                  );
  if (EFI_ERROR (Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto MouseErrorExit;
  }

  return Status;

MouseErrorExit:
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

  return Status;
}

/**
  De-initialise keyboard portion of the driver.

  @param Dev   - the device instance

  @return status of operation

**/
STATIC
EFI_STATUS
Ps2MuxKeyboardDeinit (
  IN OUT PS2MUX_DEV           *Dev,
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
  if (Dev->I2cDevice != NULL) {
    Status = EFI_SUCCESS;
    while (!EFI_ERROR (Status)) {
      Status = KeyboardRead (Dev, &Data);
    }
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
  De-initialise mouse portion of the driver.

  @param Dev   - the device instance

  @return status of operation

**/
STATIC
EFI_STATUS
Ps2MuxMouseDeinit (
  IN OUT PS2MUX_DEV           *Dev,
  IN EFI_HANDLE               Controller
  )
{
  EFI_STATUS                  Status;
  UINT8                       Data;

  //
  // Report that the mouse is being disabled
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_MOUSE | EFI_P_PC_DISABLE,
    Dev->DevicePath
    );

  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiSimplePointerProtocolGuid,
                  &Dev->SimplePointerProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Since there will be no timer handler for mouse input any more,
  // exhaust input data just in case there is still mouse data left
  //
  if (Dev->I2cDevice != NULL) {
    while (MouseReadData (Dev, &Data)) {
      MicroSecondDelay (PS2MUX_RX_DELAY);
    }
  }

  if (Dev->SimplePointerProtocol.WaitForInput != NULL) {
    gBS->CloseEvent (Dev->SimplePointerProtocol.WaitForInput);
    Dev->SimplePointerProtocol.WaitForInput = NULL;
  }

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
Ps2MuxDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                                 Status;
  EFI_I2C_IO_PROTOCOL                        *Dev;
  EFI_DEVICE_PATH_PROTOCOL                   *DevicePath;

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

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiI2cIoProtocolGuid,
                  (VOID **) &Dev,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (CompareGuid (Dev->DeviceGuid, &gPs2MuxI2cDeviceGuid)) {
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_UNSUPPORTED;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiI2cIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return Status;
}

/**
  Create PS2MUX_DEV instance on controller.

  @param This         Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller   driver controller handle
  @param RemainingDevicePath Children's device path

  @retval whether success to create PS/2 Multiplexer driver instance.
**/
EFI_STATUS
EFIAPI
Ps2MuxDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                                Status;
  EFI_I2C_IO_PROTOCOL                       *I2cDevice;
  PS2MUX_DEV                                *Dev;
  EFI_STATUS_CODE_VALUE                     StatusCode;
  EFI_DEVICE_PATH_PROTOCOL                  *DevicePath;
  PLATFORM_I2C_CONTROLLER                   *CtrlNode;

  StatusCode = 0;

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

  // Extract parent controller BaseAddress
  CtrlNode = (PLATFORM_I2C_CONTROLLER *)DevicePath;
  PcdSet32S (PcdPs2MuxI2cBaseAddr, CtrlNode->BaseAddress);
  //
  // Get the I2C I/O Protocol on Controller's handle
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiI2cIoProtocolGuid,
                  (VOID **)&I2cDevice,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  DEBUG ((EFI_D_INFO, "Ps2Mux: Connected to I2C device "
          "(instance: %p, device index: %u)\r\n",
          I2cDevice, I2cDevice->DeviceIndex));
  //
  // Allocate private data
  //
  Dev = AllocateZeroPool (sizeof (PS2MUX_DEV));
  if (Dev == NULL) {
    Status      = EFI_OUT_OF_RESOURCES;
    StatusCode  = EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_CONTROLLER_ERROR;
    goto ErrorExit;
  }
  //
  // Setup the device instance
  //
  Dev->Signature              = PS2MUX_DEV_SIGNATURE;
  Dev->Handle                 = Controller;
  Dev->I2cDevice              = I2cDevice;
  Dev->DevicePath             = DevicePath;
  Dev->KeyboardEnabled        = FALSE;
  Dev->MouseEnabled           = FALSE;

  Status = Ps2MuxKeyboardInit (Dev);
  Dev->KeyboardEnabled = !EFI_ERROR (Status);

  Status = Ps2MuxMouseInit (Dev);
  Dev->MouseEnabled = !EFI_ERROR (Status);

  if (Dev->KeyboardEnabled ||
      Dev->MouseEnabled) {
    //
    // Setup a periodic timer, used for reading multiplexer data at a fixed interval
    //
    Status = gBS->CreateEvent (
                    EVT_TIMER | EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    Ps2MuxTimerHandler,
                    Dev,
                    &Dev->TimerEvent
                    );
    if (EFI_ERROR (Status)) {
      Status      = EFI_OUT_OF_RESOURCES;
      StatusCode  = EFI_P_EC_CONTROLLER_ERROR;
      StatusCode |= Dev->KeyboardEnabled ?
                    EFI_PERIPHERAL_KEYBOARD :
                    EFI_PERIPHERAL_MOUSE;
      goto ErrorExit;
    }
    Ps2MuxPollStart (Dev);
  } else {
    Status      = EFI_DEVICE_ERROR;
    goto ErrorExit;
  }

  Dev->ControllerNameTable = NULL;
  AddUnicodeString2 (
    "eng",
    gPs2MuxComponentName.SupportedLanguages,
    &Dev->ControllerNameTable,
    L"PS/2 Multiplexer Device",
    TRUE
    );
  AddUnicodeString2 (
    "en",
    gPs2MuxComponentName2.SupportedLanguages,
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

  if (Dev->MouseEnabled) {
    //
    // Install protocol interfaces for the mouse device.
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Controller,
                    &gEfiSimplePointerProtocolGuid,
                    &Dev->SimplePointerProtocol,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      StatusCode = EFI_PERIPHERAL_MOUSE | EFI_P_EC_CONTROLLER_ERROR;
      goto ErrorExit;
    }
  }

  DEBUG ((EFI_D_INFO, "Ps2Mux: initialisation complete (I2C device index %u)\r\n",
          Dev->I2cDevice->DeviceIndex));

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
    Ps2MuxPollStop (Dev);
    gBS->CloseEvent (Dev->TimerEvent);
  }
  if ((Dev != NULL) && Dev->MouseEnabled) {
    Ps2MuxMouseDeinit (Dev, Controller);
  }
  if ((Dev != NULL) && Dev->KeyboardEnabled) {
    Ps2MuxKeyboardDeinit (Dev, Controller);
  }
  if ((Dev != NULL) && (Dev->ControllerNameTable != NULL)) {
    FreeUnicodeStringTable (Dev->ControllerNameTable);
  }
  if (Dev != NULL) {
    gBS->FreePool (Dev);
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiI2cIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

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
Ps2MuxDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
{
  EFI_STATUS                     Status;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
  EFI_SIMPLE_POINTER_PROTOCOL    *SimplePointerProtocol;
  PS2MUX_DEV                     *Dev;

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

    Dev = PS2MUX_DEV_FROM_SIMPLE_POINTER (SimplePointerProtocol);
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

    Dev = PS2MUX_DEV_FROM_SIMPLE_TEXT_INPUT (ConIn);
  }

  if (Dev->TimerEvent != NULL) {
    Ps2MuxPollStop (Dev);
    gBS->CloseEvent (Dev->TimerEvent);
    Dev->TimerEvent = NULL;
  }
  //
  // Disable Mouse
  //
  if (Dev->MouseEnabled) {
    Ps2MuxMouseDeinit (Dev, Controller);
  }
  //
  // Disable Keyboard
  //
  if (Dev->KeyboardEnabled) {
    Ps2MuxKeyboardDeinit (Dev, Controller);
  }
  FreeUnicodeStringTable (Dev->ControllerNameTable);
  gBS->CloseProtocol (
         Controller,
         &gEfiI2cIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );
  gBS->FreePool (Dev);

  return EFI_SUCCESS;
}

/**
  The module Entry Point for module Ps2Mux.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializePs2Mux(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gPs2MuxDriver,
             ImageHandle,
             &gPs2MuxComponentName,
             &gPs2MuxComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Unload function for module Ps2Mux.

  @param  ImageHandle[in]        The allocated handle for the EFI image

  @retval EFI_SUCCESS            The driver was unloaded successfully
  @retval EFI_INVALID_PARAMETER  ImageHandle is not a valid image handle.

**/
EFI_STATUS
EFIAPI
UnloadPs2Mux (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_I2C_IO_PROTOCOL *Dev;
  EFI_STATUS          Status;
  EFI_HANDLE          *HandleBuffer;
  UINTN               HandleCount;
  UINTN               Index;

  //
  // Retrieve all I2C device I/O handles in the handle database
  //
  Status = gBS->LocateHandleBuffer (ByProtocol,
                                    &gEfiI2cIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Disconnect the driver from the handles in the handle database
  //
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->OpenProtocol (
                    HandleBuffer[Index],
                    &gEfiI2cIoProtocolGuid,
                    (VOID **) &Dev,
                    gImageHandle,
                    HandleBuffer[Index],
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status) &&
        CompareGuid (Dev->DeviceGuid, &gPs2MuxI2cDeviceGuid)) {
      Status = gBS->DisconnectController (HandleBuffer[Index],
                                          gImageHandle,
                                          NULL);
    }
  }

  //
  // Free the handle array
  //
  gBS->FreePool (HandleBuffer);

  //
  // Uninstall protocols installed by the driver in its entrypoint
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (ImageHandle,
                  &gEfiDriverBindingProtocolGuid,
                  &gPs2MuxDriver,
                  NULL
                  );

  return EFI_SUCCESS;
}
