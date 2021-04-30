/** @file
  T-Platforms EC driver. Routines that interacts with callers,
  conforming to EFI driver model.

Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "TpBmc.h"

#define TP_BMC_REG_ID1             0x00
#define TP_BMC_REG_ID2             0x01
#define TP_BMC_REG_ID3             0x02
#define TP_BMC_REG_ID4             0x03
#define TP_BMC_REG_SOFTOFF_RQ      0x04
#define TP_BMC_REG_PWROFF_RQ       0x05
#define TP_BMC_REG_PWRBTN_STATE    0x06
#define TP_BMC_REG_VERSION1        0x07
#define TP_BMC_REG_VERSION2        0x08
#define TP_BMC_REG_BOOTREASON      0x09
#define TP_BMC_REG_BOOTREASON_ARG  0x0A
#define TP_BMC_REG_CAP             0x0F

#define TP_BMC_REG_ID1_VAL         0x49
#define TP_BMC_REG_ID2_VAL         0x54
#define TP_BMC_REG_ID3_VAL         0x58
#define TP_BMC_REG_ID4_VAL0        0x32
#define TP_BMC_REG_ID4_VAL1        0x02
#define TP_BMC_REG_VERSION1_VAL    0x00
#define TP_BMC_REG_VERSION2_VAL0   0x02
#define TP_BMC_REG_VERSION2_VAL1   0x03

#define BMC_REG_SIZE                 1
#define BMC_TIMER_INTERVAL           100000

typedef struct {
  UINTN                           OperationCount;
  EFI_I2C_OPERATION               Address;
  EFI_I2C_OPERATION               Value;
} TP_BMC_I2C_READ_REQUEST;

typedef struct {
  UINTN                           OperationCount;
  EFI_I2C_OPERATION               Request;
} TP_BMC_I2C_WRITE_REQUEST;

TP_BMC_DEV                        gTpBmcDev;

/**
  Read data from the EC register.

  @param  BmcDev                Pointer to instance of TP_BMC_DEV.
  @param  RegAddress            Register address.
  @param  Value                 Pointer to the variable to store the value in.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_DEVICE_ERROR      The register value could not be retrieved
                                due to hardware error.
**/
STATIC
EFI_STATUS
TpBmcReadReg (
  IN  TP_BMC_DEV      *BmcDev,
  IN  UINT8           RegAddress,
  OUT UINT8           *Value
  )
{
  TP_BMC_I2C_READ_REQUEST Request;
  UINT8 RegData[BMC_REG_SIZE];
  EFI_STATUS Status;

  Status = EFI_DEVICE_ERROR;

  if (BmcDev->I2cDevice != NULL) {
    Request.OperationCount = 2;
    Request.Address.Flags = 0;
    Request.Address.LengthInBytes = sizeof(UINT8);
    Request.Address.Buffer = &RegAddress;
    Request.Value.Flags = I2C_FLAG_READ;
    Request.Value.LengthInBytes = sizeof(RegData);
    Request.Value.Buffer = RegData;
    Status = BmcDev->I2cDevice->QueueRequest (BmcDev->I2cDevice, 0,
                                              NULL,
                                              (EFI_I2C_REQUEST_PACKET *)&Request,
                                              NULL);
    if (!EFI_ERROR (Status)) {
      *Value = RegData[0];
    } else {
      if (Status != EFI_TIMEOUT) {
        DEBUG ((EFI_D_ERROR, "TpBmc: read register %02X failed: %r\r\n",
                RegAddress, Status));
      }
      Status = EFI_DEVICE_ERROR;
    }
  }

  return Status;
}

/**
  Write data to the EC register.

  @param  BmcDev                Pointer to instance of TP_BMC_DEV.
  @param  RegAddress            Register address.
  @param  Value                 Value to be written.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_DEVICE_ERROR      The value could not be stored in the register
                                due to hardware error.
**/
STATIC
EFI_STATUS
TpBmcWriteReg (
  IN  TP_BMC_DEV      *BmcDev,
  IN  UINT8           RegAddress,
  IN  UINT8           Value
  )
{
  TP_BMC_I2C_WRITE_REQUEST Request;
  UINT8 RequestData[1 + BMC_REG_SIZE];
  EFI_STATUS Status;

  Status = EFI_DEVICE_ERROR;

  if (BmcDev->I2cDevice != NULL) {
    Request.OperationCount = 1;
    Request.Request.Flags = 0;
    Request.Request.LengthInBytes = sizeof(RequestData);
    Request.Request.Buffer = RequestData;
    RequestData[0] = RegAddress;
    RequestData[1] = Value;
    Status = BmcDev->I2cDevice->QueueRequest (BmcDev->I2cDevice, 0,
                                              NULL,
                                              (EFI_I2C_REQUEST_PACKET *)&Request,
                                              NULL);
    if (EFI_ERROR (Status)) {
      if (Status != EFI_TIMEOUT) {
        DEBUG ((EFI_D_ERROR, "TpBmc: write regsiter %02X failed: %r\r\n",
                RegAddress, Status));
      }
      Status = EFI_DEVICE_ERROR;
    }
  }

  return Status;
}

