/** @file BaikalHdmi.c

  This file contains Baikal HDMI TX support functions

  Copyright (C) 2020 Baikal Electronics JSC

  Author: Pavel Parkhomenko <Pavel.Parkhomenko@baikalelectronics.ru>

**/

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/LcdHwLib.h>
#include <Library/BaikalVduPlatformLib.h>
#include <Library/TimerLib.h>

#include "BaikalHdmi.h"
#include "BaikalVdu.h"

BOOLEAN
HdmiPhyWaitI2CDone(
  IN UINT32 Timeout
  )
{
    UINT32 Val;

    while ((Val = MmioRead32(BAIKAL_HDMI_IH_I2CMPHY_STAT0) & 0x3) == 0) {
        if (Timeout-- == 0)
            return FALSE;
        MicroSecondDelay(1000);
    }
    MmioWrite32(BAIKAL_HDMI_IH_I2CMPHY_STAT0, Val);

    return TRUE;
}

VOID
HdmiPhyI2CWrite(
  IN UINT8 Addr,
  IN UINT16 Data
  )
{
  MmioWrite32(BAIKAL_HDMI_IH_I2CMPHY_STAT0, 0xFF);
  MmioWrite32(BAIKAL_HDMI_PHY_I2CM_ADDRESS_ADDR, Addr);
  MmioWrite32(BAIKAL_HDMI_PHY_I2CM_DATAO_1_ADDR, Data >> 8);
  MmioWrite32(BAIKAL_HDMI_PHY_I2CM_DATAO_0_ADDR, (Data >> 0) & 0xFF) ;
  MmioWrite32(BAIKAL_HDMI_PHY_I2CM_OPERATION_ADDR, BAIKAL_HDMI_PHY_I2CM_OPERATION_ADDR_WRITE);
  HdmiPhyWaitI2CDone(1000);
}

VOID
HdmiPhyPowerOff(
  VOID
  )
{
  UINTN i;
  UINT32 Val;
  
  Val = MmioRead32(BAIKAL_HDMI_PHY_CONF0);
  Val &= ~BAIKAL_HDMI_PHY_CONF0_GEN2_TXPWRON_MASK;
  MmioWrite32(BAIKAL_HDMI_PHY_CONF0, Val);

  /*
   * Wait for TX_PHY_LOCK to be deasserted to indicate that the PHY went
   * to low power mode.
   */ 
  for (i = 0; i < 5; ++i) {
    Val = MmioRead32(BAIKAL_HDMI_PHY_STAT0);
    if (!(Val & BAIKAL_HDMI_PHY_TX_PHY_LOCK))
      break;

    MicroSecondDelay(1000);
  }

  if (Val & BAIKAL_HDMI_PHY_TX_PHY_LOCK)
    DEBUG((DEBUG_ERROR, "HDMI PHY failed to power down\n"));
  else
    DEBUG((DEBUG_INFO, "HDMI PHY powered down in %u iterations\n", i));

  Val = MmioRead32(BAIKAL_HDMI_PHY_CONF0);
  Val |= BAIKAL_HDMI_PHY_CONF0_GEN2_PDDQ_MASK;
  MmioWrite32(BAIKAL_HDMI_PHY_CONF0, Val);

}

EFI_STATUS
HdmiPhyPowerOn(
  VOID
  )
{
  UINTN i;
  UINT32 Val;

  Val = MmioRead32(BAIKAL_HDMI_PHY_CONF0);
  Val |= BAIKAL_HDMI_PHY_CONF0_GEN2_TXPWRON_MASK;
  Val &= ~BAIKAL_HDMI_PHY_CONF0_GEN2_PDDQ_MASK;
  MmioWrite32(BAIKAL_HDMI_PHY_CONF0, Val);
  
  /* Wait for PHY PLL lock */
  for (i = 0; i < 5; ++i) {
    Val = MmioRead32(BAIKAL_HDMI_PHY_STAT0) & BAIKAL_HDMI_PHY_TX_PHY_LOCK;
    if (Val)
      break;

    MicroSecondDelay(1000);
  }

  if (!Val) {
    DEBUG((DEBUG_ERROR, "HDMI PHY PLL failed to lock\n"));
    return EFI_NOT_FOUND;
  }

  DEBUG((DEBUG_INFO, "HDMI PHY PLL locked %u iterations\n", i));
  return EFI_SUCCESS;
}

