/** @file

 This is header file with LcdPlatformLib function prototypes
 and constants for Baikal VDU

 Copyright (C) 2020 Baikal Electronics JSC

 Author: Pavel Parkhomenko <Pavel.Parkhomenko@baikalelectronics.ru>

 Parts of this file were based on sources as follows:

 Copyright (c) 2011-2018, ARM Ltd. All rights reserved.<BR>
 SPDX-License-Identifier: BSD-2-Clause-Patent

 **/

#ifndef BAIKAL_VDU_PLATFORM_LIB_H_
#define BAIKAL_VDU_PLATFORM_LIB_H_

#include <Protocol/GraphicsOutput.h>

#define LCD_VRAM_SIZE                     SIZE_16MB

// Modes definitions
#define VGA                               0
#define SVGA                              1
#define XGA                               2
#define HD720                             3
#define WXGA                              4
#define SXGA                              5
#define WSXGA                             6
#define UXGA                              7
#define HD                                8
#define QHD                               9

// VGA Mode: 640 x 480 @ 60
#define VGA_H_RES_PIXELS                  640
#define VGA_V_RES_PIXELS                  480
#define VGA_OSC_FREQUENCY                 24250000

#define VGA_H_FRONT_PORCH                 16
#define VGA_H_SYNC                        96
#define VGA_H_BACK_PORCH                  48

#define VGA_V_FRONT_PORCH                 10
#define VGA_V_SYNC                        2
#define VGA_V_BACK_PORCH                  33

#define VGA_OPMODE                        0x00b3
#define VGA_CURRENT                       0x0000
#define VGA_GMP                           0x0000
#define VGA_TXTER                         0x0004
#define VGA_VLEVCTRL                      0x0232
#define VGA_CKSYMTXCTRL                   0x8009

// SVGA Mode: 800 x 600 @ 60
#define SVGA_H_RES_PIXELS                 800
#define SVGA_V_RES_PIXELS                 600
#define SVGA_OSC_FREQUENCY                40000000

#define SVGA_H_FRONT_PORCH                40
#define SVGA_H_SYNC                       128
#define SVGA_H_BACK_PORCH                 88

#define SVGA_V_FRONT_PORCH                1
#define SVGA_V_SYNC                       4
#define SVGA_V_BACK_PORCH                 23

#define SVGA_OPMODE                       0x00b3
#define SVGA_CURRENT                      0x0000
#define SVGA_GMP                          0x0000
#define SVGA_TXTER                        0x0004
#define SVGA_VLEVCTRL                     0x0232
#define SVGA_CKSYMTXCTRL                  0x8009

// XGA Mode: 1024 x 768 @ 60
#define XGA_H_RES_PIXELS                  1024
#define XGA_V_RES_PIXELS                  768
#define XGA_OSC_FREQUENCY                 65000000

#define XGA_H_FRONT_PORCH                 24 
#define XGA_H_SYNC                        136
#define XGA_H_BACK_PORCH                  160

#define XGA_V_FRONT_PORCH                 3
#define XGA_V_SYNC                        6
#define XGA_V_BACK_PORCH                  29

#define XGA_OPMODE                        0x0072
#define XGA_CURRENT                       0x0008
#define XGA_GMP                           0x0001
#define XGA_TXTER                         0x0004
#define XGA_VLEVCTRL                      0x0232
#define XGA_CKSYMTXCTRL                   0x8009

// HD720 Mode: 1280 x 720 @ 60
#define HD720_H_RES_PIXELS                1280
#define HD720_V_RES_PIXELS                720
#define HD720_OSC_FREQUENCY               74250000

#define HD720_H_FRONT_PORCH               110
#define HD720_H_SYNC                      40
#define HD720_H_BACK_PORCH                220

#define HD720_V_SYNC                      5
#define HD720_V_FRONT_PORCH               5
#define HD720_V_BACK_PORCH                20

#define HD720_OPMODE                      0x0072
#define HD720_CURRENT                     0x0008
#define HD720_GMP                         0x0001
#define HD720_TXTER                       0x0004
#define HD720_VLEVCTRL                    0x0232
#define HD720_CKSYMTXCTRL                 0x8009

// WXGA Mode: 1280 x 800 @ 60
#define WXGA_H_RES_PIXELS                 1280
#define WXGA_V_RES_PIXELS                 800
#define WXGA_OSC_FREQUENCY                71000000

#define WXGA_H_FRONT_PORCH                48
#define WXGA_H_SYNC                       32
#define WXGA_H_BACK_PORCH                 80

#define WXGA_V_FRONT_PORCH                3
#define WXGA_V_SYNC                       6
#define WXGA_V_BACK_PORCH                 14

