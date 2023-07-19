/** @file
  This file contains Baikal VDU driver functions

  Copyright (c) 2019 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  Author: Pavel Parkhomenko <Pavel.Parkhomenko@baikalelectronics.ru>

  Parts of this file were based on sources as follows:

  Copyright (c) 2011-2018, ARM Ltd. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/LcdPlatformLib.h>
#include <Library/BaikalVduPlatformLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/Cpu.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/EdidActive.h>
#include <Protocol/FdtClient.h>

#include "BaikalHdmi.h"
#include "BaikalVdu.h"

typedef struct {
  UINT32            Mode;
  LCD_BPP           Bpp;
  UINT32            OscFreq;

  SCAN_TIMINGS      Horizontal;
  SCAN_TIMINGS      Vertical;
  HDMI_PHY_SETTINGS PhySettings;
  UINT32            LvdsPorts;
  UINT32            LvdsOutBpp;

  BOOLEAN           IsActive;
} DISPLAY_MODE;

/** The display modes supported by the platform.
**/
STATIC DISPLAY_MODE mDisplayModes[] = {
  { // Mode 0 : VGA : 640 x 480 x 24 bpp
    VGA, LcdBitsPerPixel_24,
    BAIKAL_VGA_OSC_FREQUENCY,
    {VGA_H_RES_PIXELS, BAIKAL_VGA_H_SYNC, BAIKAL_VGA_H_BACK_PORCH, BAIKAL_VGA_H_FRONT_PORCH},
    {VGA_V_RES_PIXELS, BAIKAL_VGA_V_SYNC, BAIKAL_VGA_V_BACK_PORCH, BAIKAL_VGA_V_FRONT_PORCH},
    {HDMI_PHY_VGA_OPMODE, HDMI_PHY_VGA_CURRENT, HDMI_PHY_VGA_GMP,
     HDMI_PHY_VGA_TXTER, HDMI_PHY_VGA_VLEVCTRL, HDMI_PHY_VGA_CKSYMTXCTRL},
    1, 18
  },
  { // Mode 1 : SVGA : 800 x 600 x 24 bpp
    SVGA, LcdBitsPerPixel_24,
    BAIKAL_SVGA_OSC_FREQUENCY,
    {SVGA_H_RES_PIXELS, BAIKAL_SVGA_H_SYNC, BAIKAL_SVGA_H_BACK_PORCH, BAIKAL_SVGA_H_FRONT_PORCH},
    {SVGA_V_RES_PIXELS, BAIKAL_SVGA_V_SYNC, BAIKAL_SVGA_V_BACK_PORCH, BAIKAL_SVGA_V_FRONT_PORCH},
    {HDMI_PHY_SVGA_OPMODE, HDMI_PHY_SVGA_CURRENT, HDMI_PHY_SVGA_GMP,
     HDMI_PHY_SVGA_TXTER, HDMI_PHY_SVGA_VLEVCTRL, HDMI_PHY_SVGA_CKSYMTXCTRL},
    1, 18
  },
  { // Mode 2 : XGA : 1024 x 768 x 24 bpp
    XGA, LcdBitsPerPixel_24,
    BAIKAL_XGA_OSC_FREQUENCY,
    {XGA_H_RES_PIXELS, BAIKAL_XGA_H_SYNC, BAIKAL_XGA_H_BACK_PORCH, BAIKAL_XGA_H_FRONT_PORCH},
    {XGA_V_RES_PIXELS, BAIKAL_XGA_V_SYNC, BAIKAL_XGA_V_BACK_PORCH, BAIKAL_XGA_V_FRONT_PORCH},
    {HDMI_PHY_XGA_OPMODE, HDMI_PHY_XGA_CURRENT, HDMI_PHY_XGA_GMP,
     HDMI_PHY_XGA_TXTER, HDMI_PHY_XGA_VLEVCTRL, HDMI_PHY_XGA_CKSYMTXCTRL},
    1, 18
  },
  { // Mode 3 : HD720 : 1280 x 720 x 24 bpp
    HD720, LcdBitsPerPixel_24,
    BAIKAL_HD720_OSC_FREQUENCY,
    {HD720_H_RES_PIXELS, BAIKAL_HD720_H_SYNC, BAIKAL_HD720_H_BACK_PORCH, BAIKAL_HD720_H_FRONT_PORCH},
    {HD720_V_RES_PIXELS, BAIKAL_HD720_V_SYNC, BAIKAL_HD720_V_BACK_PORCH, BAIKAL_HD720_V_FRONT_PORCH},
    {HDMI_PHY_HD720_OPMODE, HDMI_PHY_HD720_CURRENT, HDMI_PHY_HD720_GMP,
     HDMI_PHY_HD720_TXTER, HDMI_PHY_HD720_VLEVCTRL, HDMI_PHY_HD720_CKSYMTXCTRL},
    1, 18
  },
  { // Mode 4 : WXGA : 1280 x 800 x 24 bpp
    WXGA, LcdBitsPerPixel_24,
    BAIKAL_WXGA_OSC_FREQUENCY,
    {WXGA_H_RES_PIXELS, BAIKAL_WXGA_H_SYNC, BAIKAL_WXGA_H_BACK_PORCH, BAIKAL_WXGA_H_FRONT_PORCH},
    {WXGA_V_RES_PIXELS, BAIKAL_WXGA_V_SYNC, BAIKAL_WXGA_V_BACK_PORCH, BAIKAL_WXGA_V_FRONT_PORCH},
    {HDMI_PHY_WXGA_OPMODE, HDMI_PHY_WXGA_CURRENT, HDMI_PHY_WXGA_GMP,
     HDMI_PHY_WXGA_TXTER, HDMI_PHY_SXGA_VLEVCTRL, HDMI_PHY_SXGA_CKSYMTXCTRL},
    1, 18
  },
  { // Mode 5 : SXGA : 1280 x 1024 x 24 bpp
    SXGA, LcdBitsPerPixel_24,
    BAIKAL_SXGA_OSC_FREQUENCY,
    {SXGA_H_RES_PIXELS, BAIKAL_SXGA_H_SYNC, BAIKAL_SXGA_H_BACK_PORCH, BAIKAL_SXGA_H_FRONT_PORCH},
    {SXGA_V_RES_PIXELS, BAIKAL_SXGA_V_SYNC, BAIKAL_SXGA_V_BACK_PORCH, BAIKAL_SXGA_V_FRONT_PORCH},
    {HDMI_PHY_SXGA_OPMODE, HDMI_PHY_SXGA_CURRENT, HDMI_PHY_SXGA_GMP,
     HDMI_PHY_SXGA_TXTER, HDMI_PHY_SXGA_VLEVCTRL, HDMI_PHY_SXGA_CKSYMTXCTRL},
    1, 24
  },
  { // Mode 6 : WSXGA : 1680 x 1050 x 24 bpp
    WSXGA, LcdBitsPerPixel_24,
    BAIKAL_WSXGA_OSC_FREQUENCY,
    {WSXGA_H_RES_PIXELS, BAIKAL_WSXGA_H_SYNC, BAIKAL_WSXGA_H_BACK_PORCH, BAIKAL_WSXGA_H_FRONT_PORCH},
    {WSXGA_V_RES_PIXELS, BAIKAL_WSXGA_V_SYNC, BAIKAL_WSXGA_V_BACK_PORCH, BAIKAL_WSXGA_V_FRONT_PORCH},
    {HDMI_PHY_WSXGA_OPMODE, HDMI_PHY_WSXGA_CURRENT, HDMI_PHY_WSXGA_GMP,
     HDMI_PHY_WSXGA_TXTER, HDMI_PHY_WSXGA_VLEVCTRL, HDMI_PHY_WSXGA_CKSYMTXCTRL},
    2, 24
  },
  { // Mode 7 : UXGA : 1600 x 1200 x 24 bpp
    UXGA, LcdBitsPerPixel_24,
    BAIKAL_UXGA_OSC_FREQUENCY,
    {UXGA_H_RES_PIXELS, BAIKAL_UXGA_H_SYNC, BAIKAL_UXGA_H_BACK_PORCH, BAIKAL_UXGA_H_FRONT_PORCH},
    {UXGA_V_RES_PIXELS, BAIKAL_UXGA_V_SYNC, BAIKAL_UXGA_V_BACK_PORCH, BAIKAL_UXGA_V_FRONT_PORCH},
    {HDMI_PHY_UXGA_OPMODE, HDMI_PHY_UXGA_CURRENT, HDMI_PHY_UXGA_GMP,
     HDMI_PHY_UXGA_TXTER, HDMI_PHY_UXGA_VLEVCTRL, HDMI_PHY_UXGA_CKSYMTXCTRL},
    2, 24
  },
  { // Mode 8 : FullHD : 1920 x 1080 x 24 bpp
    HD, LcdBitsPerPixel_24,
    BAIKAL_HD_OSC_FREQUENCY,
    {HD_H_RES_PIXELS, BAIKAL_HD_H_SYNC, BAIKAL_HD_H_BACK_PORCH, BAIKAL_HD_H_FRONT_PORCH},
    {HD_V_RES_PIXELS, BAIKAL_HD_V_SYNC, BAIKAL_HD_V_BACK_PORCH, BAIKAL_HD_V_FRONT_PORCH},
    {HDMI_PHY_HD_OPMODE, HDMI_PHY_HD_CURRENT, HDMI_PHY_HD_GMP,
     HDMI_PHY_HD_TXTER, HDMI_PHY_HD_VLEVCTRL, HDMI_PHY_HD_CKSYMTXCTRL},
    2, 24
  },
  { // Mode 9 : QHD : 2560 x 1440 x 24 bpp
    QHD, LcdBitsPerPixel_24,
    BAIKAL_QHD_OSC_FREQUENCY,
    {BAIKAL_QHD_H_RES_PIXELS, BAIKAL_QHD_H_SYNC, BAIKAL_QHD_H_BACK_PORCH, BAIKAL_QHD_H_FRONT_PORCH},
    {BAIKAL_QHD_V_RES_PIXELS, BAIKAL_QHD_V_SYNC, BAIKAL_QHD_V_BACK_PORCH, BAIKAL_QHD_V_FRONT_PORCH},
    {HDMI_PHY_QHD_OPMODE, HDMI_PHY_QHD_CURRENT, HDMI_PHY_QHD_GMP,
     HDMI_PHY_QHD_TXTER, HDMI_PHY_QHD_VLEVCTRL, HDMI_PHY_QHD_CKSYMTXCTRL},
    4, 24
  },
};

