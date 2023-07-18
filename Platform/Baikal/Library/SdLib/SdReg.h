/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef SD_REG_H
#define SD_REG_H

// Controller registers
#define SDHCI_DMA_ADDRESS                 0x00
#define SDHCI_ARGUMENT2                   0x00
#define SDHCI_32BIT_BLK_CNT               0x00

#define SDHCI_BLOCK_SIZE                  0x04
#define SDHCI_MAKE_BLKSZ(Dma, BlkSz)      ((((Dma) & 0x7) << 12) | ((BlkSz) & 0xFFF))

#define SDHCI_16BIT_BLK_CNT               0x06

#define SDHCI_ARGUMENT                    0x08

#define SDHCI_TRANSFER_MODE               0x0C
#define SDHCI_TRNS_DMA                    (1 << 0)
#define SDHCI_TRNS_BLK_CNT_EN             (1 << 1)
#define SDHCI_TRNS_AUTO_CMD12             (1 << 2)
#define SDHCI_TRNS_AUTO_CMD23             (1 << 3)
#define SDHCI_TRNS_AUTO_SEL               0x0C
#define SDHCI_TRNS_READ                   (1 << 4)
#define SDHCI_TRNS_MULTI                  (1 << 5)

#define SDHCI_COMMAND                     0x0E
#define SDHCI_CMD_RESP_MASK               0x03
#define SDHCI_CMD_CRC                     0x08
#define SDHCI_CMD_INDEX                   0x10
#define SDHCI_CMD_DATA                    0x20
#define SDHCI_CMD_ABORTCMD                0xC0

#define SDHCI_RESPONSE                    0x10
#define SDHCI_RESPONSE_0                  (SDHCI_RESPONSE + 0x4 * 0)
#define SDHCI_RESPONSE_1                  (SDHCI_RESPONSE + 0x4 * 1)
#define SDHCI_RESPONSE_2                  (SDHCI_RESPONSE + 0x4 * 2)
#define SDHCI_RESPONSE_3                  (SDHCI_RESPONSE + 0x4 * 3)

#define SDHCI_BUFFER                      0x20

#define SDHCI_PRESENT_STATE               0x24
#define SDHCI_CMD_INHIBIT                 (1 << 0)
#define SDHCI_DATA_INHIBIT                (1 << 1)
#define SDHCI_DATA_74                     0x000000F0
#define SDHCI_DOING_WRITE                 (1 << 8)
#define SDHCI_DOING_READ                  (1 << 9)
#define SDHCI_SPACE_AVAILABLE             (1 << 10)
#define SDHCI_DATA_AVAILABLE              (1 << 11)
#define SDHCI_CARD_PRESENT                (1 << 16)
#define SDHCI_CARD_PRES_SHIFT             16
#define SDHCI_CD_STABLE                   (1 << 17)
#define SDHCI_CD_LVL                      (1 << 18)
#define SDHCI_CD_LVL_SHIFT                18
#define SDHCI_WRITE_PROTECT               (1 << 19)
#define SDHCI_DATA_30                     0x00F00000
#define SDHCI_DATA_LVL_SHIFT              20
#define SDHCI_DATA_0_LVL_MASK             0x00100000
#define SDHCI_CMD_LVL                     (1 << 24)
#define SDHCI_VOLTAGE_STABLE              (1 << 25)

#define SDHCI_HOST_CONTROL                0x28
#define SDHCI_CTRL_LED                    0x01
#define SDHCI_CTRL_4BITBUS                0x02
#define SDHCI_CTRL_HISPD                  0x04
#define SDHCI_CTRL_DMA_MASK               0x18
#define SDHCI_CTRL_SDMA                   0x00
#define SDHCI_CTRL_ADMA1                  0x08
#define SDHCI_CTRL_ADMA32                 0x10
#define SDHCI_CTRL_ADMA64                 0x18
#define SDHCI_CTRL_ADMA3                  0x18
#define SDHCI_CTRL_8BITBUS                0x20
#define SDHCI_CTRL_CDTEST_INS             0x40
#define SDHCI_CTRL_CDTEST_EN              0x80

