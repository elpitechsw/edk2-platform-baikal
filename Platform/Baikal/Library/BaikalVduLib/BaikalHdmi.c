/** @file
  This file contains Baikal HDMI TX support functions

  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  Author: Pavel Parkhomenko <Pavel.Parkhomenko@baikalelectronics.ru>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaikalVduPlatformLib.h>
#include <Library/TimerLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include "BaikalHdmi.h"

#define DDC_I2C_ADDR  0x50

STATIC CONST UINT8  mEdidSignature[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};

STATIC
EFI_STATUS
HdmiWaitI2cDone (
  IN UINT32 Timeout
  )
{
  UINT32  Val;

  while (TRUE) {
    Val = MmioRead32 (BAIKAL_HDMI_IH_I2CM_STAT0);
    Val &= BAIKAL_HDMI_IH_I2CM_STAT0_DONE |
           BAIKAL_HDMI_IH_I2CM_STAT0_ERROR;

    if (Val != 0) {
      MmioWrite32 (BAIKAL_HDMI_IH_I2CM_STAT0, Val);
      if ((Val & BAIKAL_HDMI_IH_I2CM_STAT0_ERROR) != 0) {
        return EFI_DEVICE_ERROR;
      }
      return EFI_SUCCESS;
    }

    if (Timeout-- == 0) {
      return EFI_TIMEOUT;
    }
    MicroSecondDelay (1000);
  }
}

STATIC
EFI_STATUS
HdmiPhyWaitI2cDone (
  IN UINT32 Timeout
  )
{
  UINT32  Val;

  while (TRUE) {
    Val = MmioRead32 (BAIKAL_HDMI_IH_I2CMPHY_STAT0);
    Val &= BAIKAL_HDMI_IH_I2CMPHY_STAT0_DONE |
           BAIKAL_HDMI_IH_I2CMPHY_STAT0_ERROR;

    if (Val != 0) {
      MmioWrite32 (BAIKAL_HDMI_IH_I2CMPHY_STAT0, Val);
      if ((Val & BAIKAL_HDMI_IH_I2CMPHY_STAT0_ERROR) != 0) {
        return EFI_DEVICE_ERROR;
      }
      return EFI_SUCCESS;
    }

    if (Timeout-- == 0) {
      return EFI_TIMEOUT;
    }
    MicroSecondDelay (1000);
  }
}

STATIC
EFI_STATUS
HdmiPhyI2cWrite (
  IN UINT8  Addr,
  IN UINT16 Data
  )
{
  EFI_STATUS  Status;

  MmioWrite32 (BAIKAL_HDMI_IH_I2CMPHY_STAT0, 0xFF);
  MmioWrite32 (BAIKAL_HDMI_PHY_I2CM_ADDRESS_ADDR, Addr);
  MmioWrite32 (BAIKAL_HDMI_PHY_I2CM_DATAO_1_ADDR, Data >> 8);
  MmioWrite32 (BAIKAL_HDMI_PHY_I2CM_DATAO_0_ADDR, (Data >> 0) & 0xFF) ;
  MmioWrite32 (BAIKAL_HDMI_PHY_I2CM_OPERATION_ADDR, BAIKAL_HDMI_PHY_I2CM_OPERATION_ADDR_WRITE);

  Status = HdmiPhyWaitI2cDone (1000);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

STATIC
VOID
HdmiPhyPowerOff (VOID)
{
  UINTN   i;
  UINT32  Val;

  Val = MmioRead32 (BAIKAL_HDMI_PHY_CONF0);
  Val &= ~BAIKAL_HDMI_PHY_CONF0_GEN2_TXPWRON_MASK;
  MmioWrite32 (BAIKAL_HDMI_PHY_CONF0, Val);

  /*
   * Wait for TX_PHY_LOCK to be deasserted to indicate that the PHY went
   * to low power mode.
   */
  for (i = 0; i < 5; ++i) {
    Val = MmioRead32 (BAIKAL_HDMI_PHY_STAT0);
    if ((Val & BAIKAL_HDMI_PHY_TX_PHY_LOCK) == 0) {
      break;
    }

    MicroSecondDelay (1000);
  }

  if (Val & BAIKAL_HDMI_PHY_TX_PHY_LOCK) {
    DEBUG ((DEBUG_ERROR, "HDMI PHY failed to power down\n"));
  } else {
    DEBUG ((DEBUG_INFO, "HDMI PHY powered down in %u iterations\n", i));
  }

  Val = MmioRead32 (BAIKAL_HDMI_PHY_CONF0);
  Val |= BAIKAL_HDMI_PHY_CONF0_GEN2_PDDQ_MASK;
  MmioWrite32 (BAIKAL_HDMI_PHY_CONF0, Val);
}

