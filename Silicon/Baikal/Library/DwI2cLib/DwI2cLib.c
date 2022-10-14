/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/DwI2cLib.h>
#include <Library/TimerLib.h>

typedef struct {
  UINT32  IcCon;
  UINT32  IcTar;
  UINT32  IcSar;
  UINT32  IcHsMaddr;
  UINT32  IcDataCmd;
  UINT32  IcSsSclHcnt;
  UINT32  IcSsSclLcnt;
  UINT32  IcFsSclHcnt;
  UINT32  IcFsSclLcnt;
  UINT32  IcHsSclHcnt;
  UINT32  IcHsSclLcnt;
  UINT32  IcIntrStat;
  UINT32  IcIntrMask;
  UINT32  IcRawIntrStat;
  UINT32  IcRxTl;
  UINT32  IcTxTl;
  UINT32  IcClrIntr;
  UINT32  IcClrRxUnder;
  UINT32  IcClrRxOver;
  UINT32  IcClrTxOver;
  UINT32  IcClrRdReq;
  UINT32  IcClrTxAbrt;
  UINT32  IcClrRxDone;
  UINT32  IcClrActivity;
  UINT32  IcClrStopDet;
  UINT32  IcClrStartDet;
  UINT32  IcClrGenCall;
  UINT32  IcEnable;
  UINT32  IcStatus;
  UINT32  IcTxFlr;
  UINT32  IcRxFlr;
  UINT32  IcSdaHold;
  UINT32  IcTxAbrtSource;
  UINT32  Reserved0;
  UINT32  IcDmaCr;
  UINT32  IcDmaTdlr;
  UINT32  IcDmaRdlr;
  UINT32  IcSdaSetup;
  UINT32  IcAckGeneralCall;
  UINT32  IcEnableStatus;
  UINT32  IcFsSpkLen;
  UINT32  IcHsSpkLen;
} I2C_CONTROLLER_REGS;

#define IC_SPEED_MODE_STANDARD     1
#define IC_SPEED_MODE_FAST         2
#define IC_CON_MASTER_MODE         BIT0
#ifndef ELPITECH
#define IC_CON_SPEED               (IC_SPEED_MODE_FAST << 1)
#else
#define IC_CON_SPEED               (IC_SPEED_MODE_STANDARD << 1)
#endif
#define IC_CON_IC_SLAVE_DISABLE    BIT6

#define IC_DATA_CMD_CMD            BIT8
#define IC_DATA_CMD_STOP           BIT9

#define IC_RAW_INTR_STAT_TX_ABRT   BIT6

#define IC_ENABLE_ENABLE           BIT0

#define IC_STATUS_TFNF             BIT1
#define IC_STATUS_TFE              BIT2
#define IC_STATUS_RFNE             BIT3
#define IC_STATUS_MST_ACTIVITY     BIT5

#define IC_ENABLE_STATUS_IC_EN     BIT0

#define IC_CLK                      166
#define NANO_TO_MICRO              1000
#define MIN_FS_SCL_HIGHTIME         600
#define MIN_FS_SCL_LOWTIME         1300

INTN
I2cTxRx (
  IN   CONST EFI_PHYSICAL_ADDRESS  Base,
  IN   CONST UINTN                 TargetAddr,
  IN   CONST VOID * CONST          TxBuf,
  IN   CONST UINTN                 TxBufSize,
  OUT  VOID * CONST                RxBuf,
  IN   CONST UINTN                 RxBufSize
  )
{
  UINT64  ActivityTimestamp;
  EFI_STATUS  Status;
  volatile I2C_CONTROLLER_REGS * CONST  I2cRegs = (volatile I2C_CONTROLLER_REGS * CONST) Base;
  UINTN  RxedSize = 0;
  UINT8 * CONST  RxPtr = (UINT8 *) RxBuf;
  UINTN  TxedSize = 0;
  CONST UINT8 * CONST  TxPtr = (UINT8 *) TxBuf;

  ASSERT (I2cRegs != NULL);
  ASSERT (TargetAddr <= 0x7F);
  ASSERT (TxBuf != NULL || !TxBufSize);
  ASSERT (RxBuf != NULL || !RxBufSize);

  I2cRegs->IcEnable    = 0;
  I2cRegs->IcCon       = IC_CON_IC_SLAVE_DISABLE | IC_CON_SPEED | IC_CON_MASTER_MODE;
  I2cRegs->IcTar       = TargetAddr;
  I2cRegs->IcRxTl      = 0;
  I2cRegs->IcTxTl      = 0;
  I2cRegs->IcIntrMask  = 0;
  I2cRegs->IcFsSclHcnt = (IC_CLK * MIN_FS_SCL_HIGHTIME) / NANO_TO_MICRO;
  I2cRegs->IcFsSclLcnt = (IC_CLK * MIN_FS_SCL_LOWTIME)  / NANO_TO_MICRO;
  I2cRegs->IcEnable    = IC_ENABLE_ENABLE;
  ActivityTimestamp    = GetPerformanceCounter ();

  for (;;) {
    CONST  UINTN  IcStatus = I2cRegs->IcStatus;

    if (RxedSize < RxBufSize && (IcStatus & IC_STATUS_RFNE)) {
      RxPtr[RxedSize++] = I2cRegs->IcDataCmd;
      ActivityTimestamp = GetPerformanceCounter ();
      continue;
    }

    if (I2cRegs->IcRawIntrStat & IC_RAW_INTR_STAT_TX_ABRT) {
      Status = EFI_DEVICE_ERROR;
      break;
    } else if (TxedSize < TxBufSize + RxBufSize) {
      if (IcStatus & IC_STATUS_TFNF) {
        //
        // Driver must set STOP bit if IC_EMPTYFIFO_HOLD_MASTER_EN
        // is set. However, IC_EMPTYFIFO_HOLD_MASTER_EN cannot be
        // detected from the registers. So the STOP bit is always set
        // when writing/reading the last byte.
        //
        CONST UINT32 Stop = TxedSize < TxBufSize + RxBufSize - 1 ?
                            0 : IC_DATA_CMD_STOP;

        if (TxedSize < TxBufSize) {
          I2cRegs->IcDataCmd = Stop | TxPtr[TxedSize];
        } else {
          I2cRegs->IcDataCmd = Stop | IC_DATA_CMD_CMD;
        }

        ActivityTimestamp = GetPerformanceCounter ();
        ++TxedSize;
      } else if (!(IcStatus & IC_STATUS_MST_ACTIVITY) &&
                  GetTimeInNanoSecond (GetPerformanceCounter () - ActivityTimestamp) > 100000000) {
        Status = EFI_DEVICE_ERROR;
        break;
      }
    } else if ( (IcStatus & IC_STATUS_TFE) &&
               !(IcStatus & IC_STATUS_MST_ACTIVITY)) {
      Status = EFI_SUCCESS;
      break;
    }
  }

  I2cRegs->IcEnable = 0;

  while (I2cRegs->IcEnableStatus & IC_ENABLE_STATUS_IC_EN);

  if (EFI_ERROR (Status)) {
    return -1;
  }

  return RxedSize;
}
