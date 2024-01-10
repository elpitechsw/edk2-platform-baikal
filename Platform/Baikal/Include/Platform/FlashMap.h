/** @file
  Copyright (c) 2020 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef FLASH_MAP_H_
#define FLASH_MAP_H_

#define FLASH_MAP_FDT    0x40000
#ifdef BAIKAL_MBS_2S
#define FLASH_MAP_VAR    0x100000
#define FLASH_MAP_FIP    0x1C0000
#else
#define FLASH_MAP_VAR    0x80000
#define FLASH_MAP_FIP    0x140000
#endif
#define FLASH_MAP_BLOCK  0x800000

#endif // FLASH_MAP_H_
