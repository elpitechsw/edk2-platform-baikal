/** @file
  Copyright (c) 2023 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _RAM_MENU_STRUC_H_
#define _RAM_MENU_STRUC_H_

#include <Guid/HiiPlatformSetupFormset.h>
#include <Guid/HiiFormMapMethodGuid.h>

#define BAIKALFORMSET_GUID { \
    0xE8A74663, 0x3B53, 0x6A06, {0x19, 0x25, 0x82, 0x83, 0x70, 0x9E, 0x32, 0x94} \
}

typedef struct {
  UINT16 Speedbin;
  UINT16 Dic;
  UINT16 RttWr;
  UINT16 RttNom;
  UINT16 RttPark;
  UINT16 Cl;
  UINT16 tRCD;
  UINT16 tRP;
  UINT16 tRAS;
  UINT16 tFAW;
  UINT8  t1T2T;
  UINT16 HostVref;
  UINT16 DramVref;
  UINT8  VrefFlag;
  UINT8  Flash;
  UINT8  EccDis;
  UINT8  CrcEn;
  UINT8  ParEn;
} RAM_SETTINGS_STRUC;

#pragma pack(1)
typedef struct {
  RAM_SETTINGS_STRUC Channel0;
  RAM_SETTINGS_STRUC Channel1;
  RAM_SETTINGS_STRUC Channel2;
  RAM_SETTINGS_STRUC Channel3;
  RAM_SETTINGS_STRUC Channel4;
  RAM_SETTINGS_STRUC Channel5;
  RAM_SETTINGS_STRUC Channel6;
  RAM_SETTINGS_STRUC Channel7;
  RAM_SETTINGS_STRUC Channel8;
  RAM_SETTINGS_STRUC Channel9;
  RAM_SETTINGS_STRUC Channel10;
  RAM_SETTINGS_STRUC Channel11;
} RAM_MENU_STRUC;
#pragma pack()

#endif // _RAM_MENU_STRUC_H_