#define SDHCI_POWER_CONTROL               0x29
// SD Bus Voltage
#define SDHCI_POWER_OFF                   (0x0 << 0)
#define SDHCI_POWER_ON                    (0x1 << 0)
#define SDHCI_POWER_180                   (0x5 << 1)
#define SDHCI_POWER_300                   (0x6 << 1)
#define SDHCI_POWER_330                   (0x7 << 1)
// eMMC Bus Voltage
#define SDHCI_EMMC_POWER_120              (0x5 << 1)
#define SDHCI_EMMC_POWER_180              (0x6 << 1)
#define SDHCI_EMMC_POWER_300              (0x7 << 1)
// SD Bus Voltage2
#define SDHCI_POWER2_ON                   (0x1 << 4)
#define SDHCI_POWER2_120                  (0x4 << 5)
#define SDHCI_POWER2_180                  (0x5 << 5)

#define SDHCI_BLOCK_GAP_CONTROL           0x2A

#define SDHCI_WAKE_UP_CONTROL             0x2B
#define SDHCI_WAKE_ON_INT                 0x01
#define SDHCI_WAKE_ON_INSERT              0x02
#define SDHCI_WAKE_ON_REMOVE              0x04

#define SDHCI_CLOCK_CONTROL               0x2C
#define SDHCI_CLOCK_FREG_SELECT_LOW       (0xFF << 8)
#define SDHCI_CLOCK_FREG_SELECT_HI        (3 << 6)
#define SDHCI_CLOCK_GEN_SELECT            (1 << 5)
#define SDHCI_CLOCK_RESERVED              (1 << 4)
#define SDHCI_CLOCK_PLL_EN                (1 << 3)
#define SDHCI_CLOCK_CARD_EN               (1 << 2)
#define SDHCI_CLOCK_STABLE                (1 << 1)
#define SDHCI_CLOCK_EN                    (1 << 0)

#define SDHCI_TIMEOUT_CONTROL             0x2E
#define SDHCI_TIMEOUT_DEFAULT             0xE

#define SDHCI_SOFTWARE_RESET              0x2F
#define SDHCI_RESET_ALL                   0x01
#define SDHCI_RESET_CMD                   0x02
#define SDHCI_RESET_DATA                  0x04

#define SDHCI_INT_STATUS                  0x30
#define SDHCI_INT_ENABLE                  0x34
#define SDHCI_SIGNAL_ENABLE               0x38
#define SDHCI_INT_RESPONSE                (1 << 0)
#define SDHCI_INT_DATA_END                (1 << 1)
#define SDHCI_INT_BLK_GAP                 (1 << 2)
#define SDHCI_INT_DMA_END                 (1 << 3)
#define SDHCI_INT_SPACE_AVAIL             (1 << 4)
#define SDHCI_INT_DATA_AVAIL              (1 << 5)
#define SDHCI_INT_CARD_INSERT             (1 << 6)
#define SDHCI_INT_CARD_REMOVE             (1 << 7)
#define SDHCI_INT_CARD_INT                (1 << 8)
#define SDHCI_INT_RETUNE                  (1 << 12)
#define SDHCI_INT_FX                      (1 << 13)
#define SDHCI_INT_CQE                     (1 << 14)
#define SDHCI_INT_ERROR                   (1 << 15)

#define SDHCI_ERR_STATUS                  0x32
#define SDHCI_ERR_ENABLE                  0x36
#define SDHCI_ERR_SIGNAL_ENABLE           0x3A
#define SDHCI_ERR_TIMEOUT                 (1 << 0)
#define SDHCI_ERR_CRC                     (1 << 1)
#define SDHCI_ERR_END_BIT                 (1 << 2)
#define SDHCI_ERR_INDEX                   (1 << 3)
#define SDHCI_ERR_DATA_TIMEOUT            (1 << 4)
#define SDHCI_ERR_DATA_CRC                (1 << 5)
#define SDHCI_ERR_DATA_END_BIT            (1 << 6)
#define SDHCI_ERR_BUS_POWER               (1 << 7)
#define SDHCI_ERR_AUTO_CMD_ERR            (1 << 8)
#define SDHCI_ERR_ADMA                    (1 << 9)
#define SDHCI_ERR_TUNING                  (1 << 10)
#define SDHCI_ERR_RESP                    (1 << 11)
#define SDHCI_ERR_BOOT                    (1 << 12)

