#ifndef _I2c_PMBUS_H_
#define _I2C_PMBUS_H_

#include <Protocol/FdtClient.h>
#include <Protocol/SimpleSensor.h>

#include <Library/DwI2cLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>

#include "PmBus.h"

#define I2C_PMBUS_POLL_INTERVAL EFI_TIMER_PERIOD_MILLISECONDS(1000) // 1 S

#define I2C_PMBUS_DEV_SIGNATURE SIGNATURE_32('i', 'p', 'm', 'b')

#pragma pack(1)
typedef struct {
  UINT8                     Type;
  UINT8                     Size;
} PMBUS_CMD_IO;
#pragma pack()

/**
 Structure to describe I2C PMBUS IO handle
**/
typedef struct {
  EFI_PHYSICAL_ADDRESS      I2cBus;
  UINT8                     I2cAddr;
  UINT8                     Capability;
} I2C_PMBUS_IO;

/**
 Structure to describe I2C PMBUS device
**/
typedef struct {
  UINTN                     Signature;
  EFI_HANDLE                Handle;
  //EFI_DEVICE_PATH_PROTOCOL *DevicePath;

  EFI_EVENT                 TimerEvent;

  /// I2C device binding
  I2C_PMBUS_IO              IoCtx;

  PMBUS_CONTEXT             Context;
} I2C_PMBUS_DEV;

typedef struct {
  VENDOR_DEVICE_PATH        Dev;
  EFI_DEVICE_PATH_PROTOCOL  End;
} I2C_PMBUS_DEVICE_PATH;

#endif /* _I2C_PMBUS_H_ */