STATIC
EFI_STATUS
HdmiPhyPowerOn (VOID)
{
  UINTN   i;
  UINT32  Val;

  Val = MmioRead32 (BAIKAL_HDMI_PHY_CONF0);
  Val |= BAIKAL_HDMI_PHY_CONF0_GEN2_TXPWRON_MASK;
  Val &= ~BAIKAL_HDMI_PHY_CONF0_GEN2_PDDQ_MASK;
  MmioWrite32 (BAIKAL_HDMI_PHY_CONF0, Val);

  /* Wait for PHY PLL lock */
  for (i = 0; i < 5; ++i) {
    Val = MmioRead32 (BAIKAL_HDMI_PHY_STAT0);
    Val &= BAIKAL_HDMI_PHY_TX_PHY_LOCK;
    if (Val != 0) {
      break;
    }

    MicroSecondDelay (1000);
  }

  if (Val == 0) {
    DEBUG ((DEBUG_ERROR, "HDMI PHY PLL failed to lock\n"));
    return EFI_NOT_FOUND;
  }

  DEBUG ((DEBUG_INFO, "HDMI PHY PLL locked %u iterations\n", i));
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
HdmiPhyConfigure (
  IN CONST HDMI_PHY_SETTINGS  *PhySettings
  )
{
  UINT32  Val;

  HdmiPhyPowerOff ();

  /* Leave low power consumption mode by asserting SVSRET. */
  Val = MmioRead32 (BAIKAL_HDMI_PHY_CONF0);
  Val |= BAIKAL_HDMI_PHY_CONF0_SPARECTRL_MASK;
  MmioWrite32 (BAIKAL_HDMI_PHY_CONF0, Val);

  /* PHY reset. The reset signal is active high on Gen2 PHYs. */
  MmioWrite32 (BAIKAL_HDMI_MC_PHYRSTZ, BAIKAL_HDMI_MC_PHYRSTZ_DEASSERT);
  MmioWrite32 (BAIKAL_HDMI_MC_PHYRSTZ, 0);

  Val = MmioRead32 (BAIKAL_HDMI_MC_HEACPHY_RST);
  Val |= BAIKAL_HDMI_MC_HEACPHY_RST_ASSERT;
  MmioWrite32 (BAIKAL_HDMI_MC_HEACPHY_RST, Val);

  Val = MmioRead32 (BAIKAL_HDMI_PHY_TST0);
  Val |= BAIKAL_HDMI_PHY_TST0_TSTCLR_MASK;
  MmioWrite32 (BAIKAL_HDMI_PHY_TST0, Val);

  MmioWrite32 (BAIKAL_HDMI_PHY_I2CM_SLAVE_ADDR, BAIKAL_HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2);

  Val = MmioRead32 (BAIKAL_HDMI_PHY_TST0);
  Val &= ~BAIKAL_HDMI_PHY_TST0_TSTCLR_MASK;
  MmioWrite32 (BAIKAL_HDMI_PHY_TST0, Val);

  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_OPMODE_PLLCFG, PhySettings->Opmode);
  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_PLLCURRCTRL, PhySettings->Current);
  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_PLLGMPCTRL, PhySettings->Gmp);
  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_TXTERM, PhySettings->TxTer);
  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_VLEVCTRL, PhySettings->VlevCtrl);
  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_CKSYMTXCTRL, PhySettings->CkSymTxCtrl);

  return HdmiPhyPowerOn ();
}