/**
  Check whether probed device is a valid EC device.

  @param  BmcDev                Pointer to instance of TP_BMC_DEV.

  @retval                       TRUE if probed device is a valid EC, FALSE otherwise.

**/
STATIC
BOOLEAN
TpBmcValidate (
  IN OUT TP_BMC_DEV   *BmcDev
  )
{
  EFI_STATUS          Status;
  BOOLEAN             Result;
  UINT8               RegValue;
  UINTN               Index;
  CONST UINT8         IdRegs[] =   {
                                    TP_BMC_REG_ID1,
                                    TP_BMC_REG_ID2,
                                    TP_BMC_REG_ID3
                                    };
  CONST UINT8         IdValues[] = {
                                    TP_BMC_REG_ID1_VAL,
                                    TP_BMC_REG_ID2_VAL,
                                    TP_BMC_REG_ID3_VAL
                                    };

  for (Index = 0; (Index < ARRAY_SIZE(IdRegs)) && !EFI_ERROR (Status); Index++) {
    Status = TpBmcReadReg (BmcDev, IdRegs[Index], &RegValue);
    if (!EFI_ERROR (Status) && (RegValue != IdValues[Index])) {
      DEBUG ((EFI_D_ERROR, "TpBmc: invalid device ID byte %u "
                           "(expected %02X, got %02X)\r\n",
              Index + 1, IdValues[Index], RegValue));
      return FALSE;
    }
  }
  if (EFI_ERROR (Status)) {
    Result = FALSE;
  } else {
    Status = TpBmcReadReg (BmcDev, TP_BMC_REG_ID4, &RegValue);
    if (EFI_ERROR (Status)) {
       Result = FALSE;
    } else {
      if (RegValue == TP_BMC_REG_ID4_VAL0) {
        BmcDev->Version[0] = 0x00;
        BmcDev->Version[1] = 0x00;
        BmcDev->BootReason[0] = 0x00;
        BmcDev->BootReason[1] = 0x00;
        BmcDev->Capabilities = 0x00;
        Result = TRUE;
      } else if (RegValue == TP_BMC_REG_ID4_VAL1) {
        Status = TpBmcReadReg (BmcDev, TP_BMC_REG_VERSION1, &RegValue);
        if (EFI_ERROR (Status)) {
           return FALSE;
        } else {
          DEBUG ((EFI_D_INFO, "TpBmc: VERSION1 equ %02X\r\n", RegValue));
          BmcDev->Version[0] = RegValue;
        }
        Status = TpBmcReadReg (BmcDev, TP_BMC_REG_VERSION2, &RegValue);
        if (EFI_ERROR (Status)) {
           return FALSE;
        } else {
          DEBUG ((EFI_D_INFO, "TpBmc: VERSION2 equ %02X\r\n", RegValue));
          BmcDev->Version[1] = RegValue;
        }
        Status = TpBmcReadReg (BmcDev, TP_BMC_REG_BOOTREASON, &RegValue);
        if (EFI_ERROR (Status)) {
           return FALSE;
        } else {
          DEBUG ((EFI_D_INFO, "TpBmc: BOOTREASON equ %02X\r\n", RegValue));
          BmcDev->BootReason[0] = RegValue;
        }
        Status = TpBmcReadReg (BmcDev, TP_BMC_REG_BOOTREASON_ARG, &RegValue);
        if (EFI_ERROR (Status)) {
           return FALSE;
        } else {
          DEBUG ((EFI_D_INFO, "TpBmc: BOOTREASON_ARG equ %02X\r\n", RegValue));
          BmcDev->BootReason[1] = RegValue;
        }
        if (BmcDev->Version[1] >= TP_BMC_REG_VERSION2_VAL1) {
          Status = TpBmcReadReg (BmcDev, TP_BMC_REG_CAP, &RegValue);
          if (EFI_ERROR (Status)) {
             return FALSE;
          } else {
            DEBUG ((EFI_D_INFO, "TpBmc: CAP equ %02X\r\n", RegValue));
            BmcDev->Capabilities = RegValue;
          }
        } else {
          BmcDev->Capabilities = 0x00;
        }
        Result = TRUE;
      } else {
        DEBUG ((EFI_D_ERROR, "TpBmc: invalid device ID byte 4 "
                             "(expected %02X or %02X, got %02X)\r\n",
                TP_BMC_REG_ID4_VAL0, TP_BMC_REG_ID4_VAL1, RegValue));
        Result = FALSE;
      }
    }
  }

  return Result;
}

