/** @file
  Header file with LcdPlatformLib function prototypes and constants for Baikal VDU

  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  Author: Pavel Parkhomenko <Pavel.Parkhomenko@baikalelectronics.ru>

  Parts of this file were based on sources as follows:

  Copyright (c) 2011-2018, ARM Ltd. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_VDU_PLATFORM_LIB_H_
#define BAIKAL_VDU_PLATFORM_LIB_H_

#include <Uefi/UefiBaseType.h>
#include <Protocol/GraphicsOutput.h>

#define BAIKAL_LCD_VRAM_SIZE              SIZE_16MB

#define BAIKAL_DEFAULT_V_REFRESH_RATE     60

// VGA Mode: 640 x 480 @ 60
#define BAIKAL_VGA_OSC_FREQUENCY          25175000

#define BAIKAL_VGA_H_SYNC                 96
#define BAIKAL_VGA_H_FRONT_PORCH          16
#define BAIKAL_VGA_H_BACK_PORCH           48

#define BAIKAL_VGA_V_SYNC                 2
#define BAIKAL_VGA_V_FRONT_PORCH          10
#define BAIKAL_VGA_V_BACK_PORCH           33

#define HDMI_PHY_VGA_OPMODE               0x00b3
#define HDMI_PHY_VGA_CURRENT              0x0000
#define HDMI_PHY_VGA_GMP                  0x0000
#define HDMI_PHY_VGA_TXTER                0x0004
#define HDMI_PHY_VGA_VLEVCTRL             0x0232
#define HDMI_PHY_VGA_CKSYMTXCTRL          0x8009

// SVGA Mode: 800 x 600 @ 60
#define BAIKAL_SVGA_OSC_FREQUENCY         40000000

#define BAIKAL_SVGA_H_SYNC                128
#define BAIKAL_SVGA_H_FRONT_PORCH         40
#define BAIKAL_SVGA_H_BACK_PORCH          88

#define BAIKAL_SVGA_V_SYNC                4
#define BAIKAL_SVGA_V_FRONT_PORCH         1
#define BAIKAL_SVGA_V_BACK_PORCH          23

#define HDMI_PHY_SVGA_OPMODE              0x00b3
#define HDMI_PHY_SVGA_CURRENT             0x0000
#define HDMI_PHY_SVGA_GMP                 0x0000
#define HDMI_PHY_SVGA_TXTER               0x0004
#define HDMI_PHY_SVGA_VLEVCTRL            0x0232
#define HDMI_PHY_SVGA_CKSYMTXCTRL         0x8009

// XGA Mode: 1024 x 768 @ 60
#define BAIKAL_XGA_OSC_FREQUENCY          65000000

#define BAIKAL_XGA_H_SYNC                 136
#define BAIKAL_XGA_H_FRONT_PORCH          24
#define BAIKAL_XGA_H_BACK_PORCH           160

#define BAIKAL_XGA_V_SYNC                 6
#define BAIKAL_XGA_V_FRONT_PORCH          3
#define BAIKAL_XGA_V_BACK_PORCH           29

#define HDMI_PHY_XGA_OPMODE               0x0072
#define HDMI_PHY_XGA_CURRENT              0x0008
#define HDMI_PHY_XGA_GMP                  0x0001
#define HDMI_PHY_XGA_TXTER                0x0004
#define HDMI_PHY_XGA_VLEVCTRL             0x0232
#define HDMI_PHY_XGA_CKSYMTXCTRL          0x8009

// HD720 Mode: 1280 x 720 @ 60
#define BAIKAL_HD720_OSC_FREQUENCY        74250000

#define BAIKAL_HD720_H_SYNC               40
#define BAIKAL_HD720_H_FRONT_PORCH        110
#define BAIKAL_HD720_H_BACK_PORCH         220

#define BAIKAL_HD720_V_SYNC               5
#define BAIKAL_HD720_V_FRONT_PORCH        5
#define BAIKAL_HD720_V_BACK_PORCH         20

#define HDMI_PHY_HD720_OPMODE             0x0072
#define HDMI_PHY_HD720_CURRENT            0x0008
#define HDMI_PHY_HD720_GMP                0x0001
#define HDMI_PHY_HD720_TXTER              0x0004
#define HDMI_PHY_HD720_VLEVCTRL           0x0232
#define HDMI_PHY_HD720_CKSYMTXCTRL        0x8009

// WXGA Mode: 1280 x 800 @ 60
#define BAIKAL_WXGA_OSC_FREQUENCY         71000000

#define BAIKAL_WXGA_H_SYNC                32
#define BAIKAL_WXGA_H_FRONT_PORCH         48
#define BAIKAL_WXGA_H_BACK_PORCH          80

#define BAIKAL_WXGA_V_SYNC                6
#define BAIKAL_WXGA_V_FRONT_PORCH         3
#define BAIKAL_WXGA_V_BACK_PORCH          14

#define HDMI_PHY_WXGA_OPMODE              0x0072
#define HDMI_PHY_WXGA_CURRENT             0x0008
#define HDMI_PHY_WXGA_GMP                 0x0001
#define HDMI_PHY_WXGA_TXTER               0x0004
#define HDMI_PHY_WXGA_VLEVCTRL            0x0232
#define HDMI_PHY_WXGA_CKSYMTXCTRL         0x8009

// SXGA Mode: 1280 x 1024 @ 60
#define BAIKAL_SXGA_OSC_FREQUENCY         108000000

#define BAIKAL_SXGA_H_SYNC                112
#define BAIKAL_SXGA_H_FRONT_PORCH         48
#define BAIKAL_SXGA_H_BACK_PORCH          248

#define BAIKAL_SXGA_V_SYNC                3
#define BAIKAL_SXGA_V_FRONT_PORCH         1
#define BAIKAL_SXGA_V_BACK_PORCH          38

#define HDMI_PHY_SXGA_OPMODE              0x0051
#define HDMI_PHY_SXGA_CURRENT             0x001b
#define HDMI_PHY_SXGA_GMP                 0x0002
#define HDMI_PHY_SXGA_TXTER               0x0004
#define HDMI_PHY_SXGA_VLEVCTRL            0x0232
#define HDMI_PHY_SXGA_CKSYMTXCTRL         0x8009

// WSXGA+ Mode: 1680 x 1050 @ 60
#define BAIKAL_WSXGA_OSC_FREQUENCY        119000000

#define BAIKAL_WSXGA_H_SYNC               32
#define BAIKAL_WSXGA_H_FRONT_PORCH        48
#define BAIKAL_WSXGA_H_BACK_PORCH         80

#define BAIKAL_WSXGA_V_SYNC               6
#define BAIKAL_WSXGA_V_FRONT_PORCH        3
#define BAIKAL_WSXGA_V_BACK_PORCH         21

#define HDMI_PHY_WSXGA_OPMODE             0x0051
#define HDMI_PHY_WSXGA_CURRENT            0x001b
#define HDMI_PHY_WSXGA_GMP                0x0002
#define HDMI_PHY_WSXGA_TXTER              0x0004
#define HDMI_PHY_WSXGA_VLEVCTRL           0x0232
#define HDMI_PHY_WSXGA_CKSYMTXCTRL        0x8009

// UXGA Mode: 1600 x 1200 @ 60
#define BAIKAL_UXGA_OSC_FREQUENCY         162000000

#define BAIKAL_UXGA_H_SYNC                192
#define BAIKAL_UXGA_H_FRONT_PORCH         64
#define BAIKAL_UXGA_H_BACK_PORCH          304

#define BAIKAL_UXGA_V_SYNC                3
#define BAIKAL_UXGA_V_FRONT_PORCH         1
#define BAIKAL_UXGA_V_BACK_PORCH          46

#define HDMI_PHY_UXGA_OPMODE              0x0051
#define HDMI_PHY_UXGA_CURRENT             0x001b
#define HDMI_PHY_UXGA_GMP                 0x0002
#define HDMI_PHY_UXGA_TXTER               0x0004
#define HDMI_PHY_UXGA_VLEVCTRL            0x0230
#define HDMI_PHY_UXGA_CKSYMTXCTRL         0x8009

// FullHD Mode: 1920 x 1080 @ 60
#define BAIKAL_HD_OSC_FREQUENCY           148500000

#define BAIKAL_HD_H_SYNC                  44
#define BAIKAL_HD_H_FRONT_PORCH           88
#define BAIKAL_HD_H_BACK_PORCH            148

#define BAIKAL_HD_V_SYNC                  5
#define BAIKAL_HD_V_FRONT_PORCH           4
#define BAIKAL_HD_V_BACK_PORCH            36

#define HDMI_PHY_HD_OPMODE                0x0051
#define HDMI_PHY_HD_CURRENT               0x001b
#define HDMI_PHY_HD_GMP                   0x0002
#define HDMI_PHY_HD_TXTER                 0x0004
#define HDMI_PHY_HD_VLEVCTRL              0x0230
#define HDMI_PHY_HD_CKSYMTXCTRL           0x8009

// QHD Mode: 2560 x 1440 @ 60
#define BAIKAL_QHD_H_RES_PIXELS           2560
#define BAIKAL_QHD_V_RES_PIXELS           1440
#define BAIKAL_QHD_OSC_FREQUENCY          242000000

#define BAIKAL_QHD_H_SYNC                 32
#define BAIKAL_QHD_H_FRONT_PORCH          48
#define BAIKAL_QHD_H_BACK_PORCH           80

#define BAIKAL_QHD_V_SYNC                 5
#define BAIKAL_QHD_V_FRONT_PORCH          3
#define BAIKAL_QHD_V_BACK_PORCH           33

#define HDMI_PHY_QHD_OPMODE               0x0040
#define HDMI_PHY_QHD_CURRENT              0x0036
#define HDMI_PHY_QHD_GMP                  0x0003
#define HDMI_PHY_QHD_TXTER                0x0004
#define HDMI_PHY_QHD_VLEVCTRL             0x0273
#define HDMI_PHY_QHD_CKSYMTXCTRL          0x8009

// HDMI PHY settings
typedef struct {
  UINT32  Opmode;
  UINT32  Current;
  UINT32  Gmp;
  UINT32  TxTer;
  UINT32  VlevCtrl;
  UINT32  CkSymTxCtrl;
} HDMI_PHY_SETTINGS;

#pragma pack (push, 1)

// EDID block
typedef struct {
  UINT8   Header[8];                        // EDID header "00 FF FF FF FF FF FF 00"
  UINT16  ManufactureName;                  // EISA 3-character ID
  UINT16  ProductCode;                      // Vendor assigned code
  UINT32  SerialNumber;                     // 32-bit serial number
  UINT8   WeekOfManufacture;                // Week number
  UINT8   YearOfManufacture;                // Year
  UINT8   EdidVersion;                      // EDID Structure Version
  UINT8   EdidRevision;                     // EDID Structure Revision
  UINT8   VideoInputDefinition;
  UINT8   MaxHorizontalImageSize;           // cm
  UINT8   MaxVerticalImageSize;             // cm
  UINT8   DisplayTransferCharacteristic;
  UINT8   FeatureSupport;
  UINT8   RedGreenLowBits;                  // Rx1 Rx0 Ry1 Ry0 Gx1 Gx0 Gy1Gy0
  UINT8   BlueWhiteLowBits;                 // Bx1 Bx0 By1 By0 Wx1 Wx0 Wy1 Wy0
  UINT8   RedX;                             // Red-x Bits 9 - 2
  UINT8   RedY;                             // Red-y Bits 9 - 2
  UINT8   GreenX;                           // Green-x Bits 9 - 2
  UINT8   GreenY;                           // Green-y Bits 9 - 2
  UINT8   BlueX;                            // Blue-x Bits 9 - 2
  UINT8   BlueY;                            // Blue-y Bits 9 - 2
  UINT8   WhiteX;                           // White-x Bits 9 - 2
  UINT8   WhiteY;                           // White-x Bits 9 - 2
  UINT8   EstablishedTimings[3];
  UINT8   StandardTimingIdentification[16];
  UINT8   DetailedTimingDescriptions[72];
  UINT8   ExtensionFlag;                    // Number of (optional) 128-byte EDID extension blocks to follow
  UINT8   Checksum;
} EDID_BLOCK;

#pragma pack (pop)

#define EDID_STANDARD_TIMINGS           8
#define EDID_STANDARD_TIMING_DESC_SIZE  2

#define EDID_DETAILED_TIMINGS           4
#define EDID_DETAILED_TIMING_DESC_SIZE  18

/** Return HDMI PHY settings for the requested mode number.

  @param[in]  ModeNumber          Mode Number.
  @param[out] PhySettings         Pointer to HDMI PHY settings.

  @retval EFI_SUCCESS             HDMI PHY settings for the requested
                                  mode returned successfully.
  @retval EFI_INVALID_PARAMETER   Requested mode not found.
**/
EFI_STATUS
LcdPlatformGetHdmiPhySettings (
  IN   UINT32               ModeNumber,
  OUT  HDMI_PHY_SETTINGS  **PhySettings
  );

/** Return LVDS related information for a mode number.

  @param[in]  ModeNumber          Mode Number.
  @param[out] LvdsPorts           Pointer to LVDS ports (1, 2 or 4 supported)
  @param[out] LvdsOutBpp          Pointer to LVDS format (18 - jeida-18, 24 - vesa-24)

  @retval EFI_SUCCESS             Bits per pixel information for the requested
                                  mode returned successfully.
  @retval EFI_INVALID_PARAMETER   Requested mode not found.
**/
EFI_STATUS
LcdPlatformGetLvdsInfo (
  IN   UINT32   ModeNumber,
  OUT  UINT32  *LvdsPorts,
  OUT  UINT32  *LvdsOutBpp
  );

#endif /* BAIKAL_VDU_PLATFORM_LIB_H_ */
