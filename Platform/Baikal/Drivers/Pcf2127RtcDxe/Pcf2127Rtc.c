/** @file
  PCF2127 RTC driver. Routines that interacts with callers,
  conforming to EFI driver model.

Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Pcf2127Rtc.h"

#define PCF2127_REG_CTRL1         0x00
#define PCF2127_REG_SECS          0x03
#define PCF2127_REG_MINS          0x04
#define PCF2127_REG_HOURS         0x05
#define PCF2127_REG_DAYS          0x06
#define PCF2127_REG_MONTHS        0x08
#define PCF2127_REG_YEARS         0x09

#define PCF2127_REG_CTRL1_12_24   0x04
#define PCF2127_REG_SECS_VAL      0x7F
#define PCF2127_REG_MINS_VAL      0x7F
#define PCF2127_REG_HOURS_VAL     0x3F
#define PCF2127_REG_HOURS_AM_VAL  0x1F
#define PCF2127_REG_HOURS_AMPM    0x20
#define PCF2127_REG_DAYS_VAL      0x3F
#define PCF2127_REG_MONTHS_VAL    0x1F
#define PCF2127_REG_YEARS_VAL     0xFF

#define RTC_REG_SIZE              1

typedef struct {
  UINTN                           OperationCount;
  EFI_I2C_OPERATION               Address;
  EFI_I2C_OPERATION               Value;
} RTC_I2C_READ_REQUEST;

typedef struct {
  UINTN                           OperationCount;
  EFI_I2C_OPERATION               Request;
} RTC_I2C_WRITE_REQUEST;

PCF2127_RTC_DEV                   gPcf2127RtcDev;
STATIC EFI_RUNTIME_SERVICES       *gRT;

/**
  Read data from the RTC register.

  @param  RtcDev                Pointer to instance of PCF2127_RTC_DEV.
  @param  RegAddress            Register address.
  @param  Value                 Pointer to the variable to store the value in.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_DEVICE_ERROR      The register value could not be retrieved
                                due to hardware error.
**/
STATIC
EFI_STATUS
Pcf2127RtcReadReg (
  IN  PCF2127_RTC_DEV *RtcDev,
  IN  UINT8           RegAddress,
  OUT UINT8           *Value
  )
{
  RTC_I2C_READ_REQUEST Request;
  UINT8 RegData[RTC_REG_SIZE];
  EFI_STATUS Status;

  Status = EFI_DEVICE_ERROR;

  if (RtcDev->I2cDevice != NULL) {
    Request.OperationCount = 2;
    Request.Address.Flags = 0;
    Request.Address.LengthInBytes = sizeof(UINT8);
    Request.Address.Buffer = &RegAddress;
    Request.Value.Flags = I2C_FLAG_READ;
    Request.Value.LengthInBytes = sizeof(RegData);
    Request.Value.Buffer = RegData;
    Status = RtcDev->I2cDevice->QueueRequest (RtcDev->I2cDevice, 0,
                                              NULL,
                                              (EFI_I2C_REQUEST_PACKET *)&Request,
                                              NULL);
    if (!EFI_ERROR (Status)) {
      *Value = RegData[0];
    } else {
      if (Status != EFI_TIMEOUT) {
        DEBUG ((EFI_D_ERROR, "Pcf2127Rtc: read register %02X failed: %r\r\n",
                RegAddress, Status));
      }
      Status = EFI_DEVICE_ERROR;
    }
  }

  return Status;
}

