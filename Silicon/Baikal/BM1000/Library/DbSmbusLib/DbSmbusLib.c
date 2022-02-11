/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/DebugLib.h>

#include <Library/DbSmbusLib.h>

typedef struct {
  UINT32  Cr1;
  UINT32  Cr2;
  UINT32  Fbcr1;
  UINT32  Fifo;
  UINT32  Scd1;
  UINT32  Scd2;
  UINT32  Adr1;
  UINT32  Adr2;
  UINT32  Isr1;
  UINT32  Imr1;
  UINT32  Ivr1;
  UINT32  Fbcr2;
  UINT32  Rsbcr1;
  UINT32  Rsbcr2;
  UINT32  Srsbcr1;
  UINT32  Srsbcr2;
  UINT32  Rssfifo;
  UINT32  Isr2;
  UINT32  Imr2;
  UINT32  Ivr2;
  UINT32  Reserved0[5];
  UINT32  Sfr;
  UINT32  Reserved1[2];
  UINT32  Tocr;
} SMBUS_CONTROLLER_REGS;

#define CR1_IRT   BIT0
#define CR1_TRS   BIT1
#define CR1_MSS   BIT2
#define CR1_IEB   BIT3
#define CR1_SAS   BIT4
#define CR1_GCA   BIT5

#define CR2_FRT   BIT0
#define CR2_FTE   BIT1
#define CR2_HBD   BIT2
#define CR2_RSE   BIT3
#define CR2_RSF   BIT4

#define SCD2_SHT  BIT7

#define ISR1_FUR  BIT0
#define ISR1_FOR  BIT1
#define ISR1_FER  BIT2
#define ISR1_RNK  BIT3
#define ISR1_ALD  BIT4
#define ISR1_FFE  BIT5
#define ISR1_TCS  BIT6

#define ISR2_MSH  BIT4

#define FIFO_SIZE  16

#define SCL_CLK_DIV  499
STATIC_ASSERT (SCL_CLK_DIV < 1024, "Incorrect SCL clock divider");

INTN
SmbusTxRx (
  IN   CONST EFI_PHYSICAL_ADDRESS  Base,
  IN   CONST UINTN                 TargetAddr,
  IN   CONST VOID * CONST          TxBuf,
  IN   CONST UINTN                 TxBufSize,
  OUT  VOID * CONST                RxBuf,
  IN   CONST UINTN                 RxBufSize
  )
{
  volatile SMBUS_CONTROLLER_REGS * CONST  SmbusRegs = (volatile SMBUS_CONTROLLER_REGS * CONST) Base;
  UINTN  RxedSize = 0;
        UINT8 * CONST  RxPtr = (UINT8 *) RxBuf;
  CONST UINT8 * CONST  TxPtr = (UINT8 *) TxBuf;

  ASSERT (SmbusRegs != NULL);
  ASSERT (TargetAddr <= 0x7F);
  ASSERT (TxBuf != NULL || !TxBufSize);
  ASSERT (RxBuf != NULL || !RxBufSize);

  SmbusRegs->Cr1   = CR1_IRT;
  SmbusRegs->Cr1   = 0;
  SmbusRegs->Cr2   = 0;
  SmbusRegs->Scd1  = SCL_CLK_DIV & 0xFF;
  SmbusRegs->Scd2  = SCD2_SHT | (SCL_CLK_DIV >> 8);
  SmbusRegs->Adr1  = TargetAddr;
  SmbusRegs->Imr1  = 0;
  SmbusRegs->Imr2  = 0;
  SmbusRegs->Fbcr2 = 0;
  SmbusRegs->Cr1   = CR1_IEB;

  if (TxBufSize > 0) {
    UINTN  TxedSize;

    SmbusRegs->Cr1 |= CR1_TRS;

    for (TxedSize = 0; TxedSize < TxBufSize;) {
      UINTN  ByteCount;
      BOOLEAN  HoldBus = FALSE;

      ByteCount = TxBufSize - TxedSize;
      if (ByteCount > FIFO_SIZE) {
        ByteCount = FIFO_SIZE;
        HoldBus = TRUE;
      }

      SmbusRegs->Fbcr1 = ByteCount;
      do {
        SmbusRegs->Fifo = TxPtr[TxedSize++];
        if (SmbusRegs->Isr1 & ISR1_FOR) {
          goto Exit;
        }
      } while (--ByteCount);

      if (HoldBus) {
        SmbusRegs->Cr2 |= CR2_HBD;
      } else {
        SmbusRegs->Cr2 &= ~CR2_HBD;
      }

      SmbusRegs->Isr1 = ISR1_TCS | ISR1_FFE | ISR1_ALD |
                        ISR1_RNK | ISR1_FER | ISR1_FOR |
                        ISR1_FUR;

      SmbusRegs->Isr2 = ISR2_MSH;

      SmbusRegs->Cr2 |= CR2_FTE;

      while (!(SmbusRegs->Isr1 &
               (ISR1_TCS | ISR1_ALD | ISR1_RNK)) &&
             !(SmbusRegs->Isr2 & ISR2_MSH));

      if (SmbusRegs->Isr1 & (ISR1_ALD | ISR1_RNK)) {
        goto Exit;
      }
    }

    SmbusRegs->Cr1 &= ~CR1_TRS;
  }

  for (RxedSize = 0; RxedSize < RxBufSize;) {
    UINTN  ByteCount = MIN (RxBufSize - RxedSize, FIFO_SIZE);

    SmbusRegs->Fbcr1 = ByteCount;
    SmbusRegs->Isr1  = ISR1_TCS | ISR1_FFE | ISR1_ALD | ISR1_RNK |
                       ISR1_FER | ISR1_FOR | ISR1_FUR;

    SmbusRegs->Cr2   = CR2_FTE;

    while ((SmbusRegs->Cr2 & CR2_FTE) &&
           !(SmbusRegs->Isr1 & (ISR1_TCS | ISR1_ALD | ISR1_RNK)));

    if (SmbusRegs->Isr1 & (ISR1_ALD | ISR1_RNK)) {
      goto Exit;
    }

    do {
      RxPtr[RxedSize] = SmbusRegs->Fifo;

      if (SmbusRegs->Isr1 & ISR1_FUR) {
        goto Exit;
      }

      ++RxedSize;
    } while (--ByteCount);
  }

Exit:
  SmbusRegs->Cr1 = 0;
  return RxedSize;
}
