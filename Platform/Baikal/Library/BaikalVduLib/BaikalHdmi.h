/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  Author: Pavel Parkhomenko <Pavel.Parkhomenko@baikalelectronics.ru>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_HDMI_H_
#define BAIKAL_HDMI_H_

#include <Library/LcdPlatformLib.h>

EFI_STATUS
EFIAPI
HdmiInit (
  IN CONST SCAN_TIMINGS       *Horizontal,
  IN CONST SCAN_TIMINGS       *Vertical,
  IN CONST HDMI_PHY_SETTINGS  *PhySettings
  );

EFI_STATUS
EFIAPI
HdmiReadEdid (
  IN CONST SCAN_TIMINGS       *Horizontal,
  IN CONST SCAN_TIMINGS       *Vertical,
  IN CONST HDMI_PHY_SETTINGS  *PhySettings,
  OUT UINT8                   **EdidDataBlock,
  OUT UINTN                   *EdidSize
  );

#endif // BAIKAL_HDMI_H_