STATIC EFI_EDID_DISCOVERED_PROTOCOL mEdidDiscovered = {
  0,
  NULL
};

STATIC EFI_EDID_ACTIVE_PROTOCOL mEdidActive = {
  0,
  NULL
};

#define FdtGetTimingProperty(PropertyName, PropertyResult)                                 \
  do {                                                                                     \
    Status = FdtClient->GetNodeProperty (FdtClient, Node, PropertyName, &Prop, &PropSize); \
    if (EFI_ERROR (Status)) {                                                              \
      goto Out;                                                                            \
    }                                                                                      \
    if (PropSize == sizeof (UINT32)) {                                                     \
      PropertyResult = SwapBytes32 (*(CONST UINT32 *) Prop);                               \
    } else if (PropSize == 3 * sizeof (UINT32)) {                                          \
      PropertyResult = SwapBytes32 (((CONST UINT32 *) Prop)[1]);                           \
    } else {                                                                               \
      Status = EFI_INVALID_PARAMETER;                                                      \
      goto Out;                                                                            \
    }                                                                                      \
  } while (0)

STATIC BOOLEAN  mFdtDisplayModeInitialized = FALSE;
STATIC EFI_STATUS mFdtGetPanelTimingsStatus = EFI_SUCCESS;
STATIC DISPLAY_MODE mFdtDisplayMode;

