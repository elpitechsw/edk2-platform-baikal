/** @file
  Implement EFI RealTimeClock runtime services via RTC Lib.

  Copyright (c) 2018 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DwI2cLib.h>
#include <Library/RealTimeClockLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Protocol/FdtClient.h>

#define BCD_MASK_SECONDS       0x7F
#define BCD_MASK_MINUTES       0x7F
#define BCD_MASK_HOURS12       0x1F
#define BCD_MASK_HOURS24       0x3F
#define BCD_MASK_DAYS          0x3F
#define BCD_MASK_MONTHS        0x1F
#define ABEOZ9_BCD_MASK_YEARS  0x7F

STATIC EFI_PHYSICAL_ADDRESS  I2cBase;
STATIC UINTN                 I2cIclk;
STATIC UINTN                 RtcAddr;

STATIC CONST CHAR16  mTimeZoneVariableName[] = L"RtcTimeZone";
STATIC CONST CHAR16  mDaylightVariableName[] = L"RtcDaylight";

enum {
  RtcTypeUnknown,
  RtcTypeAbeoz9,
  RtcTypePcf212x,
  RtcTypeMax
} STATIC RtcType;

STATIC
UINT8
Bcd2Bin (
  IN  UINT8  Val
  )
{
  return (Val >> 4) * 10 + (Val & 0x0F);
}

STATIC
UINT8
Bin2Bcd (
  IN  UINT8  Val
  )
{
  return ((Val / 10) << 4) + Val % 10;
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
LibGetTime (
  OUT  EFI_TIME               *Time,
  OUT  EFI_TIME_CAPABILITIES  *Capabilities
  )
{
  UINT8       Buf[10];
  INTN        I2cDataSize;
  INTN        I2cRxedSize;
  UINT8       RegisterAddr;
  UINTN       Size;
  EFI_STATUS  Status;
  UINT8       *TimeDateBcds;

  if (Time == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (EfiAtRuntime ()) {
    return EFI_UNSUPPORTED;
  }

  if (RtcType == RtcTypeAbeoz9) {
    RegisterAddr = 0x08;
    I2cDataSize  = 7;
    TimeDateBcds = &Buf[0];
  } else if (RtcType == RtcTypePcf212x) {
    RegisterAddr = 0x00;
    I2cDataSize  = 10;
    TimeDateBcds = &Buf[3];
  } else {
    return EFI_DEVICE_ERROR;
  }

  // Get the current time zone information from non-volatile storage
  Size   = sizeof (Time->TimeZone);
  Status = EfiGetVariable (
             (CHAR16 *)mTimeZoneVariableName,
             &gEfiCallerIdGuid,
             NULL,
             &Size,
             (VOID *)&(Time->TimeZone)
             );

  if (EFI_ERROR (Status)) {
    if (Status != EFI_NOT_FOUND) {
      return Status;
    }

    // The time zone variable does not exist in non-volatile storage, so create it.
    Time->TimeZone = EFI_UNSPECIFIED_TIMEZONE;
    Status = EfiSetVariable (
               (CHAR16 *)mTimeZoneVariableName,
               &gEfiCallerIdGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
               Size,
               (VOID *)&(Time->TimeZone)
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "LibGetTime: Failed to save %s variable to non-volatile storage, Status = %r\n",
        mTimeZoneVariableName,
        Status
        ));
      return Status;
    }
  } else {
    // Check TimeZone bounds: -1440 to 1440 or 2047
    if (  ((Time->TimeZone < -1440) || (Time->TimeZone > 1440))
       && (Time->TimeZone != EFI_UNSPECIFIED_TIMEZONE))
    {
      Time->TimeZone = EFI_UNSPECIFIED_TIMEZONE;
    }
  }

  // Get the current daylight information from non-volatile storage
  Size   = sizeof (Time->Daylight);
  Status = EfiGetVariable (
             (CHAR16 *)mDaylightVariableName,
             &gEfiCallerIdGuid,
             NULL,
             &Size,
             (VOID *)&(Time->Daylight)
             );

  if (EFI_ERROR (Status)) {
    if (Status != EFI_NOT_FOUND) {
      return Status;
    }

    // The daylight variable does not exist in non-volatile storage, so create it.
    Time->Daylight = 0;
    Status = EfiSetVariable (
               (CHAR16 *)mDaylightVariableName,
               &gEfiCallerIdGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
               Size,
               (VOID *)&(Time->Daylight)
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "LibGetTime: Failed to save %s variable to non-volatile storage, Status = %r\n",
        mDaylightVariableName,
        Status
        ));
      return Status;
    }
  }

  I2cRxedSize = I2cTxRx (
                  I2cBase,
                  I2cIclk,
                  RtcAddr,
                  (UINT8 *) &RegisterAddr,
                  sizeof RegisterAddr,
                  NULL,
                  0
                  );

  if (I2cRxedSize != 0) {
    return EFI_DEVICE_ERROR;
  }

  I2cRxedSize = I2cTxRx (
                  I2cBase,
                  I2cIclk,
                  RtcAddr,
                  NULL,
                  0,
                  Buf,
                  I2cDataSize
                  );

  if (I2cRxedSize != I2cDataSize) {
    return EFI_DEVICE_ERROR;
  }

  Time->Second = Bcd2Bin (TimeDateBcds[0] & BCD_MASK_SECONDS);
  Time->Minute = Bcd2Bin (TimeDateBcds[1] & BCD_MASK_MINUTES);

  // Handle 12/24-hour modes
  if ((RtcType == RtcTypeAbeoz9  && (TimeDateBcds[2] & BIT6)) ||
      (RtcType == RtcTypePcf212x && (Buf[0] & BIT2))) {
    // 12-hour mode
    Time->Hour = Bcd2Bin (TimeDateBcds[2] & BCD_MASK_HOURS12);
    // Handle AM/PM bit
    if ((TimeDateBcds[2] & BIT5) && Time->Hour != 12) {
      Time->Hour += 12;
    }
  } else {
    // 24-hour mode
    Time->Hour = Bcd2Bin (TimeDateBcds[2] & BCD_MASK_HOURS24);
  }

  Time->Day    = Bcd2Bin (TimeDateBcds[3] & BCD_MASK_DAYS);
  Time->Month  = Bcd2Bin (TimeDateBcds[5] & BCD_MASK_MONTHS);

  if (RtcType == RtcTypeAbeoz9) {
    TimeDateBcds[6] &= ABEOZ9_BCD_MASK_YEARS;
  }

  Time->Year   = Bcd2Bin (TimeDateBcds[6]) + 2000;

  // Not supported
  Time->Pad1       = 0;
  Time->Nanosecond = 0;
  Time->Pad2       = 0;

  if (!IsTimeValid (Time)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: %04u-%02u-%02u %02u:%02u:%02u is invalid date/time\n",
      __func__,
      Time->Year,
      Time->Month,
      Time->Day,
      Time->Hour,
      Time->Minute,
      Time->Second
      ));

    // Set default time
    Time->Year   = TIME_BUILD_YEAR;
    Time->Month  = 1;
    Time->Day    = 1;
    Time->Hour   = 1; // AM/PM indifferent
    Time->Minute = 0;
    Time->Second = 0;
  }

  if (Capabilities != NULL) {
    Capabilities->Resolution = 1;
    Capabilities->Accuracy   = 50000000;
    Capabilities->SetsToZero = FALSE;
  }

  return EFI_SUCCESS;
}

/**
  Sets the current local time and date information.

  @param  Time                  A pointer to the current time.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER A time field is out of range.
  @retval EFI_DEVICE_ERROR      The time could not be set due due to hardware error.
**/
EFI_STATUS
EFIAPI
LibSetTime (
  IN  EFI_TIME  *Time
  )
{
  UINT8       Buf[8];
  INTN        I2cRxedSize;
  EFI_STATUS  Status;

  if (Time == NULL || !IsTimeValid (Time)) {
    return EFI_INVALID_PARAMETER;
  }

  if (EfiAtRuntime ()) {
    return EFI_UNSUPPORTED;
  }

  if (Time->Year < 2000 ||
      (Time->Year > 2085 && RtcType == RtcTypeAbeoz9) ||
      (Time->Year > 2099 && RtcType == RtcTypePcf212x)) {
    return EFI_UNSUPPORTED;
  }

  if (RtcType == RtcTypeAbeoz9) {
    Buf[0] = 0x08;
  } else if (RtcType == RtcTypePcf212x) {
    Buf[0] = 0x03;
  } else {
    return EFI_DEVICE_ERROR;
  }

  Buf[1] = Bin2Bcd (Time->Second);
  Buf[2] = Bin2Bcd (Time->Minute);
  Buf[3] = Bin2Bcd (Time->Hour);
  Buf[4] = Bin2Bcd (Time->Day);
  Buf[6] = Bin2Bcd (Time->Month);
  Buf[7] = Bin2Bcd (Time->Year - 2000);

  // Save the current time zone information into non-volatile storage
  Status = EfiSetVariable (
             (CHAR16 *)mTimeZoneVariableName,
             &gEfiCallerIdGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
             sizeof (Time->TimeZone),
             (VOID *)&(Time->TimeZone)
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "LibSetTime: Failed to save %s variable to non-volatile storage, Status = %r\n",
      mTimeZoneVariableName,
      Status
      ));
    return Status;
  }

  // Save the current daylight information into non-volatile storage
  Status = EfiSetVariable (
             (CHAR16 *)mDaylightVariableName,
             &gEfiCallerIdGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
             sizeof (Time->Daylight),
             (VOID *)&(Time->Daylight)
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "LibSetTime: Failed to save %s variable to non-volatile storage, Status = %r\n",
      mDaylightVariableName,
      Status
      ));
    return Status;
  }

  I2cRxedSize = I2cTxRx (
                  I2cBase,
                  I2cIclk,
                  RtcAddr,
                  Buf,
                  sizeof Buf,
                  NULL,
                  0
                  );

  if (I2cRxedSize != 0) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Returns the current wakeup alarm clock setting.

  @param  Enabled               Indicates if the alarm is currently enabled or disabled.
  @param  Pending               Indicates if the alarm signal is pending and requires acknowledgement.
  @param  Time                  The current alarm setting.

  @retval EFI_SUCCESS           The alarm settings were returned.
  @retval EFI_INVALID_PARAMETER Any parameter is NULL.
  @retval EFI_DEVICE_ERROR      The wakeup time could not be retrieved due to a hardware error.