/**
  Write data to the RTC register.

  @param  RtcDev                Pointer to instance of PCF2127_RTC_DEV.
  @param  RegAddress            Register address.
  @param  Value                 Value to be written.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_DEVICE_ERROR      The value could not be stored in the register
                                due to hardware error.
**/
STATIC
EFI_STATUS
Pcf2127RtcWriteReg (
  IN  PCF2127_RTC_DEV *RtcDev,
  IN  UINT8           RegAddress,
  IN  UINT8           Value
  )
{
  RTC_I2C_WRITE_REQUEST Request;
  UINT8 RequestData[1 + RTC_REG_SIZE];
  EFI_STATUS Status;

  Status = EFI_DEVICE_ERROR;

  if (RtcDev->I2cDevice != NULL) {
    Request.OperationCount = 1;
    Request.Request.Flags = 0;
    Request.Request.LengthInBytes = sizeof(RequestData);
    Request.Request.Buffer = RequestData;
    RequestData[0] = RegAddress;
    RequestData[1] = Value;
    Status = RtcDev->I2cDevice->QueueRequest (RtcDev->I2cDevice, 0,
                                              NULL,
                                              (EFI_I2C_REQUEST_PACKET *)&Request,
                                              NULL);
    if (EFI_ERROR (Status)) {
      if (Status != EFI_TIMEOUT) {
        DEBUG ((EFI_D_ERROR, "Pcf2127Rtc: write regsiter %02X failed: %r\r\n",
                RegAddress, Status));
      }
      Status = EFI_DEVICE_ERROR;
    }
  }

  return Status;
}

/**
  Retreive the current time and date information from the RTC device.

  @param  RtcDev                Pointer to instance of PCF2127_RTC_DEV.
  @param  Time                  A pointer to storage to receive a snapshot of
                                the current time and date.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_DEVICE_ERROR      The time/date information could not be retrieved
                                due to hardware error.
**/
STATIC
EFI_STATUS
Pcf2127RtcLoadTime (
  IN  PCF2127_RTC_DEV             *RtcDev,
  OUT EFI_TIME                    *Time
  )
{
  EFI_STATUS                        Status;
  BOOLEAN                           TimeAmPm;
  UINT8                             RegValue;

  Status = Pcf2127RtcReadReg (RtcDev, PCF2127_REG_CTRL1, &RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  } else {
    TimeAmPm = (RegValue & PCF2127_REG_CTRL1_12_24) == PCF2127_REG_CTRL1_12_24;
  }
  Status = Pcf2127RtcReadReg (RtcDev, PCF2127_REG_SECS, &RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  } else {
    Time->Second = BcdToDecimal8 (RegValue & PCF2127_REG_SECS_VAL);
  }
  Status = Pcf2127RtcReadReg (RtcDev, PCF2127_REG_MINS, &RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  } else {
    Time->Minute = BcdToDecimal8 (RegValue & PCF2127_REG_MINS_VAL);
  }
  Status = Pcf2127RtcReadReg (RtcDev, PCF2127_REG_HOURS, &RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  } else {
    if (TimeAmPm) {
      Time->Hour = BcdToDecimal8 (RegValue & PCF2127_REG_HOURS_AM_VAL);
      if (RegValue & PCF2127_REG_HOURS_AMPM) {
        if (Time->Hour < 12) {
          Time->Hour += 12;
        }
      } else {
        if (Time->Hour == 12) {
          Time->Hour = 0;
        }
      }
    } else {
      Time->Hour = BcdToDecimal8 (RegValue & PCF2127_REG_HOURS_VAL);
    }
  }
  Status = Pcf2127RtcReadReg (RtcDev, PCF2127_REG_DAYS, &RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  } else {
    Time->Day = BcdToDecimal8 (RegValue & PCF2127_REG_DAYS_VAL);
  }
  Status = Pcf2127RtcReadReg (RtcDev, PCF2127_REG_MONTHS, &RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  } else {
    Time->Month = BcdToDecimal8 (RegValue & PCF2127_REG_MONTHS_VAL);
  }
  Status = Pcf2127RtcReadReg (RtcDev, PCF2127_REG_YEARS, &RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  } else {
    Time->Year = BcdToDecimal8 (RegValue & PCF2127_REG_YEARS_VAL) + 2000;
  }

  return Status;
}