STATIC BOOLEAN  mDisplayInitialized = FALSE;
STATIC EFI_STATUS mInitializeDisplayStatus = EFI_SUCCESS;

STATIC UINT32 mActiveDisplayModes = 0;

/** Helper function to tell if LVDS is enabled or disabled.
  @retval TRUE                   LVDS is enabled in the FDT.
  @retval FALSE                  LVDS is disabled in the FDT.
**/
BOOLEAN
LvdsEnabled (
  VOID
  )
{
  return mFdtDisplayModeInitialized &&
         mFdtDisplayMode.LvdsPorts > 0;
}

/** Helper function to get the total number of modes supported.

  @note Valid mode numbers are 0 to MaxMode - 1
        See Section 12.9 of the UEFI Specification 2.7

  @retval UINT32  Mode Number
**/
STATIC
UINT32
GetMaxSupportedMode (
  VOID
  )
{
  UINT32  MaxMode;

  // The following line would correctly report the total number
  // of graphics modes supported by the Baikal VDU.
  // MaxMode = ARRAY_SIZE (mDisplayModes);
  //
  // However, on some platforms it is desirable to ignore some graphics modes.
  MaxMode = FixedPcdGet32 (PcdVduMaxMode);
  if (MaxMode == 0) {
    MaxMode = ARRAY_SIZE (mDisplayModes);
  }

  return MaxMode;
}

/** Helper function to find the active display mode.

  @param[in]  ActiveModeNumber    The active mode number
  @param[out] ActiveDisplayMode   A pointer to a pointer to the active DISPLAY_MODE structure

  @retval EFI_SUCCESS             The active mode found
  @retval !(EFI_SUCCESS)          An error occured
**/
STATIC
EFI_STATUS
FindActiveDisplayMode (
  IN  UINT32        ActiveModeNumber,
  OUT DISPLAY_MODE  **ActiveDisplayMode
  )
{
  UINT32        Index;
  UINT32        ActiveIndex;
  CONST UINT32  MaxMode = GetMaxSupportedMode ();

  if (ActiveDisplayMode != NULL) {
    *ActiveDisplayMode = NULL;
  }

  if (ActiveModeNumber >= mActiveDisplayModes) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0, ActiveIndex = 0; Index < MaxMode; ++Index) {
    if (!mDisplayModes[Index].IsActive) {
      continue;
    }

    if (ActiveIndex == ActiveModeNumber) {
      if (ActiveDisplayMode != NULL) {
        *ActiveDisplayMode = &mDisplayModes[Index];
      }
      return EFI_SUCCESS;
    }

    ++ActiveIndex;
  }

  return EFI_UNSUPPORTED;
}

