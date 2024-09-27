/** @file
  Copyright (c) 2023 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _RAM_SETTINGS_H_
#define _RAM_SETTINGS_H_

#include "RamMenuDataStruct.h"

#define BAIKAL_FLASH_USE_SPD      0xD182
#define BAIKAL_FLASH_USE_STR      0xF1A3

#define BAIKAL_STR_PORT_OFFS      (256 * 1024)

typedef struct {
  UINT16 SpeedBin;
  UINT16 SpdFlagU;
  UINT16 Dic;
  UINT16 RttWr;
  UINT16 RttNom;
  UINT16 RttPark;
  UINT16 Cl;
  UINT16 tRCD;
  UINT16 tRP;
  UINT16 tRAS;
  UINT16 tFAW;
  UINT16 t1T2T;
  UINT16 HostVref;
  UINT16 DramVref;
  UINT16 VrefFlagU;
} DDR_SPD_CONFIG;

EFI_STATUS
EFIAPI
RamStrucCurrentSettings (
  IN RAM_MENU_STRUC *Variable
  );

EFI_STATUS
EFIAPI
RamStrucSaveCurrentSettings (
  IN RAM_MENU_STRUC *Variable
  );

#endif // _RAM_SETTINGS_H_
