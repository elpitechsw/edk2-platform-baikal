/** @file
  Copyright (c) 2023 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef DDR_SETTINGS_H_
#define DDR_SETTINGS_H_

#define FLASH_SPEEDBIN_2400       0x3F77
#define FLASH_SPEEDBIN_2133       0x3E33
#define FLASH_SPEEDBIN_1866       0x1622
#define FLASH_SPEEDBIN_1600       0x1600

#define BAIKAL_FLASH_USE_SPD      0xD182
#define BAIKAL_FLASH_USE_STR      0xF1A3

#define BAIKAL_DIC_RZQ_DIV_7      0x0FFF
#define BAIKAL_DIC_RZQ_DIV_5      0x0F0F

#define BAIKAL_RTTWR_DYN_OFF      0x7FFF
#define BAIKAL_RTTWR_RZQ_DIV_4    0x07FF
#define BAIKAL_RTTWR_RZQ_DIV_2    0x03FF
#define BAIKAL_RTTWR_HI_Z         0x01FF
#define BAIKAL_RTTWR_RZQ_DIV_3    0x00FF

#define BAIKAL_RTTNOM_RZQ_DIS     0x7FFF
#define BAIKAL_RTTNOM_RZQ_DIV_4   0x07FF
#define BAIKAL_RTTNOM_RZQ_DIV_2   0x03FF
#define BAIKAL_RTTNOM_RZQ_DIV_6   0x01FF
#define BAIKAL_RTTNOM_RZQ_DIV_1   0x00FF
#define BAIKAL_RTTNOM_RZQ_DIV_5   0x007F
#define BAIKAL_RTTNOM_RZQ_DIV_3   0x003F
#define BAIKAL_RTTNOM_RZQ_DIV_7   0x0037

#define BAIKAL_RTTPARK_RZQ_DIS    0x7FFF
#define BAIKAL_RTTPARK_RZQ_DIV_4  0x07FF
#define BAIKAL_RTTPARK_RZQ_DIV_2  0x03FF
#define BAIKAL_RTTPARK_RZQ_DIV_6  0x01FF
#define BAIKAL_RTTPARK_RZQ_DIV_1  0x00FF
#define BAIKAL_RTTPARK_RZQ_DIV_5  0x007F
#define BAIKAL_RTTPARK_RZQ_DIV_3  0x003F
#define BAIKAL_RTTPARK_RZQ_DIV_7  0x0037

#define BAIKAL_FLASH_DUMMY        0x7B7B

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

#endif // DDR_SETTINGS_H_