/** Helper function to get LVDS panel timings from FDT.

  @param[out] FdtDisplayMode    A pointer to a pointer to the DISPLAY_MODE structure with panel
                                timings (see below).

  @retval EFI_SUCCESS           There is a valid panel-lvds node in the FDT with valid panel-timings
                                subnode. Timings have been read and will override hard-coded timings
                                for a compatible video mode.
  @retval !(EFI_SUCCESS)        An error occured.
**/
STATIC
EFI_STATUS
FdtGetPanelTimings (
  OUT CONST DISPLAY_MODE  **FdtDisplayMode
  )
{
  EFI_STATUS          Status;
  FDT_CLIENT_PROTOCOL *FdtClient;
  INT32               Node;
  INT32               NodePanel;
  INT32               NodePort;
  CONST VOID          *Prop;
  UINT32              PropSize;

  if (mFdtDisplayModeInitialized) {
    Status = mFdtGetPanelTimingsStatus;
    goto Out;
  }

  // This field acts as return value.
  // 0 ports on return means that either there is no "panel-lvds" in FDT
  // or there is an error in this node or its subnodes.
  // Valid values are 1, 2 or 4.
  mFdtDisplayMode.LvdsPorts = 0;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FdtClientProtocol, Status: %r\n", __func__, Status));
    goto Out;
  }

  Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,vdu", -1, &Node);
  if (EFI_ERROR (Status)) {
    goto Out;
  }
  if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
    Status = EFI_NOT_FOUND;
    goto Out;
  }
  Status = FdtClient->GetNodeProperty (FdtClient, Node, "lvds-lanes", &Prop, &PropSize);
  if (EFI_ERROR (Status)) {
    goto Out;
  }
  if (EFI_ERROR (Status)) {
    goto Out;
  }
  if (PropSize == sizeof (UINT32)) {
    mFdtDisplayMode.LvdsPorts = SwapBytes32 (*(CONST UINT32 *) Prop);
  } else {
    Status = EFI_INVALID_PARAMETER;
    goto Out;
  }
  if ((mFdtDisplayMode.LvdsPorts != 1) &&
      (mFdtDisplayMode.LvdsPorts != 2) &&
      (mFdtDisplayMode.LvdsPorts != 4)) {
    Status = EFI_INVALID_PARAMETER;
    goto Out;
  }

  Status = FdtClient->FindNextCompatibleNode (FdtClient, "panel-lvds", -1, &NodePanel);
#ifdef ELPITECH
  if (EFI_ERROR (Status)) {
    Status = FdtClient->FindNextCompatibleNode (FdtClient, "megachips,stdp4028-lvds-dp", -1, &NodePanel);
    if (!EFI_ERROR(Status) && FdtClient->IsNodeEnabled (FdtClient, NodePanel)) {
      mFdtDisplayMode = mDisplayModes[8];
      mFdtDisplayMode.LvdsPorts = 2;
      mFdtDisplayMode.LvdsOutBpp = 24;
    }
    goto Out;
  }
#else
  if (EFI_ERROR (Status)) {
    goto Out;
  }
#endif
  if (!FdtClient->IsNodeEnabled (FdtClient, NodePanel)) {
    Status = EFI_NOT_FOUND;
    goto Out;
  }

  Status = FdtClient->GetNodeProperty (FdtClient, NodePanel, "data-mapping", &Prop, &PropSize);
  if (EFI_ERROR (Status)) {
    goto Out;
  }
  if (AsciiStrCmp ((CONST CHAR8 *) Prop, "jeida-18") != 0 &&
      AsciiStrCmp ((CONST CHAR8 *) Prop, "vesa-24")  != 0)
  {
    Status = EFI_UNSUPPORTED;
    goto Out;
  }
  if (AsciiStrCmp ((CONST CHAR8 *) Prop, "jeida-18") == 0) {
    mFdtDisplayMode.LvdsOutBpp = 18;
  } else { // "vesa-24"
    mFdtDisplayMode.LvdsOutBpp = 24;
  }

  Status = FdtClient->FindNextSubnode (FdtClient, "panel-timing", NodePanel, &Node);
  if (EFI_ERROR (Status)) {
    goto Out;
  }

  FdtGetTimingProperty ("clock-frequency", mFdtDisplayMode.OscFreq);
  FdtGetTimingProperty ("hactive", mFdtDisplayMode.Horizontal.Resolution);
  FdtGetTimingProperty ("vactive", mFdtDisplayMode.Vertical.Resolution);
  FdtGetTimingProperty ("hsync-len", mFdtDisplayMode.Horizontal.Sync);
  FdtGetTimingProperty ("hfront-porch", mFdtDisplayMode.Horizontal.FrontPorch);
  FdtGetTimingProperty ("hback-porch", mFdtDisplayMode.Horizontal.BackPorch);
  FdtGetTimingProperty ("vsync-len", mFdtDisplayMode.Vertical.Sync);
  FdtGetTimingProperty ("vfront-porch", mFdtDisplayMode.Vertical.FrontPorch);
  FdtGetTimingProperty ("vback-porch", mFdtDisplayMode.Vertical.BackPorch);

  Status = FdtClient->FindNextSubnode (FdtClient, "port", NodePanel, &NodePort);
  if (EFI_ERROR (Status)) {
    goto Out;
  }

