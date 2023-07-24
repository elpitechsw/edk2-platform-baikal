#ifndef _PMBUS_H_
#define _PMBUS_H_

#include <IndustryStandard/PmBus.h>
#include <Protocol/SimpleSensor.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#define PMBUS_SENSOR_VIN 0
#define PMBUS_SENSOR_VOUT 1
#define PMBUS_SENSOR_IIN 2
#define PMBUS_SENSOR_IOUT 3
#define PMBUS_SENSOR_PIN 4
#define PMBUS_SENSOR_POUT 5
#define PMBUS_SENSOR_EIN 6
#define PMBUS_SENSOR_EOUT 7
#define PMBUS_SENSOR_TEMP(n) (8 + (n))
#define PMBUS_SENSOR_FAN(n) (11 + (n))

#define PMBUS_NUM_SENSORS 16

#define PMBUS_FORMAT_LINEAR11 1
#define PMBUS_FORMAT_ULINEAR16 2

#define PMBUS_EXP2_VOLT -16
#define PMBUS_EXP2_AMP -16
#define PMBUS_EXP2_WATT -8
#define PMBUS_EXP2_CDEG -2
#define PMBUS_EXP2_RPM 0

typedef UINT32 PMBUS_PAGE_MASK;
typedef UINT16 PMBUS_SENSOR_MASK;
typedef INT32  PMBUS_SENSOR_DATA;

typedef struct {
  /** Sensors mask per page (if bit is set then corresponding sensor is present) */
  PMBUS_SENSOR_MASK      SensorMask;
  /** Offset for values of page */
  UINT16                 ValueOffset;
} PMBUS_PAGE_DATA;

typedef struct {
  /** Pages mask (if bit is set then corresponding page is present) */
  PMBUS_PAGE_MASK        PageMask;
  /** Per page datas */
  PMBUS_PAGE_DATA       *PageData;
  /** Sensors values */
  PMBUS_SENSOR_DATA     *Value;

  /** Sensor listeners */
  LIST_ENTRY             Listeners;

  /** Simple sensor protocol */
  SIMPLE_SENSOR_PROTOCOL SimpleSensor;
} PMBUS_CONTEXT;

typedef EFI_STATUS
(EFIAPI *PMBUS_IO_FN) (
  IN     VOID         *Ctx,
  IN     BOOLEAN       Dir,
  IN OUT PMB_IO_TRANS *Trans
  );

typedef struct {
  UINTN NumPages;
  UINTN NumSensors;
  PMBUS_PAGE_MASK PageMask;
  PMBUS_SENSOR_MASK SensorMask[PMB_MAX_PAGE + 1];
} PMBUS_SCAN_DATA;

EFI_STATUS
PmBusScanSensors (
  IN     VOID             *IoCtx,
  IN     PMBUS_IO_FN       IoFn,
  OUT    PMBUS_SCAN_DATA  *ScanData
  );

#define PMBUS_CONTEXT_MEMORY_SIZE(ScanData)          \
  (sizeof(PMBUS_PAGE_DATA) * (ScanData)->NumPages +  \
   sizeof(PMBUS_SENSOR_DATA) * (ScanData)->NumSensors)

VOID
PmBusInitSensors (
  IN     VOID               *Memory,
  IN     PMBUS_SCAN_DATA    *ScanData,
  OUT    PMBUS_CONTEXT      *Context
  );

EFI_STATUS
PmBusPollSensors (
  IN     VOID               *IoCtx,
  IN     PMBUS_IO_FN         IoFn,
  IN OUT PMBUS_CONTEXT      *Context
  );

VOID
PmBusNotifyListeners (
  IN OUT PMBUS_CONTEXT *Context
  );

#endif /* _PMBUS_H_ */
