/** @file
  This file contains Baikal VDU driver functions

  Copyright (c) 2019 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  Author: Pavel Parkhomenko <Pavel.Parkhomenko@baikalelectronics.ru>

  Parts of this file were based on sources as follows:

  Copyright (c) 2011-2018, ARM Ltd. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/ArmSmcLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/LcdHwLib.h>
#include <Library/LcdPlatformLib.h>
#include <Library/BaikalVduPlatformLib.h>
#include <Library/TimerLib.h>
#include <Library/MemoryAllocationLib.h>
#include "BaikalHdmi.h"
#include "BaikalVdu.h"

#define BAIKAL_SMC_CMU_CMD           0xC2000000
#define BAIKAL_SMC_CMU_PLL_SET_RATE  0

EFI_STATUS
BaikalSetVduFrequency (
  IN CONST UINT32  CmuBase,
  IN CONST UINT32  RefFreq,
  IN CONST UINT32  OscFreq
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_CMU_CMD;
  ArmSmcArgs.Arg1 = CmuBase;
  ArmSmcArgs.Arg2 = BAIKAL_SMC_CMU_PLL_SET_RATE;
  ArmSmcArgs.Arg3 = OscFreq;
  ArmSmcArgs.Arg4 = RefFreq;

  ArmCallSmc (&ArmSmcArgs);
  if (ArmSmcArgs.Arg0) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
LcdIdentifyHdmi (
  VOID
  )
{
  // Check if this is a Baikal VDU
  if (MmioRead32 (BAIKAL_VDU_CIR(BM1000_HDMI_VDU_BASE)) == BAIKAL_VDU_PERIPH_ID) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
LcdIdentifyLvds (
  VOID
  )
{
  // Check if this is a Baikal VDU
  if (MmioRead32 (BAIKAL_VDU_CIR(BM1000_LVDS_VDU_BASE)) == BAIKAL_VDU_PERIPH_ID) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
LcdSetFramebufferBase (
  IN EFI_PHYSICAL_ADDRESS  VduBase,
  IN EFI_PHYSICAL_ADDRESS  VramBaseAddress
  )
{
  // Define start of the VRAM. This never changes for any graphics mode
  MmioWrite32 (BAIKAL_VDU_DBAR(VduBase), (UINT32) VramBaseAddress);

  // Disable all interrupts from the VDU
  MmioWrite32 (BAIKAL_VDU_IMR(VduBase), 0);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
LcdSetupHdmi (
  IN UINT32  ModeNumber
  )
{
  EFI_STATUS         Status;
  SCAN_TIMINGS       *Horizontal;
  SCAN_TIMINGS       *Vertical;
  HDMI_PHY_SETTINGS  *PhySettings;

  // Set the video mode timings and other relevant information
  Status = LcdPlatformGetTimings (
             ModeNumber,
             &Horizontal,
             &Vertical
             );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  ASSERT (Horizontal != NULL);
  ASSERT (Vertical != NULL);

  Status = LcdPlatformGetHdmiPhySettings (
             ModeNumber,
             &PhySettings
             );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  ASSERT (PhySettings != NULL);

  Status = HdmiInit (
             Horizontal,
             Vertical,
             PhySettings
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
LcdSetTimings (
  IN EFI_PHYSICAL_ADDRESS  VduBase,
  IN SCAN_TIMINGS          *Horizontal,
  IN SCAN_TIMINGS          *Vertical,
  IN LCD_BPP               LcdBpp,
  IN BOOLEAN               IsBgr,
  IN UINT32                LvdsPorts,
  IN UINT32                LvdsOutBpp
  )
{
  UINT32  BufSize;
  UINT32  LcdControl;
  UINT32  Hfp = Horizontal->FrontPorch;
  UINT32  Hsw = Horizontal->Sync;

  if (VduBase == BM1000_LVDS_VDU_BASE && LvdsPorts == 2) {
    --Hfp;
  } else {
    --Hsw;
  }

  MmioWrite32 (
    BAIKAL_VDU_HTR(VduBase),
    HOR_AXIS_PANEL (
      Horizontal->BackPorch,
      Hfp,
      Hsw,
      Horizontal->Resolution
      )
    );

  if (Horizontal->Resolution > 4080 || Horizontal->Resolution % 16 != 0) {
    MmioWrite32 (
      BAIKAL_VDU_HPPLOR(VduBase),
      Horizontal->Resolution | BAIKAL_VDU_HPPLOR_HPOE
      );
  }

  MmioWrite32 (
    BAIKAL_VDU_VTR1(VduBase),
    VER_AXIS_PANEL (
      Vertical->BackPorch,
      Vertical->FrontPorch,
      Vertical->Sync
      )
    );

  MmioWrite32 (
    BAIKAL_VDU_VTR2(VduBase),
    Vertical->Resolution
    );

  MmioWrite32 (
    BAIKAL_VDU_HVTER(VduBase),
    TIMINGS_EXT (
      Horizontal->BackPorch,
      Hfp,
      Hsw,
      Vertical->BackPorch,
      Vertical->FrontPorch,
      Vertical->Sync
      )
    );

  // Set control register
  LcdControl = BAIKAL_VDU_CR1_DEP | BAIKAL_VDU_CR1_FDW_16_WORDS;
  if (VduBase == BM1000_HDMI_VDU_BASE) {
     LcdControl |= BAIKAL_VDU_CR1_LCE | BAIKAL_VDU_CR1_OPS_LCD24;
  } else { // VduBase == BM1000_LVDS_VDU_BASE
    if (LvdsEnabled ()) {
      if (LvdsOutBpp == 24) {
        LcdControl |= BAIKAL_VDU_CR1_LCE | BAIKAL_VDU_CR1_OPS_LCD24;
      } else { // LvdsOutBpp == 18
        LcdControl |= BAIKAL_VDU_CR1_LCE | BAIKAL_VDU_CR1_OPS_LCD18;
      }
    }
  }

  if (IsBgr) {
    LcdControl |= BAIKAL_VDU_CR1_BGR;
  }

  BufSize = Horizontal->Resolution * Vertical->Resolution;

  switch (LcdBpp) {
  case LcdBitsPerPixel_1:
    LcdControl |= BAIKAL_VDU_CR1_BPP1;
    BufSize /= 8;
    break;
  case LcdBitsPerPixel_2:
    LcdControl |= BAIKAL_VDU_CR1_BPP2;
    BufSize /= 4;
    break;
  case LcdBitsPerPixel_4:
    LcdControl |= BAIKAL_VDU_CR1_BPP4;
    BufSize /= 2;
    break;
  case LcdBitsPerPixel_8:
    LcdControl |= BAIKAL_VDU_CR1_BPP8;
    break;
  case LcdBitsPerPixel_16_555:
    LcdControl |= BAIKAL_VDU_CR1_OPS_555;
  case LcdBitsPerPixel_16_565:
    LcdControl |= BAIKAL_VDU_CR1_BPP16;
    BufSize *= 2;
    break;
  case LcdBitsPerPixel_24:
    LcdControl |= BAIKAL_VDU_CR1_BPP24;
    BufSize *= 4;
    break;
  default:
    return EFI_NOT_FOUND;
  }

  MmioWrite32 (BAIKAL_VDU_PCTR(VduBase), BAIKAL_VDU_PCTR_PCR + BAIKAL_VDU_PCTR_PCI);
  MmioWrite32 (BAIKAL_VDU_MRR(VduBase),
    ((MmioRead32 (BAIKAL_VDU_DBAR(VduBase)) + BufSize - 1) & BAIKAL_VDU_MRR_DEAR_MRR_MASK) | BAIKAL_VDU_MRR_OUTSTND_RQ(4)
    );
  MmioWrite32 (BAIKAL_VDU_CR1(VduBase), LcdControl);

  if (VduBase == BM1000_LVDS_VDU_BASE) {
    switch (LvdsPorts) {
    case 4:
      MmioWrite32 (
        BAIKAL_VDU_GPIOR(VduBase),
        BAIKAL_VDU_GPIOR_UHD_ENB + BAIKAL_VDU_GPIOR_UHD_QUAD_PORT
        );
      break;
    case 2:
      MmioWrite32 (
        BAIKAL_VDU_GPIOR(VduBase),
        BAIKAL_VDU_GPIOR_UHD_ENB + BAIKAL_VDU_GPIOR_UHD_DUAL_PORT
        );
      break;
    case 1:
      MmioWrite32 (
        BAIKAL_VDU_GPIOR(VduBase),
        BAIKAL_VDU_GPIOR_UHD_ENB + BAIKAL_VDU_GPIOR_UHD_SNGL_PORT
        );
      break;
    }
  }

  return EFI_SUCCESS;
}

/** Check for presence of a Baikal VDU.

  @retval EFI_SUCCESS          Returns success if platform implements a
                               Baikal VDU controller.

  @retval EFI_NOT_FOUND        Baikal VDU controller not found.
**/
EFI_STATUS
LcdIdentify (
  VOID
  )
{
  if (LcdIdentifyHdmi () == EFI_SUCCESS && LcdIdentifyLvds () == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

/** Initialize display.

  @param[in]  VramBaseAddress    Address of the framebuffer.

  @retval EFI_SUCCESS            Initialization of display successful.
**/
EFI_STATUS
LcdInitialize (
  IN EFI_PHYSICAL_ADDRESS  VramBaseAddress
  )
{
  LcdSetFramebufferBase (BM1000_HDMI_VDU_BASE, VramBaseAddress);
  LcdSetFramebufferBase (BM1000_LVDS_VDU_BASE, VramBaseAddress);
  return EFI_SUCCESS;
}

/** Set requested mode of the display.

  @param[in] ModeNumber          Display mode number.

  @retval EFI_SUCCESS            Display mode set successfuly.
  @retval !(EFI_SUCCESS)         Other errors.
**/
EFI_STATUS
LcdSetMode (
  IN UINT32  ModeNumber
  )
{
  EFI_STATUS    Status;
  SCAN_TIMINGS  *Horizontal;
  SCAN_TIMINGS  *Vertical;
  LCD_BPP       LcdBpp;
  BOOLEAN       IsBgr;
  UINT32        LvdsPorts;
  UINT32        LvdsOutBpp;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  ModeInfo;

  LcdSetupHdmi (ModeNumber);

  // Set the video mode timings and other relevant information
  Status = LcdPlatformGetTimings (
             ModeNumber,
             &Horizontal,
             &Vertical
             );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  ASSERT (Horizontal != NULL);
  ASSERT (Vertical != NULL);

  Status = LcdPlatformGetBpp (ModeNumber, &LcdBpp);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  // Get the pixel format information
  Status = LcdPlatformQueryMode (ModeNumber, &ModeInfo);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  if (ModeInfo.PixelFormat == PixelBlueGreenRedReserved8BitPerColor) {
    IsBgr = TRUE;
  } else {
    IsBgr = FALSE;
  }

  LcdSetTimings (BM1000_HDMI_VDU_BASE, Horizontal, Vertical, LcdBpp, IsBgr, 0, 0);

  if (LvdsEnabled ()) {
    Status = LcdPlatformGetLvdsInfo (ModeNumber, &LvdsPorts, &LvdsOutBpp);
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
    Status = LcdPlatformGetLvdsTimings (
               &Horizontal,
               &Vertical
               );
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
    LcdSetTimings (BM1000_LVDS_VDU_BASE, Horizontal, Vertical, LcdBpp, IsBgr, LvdsPorts, LvdsOutBpp);
  }

  return EFI_SUCCESS;
}

VOID
HdmiShutdown (
  VOID
  )
{
  MmioAnd32 (BAIKAL_VDU_CR1(BM1000_HDMI_VDU_BASE), ~BAIKAL_VDU_CR1_LCE);
}

VOID
LvdsShutdown (
  VOID
  )
{
  MmioAnd32 (BAIKAL_VDU_CR1(BM1000_LVDS_VDU_BASE), ~BAIKAL_VDU_CR1_LCE);
}

// De-initializes the display
VOID
LcdShutdown (
  VOID
  )
{
  // Disable the controllers
  HdmiShutdown();
  LvdsShutdown();
}