Out:
  if (EFI_ERROR (Status)) {
    mFdtDisplayMode.LvdsPorts = 0;
  }

  mFdtDisplayModeInitialized = TRUE;
  mFdtGetPanelTimingsStatus = Status;

  if (FdtDisplayMode != NULL) {
    *FdtDisplayMode = &mFdtDisplayMode;
  }
  return mFdtGetPanelTimingsStatus;
}

/** Parse the Detailed Timing, Standard Timing and Established Timing
    in EDID data block.

  @param[in]  EdidBuffer        Pointer to EDID data
  @param[in]  EdidBufferSize    Size of EDID data

  @retval TRUE                  The EDID data is valid
  @retval FALSE                 The EDID data is invalid
**/
STATIC
BOOLEAN
ParseEdidData (
  IN CONST VOID *EdidBuffer,
  IN UINTN      EdidBufferSize
  )
{
  CONST EDID_BLOCK  *EdidDataBlock = EdidBuffer;
  CONST UINT32      MaxMode = GetMaxSupportedMode ();
  UINTN             Index;
  CONST UINT8       *Byte;
  UINTN             Mode;
  SCAN_TIMINGS      Horizontal;
  SCAN_TIMINGS      Vertical;
  UINT32            PixelClock;
  UINT8             RefreshRate;

  //
  // Detailed Timings
  //
  for (Index = 0; Index < EDID_DETAILED_TIMINGS; ++Index) {
    Byte = &EdidDataBlock->DetailedTimingDescriptions[Index * EDID_DETAILED_TIMING_DESC_SIZE];

    PixelClock = ((Byte[1] << 8) | Byte[0]) * 10000;

    // Not a Detailed Timing Descriptor
    if (PixelClock == 0) {
      continue;
    }

    Horizontal.Resolution = ((Byte[4] & 0xF0) << 4) | Byte[2];
    Horizontal.FrontPorch = ((Byte[11] & 0xC0) << 2) | Byte[8];
    Horizontal.Sync       = ((Byte[11] & 0x30) << 4) | Byte[9];
    Horizontal.BackPorch  = (((Byte[4] & 0x0F) << 8) | Byte[3]) -
                            (Horizontal.FrontPorch + Horizontal.Sync);

    Vertical.Resolution = ((Byte[7] & 0xF0) << 4) | Byte[5];
    Vertical.FrontPorch = ((Byte[11] & 0x0C) << 2) | ((Byte[10] & 0xF0) >> 4);
    Vertical.Sync       = ((Byte[11] & 0x03) << 4) | (Byte[10] & 0x0F);
    Vertical.BackPorch  = (((Byte[7] & 0x0F) << 8) | Byte[6]) -
                          (Vertical.FrontPorch + Vertical.Sync);

    for (Mode = 0; Mode < MaxMode; ++Mode) {
      if (Vertical.Resolution == mDisplayModes[Mode].Vertical.Resolution &&
          Horizontal.Resolution == mDisplayModes[Mode].Horizontal.Resolution)
      {
        mDisplayModes[Mode].OscFreq = PixelClock;
        mDisplayModes[Mode].Horizontal = Horizontal;
        mDisplayModes[Mode].Vertical = Vertical;

        mDisplayModes[Mode].IsActive = TRUE;
      }
    }
  }

  //
  // Standard Timings
  //
  for (Index = 0; Index < EDID_STANDARD_TIMINGS; ++Index) {
    Byte = &EdidDataBlock->StandardTimingIdentification[Index * EDID_STANDARD_TIMING_DESC_SIZE];

    // Unused Standard Timing data fields shall be set to 01h, 01h
    if (Byte[0] <= 1 &&
        Byte[1] == 1)
    {
      continue;
    }

    RefreshRate = (Byte[1] & 0x3F) + 60;
    if (RefreshRate != BAIKAL_DEFAULT_V_REFRESH_RATE) {
      continue;
    }

    Horizontal.Resolution = (Byte[0] + 31) * 8;
    switch (Byte[1] & 0xC0) {
    case 0x00:
      Vertical.Resolution = (Horizontal.Resolution * 10) / 16;
      break;

    case 0x40:
      Vertical.Resolution = (Horizontal.Resolution * 3) / 4;
      break;

    case 0x80:
      Vertical.Resolution = (Horizontal.Resolution * 4) / 5;
      break;

    case 0xC0:
      Vertical.Resolution = (Horizontal.Resolution * 9) / 16;
      break;

    default:
      continue;
    }

    for (Mode = 0; Mode < MaxMode; ++Mode) {
      if (Vertical.Resolution == mDisplayModes[Mode].Vertical.Resolution &&
          Horizontal.Resolution == mDisplayModes[Mode].Horizontal.Resolution)
      {
        mDisplayModes[Mode].IsActive = TRUE;
      }
    }
  }

  //
  // Established Timings
  //
  for (Mode = 0; Mode < MaxMode; ++Mode) {
    switch (mDisplayModes[Mode].Mode) {
    case VGA:
      if ((EdidDataBlock->EstablishedTimings[0] & 0x20) != 0) {
        mDisplayModes[Mode].IsActive = TRUE;
      }
      break;

    case SVGA:
      if ((EdidDataBlock->EstablishedTimings[0] & 0x01) != 0) {
        mDisplayModes[Mode].IsActive = TRUE;
      }
      break;

    case XGA:
      if ((EdidDataBlock->EstablishedTimings[1] & 0x08) != 0) {
        mDisplayModes[Mode].IsActive = TRUE;
      }
      break;

    default:
      break;
    }
  }

  return TRUE;
}

