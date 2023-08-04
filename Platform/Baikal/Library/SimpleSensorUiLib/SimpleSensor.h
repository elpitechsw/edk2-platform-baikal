#ifndef _SIMPLE_SENSOR_H_
#define _SIMPLE_SENSOR_H_

#include <Uefi.h>
#include <Guid/MdeModuleHii.h>

//
// Libraries
//
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>

//
// Protocols
//
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/SimpleSensor.h>

#include "SimpleSensorData.h"

//
// These are the VFR compiler generated data representing our VFR data.
//
extern UINT8                       SimpleSensorFormBin[];

#define SIMPLE_SENSOR_CONTEXT_SIGNATURE          \
  SIGNATURE_32 ('s', 's', 'u', 'l')

#pragma pack(1)
//
// Sensor element
//
typedef struct {
  SIMPLE_SENSOR_INFO   Info;
  SIMPLE_SENSOR_VAL    Value;
  EFI_STRING_ID        NameId;
  EFI_STRING_ID        ValueId;
  CHAR16               ValueStr[16];
} SIMPLE_SENSOR_ELEMENT;
#pragma pack()

#pragma pack(1)
//
// Data structure for HII routing and accessing
//
typedef struct {
  UINT32                           Signature;

  EFI_HANDLE                       DriverHandle;
  EFI_HII_HANDLE                   HiiHandle;

  //
  // Consumed protocol
  //
  SIMPLE_SENSOR_PROTOCOL          *SimpleSensor;

  //
  // Sensors listener
  //
  SIMPLE_SENSOR_LISTENER           SensorsListener;

  //
  // Sensors data
  //
  SIMPLE_SENSOR_ELEMENT           *Sensor;
  UINTN                            NumSensors;

} SIMPLE_SENSOR_CONTEXT;
#pragma pack()

#define SIMPLE_SENSOR_CONTEXT_FROM_THIS(a)    \
  CR (a, SIMPLE_SENSOR_CONTEXT, ConfigAccess, \
      SIMPLE_SENSOR_CONTEXT_SIGNATURE)

#pragma pack(1)
///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;
#pragma pack()

#endif /* _SIMPLE_SENSOR_H_ */
