/** @file
  This file contains Baikal HDMI TX support functions

  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
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

#define HDMI_REG_WIDTH  4

// Interrupt Registers
#define BAIKAL_HDMI_IH_I2CM_STAT0            ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x0105 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_IH_I2CMPHY_STAT0         ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x0108 * HDMI_REG_WIDTH)

// Video Sample Registers
#define BAIKAL_HDMI_TX_INVID0                ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x0200 * HDMI_REG_WIDTH)

// Frame Composer Registers
#define BAIKAL_HDMI_FC_INVIDCONF             ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1000 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_INHACTV0              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1001 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_INHACTV1              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1002 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_INHBLANK0             ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1003 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_INHBLANK1             ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1004 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_INVACTV0              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1005 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_INVACTV1              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1006 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_INVBLANK              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1007 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_HSYNCINDELAY0         ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1008 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_HSYNCINDELAY1         ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1009 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_HSYNCINWIDTH0         ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x100A * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_HSYNCINWIDTH1         ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x100B * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_VSYNCINDELAY          ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x100C * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_VSYNCINWIDTH          ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x100D * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_CTRLDUR               ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1011 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_EXCTRLDUR             ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1012 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_EXCTRLSPAC            ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1013 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_CH0PREAM              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1014 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_CH1PREAM              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1015 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_CH2PREAM              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1016 * HDMI_REG_WIDTH)

// Frame Composer Debug Registers
#define BAIKAL_HDMI_FC_DBGFORCE              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1200 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_DBGTMDS0              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x1219 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_DBGTMDS1              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x121A * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_FC_DBGTMDS2              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x121B * HDMI_REG_WIDTH)

// Main Controller Registers
#define BAIKAL_HDMI_MC_CLKDIS                ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x4001 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_MC_PHYRSTZ               ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x4005 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_MC_HEACPHY_RST           ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x4007 * HDMI_REG_WIDTH)

// I2C Master Registers (E-DDC)
#define BAIKAL_HDMI_I2CM_SLAVE               ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x7E00 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_I2CM_ADDRESS             ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x7E01 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_I2CM_DATAI               ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x7E03 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_I2CM_OPERATION           ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x7E04 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_I2CM_INT                 ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x7E05 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_I2CM_CTLINT              ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x7E06 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_I2CM_DIV                 ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x7E07 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_I2CM_SOFTRSTZ            ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x7E09 * HDMI_REG_WIDTH)

// HDMI Master PHY Registers
#define BAIKAL_HDMI_PHY_CONF0                ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3000 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_PHY_TST0                 ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3001 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_PHY_STAT0                ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3004 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_PHY_I2CM_SLAVE_ADDR      ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3020 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_PHY_I2CM_ADDRESS_ADDR    ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3021 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_PHY_I2CM_DATAO_1_ADDR    ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3022 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_PHY_I2CM_DATAO_0_ADDR    ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3023 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_PHY_I2CM_DATAI_1_ADDR    ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3024 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_PHY_I2CM_DATAI_0_ADDR    ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3025 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_PHY_I2CM_OPERATION_ADDR  ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3026 * HDMI_REG_WIDTH)
#define BAIKAL_HDMI_PHY_I2CM_INT_ADDR        ((UINTN)FixedPcdGet32 (PcdHdmiBase) + 0x3027 * HDMI_REG_WIDTH)

#define BAIKAL_HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2  0x69

#define BAIKAL_HDMI_PHY_OPMODE_PLLCFG  0x06 // Mode of operation and PLL dividers
#define BAIKAL_HDMI_PHY_PLLCURRCTRL    0x10 // PLL current
#define BAIKAL_HDMI_PHY_PLLGMPCTRL     0x15 // PLL Gmp (conductance)
#define BAIKAL_HDMI_PHY_TXTERM         0x19 // Rterm
#define BAIKAL_HDMI_PHY_VLEVCTRL       0x0E // Voltage levels
#define BAIKAL_HDMI_PHY_CKSYMTXCTRL    0x09 // Tx symbols control and slope boost

enum {
  // IH_I2CM_STAT0 field values
  BAIKAL_HDMI_IH_I2CM_STAT0_DONE = 0x2,
  BAIKAL_HDMI_IH_I2CM_STAT0_ERROR = 0x1,

  // IH_I2CMPHY_STAT0 field values
  BAIKAL_HDMI_IH_I2CMPHY_STAT0_DONE = 0x2,
  BAIKAL_HDMI_IH_I2CMPHY_STAT0_ERROR = 0x1,

  // PHY_TST0 field values
  BAIKAL_HDMI_PHY_TST0_TSTCLR_MASK = 0x20,

  // PHY_STAT0 field values
  BAIKAL_HDMI_PHY_TX_PHY_LOCK = 0x01,

  // TX_INVID0 field values
  BAIKAL_HDMI_TX_INVID0_VIDEO_MAPPING_MASK = 0x1F,
  BAIKAL_HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET = 0,

  // FC_INVIDCONF field values
  BAIKAL_HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_MASK = 0x40,
  BAIKAL_HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_HIGH = 0x40,
  BAIKAL_HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_LOW = 0x00,
  BAIKAL_HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_MASK = 0x20,
  BAIKAL_HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_HIGH = 0x20,
  BAIKAL_HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_LOW = 0x00,
  BAIKAL_HDMI_FC_INVIDCONF_DE_IN_POLARITY_MASK = 0x10,
  BAIKAL_HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH = 0x10,
  BAIKAL_HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_LOW = 0x00,
  BAIKAL_HDMI_FC_INVIDCONF_DVI_MODEZ_MASK = 0x8,
  BAIKAL_HDMI_FC_INVIDCONF_DVI_MODEZ_HDMI_MODE = 0x8,
  BAIKAL_HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE = 0x0,
  BAIKAL_HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_MASK = 0x2,
  BAIKAL_HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_HIGH = 0x2,
  BAIKAL_HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW = 0x0,
  BAIKAL_HDMI_FC_INVIDCONF_IN_I_P_MASK = 0x1,
  BAIKAL_HDMI_FC_INVIDCONF_IN_I_P_INTERLACED = 0x1,
  BAIKAL_HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE = 0x0,

  // PHY_CONF0 field values
  BAIKAL_HDMI_PHY_CONF0_PDZ_MASK = 0x80,
  BAIKAL_HDMI_PHY_CONF0_PDZ_OFFSET = 7,
  BAIKAL_HDMI_PHY_CONF0_ENTMDS_MASK = 0x40,
  BAIKAL_HDMI_PHY_CONF0_ENTMDS_OFFSET = 6,
  BAIKAL_HDMI_PHY_CONF0_SPARECTRL_MASK = 0x20,
  BAIKAL_HDMI_PHY_CONF0_SPARECTRL_OFFSET = 5,
  BAIKAL_HDMI_PHY_CONF0_GEN2_PDDQ_MASK = 0x10,
  BAIKAL_HDMI_PHY_CONF0_GEN2_PDDQ_OFFSET = 4,
  BAIKAL_HDMI_PHY_CONF0_GEN2_TXPWRON_MASK = 0x8,
  BAIKAL_HDMI_PHY_CONF0_GEN2_TXPWRON_OFFSET = 3,
  BAIKAL_HDMI_PHY_CONF0_GEN2_ENHPDRXSENSE_MASK = 0x4,
  BAIKAL_HDMI_PHY_CONF0_GEN2_ENHPDRXSENSE_OFFSET = 2,
  BAIKAL_HDMI_PHY_CONF0_SELDATAENPOL_MASK = 0x2,
  BAIKAL_HDMI_PHY_CONF0_SELDATAENPOL_OFFSET = 1,
  BAIKAL_HDMI_PHY_CONF0_SELDIPIF_MASK = 0x1,
  BAIKAL_HDMI_PHY_CONF0_SELDIPIF_OFFSET = 0,

  // PHY_I2CM_OPERATION_ADDR field values
  BAIKAL_HDMI_PHY_I2CM_OPERATION_ADDR_WRITE = 0x10,
  BAIKAL_HDMI_PHY_I2CM_OPERATION_ADDR_READ = 0x1,

  // MC_CLKDIS field values
  BAIKAL_HDMI_MC_CLKDIS_HDCPCLK_DISABLE = 0x40,
  BAIKAL_HDMI_MC_CLKDIS_CECCLK_DISABLE = 0x20,
  BAIKAL_HDMI_MC_CLKDIS_CSCCLK_DISABLE = 0x10,
  BAIKAL_HDMI_MC_CLKDIS_AUDCLK_DISABLE = 0x8,
  BAIKAL_HDMI_MC_CLKDIS_PREPCLK_DISABLE = 0x4,
  BAIKAL_HDMI_MC_CLKDIS_TMDSCLK_DISABLE = 0x2,
  BAIKAL_HDMI_MC_CLKDIS_PIXELCLK_DISABLE = 0x1,

  // MC_PHYRSTZ field values
  BAIKAL_HDMI_MC_PHYRSTZ_ASSERT = 0x0,
  BAIKAL_HDMI_MC_PHYRSTZ_DEASSERT = 0x1,

  // MC_HEACPHY_RST field values
  BAIKAL_HDMI_MC_HEACPHY_RST_ASSERT = 0x1,
  BAIKAL_HDMI_MC_HEACPHY_RST_DEASSERT = 0x0,

  // I2CM_OPERATION field values
  BAIKAL_HDMI_I2CM_OPERATION_WRITE = 0x10,
  BAIKAL_HDMI_I2CM_OPERATION_READ_EXT = 0x2,
  BAIKAL_HDMI_I2CM_OPERATION_READ = 0x1,
};

#define DDC_I2C_ADDR  0x50

STATIC CONST UINT8  mEdidSignature[] = {
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00
};

STATIC
EFI_STATUS
HdmiWaitI2cDone (
  IN UINT32  Timeout
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
  IN UINT32  Timeout
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
  IN UINT8   Addr,
  IN UINT16  Data
  )
{
  EFI_STATUS  Status;

  MmioWrite32 (BAIKAL_HDMI_IH_I2CMPHY_STAT0, 0xFF);
  MmioWrite32 (BAIKAL_HDMI_PHY_I2CM_ADDRESS_ADDR, Addr);
  MmioWrite32 (BAIKAL_HDMI_PHY_I2CM_DATAO_1_ADDR, Data >> 8);
  MmioWrite32 (BAIKAL_HDMI_PHY_I2CM_DATAO_0_ADDR, Data & 0xFF) ;
  MmioWrite32 (
    BAIKAL_HDMI_PHY_I2CM_OPERATION_ADDR,
    BAIKAL_HDMI_PHY_I2CM_OPERATION_ADDR_WRITE
    );

  Status = HdmiPhyWaitI2cDone (1000);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

STATIC
VOID
HdmiPhyPowerOff (
  VOID
  )
{
  UINTN   Iter;
  UINT32  Val;

  MmioAnd32 (
     BAIKAL_HDMI_PHY_CONF0,
    ~BAIKAL_HDMI_PHY_CONF0_GEN2_TXPWRON_MASK
    );

  //
  // Wait for TX_PHY_LOCK to be deasserted to indicate that the PHY went
  // to low power mode.
  //
  for (Iter = 0; Iter < 5; ++Iter) {
    Val = MmioRead32 (BAIKAL_HDMI_PHY_STAT0);
    if ((Val & BAIKAL_HDMI_PHY_TX_PHY_LOCK) == 0) {
      break;
    }

    MicroSecondDelay (1000);
  }

  if (Val & BAIKAL_HDMI_PHY_TX_PHY_LOCK) {
    DEBUG ((DEBUG_ERROR, "HDMI PHY failed to power down\n"));
  } else {
    DEBUG ((DEBUG_INFO, "HDMI PHY powered down in %u iterations\n", Iter));
  }

  MmioOr32 (
    BAIKAL_HDMI_PHY_CONF0,
    BAIKAL_HDMI_PHY_CONF0_GEN2_PDDQ_MASK
    );
}

STATIC
EFI_STATUS
HdmiPhyPowerOn (
  VOID
  )
{
  UINTN   Iter;
  UINT32  Val;

  Val = MmioRead32 (BAIKAL_HDMI_PHY_CONF0);
  Val |= BAIKAL_HDMI_PHY_CONF0_GEN2_TXPWRON_MASK;
  Val &= ~BAIKAL_HDMI_PHY_CONF0_GEN2_PDDQ_MASK;
  MmioWrite32 (BAIKAL_HDMI_PHY_CONF0, Val);

  // Wait for PHY PLL lock
  for (Iter = 0; Iter < 5; ++Iter) {
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

  DEBUG ((DEBUG_INFO, "HDMI PHY PLL locked %u iterations\n", Iter));
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
HdmiPhyConfigure (
  IN CONST HDMI_PHY_SETTINGS  *PhySettings
  )
{
  HdmiPhyPowerOff ();

  // Leave low power consumption mode by asserting SVSRET
  MmioOr32 (
    BAIKAL_HDMI_PHY_CONF0,
    BAIKAL_HDMI_PHY_CONF0_SPARECTRL_MASK
    );

  // PHY reset. The reset signal is active high on Gen2 PHYs.
  MmioWrite32 (BAIKAL_HDMI_MC_PHYRSTZ, BAIKAL_HDMI_MC_PHYRSTZ_DEASSERT);
  MmioWrite32 (BAIKAL_HDMI_MC_PHYRSTZ, 0);

  MmioOr32 (
    BAIKAL_HDMI_MC_HEACPHY_RST,
    BAIKAL_HDMI_MC_HEACPHY_RST_ASSERT
    );

  MmioOr32 (
    BAIKAL_HDMI_PHY_TST0,
    BAIKAL_HDMI_PHY_TST0_TSTCLR_MASK
    );

  MmioWrite32 (
    BAIKAL_HDMI_PHY_I2CM_SLAVE_ADDR,
    BAIKAL_HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2
    );

  MmioAnd32 (
     BAIKAL_HDMI_PHY_TST0,
    ~BAIKAL_HDMI_PHY_TST0_TSTCLR_MASK
    );

  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_OPMODE_PLLCFG, PhySettings->Opmode);
  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_PLLCURRCTRL,   PhySettings->Current);
  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_PLLGMPCTRL,    PhySettings->Gmp);
  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_TXTERM,        PhySettings->TxTer);
  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_VLEVCTRL,      PhySettings->VlevCtrl);
  HdmiPhyI2cWrite (BAIKAL_HDMI_PHY_CKSYMTXCTRL,   PhySettings->CkSymTxCtrl);

  return HdmiPhyPowerOn ();
}

// HDMI Initialization Step B.1
STATIC
VOID
HdmiInitAvComposer (
  IN CONST SCAN_TIMINGS  *Horizontal,
  IN CONST SCAN_TIMINGS  *Vertical
  )
{
  UINT16  HBlank;
  UINT8   VBlank;
  UINT8   Val;

  HBlank = Horizontal->FrontPorch + Horizontal->Sync + Horizontal->BackPorch;
  VBlank = Vertical->FrontPorch + Vertical->Sync + Vertical->BackPorch;

  Val = MmioRead32 (BAIKAL_HDMI_TX_INVID0);
  Val |= 1 << BAIKAL_HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET;
  MmioWrite32 (BAIKAL_HDMI_TX_INVID0, Val);

  Val = MmioRead32 (BAIKAL_HDMI_FC_INVIDCONF);
  Val &= ~(BAIKAL_HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_MASK |
           BAIKAL_HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_MASK |
           BAIKAL_HDMI_FC_INVIDCONF_DE_IN_POLARITY_MASK    |
           BAIKAL_HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_MASK  |
           BAIKAL_HDMI_FC_INVIDCONF_IN_I_P_MASK);
  Val |= BAIKAL_HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_LOW |
         BAIKAL_HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_LOW |
         BAIKAL_HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH   |
         BAIKAL_HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW  |
         BAIKAL_HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE;
  MmioWrite32 (BAIKAL_HDMI_FC_INVIDCONF, Val);

  MmioWrite32 (BAIKAL_HDMI_FC_INHACTV1, Horizontal->Resolution >> 8);
  MmioWrite32 (BAIKAL_HDMI_FC_INHACTV0, Horizontal->Resolution & 0xFF);

  MmioWrite32 (BAIKAL_HDMI_FC_INVACTV1, Vertical->Resolution >> 8);
  MmioWrite32 (BAIKAL_HDMI_FC_INVACTV0, Vertical->Resolution & 0xFF);

  MmioWrite32 (BAIKAL_HDMI_FC_INHBLANK1, HBlank >> 8);
  MmioWrite32 (BAIKAL_HDMI_FC_INHBLANK0, HBlank & 0xFF);

  MmioWrite32 (BAIKAL_HDMI_FC_INVBLANK, VBlank);

  MmioWrite32 (BAIKAL_HDMI_FC_HSYNCINDELAY1, Horizontal->FrontPorch >> 8);
  MmioWrite32 (BAIKAL_HDMI_FC_HSYNCINDELAY0, Horizontal->FrontPorch & 0xFF);

  MmioWrite32 (BAIKAL_HDMI_FC_VSYNCINDELAY, Vertical->FrontPorch);

  MmioWrite32 (BAIKAL_HDMI_FC_HSYNCINWIDTH1, Horizontal->Sync >> 8);
  MmioWrite32 (BAIKAL_HDMI_FC_HSYNCINWIDTH0, Horizontal->Sync & 0xFF);

  MmioWrite32 (BAIKAL_HDMI_FC_VSYNCINWIDTH, Vertical->Sync);

  Val = MmioRead32 (BAIKAL_HDMI_FC_INVIDCONF);
  Val &= ~BAIKAL_HDMI_FC_INVIDCONF_DVI_MODEZ_MASK;
  Val |=  BAIKAL_HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE;
  MmioWrite32 (BAIKAL_HDMI_FC_INVIDCONF, Val);
}

// HDMI Initialization Step B.2
STATIC
EFI_STATUS
HdmiPhyInit (
  IN CONST HDMI_PHY_SETTINGS  *PhySettings
  )
{
  UINTN       Iter;
  UINT32      Val;
  EFI_STATUS  Status;

  // HDMI Phy spec says to do the phy initialization sequence twice
  for (Iter = 0; Iter < 2; ++Iter) {
    Val = MmioRead32 (BAIKAL_HDMI_PHY_CONF0);
    Val |=  BAIKAL_HDMI_PHY_CONF0_SELDATAENPOL_MASK;
    Val &= ~BAIKAL_HDMI_PHY_CONF0_SELDIPIF_MASK;
    MmioWrite32 (BAIKAL_HDMI_PHY_CONF0, Val);

    Status = HdmiPhyConfigure (PhySettings);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

// HDMI Initialization Step B.3
STATIC
VOID
HdmiEnableVideoPath (
  UINT32  Sync
  )
{
  UINT32  Val;

  // control period minimum duration
  MmioWrite32 (BAIKAL_HDMI_FC_CTRLDUR, 12);
  MmioWrite32 (BAIKAL_HDMI_FC_EXCTRLDUR, 32);
  MmioWrite32 (BAIKAL_HDMI_FC_EXCTRLSPAC, 1);

  // Set to fill TMDS data channels
  MmioWrite32 (BAIKAL_HDMI_FC_CH0PREAM, 0x0B);
  MmioWrite32 (BAIKAL_HDMI_FC_CH1PREAM, 0x16);
  MmioWrite32 (BAIKAL_HDMI_FC_CH2PREAM, 0x21);

  // Enable pixel clock and tmds data path
  Val = 0x7F;
  Val &= ~BAIKAL_HDMI_MC_CLKDIS_PIXELCLK_DISABLE;
  MmioWrite32 (BAIKAL_HDMI_MC_CLKDIS, Val);

  Val &= ~BAIKAL_HDMI_MC_CLKDIS_TMDSCLK_DISABLE;
  MmioWrite32 (BAIKAL_HDMI_MC_CLKDIS, Val);

  //
  // After each CLKDIS reset it is mandatory to
  // set up VSYNC active edge delay (in lines).
  //
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
  UINT8       Checksum;
  UINT8       EdidData[sizeof (EDID_BLOCK)];
  UINTN       Idx;
  EFI_STATUS  Status;

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

  for (Idx = 0, Checksum = 0; Idx < ARRAY_SIZE (EdidData); ++Idx) {
    MmioWrite32 (BAIKAL_HDMI_I2CM_ADDRESS, Idx);
    MmioWrite32 (BAIKAL_HDMI_I2CM_OPERATION, BAIKAL_HDMI_I2CM_OPERATION_READ);

    Status = HdmiWaitI2cDone (100);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    EdidData[Idx] = MmioRead32 (BAIKAL_HDMI_I2CM_DATAI);
    Checksum += EdidData[Idx];
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
