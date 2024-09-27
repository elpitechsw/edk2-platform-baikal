/** @file
  Copyright (c) 2023 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _RAM_MENU_STRUC_H_
#define _RAM_MENU_STRUC_H_

#include <Guid/HiiPlatformSetupFormset.h>
#include <Guid/HiiFormMapMethodGuid.h>

#define BAIKALFORMSET_GUID { \
    0xE8A74663, 0x3B53, 0x6A06, {0x19, 0x25, 0x82, 0x83, 0x70, 0x9E, 0x32, 0x92} \
}

#pragma pack(1)
typedef struct {
  UINT16 Speedbin0;
  UINT16 Speedbin1;

  UINT16 Dic0;
  UINT16 RttWr0;
  UINT16 RttNom0;
  UINT16 RttPark0;
  UINT16 Dic1;
  UINT16 RttWr1;
  UINT16 RttNom1;
  UINT16 RttPark1;

  UINT16 Cl0;
  UINT16 tRCD0;
  UINT16 tRP0;
  UINT16 tRAS0;
  UINT16 tFAW0;
  UINT16 Cl1;
  UINT16 tRCD1;
  UINT16 tRP1;
  UINT16 tRAS1;
  UINT16 tFAW1;

  UINT8  t1T2T0;
  UINT8  t1T2T1;

  UINT16 HostVref0;
  UINT16 HostVref1;
  UINT16 DramVref0;
  UINT16 DramVref1;
  UINT8  VrefFlag0;
  UINT8  VrefFlag1;

  UINT8  Flash0;
  UINT8  Flash1;
} RAM_MENU_STRUC;
#pragma pack()

#endif // _RAM_MENU_STRUC_H_