/**
  Request EC to perform system poweroff sequence.

  @param  BmcDev                Pointer to instance of TP_BMC_DEV.

**/
STATIC
VOID
TpBmcPoweroff (
  IN OUT TP_BMC_DEV   *BmcDev
  )
{
  DEBUG ((EFI_D_INFO, "TpBmc: system power off\r\n"));
  TpBmcWriteReg (BmcDev, TP_BMC_REG_PWROFF_RQ, 0x01);
}

/**
  Check EC SoftOff request status and issue system poweroff request
  when needed.

  @param  BmcDev                Pointer to instance of TP_BMC_DEV.

**/
STATIC
VOID
TpBmcSoftoffRqCheck (
  IN OUT TP_BMC_DEV   *BmcDev
  )
{
  EFI_STATUS          Status;
  UINT8               RegValue;

  Status = TpBmcReadReg (BmcDev, TP_BMC_REG_SOFTOFF_RQ, &RegValue);
  if (!EFI_ERROR (Status)) {
    if (RegValue != BmcDev->SoftOffRq) {
      if (RegValue != 0x00) {
        DEBUG ((EFI_D_INFO, "TpBmc: power off requested "
                            "(SOFTOFF_RQ equ %02X)\r\n",
                RegValue));
        TpBmcPoweroff (BmcDev);
      }
      BmcDev->SoftOffRq = RegValue;
    }
  }
}

//
// DriverBinding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gTpBmcDriver = {
  TpBmcDriverSupported,
  TpBmcDriverStart,
  TpBmcDriverStop,
  0xa,
  NULL,
  NULL
};

/**
  Enable EC polling.

  @param BmcDev   - the device instance

  @return status of operation

**/
EFI_STATUS
BmcPollStart (
  IN OUT TP_BMC_DEV   *BmcDev
  )
{
  EFI_STATUS              Status;

  Status = gBS->SetTimer (
                  BmcDev->TimerEvent,
                  TimerPeriodic,
                  BMC_TIMER_INTERVAL
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
  }
  return Status;
}

/**
  Disable EC polling.

  @param BmcDev   - the device instance

  @return status of operation

**/
EFI_STATUS
BmcPollStop (
  IN OUT TP_BMC_DEV   *BmcDev
  )
{
  EFI_STATUS              Status;

  Status = gBS->SetTimer (
                  BmcDev->TimerEvent,
                  TimerCancel,
                  BMC_TIMER_INTERVAL
                  );
  if (EFI_ERROR (Status)) {
    Status      = EFI_OUT_OF_RESOURCES;
  }
  return Status;
}