#define WXGA_OPMODE                       0x0072
#define WXGA_CURRENT                      0x0008
#define WXGA_GMP                          0x0001
#define WXGA_TXTER                        0x0004
#define WXGA_VLEVCTRL                     0x0232
#define WXGA_CKSYMTXCTRL                  0x8009

// SXGA Mode: 1280 x 1024 @ 60
#define SXGA_H_RES_PIXELS                 1280
#define SXGA_V_RES_PIXELS                 1024
#define SXGA_OSC_FREQUENCY                108000000

#define SXGA_H_FRONT_PORCH                48
#define SXGA_H_SYNC                       112
#define SXGA_H_BACK_PORCH                 248

#define SXGA_V_FRONT_PORCH                1
#define SXGA_V_SYNC                       3
#define SXGA_V_BACK_PORCH                 38

#define SXGA_OPMODE                       0x0051
#define SXGA_CURRENT                      0x001b
#define SXGA_GMP                          0x0002
#define SXGA_TXTER                        0x0004
#define SXGA_VLEVCTRL                     0x0232
#define SXGA_CKSYMTXCTRL                  0x8009

// WSXGA+ Mode: 1680 x 1050 @ 60
#define WSXGA_H_RES_PIXELS                1680
#define WSXGA_V_RES_PIXELS                1050
#define WSXGA_OSC_FREQUENCY               119000000

#define WSXGA_H_FRONT_PORCH               48
#define WSXGA_H_SYNC                      32
#define WSXGA_H_BACK_PORCH                80

#define WSXGA_V_FRONT_PORCH               3
#define WSXGA_V_SYNC                      6
#define WSXGA_V_BACK_PORCH                21

#define WSXGA_OPMODE                      0x0051
#define WSXGA_CURRENT                     0x001b
#define WSXGA_GMP                         0x0002
#define WSXGA_TXTER                       0x0004
#define WSXGA_VLEVCTRL                    0x0232
#define WSXGA_CKSYMTXCTRL                 0x8009

// UXGA Mode: 1600 x 1200 @ 60
#define UXGA_H_RES_PIXELS                 1600
#define UXGA_V_RES_PIXELS                 1200
#define UXGA_OSC_FREQUENCY                162000000

#define UXGA_H_FRONT_PORCH                64
#define UXGA_H_SYNC                       192
#define UXGA_H_BACK_PORCH                 304

#define UXGA_V_FRONT_PORCH                1
#define UXGA_V_SYNC                       3
#define UXGA_V_BACK_PORCH                 46

#define UXGA_OPMODE                       0x0051
#define UXGA_CURRENT                      0x001b
#define UXGA_GMP                          0x0002
#define UXGA_TXTER                        0x0004
#define UXGA_VLEVCTRL                     0x0230
#define UXGA_CKSYMTXCTRL                  0x8009

// FullHD Mode: 1920 x 1080 @ 60
#define HD_H_RES_PIXELS                   1920
#define HD_V_RES_PIXELS                   1080
#define HD_OSC_FREQUENCY                  148500000

#define HD_H_FRONT_PORCH                  88
#define HD_H_SYNC                         44
#define HD_H_BACK_PORCH                   148

#define HD_V_FRONT_PORCH                  4
#define HD_V_SYNC                         5
#define HD_V_BACK_PORCH                   36

#define HD_OPMODE                         0x0051
#define HD_CURRENT                        0x001b
#define HD_GMP                            0x0002
#define HD_TXTER                          0x0004
#define HD_VLEVCTRL                       0x0230
#define HD_CKSYMTXCTRL                    0x8009

// QHD Mode: 2560 x 1440 @ 60
#define QHD_H_RES_PIXELS                  2560
#define QHD_V_RES_PIXELS                  1440
#define QHD_OSC_FREQUENCY                 242000000

#define QHD_H_FRONT_PORCH                 48
#define QHD_H_SYNC                        32
#define QHD_H_BACK_PORCH                  80

#define QHD_V_FRONT_PORCH                 3
#define QHD_V_SYNC                        5
#define QHD_V_BACK_PORCH                  33

#define QHD_OPMODE                        0x0040
#define QHD_CURRENT                       0x0036
#define QHD_GMP                           0x0003
#define QHD_TXTER                         0x0004
#define QHD_VLEVCTRL                      0x0273
#define QHD_CKSYMTXCTRL                   0x8009


// Possible bits per pixel variants.
typedef enum {
  LCD_BITS_PER_PIXEL_1 = 0,
  LCD_BITS_PER_PIXEL_2,
  LCD_BITS_PER_PIXEL_4,
  LCD_BITS_PER_PIXEL_8,
  LCD_BITS_PER_PIXEL_16_555,
  LCD_BITS_PER_PIXEL_24,
  LCD_BITS_PER_PIXEL_16_565
} LCD_BPP;