/** Video mode related PCD setting helper function

  @param[in]  Width        Width of the screen
  @param[in]  Height       Height of the screen
**/
STATIC
VOID
SetVideoModePcds (
  IN UINT32 Width,
  IN UINT32 Height
  )
{
  PcdSet32S (PcdSetupVideoHorizontalResolution, Width);
  PcdSet32S (PcdSetupVideoVerticalResolution, Height);
  PcdSet32S (PcdSetupConOutColumn, Width / EFI_GLYPH_WIDTH);
  PcdSet32S (PcdSetupConOutRow, Height / EFI_GLYPH_HEIGHT);

  PcdSet32S (PcdVideoHorizontalResolution, Width);
  PcdSet32S (PcdVideoVerticalResolution, Height);
  PcdSet32S (PcdConOutColumn, Width / EFI_GLYPH_WIDTH);
  PcdSet32S (PcdConOutRow, Height / EFI_GLYPH_HEIGHT);
}

/** Display initialization function.

  @retval EFI_SUCCESS            Display initialized successfully.
  @retval EFI_UNSUPPORTED        PcdGopPixelFormat must be
                                 PixelRedGreenBlueReserved8BitPerColor OR
                                 PixelBlueGreenRedReserved8BitPerColor
                                 any other format is not supported
  @retval !(EFI_SUCCESS)         Other errors.
**/
STATIC
EFI_STATUS
InitializeDisplay (
  VOID
  )
{
  UINT32                    Index;
  EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
  CONST DISPLAY_MODE        *FdtDisplayMode;
  CONST UINT32              MaxMode = GetMaxSupportedMode ();
  UINTN                     EdidDataSize;
  UINT8                     *EdidDataBlock = NULL;

  if (mDisplayInitialized) {
    goto Out;
  }

  mInitializeDisplayStatus = EFI_SUCCESS;

  mActiveDisplayModes = 0;

  // PixelBitMask and PixelBltOnly pixel formats are not supported
  PixelFormat = FixedPcdGet32 (PcdGopPixelFormat);
  if (PixelFormat != PixelRedGreenBlueReserved8BitPerColor &&
      PixelFormat != PixelBlueGreenRedReserved8BitPerColor)
  {
    ASSERT (PixelFormat == PixelRedGreenBlueReserved8BitPerColor ||
            PixelFormat == PixelBlueGreenRedReserved8BitPerColor);
    mInitializeDisplayStatus = EFI_UNSUPPORTED;
    goto Out;
  }

  FdtGetPanelTimings (&FdtDisplayMode);
  // If a video mode is specified in FDT, find a mode with corresponding resolution
  // and make this mode the only available.
  if (FdtDisplayMode->LvdsPorts != 0) {
    for (Index = 0; Index < MaxMode; ++Index) {
      // Override hard-coded timings by FDT timings
      // once a compatible video mode is found.
      if (FdtDisplayMode->Vertical.Resolution == mDisplayModes[Index].Vertical.Resolution &&
          FdtDisplayMode->Horizontal.Resolution == mDisplayModes[Index].Horizontal.Resolution)
      {
        SetVideoModePcds (FdtDisplayMode->Horizontal.Resolution,
                          FdtDisplayMode->Vertical.Resolution);

        mDisplayModes[Index].OscFreq = FdtDisplayMode->OscFreq;

        mDisplayModes[Index].Horizontal.Sync = FdtDisplayMode->Horizontal.Sync;
        mDisplayModes[Index].Horizontal.FrontPorch = FdtDisplayMode->Horizontal.FrontPorch;
        mDisplayModes[Index].Horizontal.BackPorch = FdtDisplayMode->Horizontal.BackPorch;

        mDisplayModes[Index].Vertical.Sync = FdtDisplayMode->Vertical.Sync;
        mDisplayModes[Index].Vertical.FrontPorch = FdtDisplayMode->Vertical.FrontPorch;
        mDisplayModes[Index].Vertical.BackPorch = FdtDisplayMode->Vertical.BackPorch;

        mDisplayModes[Index].LvdsPorts = FdtDisplayMode->LvdsPorts;
        mDisplayModes[Index].LvdsOutBpp = FdtDisplayMode->LvdsOutBpp;

        mDisplayModes[Index].IsActive = TRUE;
        break;
      }
    }
  } else {
    for (Index = 0; Index < MaxMode; ++Index) {
      if (mDisplayModes[Index].Mode == VGA) {
        mInitializeDisplayStatus = HdmiReadEdid (
                                     &mDisplayModes[Index].Horizontal,
                                     &mDisplayModes[Index].Vertical,
                                     &mDisplayModes[Index].PhySettings,
                                     &EdidDataBlock,
                                     &EdidDataSize
                                     );
        if (!EFI_ERROR (mInitializeDisplayStatus)) {
          if (!ParseEdidData (EdidDataBlock, EdidDataSize)) {
            FreePool (EdidDataBlock);
            EdidDataBlock = NULL;
          } else {
            mEdidDiscovered.SizeOfEdid = EdidDataSize;
            mEdidDiscovered.Edid = EdidDataBlock;

            mEdidActive.SizeOfEdid = mEdidDiscovered.SizeOfEdid;
            mEdidActive.Edid = mEdidDiscovered.Edid;
          }
          break;
        }
      }
    }
  }

  // If video mode was not discovered, set 800x600 as the only active mode
  if (Index >= MaxMode) {
        SetVideoModePcds (mDisplayModes[SVGA].Horizontal.Resolution,
                          mDisplayModes[SVGA].Vertical.Resolution);
        mDisplayModes[SVGA].IsActive = TRUE;
  }

  for (Index = 0; Index < MaxMode; ++Index) {
    if (mDisplayModes[Index].IsActive) {
      ++mActiveDisplayModes;
    }
  }

Out:
  mDisplayInitialized = TRUE;

  if (mActiveDisplayModes > 0) {
    mInitializeDisplayStatus = EFI_SUCCESS;
  }

  return mInitializeDisplayStatus;
}