EFI_STATUS 
HdmiPhyConfigure(
  IN HDMI_PHY_SETTINGS *PhySettings
  )
{
  UINT32 Val;
	
  HdmiPhyPowerOff();

  /* Leave low power consumption mode by asserting SVSRET. */
  Val = MmioRead32(BAIKAL_HDMI_PHY_CONF0);
  Val |= BAIKAL_HDMI_PHY_CONF0_SPARECTRL_MASK;
  MmioWrite32(BAIKAL_HDMI_PHY_CONF0, Val);

  /* PHY reset. The reset signal is active high on Gen2 PHYs. */
  MmioWrite32(BAIKAL_HDMI_MC_PHYRSTZ, BAIKAL_HDMI_MC_PHYRSTZ_DEASSERT);
  MmioWrite32(BAIKAL_HDMI_MC_PHYRSTZ, 0);

  Val = MmioRead32(BAIKAL_HDMI_MC_HEACPHY_RST);
  Val |= BAIKAL_HDMI_MC_HEACPHY_RST_ASSERT;
  MmioWrite32(BAIKAL_HDMI_MC_HEACPHY_RST, Val);

  Val = MmioRead32(BAIKAL_HDMI_PHY_TST0);
  Val |= BAIKAL_HDMI_PHY_TST0_TSTCLR_MASK;
  MmioWrite32(BAIKAL_HDMI_PHY_TST0, Val);

  MmioWrite32(BAIKAL_HDMI_PHY_I2CM_SLAVE_ADDR, BAIKAL_HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2);

  Val = MmioRead32(BAIKAL_HDMI_PHY_TST0);
  Val &= ~BAIKAL_HDMI_PHY_TST0_TSTCLR_MASK;
  MmioWrite32(BAIKAL_HDMI_PHY_TST0, Val);

  HdmiPhyI2CWrite(BAIKAL_HDMI_PHY_OPMODE_PLLCFG, PhySettings->Opmode);
  HdmiPhyI2CWrite(BAIKAL_HDMI_PHY_PLLCURRCTRL, PhySettings->Current);
  HdmiPhyI2CWrite(BAIKAL_HDMI_PHY_PLLGMPCTRL, PhySettings->Gmp);
  HdmiPhyI2CWrite(BAIKAL_HDMI_PHY_TXTERM, PhySettings->TxTer);
  HdmiPhyI2CWrite(BAIKAL_HDMI_PHY_VLEVCTRL, PhySettings->VlevCtrl);
  HdmiPhyI2CWrite(BAIKAL_HDMI_PHY_CKSYMTXCTRL, PhySettings->CkSymTxCtrl);

  return HdmiPhyPowerOn();

}