// Display timing settings.
typedef struct {
  UINT32                      Resolution;
  UINT32                      Sync;
  UINT32                      BackPorch;
  UINT32                      FrontPorch;
} SCAN_TIMINGS;

// HDMI PHY settings.
typedef struct {
  UINT32                      Opmode;
  UINT32                      Current;
  UINT32                      Gmp;
  UINT32                      TxTer;
  UINT32                      VlevCtrl;
  UINT32                      CkSymTxCtrl;
} HDMI_PHY_SETTINGS;

/** Platform related initialization function.

  @param[in] Handle              Handle to the LCD device instance.

  @retval EFI_SUCCESS            Plaform library initialized successfully.
  @retval !(EFI_SUCCESS)         Other errors.
**/
EFI_STATUS
LcdPlatformInitializeDisplay (
  IN EFI_HANDLE   Handle
  );

/** Allocate VRAM memory in DRAM for the framebuffer
  (unless it is reserved already).

  The allocated address can be used to set the framebuffer.

  @param[out] VramBaseAddress      A pointer to the framebuffer address.
  @param[out] VramSize             A pointer to the size of the frame
                                   buffer in bytes

  @retval EFI_SUCCESS              Frame buffer memory allocated successfully.
  @retval !(EFI_SUCCESS)           Other errors.
**/
EFI_STATUS
LcdPlatformGetVram (
  OUT EFI_PHYSICAL_ADDRESS*                 VramBaseAddress,
  OUT UINTN*                                VramSize
  );

/** Return total number of modes supported.

  Note: Valid mode numbers are 0 to MaxMode - 1
  See Section 12.9 of the UEFI Specification 2.7

  @retval UINT32             Mode Number.
**/
UINT32
LcdPlatformGetMaxMode (
  VOID
  );

/** Set the requested display mode.

  @param[in] ModeNumber            Mode Number.

  @retval  EFI_SUCCESS             Mode set successfully.
  @retval  EFI_INVALID_PARAMETER   Requested mode not found.
  @retval  !(EFI_SUCCESS)          Other errors.
**/
EFI_STATUS
LcdPlatformSetMode (
  IN UINT32                                 ModeNumber
  );

/** Return information for the requested mode number.

  @param[in]  ModeNumber         Mode Number.
  @param[out] Info               Pointer for returned mode information
                                 (on success).

  @retval EFI_SUCCESS             Mode information for the requested mode
                                  returned successfully.
  @retval EFI_INVALID_PARAMETER   Requested mode not found.
**/
EFI_STATUS
LcdPlatformQueryMode (
  IN  UINT32                                ModeNumber,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *Info
  );

/** Return display timing information for the requested mode number.

  @param[in]  ModeNumber          Mode Number.

  @param[out] Horizontal          Pointer to horizontal timing parameters.
                                  (Resolution, Sync, Back porch, Front porch)
  @param[out] Vertical            Pointer to vertical timing parameters.
                                  (Resolution, Sync, Back porch, Front porch)

  @retval EFI_SUCCESS             Display timing information for the requested
                                  mode returned successfully.
  @retval EFI_INVALID_PARAMETER   Requested mode not found.
**/
EFI_STATUS
LcdPlatformGetTimings (
  IN  UINT32                              ModeNumber,
  OUT SCAN_TIMINGS                        **Horizontal,
  OUT SCAN_TIMINGS                        **Vertical
  );

/** Return HDMI PHY settings for the requested mode number.

  @param[in]  ModeNumber          Mode Number.

  @param[out] PhySettings         Pointer to HDMI PHY settings.

  @retval EFI_SUCCESS             HDMI PHY settings for the requested
                                  mode returned successfully.
  @retval EFI_INVALID_PARAMETER   Requested mode not found.
**/
EFI_STATUS
LcdPlatformGetHdmiPhySettings (
  IN  UINT32                              ModeNumber,
  OUT HDMI_PHY_SETTINGS                   **PhySettings
  );

/** Return bits per pixel information for a mode number.

  @param[in]  ModeNumber          Mode Number.

  @param[out] Bpp                 Pointer to value bits per pixel information.

  @retval EFI_SUCCESS             Bit per pixel information for the requested
                                  mode returned successfully.
  @retval EFI_INVALID_PARAMETER   Requested mode not found.
**/
EFI_STATUS
LcdPlatformGetBpp (
  IN  UINT32                                ModeNumber,
  OUT LCD_BPP*                              Bpp
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
  IN  UINT32                              ModeNumber,
  OUT UINT32  *                           LvdsPorts,
  OUT UINT32  *                           LvdsOutBpp
  );

#endif /* BAIKAL_VDU_PLATFORM_LIB_H_ */