/**
  Store the time and date information in the RTC device.

  @param  RtcDev                Pointer to instance of PCF2127_RTC_DEV.
  @param  Time                  A pointer to storage that holds a snapshot of
                                the current time and date.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_DEVICE_ERROR      The time/date information could not be stored
                                due to hardware error.
**/
STATIC
EFI_STATUS
Pcf2127RtcStoreTime (
  IN  PCF2127_RTC_DEV             *RtcDev,
  IN  EFI_TIME                    *Time
  )
{
  EFI_STATUS                        Status;
  BOOLEAN                           TimeAmPm;
  UINT8                             RegValue;
  UINT8                             Hours;

  Status = Pcf2127RtcReadReg (RtcDev, PCF2127_REG_CTRL1, &RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  } else {
    TimeAmPm = (RegValue & PCF2127_REG_CTRL1_12_24) == PCF2127_REG_CTRL1_12_24;
  }
  RegValue = DecimalToBcd8 (Time->Second) & PCF2127_REG_SECS_VAL;
  Status = Pcf2127RtcWriteReg (RtcDev, PCF2127_REG_SECS, RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  RegValue = DecimalToBcd8 (Time->Minute) & PCF2127_REG_MINS_VAL;
  Status = Pcf2127RtcWriteReg (RtcDev, PCF2127_REG_MINS, RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (TimeAmPm) {
    Hours = Time->Hour;
    RegValue = 0;
    if (Time->Hour == 0) {
      Hours = 12;
    } else if (Time->Hour == 12) {
      RegValue = PCF2127_REG_HOURS_AMPM;
    } else if (Time->Hour > 12) {
      Hours -= 12;
      RegValue = PCF2127_REG_HOURS_AMPM;
    }
    RegValue |= DecimalToBcd8 (Hours) & PCF2127_REG_HOURS_AM_VAL;
  } else {
    RegValue = DecimalToBcd8 (Time->Hour) & PCF2127_REG_HOURS_VAL;
  }
  Status = Pcf2127RtcWriteReg (RtcDev, PCF2127_REG_HOURS, RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  RegValue = DecimalToBcd8 (Time->Day) & PCF2127_REG_DAYS_VAL;
  Status = Pcf2127RtcWriteReg (RtcDev, PCF2127_REG_DAYS, RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  RegValue = DecimalToBcd8 (Time->Month) & PCF2127_REG_MONTHS_VAL;
  Status = Pcf2127RtcWriteReg (RtcDev, PCF2127_REG_MONTHS, RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  RegValue = DecimalToBcd8 (Time->Year - 2000) & PCF2127_REG_YEARS_VAL;
  Status = Pcf2127RtcWriteReg (RtcDev, PCF2127_REG_YEARS, RegValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Check whether year is a leap one.

  @param  Time                  A pointer to storage that holds a snapshot of
                                the current time and date.

  @retval                       TRUE if year is a leap year, FALSE otherwise

**/
STATIC
BOOLEAN
Pcf2127RtcIsLeapYear (
  IN  EFI_TIME                    *Time
  )
{
  if ((Time->Year % 4) == 0) {
    if ((Time->Year % 100) == 0) {
      if ((Time->Year % 400) == 0) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      return TRUE;
    }
  } else {
    return FALSE;
  }
}

/**
  Check whether day number is valid for current month.

  @param  Time                  A pointer to storage that holds a snapshot of
                                the current time and date.

  @retval                       TRUE if day number is valid, FALSE otherwise

**/
STATIC
BOOLEAN
Pcf2127RtcIsDayValid (
  IN  EFI_TIME                    *Time
  )
{
  UINT8  DaysInMonth[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  ASSERT (Time->Month >= 1);
  ASSERT (Time->Month <= 12);
  if ((Time->Day < 1) ||
      (Time->Day > DaysInMonth[Time->Month - 1]) ||
      (!Pcf2127RtcIsLeapYear (Time) && (Time->Month == 2) && (Time->Day > 28))
      ) {
    return FALSE;
  }

  return TRUE;
}

/**
  Check whether time and date information is valid.

  @param  Time                  A pointer to storage that holds a snapshot of
                                the current time and date.

  @retval                       TRUE if time/date information valid, FALSE otherwise

**/
STATIC
BOOLEAN
Pcf2127RtcCheckTime (
  IN  EFI_TIME                    *Time
  )
{
  if ((Time->Year < 2000) ||
      (Time->Year > 2099) ||
      (Time->Month < 1) ||
      (Time->Month > 12) ||
      !Pcf2127RtcIsDayValid (Time) ||
      (Time->Hour > 23) ||
      (Time->Minute > 59) ||
      (Time->Second > 59) ||
      (Time->Nanosecond > 999999999) ||
      !((Time->TimeZone == EFI_UNSPECIFIED_TIMEZONE) ||
        ((Time->TimeZone >= -1440) && (Time->TimeZone <= 1440))) ||
      (Time->Daylight & ~(EFI_TIME_ADJUST_DAYLIGHT | EFI_TIME_IN_DAYLIGHT))
      ) {
    return FALSE;
  }

  return TRUE;
}

/**
  Returns the current time and date information, and the time-keeping capabilities
  of the hardware platform.

  @param  Time                  A pointer to storage to receive a snapshot of the current time.
  @param  Capabilities          An optional pointer to a buffer to receive the real time clock
                                device's capabilities.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER Time is NULL.
  @retval EFI_DEVICE_ERROR      The time could not be retrieved due to hardware error.

**/
EFI_STATUS
EFIAPI
Pcf2127RtcGetTime (
  OUT EFI_TIME                      *Time,
  OUT EFI_TIME_CAPABILITIES         *Capabilities OPTIONAL
  )
{
  EFI_STATUS                        Status;
  PCF2127_RTC_DEV                   *RtcDev;

  if (EfiAtRuntime()) {
    return EFI_UNSUPPORTED;
  }

  RtcDev = &gPcf2127RtcDev;
  Status = (Time != NULL) ? EFI_SUCCESS : EFI_INVALID_PARAMETER;
  if (!EFI_ERROR (Status)) {
    Time->Pad1 = 0;
 	Time->Nanosecond = 0;
 	Time->TimeZone = 0;
 	Time->Daylight = 0;
 	Time->Pad2 = 0;
    Status = Pcf2127RtcLoadTime (RtcDev, Time);

    if (!EFI_ERROR (Status) && (Capabilities != NULL)) {
      Capabilities->Resolution = 1;
      Capabilities->Accuracy = 50000000;
      Capabilities->SetsToZero = FALSE;
    }
  }

  return Status;
}

/**
  Sets the current local time and date information.

  @param  Time                  A pointer to the current time.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.

**/
EFI_STATUS
EFIAPI
Pcf2127RtcSetTime (
  IN EFI_TIME                       *Time
  )
{
  EFI_STATUS                        Status;
  PCF2127_RTC_DEV                   *RtcDev;

  if (EfiAtRuntime()) {
    return EFI_UNSUPPORTED;
  }

  RtcDev = &gPcf2127RtcDev;
  if ((Time != NULL) && Pcf2127RtcCheckTime (Time)) {
    Status = Pcf2127RtcStoreTime (RtcDev, Time);
  } else {
    Status = EFI_INVALID_PARAMETER;
  }

  return Status;
}

/**
  Returns the current wakeup alarm clock setting.

  @param  Enabled               Indicates if the alarm is currently enabled or disabled.
  @param  Pending               Indicates if the alarm signal is pending and requires acknowledgement.
  @param  Time                  The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this
                                platform.

**/
EFI_STATUS
EFIAPI
Pcf2127RtcGetWakeupTime (
  OUT BOOLEAN                       *Enabled,
  OUT BOOLEAN                       *Pending,
  OUT EFI_TIME                      *Time
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Sets the system wakeup alarm clock time.

  @param  Enabled               Enable or disable the wakeup alarm.
  @param  Time                  If Enable is TRUE, the time to set the wakeup alarm for.

  @retval EFI_SUCCESS           If Enable is TRUE, then the wakeup alarm was enabled. If
                                Enable is FALSE, then the wakeup alarm was disabled.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be set due to a hardware error.
  @retval EFI_UNSUPPORTED       A wakeup timer is not supported on this platform.

**/
EFI_STATUS
EFIAPI
Pcf2127RtcSetWakeupTime (
  IN BOOLEAN                        Enable,
  IN EFI_TIME                       *Time OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}

//
// DriverBinding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gPcf2127RtcDriver = {
  Pcf2127RtcDriverSupported,
  Pcf2127RtcDriverStart,
  Pcf2127RtcDriverStop,
  0xa,
  NULL,
  NULL
};

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
Pcf2127RtcDriverSupported (
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

  if (CompareGuid (Dev->DeviceGuid, &gPcf2127RtcI2cDeviceGuid)) {
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
  Start this driver on Controller by opening a I2cIo protocol, initialising
  PCF2727_RTC_DEV device and install gEfiRealTimeClockArchProtocolGuid finally.

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
Pcf2127RtcDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                          Status;
  EFI_I2C_IO_PROTOCOL                 *I2cDevice;
  PCF2127_RTC_DEV                     *RtcDev;
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
  DEBUG ((EFI_D_INFO, "Pcf2127Rtc: Connected to I2C device "
          "(instance: %p, device index: %u)\r\n",
          I2cDevice, I2cDevice->DeviceIndex));

  RtcDev = &gPcf2127RtcDev;
  //
  // Setup the device instance
  //
  RtcDev->Signature       = PCF2127_RTC_DEV_SIGNATURE;
  RtcDev->Handle          = Controller;
  RtcDev->DevicePath      = DevicePath;
  RtcDev->I2cDevice       = I2cDevice;

  if (gRT != NULL) {
    gRT->GetTime          = Pcf2127RtcGetTime;
    gRT->SetTime          = Pcf2127RtcSetTime;
    gRT->GetWakeupTime    = Pcf2127RtcGetWakeupTime;
    gRT->SetWakeupTime    = Pcf2127RtcSetWakeupTime;
  }

  if (RtcDev->ControllerNameTable != NULL) {
    FreeUnicodeStringTable (RtcDev->ControllerNameTable);
  }
  RtcDev->ControllerNameTable = NULL;
  AddUnicodeString2 (
    "eng",
    gPcf2127RtcComponentName.SupportedLanguages,
    &RtcDev->ControllerNameTable,
    L"PCF2127 RTC Device",
    TRUE
    );
  AddUnicodeString2 (
    "en",
    gPcf2127RtcComponentName2.SupportedLanguages,
    &RtcDev->ControllerNameTable,
    L"PCF2127 RTC Device",
    FALSE
    );


  //
  // Install protocol interfaces for the RTC device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiRealTimeClockArchProtocolGuid,
                  NULL,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  DEBUG ((EFI_D_INFO, "Pcf2127Rtc: initialisation complete (I2C device index %u)\r\n",
          RtcDev->I2cDevice->DeviceIndex));

  return Status;

ErrorExit:

  if (StatusCode != 0) {
    REPORT_STATUS_CODE_WITH_DEVICE_PATH (
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      StatusCode,
      DevicePath
      );
  }

  if (RtcDev->ControllerNameTable != NULL) {
    FreeUnicodeStringTable (RtcDev->ControllerNameTable);
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
Pcf2127RtcDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
  )
{
  PCF2127_RTC_DEV             *RtcDev;

  RtcDev = &gPcf2127RtcDev;

  FreeUnicodeStringTable (RtcDev->ControllerNameTable);

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
  The user Entry Point for module Pcf2127Rtc. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializePcf2127Rtc(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  gPcf2127RtcDev.Signature           = PCF2127_RTC_DEV_SIGNATURE;
  gPcf2127RtcDev.Handle              = NULL;
  gPcf2127RtcDev.DevicePath          = NULL;
  gPcf2127RtcDev.I2cDevice           = NULL;
  gPcf2127RtcDev.ControllerNameTable = NULL;
  gRT                                = (SystemTable == NULL) ?
                                       NULL :
                                       SystemTable->RuntimeServices;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gPcf2127RtcDriver,
             ImageHandle,
             &gPcf2127RtcComponentName,
             &gPcf2127RtcComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Unload function for module Pcf2127Rtc.

  @param  ImageHandle[in]        The allocated handle for the EFI image

  @retval EFI_SUCCESS            The driver was unloaded successfully
  @retval EFI_INVALID_PARAMETER  ImageHandle is not a valid image handle.

**/
EFI_STATUS
EFIAPI
UnloadPcf2127Rtc (
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
        CompareGuid (Dev->DeviceGuid, &gPcf2127RtcI2cDeviceGuid)) {
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
                  &gPcf2127RtcDriver,
                  NULL
                  );

  return EFI_SUCCESS;
}