/* HDMI Initialization Step B.1 */
VOID
HdmiInitAvComposer(
  IN SCAN_TIMINGS *Horizontal,
  IN SCAN_TIMINGS *Vertical
  )
{
  UINT16 HBlank;
  UINT8 VBlank;
  UINT8 Val;
  UINT8 Mask;
  HBlank = Horizontal->FrontPorch + Horizontal->Sync + Horizontal->BackPorch;
  VBlank = Vertical->FrontPorch + Vertical->Sync + Vertical->BackPorch;
  Val = MmioRead32(BAIKAL_HDMI_TX_INVID0);
  Val |= (1 << BAIKAL_HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET);
  MmioWrite32(BAIKAL_HDMI_TX_INVID0, Val);
  Val = MmioRead32(BAIKAL_HDMI_FC_INVIDCONF);
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

  MmioWrite32(BAIKAL_HDMI_FC_INVIDCONF, Val);
  MmioWrite32(BAIKAL_HDMI_FC_INHACTV1, Horizontal->Resolution >> 8);
  MmioWrite32(BAIKAL_HDMI_FC_INHACTV0, (Horizontal->Resolution >> 0) & 0xFF);
  MmioWrite32(BAIKAL_HDMI_FC_INVACTV1, Vertical->Resolution >> 8);
  MmioWrite32(BAIKAL_HDMI_FC_INVACTV0, (Vertical->Resolution >> 0) & 0xFF);
  MmioWrite32(BAIKAL_HDMI_FC_INHBLANK1, HBlank >> 8);
  MmioWrite32(BAIKAL_HDMI_FC_INHBLANK0, (HBlank >> 0) & 0xFF);
  MmioWrite32(BAIKAL_HDMI_FC_INVBLANK, VBlank);
  MmioWrite32(BAIKAL_HDMI_FC_HSYNCINDELAY1, Horizontal->FrontPorch >> 8);
  MmioWrite32(BAIKAL_HDMI_FC_HSYNCINDELAY0, (Horizontal->FrontPorch >> 0) & 0xFF);
  MmioWrite32(BAIKAL_HDMI_FC_VSYNCINDELAY, Vertical->FrontPorch);
  MmioWrite32(BAIKAL_HDMI_FC_HSYNCINWIDTH1, Horizontal->Sync >> 8);
  MmioWrite32(BAIKAL_HDMI_FC_HSYNCINWIDTH0, (Horizontal->Sync >> 0) & 0xFF);
  MmioWrite32(BAIKAL_HDMI_FC_VSYNCINWIDTH, Vertical->Sync);

  Val = MmioRead32(BAIKAL_HDMI_FC_INVIDCONF);
  Val &= ~BAIKAL_HDMI_FC_INVIDCONF_DVI_MODEZ_MASK;
  Val |= BAIKAL_HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE;
  MmioWrite32(BAIKAL_HDMI_FC_INVIDCONF, Val);
}

/* HDMI Initialization Step B.2 */
EFI_STATUS
HdmiPhyInit(
  IN HDMI_PHY_SETTINGS *PhySettings
  )
{
  UINTN i;
  UINT32 Val;
  EFI_STATUS Ret;

  /* HDMI Phy spec says to do the phy initialization sequence twice */
  for (i = 0; i < 2; i++) {
    
    Val = MmioRead32(BAIKAL_HDMI_PHY_CONF0);
    Val |= BAIKAL_HDMI_PHY_CONF0_SELDATAENPOL_MASK;
    Val &= ~BAIKAL_HDMI_PHY_CONF0_SELDIPIF_MASK;
    MmioWrite32(BAIKAL_HDMI_PHY_CONF0, Val);
 
    if ((Ret = HdmiPhyConfigure(PhySettings)) != EFI_SUCCESS)
      return Ret;
  }

  return EFI_SUCCESS;
}

/* HDMI Initialization Step B.3 */
VOID
HdmiEnableVideoPath(
  UINT32 Sync
  )
{
  UINT32 Val;

  /* control period minimum duration */
  MmioWrite32(BAIKAL_HDMI_FC_CTRLDUR, 12);
  MmioWrite32(BAIKAL_HDMI_FC_EXCTRLDUR, 32);
  MmioWrite32(BAIKAL_HDMI_FC_EXCTRLSPAC, 1);

  /* Set to fill TMDS data channels */
  MmioWrite32(BAIKAL_HDMI_FC_CH0PREAM, 0x0B);
  MmioWrite32(BAIKAL_HDMI_FC_CH1PREAM, 0x16);
  MmioWrite32(BAIKAL_HDMI_FC_CH2PREAM, 0x21);

  /* Enable pixel clock and tmds data path */
  Val = 0x7F;
  Val &= ~BAIKAL_HDMI_MC_CLKDIS_PIXELCLK_DISABLE;
  MmioWrite32(BAIKAL_HDMI_MC_CLKDIS, Val);

  Val &= ~BAIKAL_HDMI_MC_CLKDIS_TMDSCLK_DISABLE;
  MmioWrite32(BAIKAL_HDMI_MC_CLKDIS, Val);

  /* After each CLKDIS reset it is mandatory to
     set up VSYNC active edge delay (in lines) */
  MmioWrite32(BAIKAL_HDMI_FC_VSYNCINWIDTH, Sync);
}