/* HDMI Initialization Step B.1 */
STATIC
VOID
HdmiInitAvComposer (
  IN CONST SCAN_TIMINGS *Horizontal,
  IN CONST SCAN_TIMINGS *Vertical
  )
{
  UINT16  HBlank;
  UINT8   VBlank;
  UINT8   Val;
  UINT8   Mask;

  HBlank = Horizontal->FrontPorch + Horizontal->Sync + Horizontal->BackPorch;
  VBlank = Vertical->FrontPorch + Vertical->Sync + Vertical->BackPorch;

  Val = MmioRead32 (BAIKAL_HDMI_TX_INVID0);
  Val |= (1 << BAIKAL_HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET);
  MmioWrite32 (BAIKAL_HDMI_TX_INVID0, Val);

  Val = MmioRead32 (BAIKAL_HDMI_FC_INVIDCONF);
  Mask = BAIKAL_HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_MASK |
         BAIKAL_HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_MASK |
         BAIKAL_HDMI_FC_INVIDCONF_DE_IN_POLARITY_MASK |
         BAIKAL_HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_MASK |
         BAIKAL_HDMI_FC_INVIDCONF_IN_I_P_MASK;
  Val &= ~Mask;
  Val |= BAIKAL_HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_LOW |
         BAIKAL_HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_LOW |
         BAIKAL_HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH |
         BAIKAL_HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW |
         BAIKAL_HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE;
  MmioWrite32 (BAIKAL_HDMI_FC_INVIDCONF, Val);

  MmioWrite32 (BAIKAL_HDMI_FC_INHACTV1, Horizontal->Resolution >> 8);
  MmioWrite32 (BAIKAL_HDMI_FC_INHACTV0, (Horizontal->Resolution >> 0) & 0xFF);

  MmioWrite32 (BAIKAL_HDMI_FC_INVACTV1, Vertical->Resolution >> 8);
  MmioWrite32 (BAIKAL_HDMI_FC_INVACTV0, (Vertical->Resolution >> 0) & 0xFF);

  MmioWrite32 (BAIKAL_HDMI_FC_INHBLANK1, HBlank >> 8);
  MmioWrite32 (BAIKAL_HDMI_FC_INHBLANK0, (HBlank >> 0) & 0xFF);

  MmioWrite32 (BAIKAL_HDMI_FC_INVBLANK, VBlank);

  MmioWrite32 (BAIKAL_HDMI_FC_HSYNCINDELAY1, Horizontal->FrontPorch >> 8);
  MmioWrite32 (BAIKAL_HDMI_FC_HSYNCINDELAY0, (Horizontal->FrontPorch >> 0) & 0xFF);

  MmioWrite32 (BAIKAL_HDMI_FC_VSYNCINDELAY, Vertical->FrontPorch);

  MmioWrite32 (BAIKAL_HDMI_FC_HSYNCINWIDTH1, Horizontal->Sync >> 8);
  MmioWrite32 (BAIKAL_HDMI_FC_HSYNCINWIDTH0, (Horizontal->Sync >> 0) & 0xFF);

  MmioWrite32 (BAIKAL_HDMI_FC_VSYNCINWIDTH, Vertical->Sync);

  Val = MmioRead32 (BAIKAL_HDMI_FC_INVIDCONF);
  Val &= ~BAIKAL_HDMI_FC_INVIDCONF_DVI_MODEZ_MASK;
  Val |= BAIKAL_HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE;
  MmioWrite32 (BAIKAL_HDMI_FC_INVIDCONF, Val);
}

