/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Base.h>
#include <PiDxe.h>
#include <Protocol/BlockIo.h>
#include <Protocol/FirmwareVolumeBlock.h>
#include <Protocol/FdtClient.h>
#include <Protocol/Cpu.h>
#include <IndustryStandard/Sd.h>
#include <Library/ArmSmcLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DebugLib.h>
#include "SdReg.h"

#define BAIKAL_SMC_CMU_CMD               0xC2000000
#define BAIKAL_SMC_CMU_CLKCH_SET_RATE    6
#define BAIKAL_SMC_CMU_CLKCH_GET_RATE    7
#define BAIKAL_SMC_CMU_CLKCH_ROUND_RATE  10
#define BM1000_MMAVLSP_CMU0_BASE         0x20000000
#define BAIKAL_CLKCH_MSHC                19

#define INIT_CLOCK     (300 * 1000)
#define DEFAULT_CLOCK  (25 * 1000 * 1000)

#define IS_READ   TRUE
#define IS_WRITE  FALSE

#define SDHCI_PARTSIZE  (1024 * SDHCI_BLOCK_SIZE_DEFAULT)

STATIC EFI_PHYSICAL_ADDRESS  mBase = 0x202E0000;
STATIC UINT64  mTotalSize = 0;
STATIC UINT8 buffer[SDHCI_BLOCK_SIZE_DEFAULT]; // TODO: allocate

#define WAIT(X) ({               \
  EFI_STATUS  Ret = EFI_SUCCESS; \
  UINTN       Try = 1000 * 1000; \
  while (X) {                    \
    if (!Try--) {                \
      Ret = EFI_TIMEOUT;         \
      break;                     \
    }                            \
    MicroSecondDelay (1);        \
  };                             \
  Ret;                           \
})

enum SpeedMode {
  SD_DEFAULT,
  SD_HIGH,
  SD_12,
  SD_25,
  SD_50,
  SD_104,
  DD_50
};

enum CmdType {
  SdCommandTypeBc,  // Broadcast commands, no response
  SdCommandTypeBcr, // Broadcast commands with response
  SdCommandTypeAc,  // Addressed(point-to-point) commands
  SdCommandTypeAdtc // Addressed(point-to-point) data transfer commands
};

enum RespType {
  SdResponseTypeNo,
  SdResponseTypeR1,
  SdResponseTypeR1b,
  SdResponseTypeR2,
  SdResponseTypeR3,
  SdResponseTypeR4,
  SdResponseTypeR5,
  SdResponseTypeR5b,
  SdResponseTypeR6,
  SdResponseTypeR7
};

STATIC
UINT64
SdCalcCapacity (
  IN  VOID  *CsdRaw,
  IN  VOID  *ExtCsdRaw
  );

STATIC
EFI_STATUS
SdConfigBusWidth (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 BusWidth
  );

STATIC
UINTN
SdRegSize (
  IN  UINTN  Reg
  )
{
  switch (Reg) {
  case SDHCI_DMA_ADDRESS:
  case SDHCI_ARGUMENT:
  case SDHCI_BUFFER:
  case SDHCI_PRESENT_STATE:
  case SDHCI_MSHC_VER:
  case SDHCI_CAPABILITIES:
  case SDHCI_CAPABILITIES_1:
  case SDHCI_MAX_CURRENT:
  case SDHCI_ADMA_ADDRESS:
  case SDHCI_ADMA_ADDRESS_HI:
  case SDHCI_RESPONSE_0:
  case SDHCI_RESPONSE_1:
  case SDHCI_RESPONSE_2:
  case SDHCI_RESPONSE_3:
    return 32;

  case SDHCI_TRANSFER_MODE:
  case SDHCI_BLOCK_SIZE:
  case SDHCI_16BIT_BLK_CNT:
  case SDHCI_COMMAND:
  case SDHCI_CLOCK_CONTROL:
  case SDHCI_INT_STATUS:
  case SDHCI_ERR_STATUS:
  case SDHCI_INT_ENABLE:
  case SDHCI_ERR_ENABLE:
  case SDHCI_SIGNAL_ENABLE:
  case SDHCI_ERR_SIGNAL_ENABLE:
  case SDHCI_AUTO_CMD_STATUS:
  case SDHCI_HOST_CONTROL2:
  case SDHCI_SET_INT_ERROR:
  case SDHCI_SET_ACMD12_ERROR:
  case SDHCI_PRESET_INIT:
  case SDHCI_PRESET_DS:
  case SDHCI_PRESET_HS:
  case SDHCI_PRESET_FOR_SDR12:
  case SDHCI_PRESET_FOR_SDR25:
  case SDHCI_PRESET_FOR_SDR50:
  case SDHCI_PRESET_FOR_SDR104:
  case SDHCI_PRESET_FOR_DDR50:
  case SDHCI_PRESET_FOR_HS400:
  case SDHCI_SLOT_INT_STATUS:
  case SDHCI_HOST_VERSION:
  case SDHCI_EMMC_CONTROL:
    return 16;

  case SDHCI_TIMEOUT_CONTROL:
  case SDHCI_SOFTWARE_RESET:
  case SDHCI_ADMA_ERROR:
  case SDHCI_MSHC_CTRL:
  case SDHCI_HOST_CONTROL:
  case SDHCI_POWER_CONTROL:
  case SDHCI_BLOCK_GAP_CONTROL:
  case SDHCI_WAKE_UP_CONTROL:
    return 8;
  }

  return EFI_INVALID_PARAMETER;
}

STATIC
UINTN
SdRegWrite (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Val,
  IN  UINTN                 Reg
  )
{
  UINTN  RegSize = SdRegSize (Reg);

  if (EFI_ERROR (RegSize)) {
    return RegSize;
  }
  switch (RegSize) {
  case 32:
    *(UINT32 *) (Base + Reg) = Val;
    break;
  case 16:
    *(UINT16 *) (Base + Reg) = Val;
    break;
  case 8:
    *(UINT8 *) (Base + Reg) = Val;
    break;
  }
  return EFI_SUCCESS;
}