/** Baikal VDU Platform specific initialization function.

  @param[in] Handle              Handle to the LCD device instance.

  @retval EFI_SUCCESS            Plaform library initialized successfully.
  @retval EFI_UNSUPPORTED        PcdGopPixelFormat must be
                                 PixelRedGreenBlueReserved8BitPerColor OR
                                 PixelBlueGreenRedReserved8BitPerColor
                                 any other format is not supported
  @retval !(EFI_SUCCESS)         Other errors.
**/
EFI_STATUS
LcdPlatformInitializeDisplay (
  IN EFI_HANDLE Handle
  )
{
  EFI_STATUS  Status;

  Status = InitializeDisplay ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Install the EDID Protocols
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiEdidDiscoveredProtocolGuid,
                  &mEdidDiscovered,
                  &gEfiEdidActiveProtocolGuid,
                  &mEdidActive,
                  NULL
                  );
  return Status;
}

/** Allocate VRAM memory in DRAM for the framebuffer
  (unless it is reserved already).

  The allocated address can be used to set the framebuffer.

  @param[out] VramBaseAddress     A pointer to the framebuffer address.
  @param[out] VramSize            A pointer to the size of the framebuffer
                                  in bytes

  @retval EFI_SUCCESS             Framebuffer memory allocated successfully.
  @retval !(EFI_SUCCESS)          Other errors.
**/
EFI_STATUS
LcdPlatformGetVram (
  OUT EFI_PHYSICAL_ADDRESS  *VramBaseAddress,
  OUT UINTN                 *VramSize
  )
{
  EFI_STATUS            Status;
  EFI_CPU_ARCH_PROTOCOL *Cpu;

  //ASSERT (VramBaseAddress != NULL);
  //ASSERT (VramSize != NULL);

  *VramBaseAddress = (EFI_PHYSICAL_ADDRESS) (SIZE_4GB - 1);
  *VramSize = BAIKAL_LCD_VRAM_SIZE;

  // Allocate the VRAM from the DRAM so that nobody else uses it.
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiReservedMemoryType,
                  EFI_SIZE_TO_PAGES ((UINTN)BAIKAL_LCD_VRAM_SIZE),
                  VramBaseAddress
                  );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  // Ensure the Cpu architectural protocol is already installed
  Status = gBS->LocateProtocol (
                  &gEfiCpuArchProtocolGuid,
                  NULL,
                  (VOID **)&Cpu
                  );
  if (!EFI_ERROR (Status)) {
    // The VRAM is inside the DRAM, which is cacheable.
    // Mark the VRAM as write-combining (uncached) and non-executable.
    Status = Cpu->SetMemoryAttributes (
                    Cpu,
                    *VramBaseAddress,
                    *VramSize,
                    EFI_MEMORY_WC | EFI_MEMORY_XP
                    );
  }
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    gBS->FreePages (*VramBaseAddress, EFI_SIZE_TO_PAGES (*VramSize));
  }

  return Status;
}