#define SDHCI_AUTO_CMD_STATUS             0x3C
#define SDHCI_AUTO_CMD_TIMEOUT            0x00000002
#define SDHCI_AUTO_CMD_CRC                0x00000004
#define SDHCI_AUTO_CMD_END_BIT            0x00000008
#define SDHCI_AUTO_CMD_INDEX              0x00000010

#define SDHCI_HOST_CONTROL2               0x3E
#define SDHCI_CTRL_UHS_MASK               0x0007
#define SDHCI_CTRL_UHS_SDR12              0x0000
#define SDHCI_CTRL_UHS_SDR25              0x0001
#define SDHCI_CTRL_UHS_SDR50              0x0002
#define SDHCI_CTRL_UHS_SDR104             0x0003
#define SDHCI_CTRL_UHS_DDR50              0x0004
#define SDHCI_CTRL_HS400                  0x0007
#define SDHCI_CTRL_VDD_180                0x0008
#define SDHCI_CTRL_DRV_TYPE_MASK          0x0030
#define SDHCI_CTRL_DRV_TYPE_B             0x0000
#define SDHCI_CTRL_DRV_TYPE_A             0x0010
#define SDHCI_CTRL_DRV_TYPE_C             0x0020
#define SDHCI_CTRL_DRV_TYPE_D             0x0030
#define SDHCI_CTRL_EXEC_TUNING            0x0040
#define SDHCI_CTRL_TUNED_CLK              0x0080
#define SDHCI_CMD23_ENABLE                0x0800
#define SDHCI_CTRL_V4_MODE                0x1000
#define SDHCI_CTRL_64BIT_ADDR             0x2000
#define SDHCI_CTRL_ASYNC                  0x4000
#define SDHCI_CTRL_PRESET_VAL_ENABLE      0x8000

#define SDHCI_CAPABILITIES                0x40
#define SDHCI_TIMEOUT_CLK_MASK            0x0000003F
#define SDHCI_TIMEOUT_CLK_SHIFT           0
#define SDHCI_TIMEOUT_CLK_UNIT            0x00000080
#define SDHCI_CLOCK_BASE_MASK             0x00003F00
#define SDHCI_CLOCK_V3_BASE_MASK          0x0000FF00
#define SDHCI_CLOCK_BASE_SHIFT            8
#define SDHCI_MAX_BLOCK_MASK              0x00030000
#define SDHCI_MAX_BLOCK_SHIFT             16
#define SDHCI_CAN_DO_8BIT                 0x00040000
#define SDHCI_CAN_DO_ADMA2                0x00080000
#define SDHCI_CAN_DO_ADMA1                0x00100000
#define SDHCI_CAN_DO_HISPD                0x00200000
#define SDHCI_CAN_DO_SDMA                 0x00400000
#define SDHCI_CAN_DO_SUSPEND              0x00800000
#define SDHCI_CAN_VDD_330                 0x01000000
#define SDHCI_CAN_VDD_300                 0x02000000
#define SDHCI_CAN_VDD_180                 0x04000000
#define SDHCI_CAN_64BIT_V4                0x08000000
#define SDHCI_CAN_64BIT                   0x10000000
#define SDHCI_SUPPORT_SDR50               0x00000001
#define SDHCI_SUPPORT_SDR104              0x00000002
#define SDHCI_SUPPORT_DDR50               0x00000004
#define SDHCI_SUPPORT_UHS2                0x00000008
#define SDHCI_DRIVER_TYPE_A               0x00000010
#define SDHCI_DRIVER_TYPE_C               0x00000020
#define SDHCI_DRIVER_TYPE_D               0x00000040
#define SDHCI_RETUNING_TIMER_COUNT_MASK   0x00000F00
#define SDHCI_RETUNING_TIMER_COUNT_SHIFT  8
#define SDHCI_USE_SDR50_TUNING            0x00002000
#define SDHCI_RETUNING_MODE_MASK          0x0000C000
#define SDHCI_RETUNING_MODE_SHIFT         14
#define SDHCI_CLOCK_MUL_MASK              0x00FF0000
#define SDHCI_CLOCK_MUL_SHIFT             16
#define SDHCI_CAN_DO_ADMA3                0x08000000
#define SDHCI_SUPPORT_VDD2_18             0x10000000
#define SDHCI_SUPPORT_HS400               0x80000000 // Non-standard