/**
  Test to see if this driver supports Controller. Any Controller
  than contains a I2cIo protocol can be supported.

  @param  This                Protocol instance pointer.
  @param  Controller          Handle of device to test
  @param  RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

  @retval EFI_SUCCESS         This driver supports this device
  @retval EFI_ALREADY_STARTED This driver is already running on this device
  @retval other               This driver does not support this device

**/
EFI_STATUS
EFIAPI
TpBmcDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                        Status;
  EFI_I2C_IO_PROTOCOL               *Dev;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;

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

  if (CompareGuid (Dev->DeviceGuid, &gTpBmcI2cDeviceGuid)) {
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
  Start this driver on Controller by opening a I2cIo protocol and initialising
  TP_BMC_DEV device.

  @param  This                 Protocol instance pointer.
  @param  Controller           Handle of device to bind driver to
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          This driver is added to Controller
  @retval EFI_ALREADY_STARTED  This driver is already running on Controller
  @retval other                This driver does not support this device

**/
EFI_STATUS
EFIAPI
TpBmcDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                          Status;
  EFI_I2C_IO_PROTOCOL                 *I2cDevice;
  TP_BMC_DEV                          *BmcDev;
  EFI_STATUS_CODE_VALUE               StatusCode;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;

  StatusCode  = 0;

  //
  // Open the device path protocol
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
  DEBUG ((EFI_D_INFO, "TpBmc: Connected to I2C device "
          "(instance: %p, device index: %u)\r\n",
          I2cDevice, I2cDevice->DeviceIndex));

  BmcDev = &gTpBmcDev;
  //
  // Setup the device instance
  //
  BmcDev->Signature       = TP_BMC_DEV_SIGNATURE;
  BmcDev->Handle          = Controller;
  BmcDev->DevicePath      = DevicePath;
  BmcDev->I2cDevice       = I2cDevice;
  BmcDev->SoftOffRq       = 0x00;

  if (!TpBmcValidate (BmcDev)) {
    Status = EFI_DEVICE_ERROR;
    goto ErrorExit;
  } else {
    DEBUG ((EFI_D_INFO, "TpBmc: detected a valid EC device\r\n"));
  }

  if (BmcDev->TimerEvent != NULL) {
    BmcPollStop (BmcDev);
    gBS->CloseEvent (BmcDev->TimerEvent);
  }
  BmcDev->TimerEvent = NULL;
  //
  // Setup a periodic timer, used to poll mouse state
  //
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  PollBmc,
                  BmcDev,
                  &BmcDev->TimerEvent
                  );
  if (EFI_ERROR (Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }
  BmcPollStart(BmcDev);

  if (BmcDev->ControllerNameTable != NULL) {
    FreeUnicodeStringTable (BmcDev->ControllerNameTable);
  }
  BmcDev->ControllerNameTable = NULL;
  AddUnicodeString2 (
    "eng",
    gTpBmcComponentName.SupportedLanguages,
    &BmcDev->ControllerNameTable,
    L"T-Plaftorms EC Device",
    TRUE
    );
  AddUnicodeString2 (
    "en",
    gTpBmcComponentName2.SupportedLanguages,
    &BmcDev->ControllerNameTable,
    L"T-Plaftorms EC Device",
    FALSE
    );

  DEBUG ((EFI_D_INFO, "TpBmc: initialisation complete (I2C device index %u)\r\n",
          BmcDev->I2cDevice->DeviceIndex));

  return Status;

ErrorExit:

  if (StatusCode != 0) {
    REPORT_STATUS_CODE_WITH_DEVICE_PATH (
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      StatusCode,
      DevicePath
      );
  }

  if (BmcDev->ControllerNameTable != NULL) {
    FreeUnicodeStringTable (BmcDev->ControllerNameTable);
  }

  if (BmcDev->TimerEvent != NULL) {
    gBS->CloseEvent (BmcDev->TimerEvent);
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiDevicePathProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  gBS->CloseProtocol (
         Controller,
         &gEfiI2cIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return Status;
}

/**
  Stop this driver on Controller. Support stopping any child handles
  created by this driver.

  @param  This              Protocol instance pointer.
  @param  Controller        Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed from Controller
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
TpBmcDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
  )
{
  TP_BMC_DEV               *BmcDev;

  BmcDev = &gTpBmcDev;

  FreeUnicodeStringTable (BmcDev->ControllerNameTable);

  //
  // Cancel EC polling timer, close timer event
  //
  BmcPollStop (BmcDev);
  gBS->CloseEvent (BmcDev->TimerEvent);

  gBS->CloseProtocol (
         Controller,
         &gEfiDevicePathProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  gBS->CloseProtocol (
         Controller,
         &gEfiI2cIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return EFI_SUCCESS;
}

/**
  Event notification function for TimerEvent event.
  Wait for SOFTOFF_RQ flag and shutdown system when it is set.

  @param Event      -  TimerEvent in TP_BMC_DEV
  @param Context    -  Pointer to TP_BMC_DEV structure

**/
VOID
EFIAPI
PollBmc (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )

{
  TP_BMC_DEV    *BmcDev;

  BmcDev = (TP_BMC_DEV *) Context;

  TpBmcSoftoffRqCheck (BmcDev);
}

/**
  The user Entry Point for module TpBmc. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializeTpBmc(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  gTpBmcDev.Signature           = TP_BMC_DEV_SIGNATURE;
  gTpBmcDev.Handle              = NULL;
  gTpBmcDev.DevicePath          = NULL;
  gTpBmcDev.I2cDevice           = NULL;
  gTpBmcDev.TimerEvent          = NULL;
  gTpBmcDev.ControllerNameTable = NULL;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gTpBmcDriver,
             ImageHandle,
             &gTpBmcComponentName,
             &gTpBmcComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Unload function for module TpBmc.

  @param  ImageHandle[in]        The allocated handle for the EFI image

  @retval EFI_SUCCESS            The driver was unloaded successfully
  @retval EFI_INVALID_PARAMETER  ImageHandle is not a valid image handle.

**/
EFI_STATUS
EFIAPI
UnloadTpBmc (
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
        CompareGuid (Dev->DeviceGuid, &gTpBmcI2cDeviceGuid)) {
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
                  &gTpBmcDriver,
                  NULL
                  );

  return EFI_SUCCESS;
}