/** Return total number of modes supported.

  Note: Valid mode numbers are 0 to MaxMode - 1
  See Section 12.9 of the UEFI Specification 2.7

  @retval UINT32             Mode Number.
**/
UINT32
LcdPlatformGetMaxMode (
  VOID
  )
{
  InitializeDisplay ();

  return mActiveDisplayModes;
}

EFI_STATUS
LcdPlatformSetMode (
  IN UINT32  ModeNumber
  )
{
  EFI_STATUS    Status;
  DISPLAY_MODE  *DisplayMode;

  Status = FindActiveDisplayMode (ModeNumber, &DisplayMode);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Set the video mode pixel frequency
  BaikalSetVduFrequency (
    BM1000_HDMI_CMU_BASE,
    FixedPcdGet32 (PcdHdmiRefFrequency),
    DisplayMode->OscFreq
    );

  BaikalSetVduFrequency (
    BM1000_LVDS_CMU_BASE,
    FixedPcdGet32 (PcdLvdsRefFrequency),
    DisplayMode->OscFreq * 7
    );

  return Status;
}

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
  )
{
  EFI_STATUS    Status;
  DISPLAY_MODE  *DisplayMode;

  ASSERT (Info != NULL);

  Status = FindActiveDisplayMode (ModeNumber, &DisplayMode);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Info->Version = 0;
  Info->HorizontalResolution = DisplayMode->Horizontal.Resolution;
  Info->VerticalResolution = DisplayMode->Vertical.Resolution;
  Info->PixelsPerScanLine = DisplayMode->Horizontal.Resolution;
  Info->PixelInformation.RedMask = 0;
  Info->PixelInformation.GreenMask = 0;
  Info->PixelInformation.BlueMask = 0;
  Info->PixelInformation.ReservedMask = 0;
  Info->PixelFormat = FixedPcdGet32 (PcdGopPixelFormat);

  return EFI_SUCCESS;
}

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
  IN  UINT32        ModeNumber,
  OUT SCAN_TIMINGS  **Horizontal,
  OUT SCAN_TIMINGS  **Vertical
  )
{
  EFI_STATUS    Status;
  DISPLAY_MODE  *DisplayMode;

  // One of the pointers is NULL
  ASSERT (Horizontal != NULL);
  ASSERT (Vertical != NULL);

  Status = FindActiveDisplayMode (ModeNumber, &DisplayMode);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *Horizontal = &DisplayMode->Horizontal;
  *Vertical   = &DisplayMode->Vertical;

  return EFI_SUCCESS;
}

/** Return HDMI PHY settings for the requested mode number.

  @param[in]  ModeNumber          Mode Number.

  @param[out] PhySettings         Pointer to HDMI PHY settings.

  @retval EFI_SUCCESS             HDMI PHY settings for the requested
                                  mode returned successfully.
  @retval EFI_INVALID_PARAMETER   Requested mode not found.
**/
EFI_STATUS
LcdPlatformGetHdmiPhySettings (
  IN  UINT32            ModeNumber,
  OUT HDMI_PHY_SETTINGS **PhySettings
  )
{
  EFI_STATUS    Status;
  DISPLAY_MODE  *DisplayMode;

  ASSERT (PhySettings != NULL);

  Status = FindActiveDisplayMode (ModeNumber, &DisplayMode);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *PhySettings = &DisplayMode->PhySettings;

  return EFI_SUCCESS;
}

/** Return bits per pixel information for a mode number.

  @param[in]  ModeNumber          Mode Number.

  @param[out] Bpp                 Pointer to bits per pixel information.

  @retval EFI_SUCCESS             Bits per pixel information for the requested
                                  mode returned successfully.
  @retval EFI_INVALID_PARAMETER   Requested mode not found.
**/
EFI_STATUS
LcdPlatformGetBpp (
  IN  UINT32  ModeNumber,
  OUT LCD_BPP *Bpp
  )
{
  EFI_STATUS    Status;
  DISPLAY_MODE  *DisplayMode;

  ASSERT (Bpp != NULL);

  Status = FindActiveDisplayMode (ModeNumber, &DisplayMode);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *Bpp = DisplayMode->Bpp;

  return EFI_SUCCESS;
}

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
  IN  UINT32  ModeNumber,
  OUT UINT32  *LvdsPorts,
  OUT UINT32  *LvdsOutBpp
  )
{
  EFI_STATUS    Status;
  DISPLAY_MODE  *DisplayMode;

  ASSERT (LvdsPorts != NULL);
  ASSERT (LvdsOutBpp != NULL);

  Status = FindActiveDisplayMode (ModeNumber, &DisplayMode);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *LvdsPorts = DisplayMode->LvdsPorts;
  *LvdsOutBpp = DisplayMode->LvdsOutBpp;

  return EFI_SUCCESS;
}