#define SDHCI_CAPABILITIES_1              0x44

#define SDHCI_MAX_CURRENT                 0x48
#define SDHCI_MAX_CURRENT_LIMIT           0xFF
#define SDHCI_MAX_CURRENT_330_MASK        0x0000FF
#define SDHCI_MAX_CURRENT_330_SHIFT       0
#define SDHCI_MAX_CURRENT_300_MASK        0x00FF00
#define SDHCI_MAX_CURRENT_300_SHIFT       8
#define SDHCI_MAX_CURRENT_180_MASK        0xFF0000
#define SDHCI_MAX_CURRENT_180_SHIFT       16
#define SDHCI_MAX_CURRENT_MULTIPLIER      4

// 4C-4F reserved for more max current

#define SDHCI_SET_ACMD12_ERROR            0x50
#define SDHCI_SET_INT_ERROR               0x52

#define SDHCI_ADMA_ERROR                  0x54

// 55-57 reserved

#define SDHCI_ADMA_ADDRESS                0x58
#define SDHCI_ADMA_ADDRESS_HI             0x5C

// 60-FB reserved

#define SDHCI_PRESET_INIT                 0x60
#define SDHCI_PRESET_DS                   0x62
#define SDHCI_PRESET_HS                   0x64
#define SDHCI_PRESET_FOR_SDR12            0x66
#define SDHCI_PRESET_FOR_SDR25            0x68
#define SDHCI_PRESET_FOR_SDR50            0x6A
#define SDHCI_PRESET_FOR_SDR104           0x6C
#define SDHCI_PRESET_FOR_DDR50            0x6E
#define SDHCI_PRESET_FOR_HS400            0x74 // Non-standard
#define SDHCI_PRESET_DRV_MASK             0xC000
#define SDHCI_PRESET_DRV_SHIFT            14
#define SDHCI_PRESET_CLKGEN_SEL_MASK      0x400
#define SDHCI_PRESET_CLKGEN_SEL_SHIFT     10
#define SDHCI_PRESET_SDCLK_FREQ_MASK      0x3FF
#define SDHCI_PRESET_SDCLK_FREQ_SHIFT     0

#define SDHCI_SLOT_INT_STATUS             0xFC

#define SDHCI_HOST_VERSION                0xFE
#define SDHCI_VENDOR_VER_MASK             0xFF00
#define SDHCI_VENDOR_VER_SHIFT            8
#define SDHCI_SPEC_VER_MASK               0x00FF
#define SDHCI_SPEC_VER_SHIFT              0
#define SDHCI_SPEC_100                    0
#define SDHCI_SPEC_200                    1
#define SDHCI_SPEC_300                    2
#define SDHCI_SPEC_400                    3
#define SDHCI_SPEC_410                    4
#define SDHCI_SPEC_420                    5

#define SDHCI_MSHC_VER                    0x504
#define SDHCI_MSHC_CTRL                   0x508
#define SDHCI_EMMC_CONTROL                0x52C
#define SDHCI_EMMC_TYPE_MMC               (1 << 0) // 0 = SD, 1 = MMC
#define SDHCI_EMMC_CRC_DISABLE            (1 << 1) // 0 = Enable, 1 = Disable
#define SDHCI_EMMC_DONT_RESET             (1 << 2) // 0 = Reset,  1 = Don't

#endif /* SD_REG_H */
