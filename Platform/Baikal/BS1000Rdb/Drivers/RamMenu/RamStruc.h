/** @file
  Copyright (c) 2023 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _RAM_SETTINGS_H_
#define _RAM_SETTINGS_H_

#if defined(BAIKAL_MBS_2S)
# include "RamMenuDataStruct2s.h"
#else
# include "RamMenuDataStruct1s.h"
#endif

#define BAIKAL_FLASH_USE_SPD      0xD182
#define BAIKAL_FLASH_USE_STR      0xF1A3

#define BAIKAL_ECC_DISABLE        0x00DE
#define BAIKAL_CRC_ENABLE         0x00EA
#define BAIKAL_PARITY_EN          0x00EA

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
  UINT16 SpdCrc;
  UINT16 EccDis;
  UINT16 CrcEn;
  UINT16 ParEn;
  UINT16 Reserved[13];
} DDR_SPD_SETTINGS;

#if defined(BAIKAL_MBS_2S)
typedef struct {
  DDR_SPD_SETTINGS Channel0;
  DDR_SPD_SETTINGS Channel1;
  DDR_SPD_SETTINGS Channel2;
  DDR_SPD_SETTINGS Channel3;
  DDR_SPD_SETTINGS Channel4;
  DDR_SPD_SETTINGS Channel5;
  DDR_SPD_SETTINGS Channel6;
  DDR_SPD_SETTINGS Channel7;
  DDR_SPD_SETTINGS Channel8;
  DDR_SPD_SETTINGS Channel9;
  DDR_SPD_SETTINGS Channel10;
  DDR_SPD_SETTINGS Channel11;
} DDR_SPD_CONFIG;
#else
typedef struct {
  DDR_SPD_SETTINGS Channel0;
  DDR_SPD_SETTINGS Channel1;
  DDR_SPD_SETTINGS Channel2;
  DDR_SPD_SETTINGS Channel3;
  DDR_SPD_SETTINGS Channel4;
  DDR_SPD_SETTINGS Channel5;
} DDR_SPD_CONFIG;
#endif

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

EFI_STATUS
EFIAPI
RamStrucConfigurationIsGood (
  IN RAM_MENU_STRUC *Variable
  );

#endif // _RAM_SETTINGS_H_