/* HDMI Initialization Step B.2 */
STATIC
EFI_STATUS
HdmiPhyInit (
  IN CONST HDMI_PHY_SETTINGS  *PhySettings
  )
{
  UINTN       i;
  UINT32      Val;
  EFI_STATUS  Status;

  /* HDMI Phy spec says to do the phy initialization sequence twice */
  for (i = 0; i < 2; i++) {
    Val = MmioRead32 (BAIKAL_HDMI_PHY_CONF0);
    Val |= BAIKAL_HDMI_PHY_CONF0_SELDATAENPOL_MASK;
    Val &= ~BAIKAL_HDMI_PHY_CONF0_SELDIPIF_MASK;
    MmioWrite32 (BAIKAL_HDMI_PHY_CONF0, Val);

    Status = HdmiPhyConfigure (PhySettings);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/* HDMI Initialization Step B.3 */
STATIC
VOID
HdmiEnableVideoPath (
  UINT32  Sync
  )
{
  UINT32  Val;

  /* control period minimum duration */
  MmioWrite32 (BAIKAL_HDMI_FC_CTRLDUR, 12);
  MmioWrite32 (BAIKAL_HDMI_FC_EXCTRLDUR, 32);
  MmioWrite32 (BAIKAL_HDMI_FC_EXCTRLSPAC, 1);

  /* Set to fill TMDS data channels */
  MmioWrite32 (BAIKAL_HDMI_FC_CH0PREAM, 0x0B);
  MmioWrite32 (BAIKAL_HDMI_FC_CH1PREAM, 0x16);
  MmioWrite32 (BAIKAL_HDMI_FC_CH2PREAM, 0x21);

  /* Enable pixel clock and tmds data path */
  Val = 0x7F;
  Val &= ~BAIKAL_HDMI_MC_CLKDIS_PIXELCLK_DISABLE;
  MmioWrite32 (BAIKAL_HDMI_MC_CLKDIS, Val);

  Val &= ~BAIKAL_HDMI_MC_CLKDIS_TMDSCLK_DISABLE;
  MmioWrite32 (BAIKAL_HDMI_MC_CLKDIS, Val);

  /* After each CLKDIS reset it is mandatory to
     set up VSYNC active edge delay (in lines) */
  MmioWrite32 (BAIKAL_HDMI_FC_VSYNCINWIDTH, Sync);
}

EFI_STATUS
EFIAPI
HdmiInit (
  IN CONST SCAN_TIMINGS       *Horizontal,
  IN CONST SCAN_TIMINGS       *Vertical,
  IN CONST HDMI_PHY_SETTINGS  *PhySettings
  )
{
  EFI_STATUS  Status;

  ASSERT (Horizontal != NULL);
  ASSERT (Vertical != NULL);
  ASSERT (PhySettings != NULL);

  HdmiInitAvComposer (Horizontal, Vertical);
  Status = HdmiPhyInit (PhySettings);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  HdmiEnableVideoPath (Vertical->Sync);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HdmiReadEdid (
  IN CONST SCAN_TIMINGS       *Horizontal,
  IN CONST SCAN_TIMINGS       *Vertical,
  IN CONST HDMI_PHY_SETTINGS  *PhySettings,
  OUT UINT8                   **EdidDataBlock,
  OUT UINTN                   *EdidSize
  )
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINT8       EdidData[sizeof (EDID_BLOCK)];
  UINT8       Checksum;

  // HDMI Initialization Step B
  Status = HdmiInit (Horizontal, Vertical, PhySettings);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // HDMI Initialization Step C
  MmioWrite32 (BAIKAL_HDMI_I2CM_SOFTRSTZ, 0);
  MmioWrite32 (BAIKAL_HDMI_I2CM_DIV, 0);
  MmioWrite32 (
    BAIKAL_HDMI_IH_I2CM_STAT0,
    BAIKAL_HDMI_IH_I2CM_STAT0_DONE | BAIKAL_HDMI_IH_I2CM_STAT0_ERROR
    );

  MmioWrite32 (BAIKAL_HDMI_I2CM_SLAVE, DDC_I2C_ADDR);

  for (Index = 0, Checksum = 0; Index < ARRAY_SIZE (EdidData); ++Index) {
    MmioWrite32 (BAIKAL_HDMI_I2CM_ADDRESS, Index);
    MmioWrite32 (BAIKAL_HDMI_I2CM_OPERATION, BAIKAL_HDMI_I2CM_OPERATION_READ);

    Status = HdmiWaitI2cDone (100);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    EdidData[Index] = MmioRead32 (BAIKAL_HDMI_I2CM_DATAI);

    Checksum += EdidData[Index];
  }

  if (CompareMem (EdidData, mEdidSignature, sizeof (mEdidSignature)) != 0) {
    return EFI_UNSUPPORTED;
  }

  if (Checksum != 0) {
    return EFI_CRC_ERROR;
  }

  *EdidDataBlock = AllocateCopyPool (
                     sizeof (EdidData),
                     EdidData
                     );
  if (*EdidDataBlock == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  *EdidSize = sizeof (EdidData);

  return EFI_SUCCESS;
}