STATIC
UINTN
SdRegRead (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Reg
  )
{
  UINTN  RegSize = SdRegSize (Reg);

  if (EFI_ERROR (RegSize)) {
    return RegSize;
  }
  switch (RegSize) {
  case 32:
    return *(UINT32 *) (Base + Reg);
  case 16:
    return *(UINT16 *) (Base + Reg);
  case 8:
    return *(UINT8 *)  (Base + Reg);
  }
  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
SdLed (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 On
  )
{
  UINT8       Ctrl;
  EFI_STATUS  Status = EFI_SUCCESS;

  Ctrl = SdRegRead (Base, SDHCI_HOST_CONTROL);
  if (On) {
    Ctrl |= SDHCI_CTRL_LED;
  } else {
    Ctrl &= ~SDHCI_CTRL_LED;
  }

  SdRegWrite (Base, Ctrl, SDHCI_HOST_CONTROL);
  return Status;
}

STATIC
EFI_STATUS
SdSetClock (
  IN  UINTN  Clock
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  UINTN         Round;

  // Calc
  ArmSmcArgs.Arg0 = BAIKAL_SMC_CMU_CMD;
  ArmSmcArgs.Arg1 = BAIKAL_CLKCH_MSHC;
  ArmSmcArgs.Arg2 = BAIKAL_SMC_CMU_CLKCH_ROUND_RATE;
  ArmSmcArgs.Arg4 = BM1000_MMAVLSP_CMU0_BASE;
  ArmSmcArgs.Arg3 = 2 * Clock;

  ArmCallSmc (&ArmSmcArgs);
  if (ArmSmcArgs.Arg0 < 0) {
    return EFI_DEVICE_ERROR;
  }

  Round = ArmSmcArgs.Arg0;

  // Set
  ArmSmcArgs.Arg0 = BAIKAL_SMC_CMU_CMD;
  ArmSmcArgs.Arg1 = BAIKAL_CLKCH_MSHC;
  ArmSmcArgs.Arg2 = BAIKAL_SMC_CMU_CLKCH_SET_RATE;
  ArmSmcArgs.Arg4 = BM1000_MMAVLSP_CMU0_BASE;
  ArmSmcArgs.Arg3 = Round;

  ArmCallSmc (&ArmSmcArgs);
  if (ArmSmcArgs.Arg0 < 0) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SdGetClock (
  IN  UINTN  *Clock
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_CMU_CMD;
  ArmSmcArgs.Arg1 = BAIKAL_CLKCH_MSHC;
  ArmSmcArgs.Arg2 = BAIKAL_SMC_CMU_CLKCH_GET_RATE;
  ArmSmcArgs.Arg4 = BM1000_MMAVLSP_CMU0_BASE;

  ArmCallSmc (&ArmSmcArgs);
  if (ArmSmcArgs.Arg0 < 0) {
    return EFI_DEVICE_ERROR;
  }

  *Clock = ArmSmcArgs.Arg0 / 2;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdSpeedMode (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Mode
  )
{
  UINT32  Ctrl  = SdRegRead (Base, SDHCI_HOST_CONTROL);
  UINT32  Ctrl2 = SdRegRead (Base, SDHCI_HOST_CONTROL2);

  switch (Mode) {
  case SD_DEFAULT:
    Ctrl  &= ~SDHCI_CTRL_HISPD;
    Ctrl2 &= ~SDHCI_CTRL_VDD_180;
    break;

  case SD_HIGH:
    Ctrl  |=  SDHCI_CTRL_HISPD;
    Ctrl2 &= ~SDHCI_CTRL_VDD_180;
    break;

  case SD_12:
    Ctrl2 |=  SDHCI_CTRL_UHS_SDR12;
    Ctrl2 |=  SDHCI_CTRL_VDD_180;
    break;

  case SD_25:
    Ctrl2 |=  SDHCI_CTRL_UHS_SDR25;
    Ctrl2 |=  SDHCI_CTRL_VDD_180;
    break;

  case SD_50:
    Ctrl2 |=  SDHCI_CTRL_UHS_SDR50;
    Ctrl2 |=  SDHCI_CTRL_VDD_180;
    break;

  case SD_104:
    Ctrl2 |=  SDHCI_CTRL_UHS_SDR104;
    Ctrl2 |=  SDHCI_CTRL_VDD_180;
    break;

  case DD_50:
    Ctrl2 |=  SDHCI_CTRL_UHS_DDR50;
    Ctrl2 |=  SDHCI_CTRL_VDD_180;
    break;

  default:
    return EFI_INVALID_PARAMETER;
  }
  SdRegWrite (Base, Ctrl,  SDHCI_HOST_CONTROL);
  SdRegWrite (Base, Ctrl2, SDHCI_HOST_CONTROL2);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdReset (
  IN  EFI_PHYSICAL_ADDRESS  Base
  )
{
  UINTN  Reg;

  // Power
  SdRegWrite (Base, SDHCI_POWER_OFF, SDHCI_POWER_CONTROL);
  MicroSecondDelay (1000);

  // IRQ
  SdRegWrite (Base, 0xFFFF, SDHCI_INT_STATUS);
  SdRegWrite (Base, 0xFFFF, SDHCI_ERR_STATUS);
  SdRegWrite (Base, 0xFFFF, SDHCI_INT_ENABLE);
  SdRegWrite (Base, 0xFFFF, SDHCI_ERR_ENABLE);
  SdRegWrite (Base, 0x0000, SDHCI_SIGNAL_ENABLE);
  SdRegWrite (Base, 0x0000, SDHCI_ERR_SIGNAL_ENABLE);

  // Config
  SdRegWrite (Base, 0, SDHCI_HOST_CONTROL);
  SdRegWrite (
    Base,
    SDHCI_CTRL_V4_MODE |
    SDHCI_CTRL_64BIT_ADDR |
    SDHCI_CTRL_ASYNC,
    SDHCI_HOST_CONTROL2
    );
  SdRegWrite (Base, 0, SDHCI_TRANSFER_MODE);
  SdRegWrite (Base, 0, SDHCI_16BIT_BLK_CNT);
  SdRegWrite (Base, 0, SDHCI_32BIT_BLK_CNT);
  SdRegWrite (Base, 0, SDHCI_ARGUMENT);
  SdRegWrite (Base, 0, SDHCI_COMMAND);
  SdRegWrite (Base, SDHCI_BLOCK_SIZE_DEFAULT, SDHCI_BLOCK_SIZE);
  SdRegWrite (Base, SDHCI_TIMEOUT_DEFAULT, SDHCI_TIMEOUT_CONTROL);

  // Clock
  Reg = SdRegRead (Base, SDHCI_CLOCK_CONTROL);
  Reg &= ~SDHCI_CLOCK_EN;
  Reg &= ~SDHCI_CLOCK_PLL_EN;
  Reg &= ~SDHCI_CLOCK_CARD_EN;
  SdRegWrite (Base, Reg, SDHCI_CLOCK_CONTROL);
  MicroSecondDelay (1000);

  SdLed (Base, FALSE);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdInitHost (
  IN  EFI_PHYSICAL_ADDRESS  Base
  )
{
  SdReset (Base);
  SdRegWrite (Base, SDHCI_POWER_330 | SDHCI_POWER_ON, SDHCI_POWER_CONTROL);
  SdRegWrite (Base, SDHCI_CLOCK_EN, SDHCI_CLOCK_CONTROL);
  MicroSecondDelay (1000);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdCardDetect (
  IN  EFI_PHYSICAL_ADDRESS  Base
  )
{
  // Clear BIT6 and BIT7 by writing 1 to these two bits if set
  SdRegWrite (Base, SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE, SDHCI_INT_STATUS);

  // Check Present State Register to see if there is a card presented
  if (SdRegRead (Base, SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT) {
    return EFI_SUCCESS;
  } else {
    return EFI_NO_MEDIA;
  }
}

STATIC
EFI_STATUS
SdCmdExec (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Index,
  IN  UINTN                 Arg,
  IN  UINTN                 CmdType,
  IN  UINTN                 RespType,
  IN  VOID                 *Buf,
  IN  UINTN                 Len,
  IN  UINTN                 IsRead
  )
{
  EFI_STATUS  Status    = EFI_SUCCESS;
  UINTN       Cmd       = Index << 8;
  UINTN       Mode      = 0;
  UINTN       Blocks    = 0;
  UINTN       Blocksize = Len < SDHCI_BLOCK_SIZE_DEFAULT ? Len : SDHCI_BLOCK_SIZE_DEFAULT;

  // Busy
  Status = WAIT (SdRegRead (Base, SDHCI_PRESENT_STATE) & (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT));
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // Clean
  SdRegWrite (Base, 0xFFFF, SDHCI_INT_STATUS);
  SdRegWrite (Base, 0xFFFF, SDHCI_ERR_STATUS);
  SdRegWrite (Base, 0x0, SDHCI_RESPONSE_0);
  SdRegWrite (Base, 0x0, SDHCI_RESPONSE_1);
  SdRegWrite (Base, 0x0, SDHCI_RESPONSE_2);
  SdRegWrite (Base, 0x0, SDHCI_RESPONSE_3);

  // Mode
  if (Len) {
    Blocks = Len / Blocksize;
    if (Blocks > 1) {
      Mode |= BIT5; // Multi Block Select
      Mode |= BIT2; // AUTO CMD12 Enable
      Mode |= BIT1; // Block Count Enable
    }

    if (IsRead) {
      Mode |= BIT4; // Data Transfer Direction Select
    }
  }

  // Cmd
  if (CmdType == SdCommandTypeAdtc) {
    Cmd |= BIT5;
  }
  switch (RespType) {
  case SdResponseTypeR1:
  case SdResponseTypeR5:
  case SdResponseTypeR6:
  case SdResponseTypeR7:
    Cmd |= BIT1 | BIT3 | BIT4;
    break;
  case SdResponseTypeR1b:
  case SdResponseTypeR5b:
    Cmd |= BIT0 | BIT1 | BIT3 | BIT4;
  case SdResponseTypeR2:
    Cmd |= BIT0 | BIT3;
    break;
  case SdResponseTypeR3:
  case SdResponseTypeR4:
    Cmd |= BIT1;
    break;
    break;
  case SdResponseTypeNo:
    break;
  default:
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  // Exec
  SdLed (Base, TRUE);
  SdRegWrite (Base, Blocks,    SDHCI_32BIT_BLK_CNT);
  SdRegWrite (Base, Blocksize, SDHCI_BLOCK_SIZE);
  SdRegWrite (Base, Mode,      SDHCI_TRANSFER_MODE);
  SdRegWrite (Base, Arg,       SDHCI_ARGUMENT);
  SdRegWrite (Base, Cmd,       SDHCI_COMMAND);

  // Wait
  Status = WAIT (!(SdRegRead (Base, SDHCI_INT_STATUS) & SDHCI_INT_RESPONSE));
  SdRegWrite (Base, SDHCI_INT_RESPONSE, SDHCI_INT_STATUS);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // Check
  if (SdRegRead (Base, SDHCI_ERR_STATUS)) {
    Status = EFI_DEVICE_ERROR;
    goto exit;
  }

  // Data
  if (Len) {
    UINT32  *P = Buf;
    for (UINTN J = 0; J < Blocks; J++) {
      if (IsRead) { // Read
        // Wait
        Status = WAIT (!(SdRegRead (Base, SDHCI_INT_STATUS) & SDHCI_INT_DATA_AVAIL));
        SdRegWrite (Base, SDHCI_INT_DATA_AVAIL, SDHCI_INT_STATUS);
        if (EFI_ERROR (Status)) {
          goto exit;
        }
        // Copy
        for (UINTN Iter = 0; Iter < Blocksize / sizeof (UINT32); ++Iter) {
          *P++ = *(UINT32 *) (Base + SDHCI_BUFFER);
        }
      } else { // Write
        // Wait
        Status = WAIT (!(SdRegRead (Base, SDHCI_INT_STATUS) & SDHCI_INT_SPACE_AVAIL));
        SdRegWrite (Base, SDHCI_INT_SPACE_AVAIL, SDHCI_INT_STATUS);
        if (EFI_ERROR (Status)) {
          goto exit;
        }
        // Copy
        for (UINTN Iter = 0; Iter < Blocksize / sizeof (UINT32); ++Iter) {
          *(UINT32 *) (Base + SDHCI_BUFFER) = *P++;
        }
      }
    }

    // Complete
    Status = WAIT (!(SdRegRead (Base, SDHCI_INT_STATUS) & SDHCI_INT_DATA_END));
    SdRegWrite (Base, SDHCI_INT_DATA_END, SDHCI_INT_STATUS);
    if (EFI_ERROR (Status)) {
      goto exit;
    }

    if (Mode & BIT5) { // Multi Block Select
      if (SdRegRead (Base, SDHCI_32BIT_BLK_CNT)) {
        Status = EFI_DEVICE_ERROR;
        goto exit;
      }
    }
  }

exit:
  SdLed (Base, FALSE);
  if (SdRegRead (Base, SDHCI_ERR_STATUS)) {
    Status = EFI_DEVICE_ERROR;
  }

  // Clean
  SdRegWrite (Base, 0xFFFF, SDHCI_INT_STATUS);
  SdRegWrite (Base, 0xFFFF, SDHCI_ERR_STATUS);

  return Status;
}

STATIC
EFI_STATUS
SdResetCmd (
  IN  EFI_PHYSICAL_ADDRESS  Base
  )
{
  UINTN  ResponseType    = SdResponseTypeNo;
  UINTN  CommandIndex    = SD_GO_IDLE_STATE; // CMD0
  UINTN  CommandType     = SdCommandTypeBc;
  UINTN  CommandArgument = 0;

  EFI_STATUS Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  return Status;
}

STATIC
EFI_STATUS
SdVoltageCheck (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 SupplyVoltage,
  IN  UINTN                 CheckPattern
  )
{
  EFI_STATUS  Status;
  UINTN       ResponseType    = SdResponseTypeR7;
  UINTN       CommandIndex    = SD_SEND_IF_COND; // CMD8
  UINTN       CommandType     = SdCommandTypeBcr;
  UINTN       CommandArgument = (SupplyVoltage << 8) | CheckPattern;

  Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (SdRegRead (Base, SDHCI_RESPONSE_0) != CommandArgument) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdGetOcrMmc (
  IN  EFI_PHYSICAL_ADDRESS  Base
  )
{
  EFI_STATUS  Status;
  UINT32      Ocr;
  UINTN       Try = 10;

  do {
    UINTN  ResponseType    = SdResponseTypeR3;
    UINTN  CommandIndex    = 1; // CMD1;
    UINTN  CommandType     = SdCommandTypeBcr;
    UINTN  CommandArgument = 0x40FF8080; // 0x40FF8080 (capacity greater than 2GB), else 0x00FF8080
    Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Ocr = SdRegRead (Base, SDHCI_RESPONSE_0);
    MicroSecondDelay (100 * 1000); // 0.1s
    if (!Try--) {
      return EFI_TIMEOUT;
    }
  } while (!(Ocr & BIT31));

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdSendOpCond (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Rca,
  IN  UINTN                 VoltageWindow,
  IN  UINTN                 S18r,
  IN  UINTN                 Xpc,
  IN  UINTN                 Hcs,
  OUT UINTN                *Ocr
  )
{
  EFI_STATUS  Status;
  UINTN       ResponseType    = SdResponseTypeR1;
  UINTN       CommandIndex    = SD_APP_CMD; // CMD55
  UINTN       CommandType     = SdCommandTypeAc;
  UINTN       CommandArgument = Rca << 16;

  Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // S18R : Switching to 1.8V Request  [BIT24]
  // 0b - Use current signal voltage
  // 1b - Switch to 1.8V signal voltage
  //
  // XPC: SDXC Power Control  [BIT28]
  // 0b - Power Saving
  // 1b - Maximum Performance
  //
  // HCS: Host Capacity Support  [BIT30]
  // 0b - SDSC Only Host
  // 1b - SDHC or SDXC Supported
  ResponseType    = SdResponseTypeR3;
  CommandIndex    = SD_SEND_OP_COND; // ACMD41
  CommandType     = SdCommandTypeBcr;
  CommandArgument =
    (VoltageWindow & 0xFFFFFF) |
    (S18r ? BIT24 : 0)         |
    (Xpc  ? BIT28 : 0)         |
    (Hcs  ? BIT30 : 0);

  Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Busy: [BIT31]
  // 0b: On Initialization
  // 1b: Initialization Complete
  //
  // CCS: Card Capacity Status [BIT30]
  // 0b: SDSC
  // 1b: SDHC or SDXC
  //
  // UHS-II: [BIT29]
  // 0b: Non UHS-II Card
  // 1b: UHS-II Card
  //
  // S18A : Switching to 1.8V Accepted  [BIT24]
  // 0b: Continues current voltage signaling
  // 1b: Ready for switching signal voltage
  *Ocr = SdRegRead (Base, SDHCI_RESPONSE_0);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdAllSendCid (
  IN  EFI_PHYSICAL_ADDRESS   Base,
  OUT VOID                  *Cid
  )
{
  EFI_STATUS  Status;
  UINT32      Resp[4];
  UINTN       ResponseType    = SdResponseTypeR2;
  UINTN       CommandIndex    = SD_ALL_SEND_CID; // CMD2
  UINTN       CommandType     = SdCommandTypeBcr;
  UINTN       CommandArgument = 0;

  Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Resp[0] = SdRegRead (Base, SDHCI_RESPONSE_0);
  Resp[1] = SdRegRead (Base, SDHCI_RESPONSE_1);
  Resp[2] = SdRegRead (Base, SDHCI_RESPONSE_2);
  Resp[3] = SdRegRead (Base, SDHCI_RESPONSE_3);
  CopyMem (((UINT8 *) Cid) + 1, (UINT8 *) Resp, sizeof (SD_CID) - 1);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdSetRca (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  OUT UINTN                *Rca
  )
{
  EFI_STATUS  Status;
  UINTN       ResponseType    = SdResponseTypeR6;
  UINTN       CommandIndex    = SD_SET_RELATIVE_ADDR; // CMD3
  UINTN       CommandType     = SdCommandTypeBcr;
  UINTN       CommandArgument = 0;

  Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *Rca = SdRegRead (Base, SDHCI_RESPONSE_0) >> 16;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdSelect (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Rca
  )
{
  UINTN  ResponseType    = SdResponseTypeR1b;
  UINTN  CommandIndex    = SD_SELECT_DESELECT_CARD; // CMD7
  UINTN  CommandType     = SdCommandTypeAc;
  UINTN  CommandArgument = Rca << 16;

  EFI_STATUS Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  return Status;
}

EFI_STATUS
SdVoltageSwitch (
  IN  EFI_PHYSICAL_ADDRESS  Base
  )
{
  EFI_STATUS  Status;
  INTN        Reg;
  INTN        Dat30;

  // 2) Voltage Switch
  UINTN  ResponseType    = SdResponseTypeR1;
  UINTN  CommandIndex    = SD_VOLTAGE_SWITCH; // CMD11
  UINTN  CommandType     = SdCommandTypeAc;
  UINTN  CommandArgument = 0;
  Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);

  // 3) Response OK?
  if (!EFI_ERROR (Status)) {
    // 4) SD Clock Enable = 0
    Reg  = SdRegRead (Base, SDHCI_CLOCK_CONTROL);
    Reg &= ~SDHCI_CLOCK_CARD_EN;
    SdRegWrite (Base, SDHCI_CLOCK_EN | SDHCI_CLOCK_PLL_EN, SDHCI_CLOCK_CONTROL);

    // 5) Check DAT[3:0] = 0
    Reg = SdRegRead (Base, SDHCI_PRESENT_STATE);
    Dat30 = (Reg & SDHCI_DATA_30) >> 20;
    if (Dat30) {
      return EFI_DEVICE_ERROR;
    }

    // 6) 1.8V Signal Enable = 1
    // Host Controller clears this bit if switching to 1.8V signaling fails
    Reg = SdRegRead (Base, SDHCI_HOST_CONTROL2);
    SdRegWrite (Base, Reg | SDHCI_CTRL_VDD_180, SDHCI_HOST_CONTROL2);

    // 7) Wait 5ms
    MicroSecondDelay (5 * 1000);

    // 8) SD Clock Enable = 1
    Reg  = SdRegRead (Base, SDHCI_CLOCK_CONTROL);
    Reg |= SDHCI_CLOCK_CARD_EN;
    SdRegWrite (Base, Reg, SDHCI_CLOCK_CONTROL);

    // 9) Wait 1ms
    MicroSecondDelay (1 * 1000);

    // 10) Check DAT[3:0] = 1111b
    Reg = SdRegRead (Base, SDHCI_PRESENT_STATE);
    Dat30 = (Reg & SDHCI_DATA_30) >> 20;

    if (!Dat30) {
      return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdSetBusWidth (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Rca,
  IN  UINTN                 BusWidth
  )
{
  EFI_STATUS  Status;
  UINTN       ResponseType    = SdResponseTypeR1;
  UINTN       CommandIndex    = SD_APP_CMD; // CMD55
  UINTN       CommandType     = SdCommandTypeAc;
  UINTN       CommandArgument = Rca << 16;

  Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ResponseType    = SdResponseTypeR1;
  CommandIndex    = SD_SET_BUS_WIDTH; // ACMD6
  CommandType     = SdCommandTypeAc;
  CommandArgument = BusWidth / 2;

  return SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
}

STATIC
EFI_STATUS
SdSwitchBusWidthMmc (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Rca,
  IN  UINTN                 BusWidth
  )
{
  // EmmcSetEXTCSD
  EFI_STATUS  Status;
  UINTN       State;
  UINTN       Try;
  UINTN       ResponseType    = SdResponseTypeR1b;
  UINTN       CommandIndex    = 6; // CMD6
  UINTN       CommandType     = SdCommandTypeAc;
  UINTN       CommandArgument =
                (((3)            & 0x03) << 24) | // 3-Write Byte
                (((183)          & 0xFF) << 16) | // EXTCSD_BUS_WIDTH [183]
                (((BusWidth / 4) & 0xFF) <<  8) | // EMMC_BUS_WIDTH_8BIT _4BIT _1BIT
                (((1)            & 0x07) <<  0);

  Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Make sure device exiting prog mode
  Try = 10;
  do {
    UINTN  ResponseType    = SdResponseTypeR1;
    UINTN  CommandIndex    = 13; // CMD13
    UINTN  CommandType     = SdCommandTypeAc;
    UINTN  CommandArgument = Rca << 16;

    Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    State = (((SdRegRead (Base, SDHCI_RESPONSE_0)) >> 9) & 0xF); // CURRENT_STATE
    MicroSecondDelay (100 * 1000); // 0.1s
    if (!Try--) {
      return EFI_TIMEOUT;
    }
  } while (State == 7); // EMMC_PRG_STATE

  Status = SdConfigBusWidth (Base,BusWidth);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
SdSwitch (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 AccessMode,
  IN  UINTN                 CommandSystem,
  IN  UINTN                 DriveStrength,
  IN  UINTN                 PowerLimit,
  IN  UINTN                 IsSwitch,
  OUT VOID                 *SwitchResp
  )
{
  UINTN  ResponseType    = SdResponseTypeR1;
  UINTN  CommandIndex    = SD_SWITCH_FUNC; // CMD6
  UINTN  CommandType     = SdCommandTypeAdtc;
  UINTN  CommandArgument =
           ((AccessMode    & 0xF) << 0)  |
           ((CommandSystem & 0xF) << 4)  |
           ((DriveStrength & 0xF) << 8)  |
           ((PowerLimit    & 0xF) << 12) |
           ((IsSwitch      & 0x1) << 31);

  UINTN IsRead = TRUE;
  UINTN Len = 64;

  return SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, SwitchResp, Len, IsRead);
}

STATIC
EFI_STATUS
SdSendStatus (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Rca,
  IN  UINTN                *DevStatus
  )
{
  UINTN  ResponseType    = SdResponseTypeR1;
  UINTN  CommandIndex    = SD_SEND_STATUS; // CMD13
  UINTN  CommandType     = SdCommandTypeAc;
  UINTN  CommandArgument = Rca << 16;

  return SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
}

EFI_STATUS
SdSendTuningBlk (
  IN  EFI_PHYSICAL_ADDRESS  Base
  )
{
  UINTN  ResponseType    = SdResponseTypeR1;
  UINTN  CommandIndex    = SD_SEND_TUNING_BLOCK; // CMD19
  UINTN  CommandType     = SdCommandTypeAdtc;
  UINTN  CommandArgument = 0;
  UINTN  IsRead = TRUE;
  UINT8  Buf[64];
  UINTN  Len = sizeof (Buf);

  EFI_STATUS Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, Buf, Len, IsRead);
  return Status;
}

STATIC
EFI_STATUS
SdGetCsd (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Rca,
  OUT VOID                 *Csd
  )
{
  EFI_STATUS  Status;
  UINT32      Resp[4];
  UINTN       ResponseType    = SdResponseTypeR2;
  UINTN       CommandIndex    = SD_SEND_CSD; // CMD9
  UINTN       CommandType     = SdCommandTypeAc;
  UINTN       CommandArgument = Rca << 16;

  Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Resp[0] = SdRegRead (Base, SDHCI_RESPONSE_0);
  Resp[1] = SdRegRead (Base, SDHCI_RESPONSE_1);
  Resp[2] = SdRegRead (Base, SDHCI_RESPONSE_2);
  Resp[3] = SdRegRead (Base, SDHCI_RESPONSE_3);
  CopyMem (((UINT8 *) Csd) + 1, (UINT8 *) Resp, sizeof (SD_CSD) - 1);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdGetExtCsd (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  OUT VOID                 *ExtCSD
  )
{
  UINTN  ResponseType    = SdResponseTypeR1;
  UINTN  CommandIndex    = 8; // EMMC_SEND_EXT_CSD, CMD8
  UINTN  CommandType     = SdCommandTypeAdtc;
  UINTN  CommandArgument = 0;
  UINTN  IsRead          = TRUE;
  UINTN  Len             = 512;

  EFI_STATUS Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, ExtCSD, Len, IsRead);
  return Status;
}

STATIC
EFI_STATUS
SdGetCid (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Rca,
  OUT VOID                 *Cid
  )
{
  EFI_STATUS  Status;
  UINT32      Resp[4];
  UINTN       ResponseType    = SdResponseTypeR2;
  UINTN       CommandIndex    = SD_SEND_CID; // CMD10
  UINTN       CommandType     = SdCommandTypeAc;
  UINTN       CommandArgument = Rca << 16;

  Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Resp[0] = SdRegRead (Base, SDHCI_RESPONSE_0);
  Resp[1] = SdRegRead (Base, SDHCI_RESPONSE_1);
  Resp[2] = SdRegRead (Base, SDHCI_RESPONSE_2);
  Resp[3] = SdRegRead (Base, SDHCI_RESPONSE_3);
  CopyMem (((UINT8 *) Cid) + 1, (UINT8 *) Resp, sizeof (SD_CID) - 1);

  return EFI_SUCCESS;
}

EFI_STATUS
SdSetBlocksize (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Blocksize
  )
{
  UINTN  ResponseType    = SdResponseTypeR1;
  UINTN  CommandIndex    = SD_SET_BLOCKLEN; // CMD16
  UINTN  CommandType     = SdCommandTypeAc;
  UINTN  CommandArgument = Blocksize;

  EFI_STATUS Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, NULL, 0, 0);
  return Status;
}

EFI_STATUS
SdRwSingleBlock (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  EFI_LBA               Lba,
  IN  VOID                 *Buffer,
  IN  UINTN                 BufferSize,
  IN  UINTN                 IsRead
  )
{
  UINTN  ResponseType    = SdResponseTypeR1;
  UINTN  CommandIndex    = IsRead ? SD_READ_SINGLE_BLOCK : SD_WRITE_SINGLE_BLOCK; // CMD17 / CMD24
  UINTN  CommandType     = SdCommandTypeAdtc;
  UINTN  CommandArgument = Lba;

  EFI_STATUS Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, Buffer, BufferSize, IsRead);
  return Status;
}

EFI_STATUS
SdRwBlocks (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  EFI_LBA               Lba,
  IN  VOID                 *Buffer,
  IN  UINTN                 BufferSize,
  IN  UINTN                 IsRead
  )
{
  UINTN  CommandIndex;
  UINTN  ResponseType    = SdResponseTypeR1;
  UINTN  CommandType     = SdCommandTypeAdtc;
  UINTN  CommandArgument = Lba;

  if (BufferSize / SDHCI_BLOCK_SIZE_DEFAULT > 1) {
    CommandIndex = IsRead ? SD_READ_MULTIPLE_BLOCK : SD_WRITE_MULTIPLE_BLOCK; // CMD18 / CMD25
  } else {
    CommandIndex = IsRead ? SD_READ_SINGLE_BLOCK : SD_WRITE_SINGLE_BLOCK; // CMD17 / CMD24
  }

  EFI_STATUS Status = SdCmdExec (Base, CommandIndex, CommandArgument, CommandType, ResponseType, Buffer, BufferSize, IsRead);

  return Status;
}

STATIC
EFI_STATUS
SdClockSupply (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 ClockFreq
  )
{
  EFI_STATUS  Status;
  INTN        Reg;

  // Disable
  Reg  = SdRegRead (Base, SDHCI_CLOCK_CONTROL);
  Reg &= ~SDHCI_CLOCK_PLL_EN;
  Reg &= ~SDHCI_CLOCK_CARD_EN;
  SdRegWrite (Base, Reg, SDHCI_CLOCK_CONTROL);

  // Config
  Status = SdSetClock (ClockFreq);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Wait
  Status = WAIT (!(SdRegRead (Base, SDHCI_CLOCK_CONTROL) & SDHCI_CLOCK_STABLE));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Enable
  Reg  = SdRegRead (Base, SDHCI_CLOCK_CONTROL);
  Reg |= SDHCI_CLOCK_PLL_EN;
  Reg |= SDHCI_CLOCK_CARD_EN;
  SdRegWrite (Base, Reg, SDHCI_CLOCK_CONTROL);

  // Reset
  SdRegWrite (Base, SDHCI_RESET_CMD,  SDHCI_SOFTWARE_RESET);
  SdRegWrite (Base, SDHCI_RESET_DATA, SDHCI_SOFTWARE_RESET);
  MicroSecondDelay (1000);
  Status = WAIT (SdRegRead (Base, SDHCI_SOFTWARE_RESET));
  if (EFI_ERROR (Status)) {
    return Status;
  }
  MicroSecondDelay (1000);

  return EFI_SUCCESS;
}

EFI_STATUS
SdClockStop (
  IN  EFI_PHYSICAL_ADDRESS  Base
  )
{
  UINT32      Clk;
  EFI_STATUS  Status;
  Status = WAIT (!(SdRegRead (Base, SDHCI_PRESENT_STATE) & (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT)));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MicroSecondDelay (1000);

  Clk  = SdRegRead (Base, SDHCI_CLOCK_CONTROL);
  Clk &= ~BIT2; // SDHCI_CLOCK_CARD_EN
  SdRegWrite (Base, Clk, SDHCI_CLOCK_CONTROL);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdConfigBusWidth (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 BusWidth
  )
{
  UINTN  HostCtrl1 = SdRegRead (Base, SDHCI_HOST_CONTROL);

  if (BusWidth == 1) {
    HostCtrl1 &= ~BIT1;
    HostCtrl1 &= ~BIT5;
  } else if (BusWidth == 4) {
    HostCtrl1 |=  BIT1;
    HostCtrl1 &= ~BIT5;
  } else if (BusWidth == 8) {
    HostCtrl1 &= ~BIT1;
    HostCtrl1 |=  BIT5;
  } else {
    return EFI_INVALID_PARAMETER;
  }

  SdRegWrite (Base, HostCtrl1, SDHCI_HOST_CONTROL);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SdSwitchBusWidthSd (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  IN  UINTN                 Rca,
  IN  UINTN                 BusWidth
  )
{
  UINTN       DevStatus;
  EFI_STATUS  Status;

  Status = SdSetBusWidth (Base, Rca, BusWidth); // ACMD6
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SdSendStatus (Base, Rca, &DevStatus); // CMD13
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((DevStatus >> 16) != 0) {
    return EFI_DEVICE_ERROR;
  }

  Status = SdConfigBusWidth (Base, BusWidth);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

UINTN
SdCalcXpc (
  IN  EFI_PHYSICAL_ADDRESS  Base
  )
{
  UINTN  MaxCurrent;
  UINTN  Capabilities;
  UINTN  Current;
  UINTN  Current330;
  UINTN  Current300;
  UINTN  Current180;

  Current    = SdRegRead (Base, SDHCI_MAX_CURRENT);
  Current330 = ((Current >> SDHCI_MAX_CURRENT_330_SHIFT) & 0xFF) * SDHCI_MAX_CURRENT_MULTIPLIER;
  Current300 = ((Current >> SDHCI_MAX_CURRENT_300_SHIFT) & 0xFF) * SDHCI_MAX_CURRENT_MULTIPLIER;
  Current180 = ((Current >> SDHCI_MAX_CURRENT_180_SHIFT) & 0xFF) * SDHCI_MAX_CURRENT_MULTIPLIER;

  Capabilities = SdRegRead (Base, SDHCI_CAPABILITIES);
  if (Capabilities & SDHCI_CAN_VDD_330) {
    MaxCurrent = Current330;
  } else if (Capabilities & SDHCI_CAN_VDD_300) {
    MaxCurrent = Current300;
  } else if (Capabilities & SDHCI_CAN_VDD_180) {
    MaxCurrent = Current180;
  } else {
    return -1;
  }

  return MaxCurrent >= 150;
}

UINTN
SdCalcS18r (
  IN  EFI_PHYSICAL_ADDRESS  Base
  )
{
  UINTN  ControllerVer;
  UINTN  S18r;

  ControllerVer = SdRegRead (Base, SDHCI_HOST_VERSION) & SDHCI_SPEC_VER_MASK;
  switch (ControllerVer) {
  case SDHCI_SPEC_100:
  case SDHCI_SPEC_200:
    S18r = FALSE;
    break;

  case SDHCI_SPEC_300:
  case SDHCI_SPEC_400:
  case SDHCI_SPEC_410:
  case SDHCI_SPEC_420:
    S18r = TRUE;
    break;

  default:
    S18r = -1;
    break;
  }

  return S18r;
}

STATIC
EFI_STATUS
SdIdentificationSd (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  OUT UINT64               *TotalSize
  )
{
  EFI_STATUS  Status;
  UINTN       Try;

  Status = SdCardDetect (Base);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // SD?
  //   CMD8
  //   ACMD41
  //   CMD2    // CID
  //   CMD3    // RCA
  //   CMD9    // CSD    // ---  InitializeSdMmcDevice
  //   CMD7
  //   ACMD51  // SCR   (SD Configuration Register)
  //   CMD6    // Switch (Mode,Group,Value)
  //   ACMD6   // Width

  SD_CID  Cid;
  SD_CSD  Csd;
  UINTN   Rca  = 0;
  UINTN   Xpc  = FALSE; // Power Control             (0b - Power Saving)
  UINTN   Hcs  = TRUE;  // Host Capacity Support     (1b - SDHC or SDXC Supported)
  UINTN   S18r = FALSE; // Switching to 1.8V Request (1b - Switch to 1.8V signal voltage)
  UINTN   Ocr  = 0;

  // 0) Power, Clock
  SdInitHost (Base);
  Status = SdClockSupply (Base, INIT_CLOCK);
  if (EFI_ERROR (Status)) {
    goto exit;
  }
  Status = SdSpeedMode (Base, SD_DEFAULT);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // 1) Send Cmd0 to the device
  Status = SdResetCmd (Base); // CMD0
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // 2) Send Cmd8 to the device
  Status = SdVoltageCheck (Base, 0x1, 0xFF); // CMD8
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // 4) Send Acmd41 with voltage window 0 to the device
  //
  // 5) Repeatly send Acmd41 with supply voltage window to the device.
  //    Note here we only support the cards complied with SD physical
  //    layer simplified spec version 2.0 and version 3.0 and above.
  Try  = 10;
  do {
    Status = SdSendOpCond (Base, Rca, Ocr, S18r, Xpc, Hcs, &Ocr); // ACMD41
    if (EFI_ERROR (Status)) {
      goto exit;
    }

    MicroSecondDelay (100 * 1000); // 0.1s
    if (!Try--) {
      goto exit;
    }
  } while (!(Ocr & BIT31));

  // 6) If the S18a bit is set and the Host Controller supports 1.8V signaling
  //    (One of support bits is set to 1: SDR50, SDR104 or DDR50 in the
  //    Capabilities register), switch its voltage to 1.8V.

  Status = SdAllSendCid (Base,  &Cid); // CMD2
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdSetRca (Base,  &Rca); // CMD3
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdGetCsd (Base, Rca, &Csd); // CMD9
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdGetCid (Base, Rca, &Cid); // CMD10
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdSelect (Base, Rca); // CMD7
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // Bus
  Status = SdSwitchBusWidthSd (Base, Rca, 4); // ACMD6, CMD13
  if (EFI_ERROR (Status)) {
    goto exit;
  }
  // Clock
  Status = SdClockSupply (Base, DEFAULT_CLOCK);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // Size
  if (TotalSize != NULL) {
    *TotalSize = SdCalcCapacity (&Csd, NULL);
  }

  return Status;

exit:
  return EFI_DEVICE_ERROR;
}

STATIC
EFI_STATUS
SdIdentificationSdio (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  OUT UINT64               *TotalSize
  )
{
  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
SdIdentificationMmc (
  IN  EFI_PHYSICAL_ADDRESS  Base,
  OUT UINT64               *TotalSize
  )
{
  // eMMC\MMC?
  //   CMD1  // OCR
  //   CMD2  // CID
  //   CMD3  // RCA
  //   CMD9  // CSD
  //   CMD7  // Select
  //   CMD8  // ECSD
  //   CMD6  // HS_TIMING
  //   CMD6  // BUS_WIDTH
  //   CMD23 // Block Count

  EFI_STATUS  Status;
  SD_CID      Cid;
  SD_CSD      Csd;
  UINTN       Rca;
  UINT8       ExtCsd[512];

  // 0) Power, Clock
  SdInitHost (Base);
  Status = SdClockSupply (Base, INIT_CLOCK);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdSpeedMode (Base, SD_DEFAULT);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdResetCmd (Base); // CMD0
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdGetOcrMmc (Base); // CMD1
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdAllSendCid (Base,  &Cid); // CMD2
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdSetRca (Base,  &Rca); // CMD3
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdGetCsd (Base, Rca, &Csd); // CMD9
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  Status = SdSelect (Base, Rca); // CMD7
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // Config
  Status = SdSwitchBusWidthMmc (Base, Rca, 8);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // Clock
  Status = SdClockSupply (Base, 25 * 1000 * 1000);
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // ExtCsd:
  Status = SdGetExtCsd (Base, ExtCsd); // CMD8
  if (EFI_ERROR (Status)) {
    goto exit;
  }

  // Size
  if (TotalSize != NULL) {
    *TotalSize = SdCalcCapacity (&Csd, ExtCsd);
  }

  return EFI_SUCCESS;

exit:
  return EFI_DEVICE_ERROR;
}

VOID
SdGetSdModelName (
  IN  VOID   *CidRaw,
  OUT CHAR8  *String
  )
{
  SD_CID  *Cid = CidRaw;
  CopyMem (String, Cid->OemId, sizeof (Cid->OemId));
  CopyMem (String + sizeof (Cid->OemId) + 1, Cid->ProductName, sizeof (Cid->ProductName));
  CopyMem (String + sizeof (Cid->OemId) + sizeof (Cid->ProductName) + 1, Cid->ProductSerialNumber, sizeof (Cid->ProductSerialNumber));
  String[sizeof (Cid->OemId)] = ' ';
  String[sizeof (Cid->OemId) + sizeof (Cid->ProductName)] = ' ';
}

STATIC
UINT64
SdCalcCapacity (
  IN  VOID  *CsdRaw,
  IN  VOID  *ExtCsdRaw
  )
{
  SD_CSD   *Csd    = CsdRaw;
  SD_CSD2  *Csd2   = CsdRaw;
  UINT8    *ExtCsd = ExtCsdRaw;
  UINTN     CSize = 0;
  UINTN     CSizeMul = 0;
  UINTN     ReadBlLen = 0;
  UINT64    Capacity = 0;
  UINTN     SecCount = 0;

  switch (Csd->CsdStructure) {
  case 0:
    CSize     = (Csd->CSizeHigh << 2 | Csd->CSizeLow) + 1;
    CSizeMul  = (1 << (Csd->CSizeMul + 2));
    ReadBlLen = (1 << (Csd->ReadBlLen));
    Capacity  = MultU64x32 (MultU64x32 ((UINT64) CSize, CSizeMul), ReadBlLen);
    break;

  case 1:
    CSize     = (Csd2->CSizeHigh << 16 | Csd2->CSizeLow) + 1;
    Capacity  = MultU64x32 ((UINT64) CSize, SIZE_512KB);
    break;

  case 2:
    break;

  case 3:
    if (!ExtCsd) {
      break;
    }

    CSize = (Csd->CSizeHigh << 2 | Csd->CSizeLow) + 1;
    if (CSize == 0xFFF) {
      Capacity  = MultU64x32 ((UINT64) CSize, SIZE_512KB);
    } else {
      SecCount =
        (ExtCsd[215] << 3 * 8) +
        (ExtCsd[214] << 2 * 8) +
        (ExtCsd[213] << 1 * 8) +
        (ExtCsd[212] << 0 * 8);

      Capacity = MultU64x32 ((UINT64) SecCount, 512);
    }

    break;
  }

  return Capacity;
}

UINT64
SdTotalSize (VOID)
{
  return mTotalSize;
}

VOID
SdPowerOff (VOID)
{
  SdReset (mBase);
}

EFI_STATUS
SdIdentification (VOID)
{
  EFI_STATUS  Status;

  if (mTotalSize) {
    return EFI_SUCCESS;
  }

  Status = SdIdentificationMmc (mBase, &mTotalSize);
  if (!EFI_ERROR (Status)) {
    goto done;
  }

  Status = SdIdentificationSd (mBase, &mTotalSize);
  if (!EFI_ERROR (Status)) {
    goto done;
  }

  Status = SdIdentificationSdio (mBase, &mTotalSize);
  if (!EFI_ERROR (Status)) {
    goto done;
  }
  return EFI_NO_MEDIA;

done:
  return EFI_SUCCESS;
}

EFI_STATUS
SdReadBlocks (
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  OUT VOID                   *Buffer
  )
{
  EFI_STATUS   Status;
  UINT8       *Buf = Buffer;
  UINTN        Size = BufferSize;

  while (Size) {
    UINTN  Part = Size > SDHCI_PARTSIZE ? SDHCI_PARTSIZE : Size;
    Status = SdRwBlocks (mBase, Lba, Buf, Part, IS_READ);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Size -= Part;
    Buf  += Part;
    Lba  += Part / SDHCI_BLOCK_SIZE_DEFAULT;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
SdWriteBlocks (
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  IN  VOID                   *Buffer
  )
{
  EFI_STATUS   Status;
  UINT8       *Buf = Buffer;
  UINTN        Size = BufferSize;

  while (Size) {
    UINTN  Part = Size > SDHCI_PARTSIZE ? SDHCI_PARTSIZE : Size;
    Status = SdRwBlocks (mBase, Lba, Buf, Part, IS_WRITE);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Size -= Part;
    Buf  += Part;
    Lba  += Part / SDHCI_BLOCK_SIZE_DEFAULT;
  }
  return EFI_SUCCESS;
}

// ------------------------------
// IO
// ------------------------------
EFI_STATUS
SdReadBlocksIo (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  OUT VOID                   *Buffer
  )
{
  return SdReadBlocks (Lba, BufferSize, Buffer);
}

EFI_STATUS
SdWriteBlocksIo (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 Lba,
  IN  UINTN                   BufferSize,
  IN  VOID                   *Buffer
  )
{
  return SdWriteBlocks (Lba, BufferSize, Buffer);
}

EFI_STATUS
SdFlushBlocksIo (
  IN  EFI_BLOCK_IO_PROTOCOL  *This
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
SdResetIo (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  BOOLEAN                 ExtendedVerification
  )
{
  return EFI_SUCCESS;
}

// ------------------------------
// nonblock access
// ------------------------------
EFI_STATUS
SdRead (
  IN UINT64   adr,
  IN VOID    *dst_,
  IN UINT64   size
  )
{
  EFI_STATUS   Status = EFI_INVALID_PARAMETER;
  UINT64       lba;
  UINT64       part;
  UINT64       offset;
  UINT8       *dst = dst_;

  while (size) {
    // Read
    lba = adr / SDHCI_BLOCK_SIZE_DEFAULT;
    Status = SdReadBlocks (lba, sizeof (buffer), buffer);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    // Copy
    offset = adr % SDHCI_BLOCK_SIZE_DEFAULT;
    part = MIN (size, sizeof (buffer) - offset);
    CopyMem (dst, buffer + offset, part);

    // Next
    adr  += part;
    dst  += part;
    size -= part;
  }

  return Status;
}

EFI_STATUS
SdWrite (
  IN UINT64   adr,
  IN VOID    *src_,
  IN UINT64   size
  )
{
  EFI_STATUS   Status = EFI_INVALID_PARAMETER;
  UINT64       lba;
  UINT64       part;
  UINT64       offset;
  UINT8       *src = src_;

  while (size) {
    // Read
    lba = adr / SDHCI_BLOCK_SIZE_DEFAULT;
    Status = SdReadBlocks (lba, sizeof (buffer), buffer);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    // Modify
    offset = adr % SDHCI_BLOCK_SIZE_DEFAULT;
    part = MIN (size, sizeof (buffer) - offset);
    CopyMem (buffer + offset, src, part);

    // Write
    Status = SdWriteBlocks (lba, sizeof (buffer), buffer);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    // Next
    adr  += part;
    src  += part;
    size -= part;
  }

  return Status;
}

// ------------------------------
// Convert
// ------------------------------
VOID
SdConvertPointers (
  VOID
  )
{
  EfiConvertPointer (0x0, (VOID **) &SdRegSize);
  EfiConvertPointer (0x0, (VOID **) &SdRegWrite);
  EfiConvertPointer (0x0, (VOID **) &SdRegRead);
  EfiConvertPointer (0x0, (VOID **) &SdSetClock);
  EfiConvertPointer (0x0, (VOID **) &SdGetClock);
  EfiConvertPointer (0x0, (VOID **) &SdSpeedMode);
  EfiConvertPointer (0x0, (VOID **) &SdInitHost);
  EfiConvertPointer (0x0, (VOID **) &SdCardDetect);
  EfiConvertPointer (0x0, (VOID **) &SdCmdExec);
  EfiConvertPointer (0x0, (VOID **) &SdResetCmd);
  EfiConvertPointer (0x0, (VOID **) &SdVoltageCheck);
  EfiConvertPointer (0x0, (VOID **) &SdGetOcrMmc);
  EfiConvertPointer (0x0, (VOID **) &SdSendOpCond);
  EfiConvertPointer (0x0, (VOID **) &SdAllSendCid);
  EfiConvertPointer (0x0, (VOID **) &SdSetRca);
  EfiConvertPointer (0x0, (VOID **) &SdSelect);
  EfiConvertPointer (0x0, (VOID **) &SdVoltageSwitch);
  EfiConvertPointer (0x0, (VOID **) &SdSetBusWidth);
  EfiConvertPointer (0x0, (VOID **) &SdSwitchBusWidthMmc);
  EfiConvertPointer (0x0, (VOID **) &SdSwitch);
  EfiConvertPointer (0x0, (VOID **) &SdSendStatus);
  EfiConvertPointer (0x0, (VOID **) &SdSendTuningBlk);
  EfiConvertPointer (0x0, (VOID **) &SdGetCsd);
  EfiConvertPointer (0x0, (VOID **) &SdGetExtCsd);
  EfiConvertPointer (0x0, (VOID **) &SdGetCid);
  EfiConvertPointer (0x0, (VOID **) &SdSetBlocksize);
  EfiConvertPointer (0x0, (VOID **) &SdRwSingleBlock);
  EfiConvertPointer (0x0, (VOID **) &SdRwBlocks);
  EfiConvertPointer (0x0, (VOID **) &SdClockSupply);
  EfiConvertPointer (0x0, (VOID **) &SdClockStop);
  EfiConvertPointer (0x0, (VOID **) &SdConfigBusWidth);
  EfiConvertPointer (0x0, (VOID **) &SdSwitchBusWidthSd);
  EfiConvertPointer (0x0, (VOID **) &SdCalcXpc);
  EfiConvertPointer (0x0, (VOID **) &SdCalcS18r);
  EfiConvertPointer (0x0, (VOID **) &SdIdentificationSd);
  EfiConvertPointer (0x0, (VOID **) &SdIdentificationSdio);
  EfiConvertPointer (0x0, (VOID **) &SdIdentificationMmc);
  EfiConvertPointer (0x0, (VOID **) &SdGetSdModelName);
  EfiConvertPointer (0x0, (VOID **) &SdCalcCapacity);
  EfiConvertPointer (0x0, (VOID **) &SdTotalSize);
  EfiConvertPointer (0x0, (VOID **) &SdIdentification);
  EfiConvertPointer (0x0, (VOID **) &SdReadBlocks);
  EfiConvertPointer (0x0, (VOID **) &SdWriteBlocks);
  EfiConvertPointer (0x0, (VOID **) &SdReadBlocksIo);
  EfiConvertPointer (0x0, (VOID **) &SdWriteBlocksIo);
  EfiConvertPointer (0x0, (VOID **) &SdFlushBlocksIo);
  EfiConvertPointer (0x0, (VOID **) &SdResetIo);
  EfiConvertPointer (0x0, (VOID **) &SdPowerOff);
  EfiConvertPointer (0x0, (VOID **) &SdRead);
  EfiConvertPointer (0x0, (VOID **) &SdWrite);
  EfiConvertPointer (0x0, (VOID **) &SdLed);

  EfiConvertPointer (0x0, (VOID **) &mBase);
  EfiConvertPointer (0x0, (VOID **) &buffer);
}
