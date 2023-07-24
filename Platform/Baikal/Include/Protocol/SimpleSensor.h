/** @file
    Copyright (c) 2023, Elpitech. All rights reserved.<BR>
    SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef SIMPLE_SENSOR_H_
#define SIMPLE_SENSOR_H_

#include <Base.h>

#define SIMPLE_SENSOR_PROTOCOL_GUID {                                      \
    0xBE417D9B, 0xBC49, 0x478C, { 0xBD, 0xE8, 0xF2, 0x62, 0xB2, 0x2F, 0x8C, 0xE2 }}

#define SIMPLE_SENSOR_TYPE_GROUP 0x00
#define SIMPLE_SENSOR_TYPE_COUNT 0x01
#define SIMPLE_SENSOR_TYPE_VOLT 0x11
#define SIMPLE_SENSOR_TYPE_AMP 0x12
#define SIMPLE_SENSOR_TYPE_WATT 0x14
#define SIMPLE_SENSOR_TYPE_CDEG 0x31
#define SIMPLE_SENSOR_TYPE_RPM 0x41

#define SIMPLE_SENSOR_ROOT_ID ((SIMPLE_SENSOR_ID)(0))

#define SIMPLE_SENSOR_MAX_NAME_LEN 16

typedef UINT16 SIMPLE_SENSOR_ID;
typedef INT32  SIMPLE_SENSOR_VAL;

typedef struct _SIMPLE_SENSOR_PROTOCOL SIMPLE_SENSOR_PROTOCOL;

#pragma pack(1)
typedef struct {
  SIMPLE_SENSOR_ID  Id;
  SIMPLE_SENSOR_ID  GrpId;
  CHAR16            Name[SIMPLE_SENSOR_MAX_NAME_LEN];
  UINT8             Type;
  INT8              Exp2;
} SIMPLE_SENSOR_INFO;
#pragma pack()

typedef EFI_STATUS
(EFIAPI *SIMPLE_SENSOR_GET_INFO) (
  IN     SIMPLE_SENSOR_PROTOCOL  *Proto,
  IN OUT SIMPLE_SENSOR_INFO      *Info
  );

typedef EFI_STATUS
(EFIAPI *SIMPLE_SENSOR_GET_DATA) (
  IN     SIMPLE_SENSOR_PROTOCOL  *Proto,
  IN     SIMPLE_SENSOR_ID         Id,
  OUT    SIMPLE_SENSOR_VAL       *Val
  );

typedef struct _SIMPLE_SENSOR_LISTENER SIMPLE_SENSOR_LISTENER;

#pragma pack(1)
struct _SIMPLE_SENSOR_LISTENER {
  LIST_ENTRY  Entry;
  VOID      (*Function)(IN OUT SIMPLE_SENSOR_LISTENER *Listener);
};
#pragma pack()

typedef EFI_STATUS
(EFIAPI *SIMPLE_SENSOR_LISTEN)(
    IN     SIMPLE_SENSOR_PROTOCOL *Proto,
    IN OUT SIMPLE_SENSOR_LISTENER *Listener,
    IN     BOOLEAN                 Register
    );

struct _SIMPLE_SENSOR_PROTOCOL {
  SIMPLE_SENSOR_GET_INFO GetInfo;
  SIMPLE_SENSOR_GET_DATA GetData;
  SIMPLE_SENSOR_LISTEN Listen;
};

extern EFI_GUID gSimpleSensorProtocolGuid;

#endif /*SIMPLE_SENSOR_H_*/