**/
EFI_STATUS
EFIAPI
LibGetWakeupTime (
  OUT  BOOLEAN   *Enabled,
  OUT  BOOLEAN   *Pending,
  OUT  EFI_TIME  *Time
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
LibSetWakeupTime (
  IN  BOOLEAN    Enabled,
  OUT EFI_TIME  *Time
  )
{
  return EFI_UNSUPPORTED;
}

/**
  This is the declaration of an EFI image entry point. This can be the entry point to an application
  written to this specification, an EFI boot service driver, or an EFI runtime driver.

  @param  ImageHandle           Handle that identifies the loaded image.
  @param  SystemTable           System Table for this image.

  @retval EFI_SUCCESS           The operation completed successfully.
**/
EFI_STATUS
EFIAPI
LibRtcInitialize (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  FDT_CLIENT_PROTOCOL  *FdtClient;
  INT32                 Node = 0;
  CONST VOID           *Prop;
  UINT32                PropSize;
  EFI_STATUS            Status;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);

  if (FdtClient->FindNextCompatibleNode (FdtClient, "abracon,abeoz9", Node, &Node) == EFI_SUCCESS) {
    RtcType = RtcTypeAbeoz9;
  } else if (FdtClient->FindNextCompatibleNode (FdtClient, "nxp,pcf2127", Node, &Node) == EFI_SUCCESS ||
             FdtClient->FindNextCompatibleNode (FdtClient, "nxp,pcf2129", Node, &Node) == EFI_SUCCESS) {
    RtcType = RtcTypePcf212x;
  } else {
    return EFI_DEVICE_ERROR;
  }

  Status = FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize);
  if (EFI_ERROR (Status) || PropSize != sizeof (UINT32)) {
    return EFI_DEVICE_ERROR;
  }

  RtcAddr = SwapBytes32 (*(CONST UINT32 *) Prop);

  if (FdtClient->FindParentNode (FdtClient, Node, &Node) == EFI_SUCCESS &&
      FdtClient->IsNodeEnabled (FdtClient, Node) &&
      FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS && PropSize == 2 * sizeof (UINT64)) {
    I2cBase = SwapBytes64 (ReadUnaligned64 (Prop));

    if (FdtClient->GetNodeProperty (FdtClient, Node, "clocks", &Prop, &PropSize) == EFI_SUCCESS && PropSize == sizeof (UINT32) &&
        FdtClient->FindNodeByPhandle (FdtClient, SwapBytes32 (*(CONST UINT32 *) Prop), &Node) == EFI_SUCCESS &&
        FdtClient->GetNodeProperty (FdtClient, Node, "clock-frequency", &Prop, &PropSize) == EFI_SUCCESS && PropSize == sizeof (UINT32)) {
      I2cIclk = SwapBytes32 (*(CONST UINT32 *) Prop);
    } else {
      return EFI_DEVICE_ERROR;
    }
  } else {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
