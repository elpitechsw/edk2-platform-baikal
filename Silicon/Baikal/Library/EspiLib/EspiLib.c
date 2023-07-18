/** @file
  Copyright (c) 2022 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/EspiLib.h>
#include <Library/GpioLib.h>
#include <Library/CmuLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <BM1000.h>

#define MMAVLSP_CMU0_CLKCHCTL_ESPI  (0x20000000 + 0x20 + 5 * 0x10)
#define ESPI_FIFO_LEN  256

#define ESPI_CR1(Base)       *(volatile UINT32 *)((Base) + 0x00) // Control 1
#define ESPI_CR2(Base)       *(volatile UINT32 *)((Base) + 0x04) // Control 2
#define ESPI_TX_FIFO(Base)   *(volatile UINT32 *)((Base) + 0x0C) // Tx FIFO
#define ESPI_TX_FBCAR(Base)  *(volatile UINT32 *)((Base) + 0x14) // Tx byte count
#define ESPI_TX_FAETR(Base)  *(volatile UINT32 *)((Base) + 0x18) // Tx almost empty threshold
#define ESPI_RX_FIFO(Base)   *(volatile UINT32 *)((Base) + 0x1C) // Rx FIFO
#define ESPI_RX_FBCAR(Base)  *(volatile UINT32 *)((Base) + 0x24) // Rx byte count
#define ESPI_RX_FAFTR(Base)  *(volatile UINT32 *)((Base) + 0x28) // Rx almost full threshold
#define ESPI_ISR(Base)       *(volatile UINT32 *)((Base) + 0x30) // IRQ status
#define ESPI_IMR(Base)       *(volatile UINT32 *)((Base) + 0x34) // IRQ mask

typedef union {
  UINT32  Val;
  struct {
    UINT32  Scr :1; // Reset
    UINT32  Sce :1; // Enable
    UINT32  Mss :1; // Select - master/slave
    UINT32  Cph :1; // Clock phase
    UINT32  Cpo :1; // Clock polarity
  } Bits;
} ESPI_CR1_T;

#define ESPI_CR1_SCR_RESET          0
#define ESPI_CR1_SCR_NORESET        1
#define ESPI_CR1_SCE_CORE_DISABLE   0
#define ESPI_CR1_SCE_CORE_ENABLE    1
#define ESPI_CR1_MSS_MODE_MASTER    0
#define ESPI_CR1_MSS_MODE_SLAVE     1
#define ESPI_CR1_CPH_CLKPHASE_ODD   0
#define ESPI_CR1_CPH_CLKPHASE_EVEN  1
#define ESPI_CR1_CPO_CLKPOLAR_LOW   0
#define ESPI_CR1_CPO_CLKPOLAR_HIGH  1

typedef union {
  UINT32  Val;
  struct {
    UINT32  Sso :3; // Slave select (ss0-ss7)
    UINT32  Srd :1; // Read disable
    UINT32  Sri :1; // Read first byte ignore
    UINT32  Mlb :1; // Least significant bit first
    UINT32  Mte :1; // Master transfer enable
  } Bits;
} ESPI_CR2_T;

#define ESPI_CR2_SRD_RX_DISABLE     0
#define ESPI_CR2_SRD_RX_ENABLE      1
#define ESPI_CR2_SRI_FIRST_RESIEV   0
#define ESPI_CR2_SRI_FIRST_IGNORE   1
#define ESPI_CR2_MLB_MSB            0
#define ESPI_CR2_MLB_LSB            1
#define ESPI_CR2_MTE_TX_DISABLE     0
#define ESPI_CR2_MTE_TX_ENABLE      1 // Self clear

#define ESPI_RX_FAFTR_DISABLE       0xFF
#define ESPI_TX_FAETR_DISABLE       0x0
#define ESPI_ISR_CLEAR_ALL          0xFF // Clear by writing "1"
#define ESPI_IMR_DISABLE_ALL        0x0
#define ESPI_IMR_ENABLE_ALL         0xFF

// ISR, IMR
typedef union {
  UINT32  Val;
  struct {
    UINT32  TxUnderrun    :1;
    UINT32  TxOverrun     :1;
    UINT32  RxUnderrun    :1;
    UINT32  RxOverrun     :1;
    UINT32  TxAlmostEmpty :1;
    UINT32  RxAlmostFull  :1;
    UINT32  TxDoneMaster  :1;
    UINT32  TxDoneSlave   :1;
    UINT32  Alert         :8;
    UINT32  RxCrcError    :1;
  } Bits;
} ESPI_IRQ_T;

STATIC UINTN  AdrMode;

//
// Low-level functions
//

STATIC
VOID
EspiReset (
  IN UINTN  Base
  )
{
  ESPI_CR1_T  Cr1;

  Cr1.Val         = ESPI_CR1 (Base);
  Cr1.Bits.Scr    = ESPI_CR1_SCR_RESET;
  ESPI_CR1 (Base) = Cr1.Val;
}

STATIC
VOID
EspiSelect (
  IN UINTN  Base,
  IN UINTN  Line,
  IN UINTN  ChipSelect
  )
{
  ESPI_CR2_T  Cr2;

  Cr2.Val         = ESPI_CR2 (Base);
  Cr2.Bits.Sso    = Line;
  ESPI_CR2 (Base) = Cr2.Val;

#ifdef BM1000_GPIO32_BASE
  EFI_PHYSICAL_ADDRESS  GpioBase = BM1000_GPIO32_BASE;

  GpioDirSet (GpioBase, Line);
  if (ChipSelect) {
    GpioOutSet (GpioBase, Line);
  } else {
    GpioOutRst (GpioBase, Line);
  }
#endif
}

STATIC
VOID
EspiTxEnable (
  IN UINTN  Base
  )
{
  INTN        Cnt;
  ESPI_CR2_T  Cr2;

  Cnt     = ESPI_TX_FBCAR (Base);
  Cr2.Val = ESPI_CR2 (Base);

  if (Cnt && Cr2.Bits.Mte != ESPI_CR2_MTE_TX_ENABLE) {
    Cr2.Bits.Mte = ESPI_CR2_MTE_TX_ENABLE;
    ESPI_CR2 (Base) = Cr2.Val;
  }
}

STATIC
VOID
EspiRxEnable (
  IN UINTN  Base
  )
{
  ESPI_CR2_T  Cr2;

  Cr2.Val         = ESPI_CR2 (Base);
  Cr2.Bits.Srd    = ESPI_CR2_SRD_RX_ENABLE;
  ESPI_CR2 (Base) = Cr2.Val;
}

STATIC
INTN
EspiTxRx (
  IN UINTN  Base,
  IN VOID   *Tx_,
  IN VOID   *Rx_,
  IN UINTN  Len
  )
{
  UINT32      Cnt;
  UINT32      Cnt0;
  UINT8       Data;
  UINT32      Free;
  UINT8       *Tx = Tx_;
  UINT8       *Rx = Rx_;
  INTN        Timeout;
  ESPI_IRQ_T  Status;

  ESPI_ISR (Base) = ESPI_ISR_CLEAR_ALL;

  while (Len) {
    Free = ESPI_FIFO_LEN - ESPI_TX_FBCAR (Base);
    Cnt0 = MIN (Free, Len);

    Cnt = Cnt0;
    while (Cnt) {
      if (Tx) {
        Data = *Tx++;
      } else {
        Data = 0xFF;
      }

      ESPI_TX_FIFO (Base) = Data;
      Cnt--;
    }

    EspiTxEnable (Base);

    Timeout = 100;
    Cnt = Cnt0;
    while (Cnt) {
      if (ESPI_RX_FBCAR (Base)) {
        Data = ESPI_RX_FIFO (Base);
        if (Rx) {
          *Rx++ = Data;
        }

        Cnt--;
      } else {
        gBS->Stall (1000); // 1ms
        if (Timeout-- == 0) {
          return -1;
        }
      }
    }

    Len -= Cnt0;
  }

  Status.Val = ESPI_ISR (Base);
  if (Status.Bits.TxUnderrun ||
      Status.Bits.TxOverrun  ||
      Status.Bits.RxUnderrun ||
      Status.Bits.RxOverrun  ||
      Status.Bits.RxCrcError) {
    return -1;
  }

  return 0;
}

//
// Bus functions
//

INTN
EspiInit (
  IN UINTN  Base
  )
{
  ESPI_CR1_T  Cr1;
  ESPI_CR2_T  Cr2;

  EspiReset (Base);

  // IRQ
  ESPI_IMR (Base) = ESPI_IMR_DISABLE_ALL;
  ESPI_ISR (Base) = ESPI_ISR_CLEAR_ALL;

  // Control 1
  Cr1.Val         = ESPI_CR1 (Base);
  Cr1.Bits.Scr    = ESPI_CR1_SCR_NORESET;
  Cr1.Bits.Sce    = ESPI_CR1_SCE_CORE_ENABLE;
  Cr1.Bits.Mss    = ESPI_CR1_MSS_MODE_MASTER;
  Cr1.Bits.Cph    = ESPI_CR1_CPH_CLKPHASE_EVEN;
  Cr1.Bits.Cpo    = ESPI_CR1_CPO_CLKPOLAR_LOW;
  ESPI_CR1 (Base) = Cr1.Val;

  // Control 2
  Cr2.Val         = ESPI_CR2 (Base);
  Cr2.Bits.Srd    = ESPI_CR2_SRD_RX_DISABLE;
  Cr2.Bits.Mte    = ESPI_CR2_MTE_TX_DISABLE;
  Cr2.Bits.Sri    = ESPI_CR2_SRI_FIRST_RESIEV;
  Cr2.Bits.Mlb    = ESPI_CR2_MLB_MSB;
  ESPI_CR2 (Base) = Cr2.Val;

  // Threshold
  ESPI_TX_FAETR (Base) = ESPI_FIFO_LEN / 2;
  ESPI_RX_FAFTR (Base) = ESPI_FIFO_LEN / 2;

  // Full duplex
  EspiRxEnable (Base);

  // Clock
  CmuClkChSetRate (MMAVLSP_CMU0_CLKCHCTL_ESPI, 10 * 1000 * 1000);
  return 0;
}

INTN
EspiTransfer (
  IN UINTN   Base,
  IN UINTN   Line,
  IN VOID   *Cmd,
  IN UINTN   CmdLen,
  IN VOID   *Tx,
  IN UINTN   TxLen,
  IN VOID   *Rx,
  IN UINTN   RxLen
  )
{
  INTN  Err;

  EspiSelect (Base, Line, 0);

  Err = EspiTxRx (Base, Cmd, NULL, CmdLen);
  if (Err) {
    return Err;
  }

  Err = EspiTxRx (Base, Tx, NULL, TxLen);
  if (Err) {
    return Err;
  }

  Err = EspiTxRx (Base, NULL, Rx, RxLen);
  if (Err) {
    return Err;
  }

  EspiSelect (Base, Line, 1);
  return 0;
}

//
// JEDEC-specific functions
//

STATIC
INTN
EspiExec (
  IN UINTN   Base,
  IN UINTN   Line,
  IN UINTN   CmdOp,
  IN UINTN   Address,
  IN VOID   *Buf_,
  IN UINTN   LenBuf
  )
{
  INTN    Err;
  UINT8   Cmd[SPI_CMD_LEN];
  UINT8   *In = 0, *Out = 0;
  UINT8   *Buf = Buf_;
  UINT32  LenCmd = 0, LenIn = 0, LenOut = 0;

  // Save the SPI flash instruction
  Cmd[0] = CmdOp;
  LenCmd += sizeof (UINT8);

  // Prepare arguments for the SPI transaction
  switch (CmdOp) {
  case CMD_FLASH_RDID:
  case CMD_FLASH_RDSR:
  case CMD_FLASH_RDNVCR:
  case CMD_FLASH_RDVCR:
  case CMD_FLASH_RDVECR:
  case CMD_FLASH_RDEAR:
  case CMD_FLASH_RFSR:
    Out = Buf;
    LenOut = LenBuf;
    break;

  case CMD_FLASH_READ:
    Out = Buf;
    LenOut = LenBuf;
    if (AdrMode == ADR_MODE_4BYTE) {
      SPI_SET_ADDRESS_4BYTE (Address, Cmd);
      LenCmd += SPI_ADR_LEN_4BYTE;
    } else if (AdrMode == ADR_MODE_3BYTE) {
      SPI_SET_ADDRESS_3BYTE (Address, Cmd);
      LenCmd += SPI_ADR_LEN_3BYTE;
    } else {
      return -1;
    }

    break;

  case CMD_FLASH_SSE:
  case CMD_FLASH_SE:
    if (AdrMode == ADR_MODE_4BYTE) {
      SPI_SET_ADDRESS_4BYTE (Address, Cmd);
      LenCmd += SPI_ADR_LEN_4BYTE;
    } else if (AdrMode == ADR_MODE_3BYTE) {
      SPI_SET_ADDRESS_3BYTE (Address, Cmd);
      LenCmd += SPI_ADR_LEN_3BYTE;
    } else {
      return -1;
    }

    break;

  case CMD_FLASH_WREAR:
  case CMD_FLASH_WRSR:
  case CMD_FLASH_WRVCR:
  case CMD_FLASH_WRVECR:
  case CMD_FLASH_WRNVCR:
    In = Buf;
    LenIn = LenBuf;
    break;

  case CMD_FLASH_EN4BYTEADDR:
  case CMD_FLASH_EX4BYTEADDR:
  case CMD_FLASH_WRDI:
  case CMD_FLASH_WREN:
  case CMD_FLASH_BE:
    break;

  case CMD_FLASH_PP:
    if (LenBuf > SPI_MAX_WRITE) {
      return -2;
    }

    In = Buf;
    LenIn = LenBuf;
    if (AdrMode == ADR_MODE_4BYTE) {
      SPI_SET_ADDRESS_4BYTE (Address, Cmd);
      LenCmd += SPI_ADR_LEN_4BYTE;
    } else if (AdrMode == ADR_MODE_3BYTE) {
      SPI_SET_ADDRESS_3BYTE (Address, Cmd);
      LenCmd += SPI_ADR_LEN_3BYTE;
    } else {
      return -1;
    }

    break;

  default:
    return -1;
  }

  Err = EspiTransfer (Base, Line, Cmd, LenCmd, In, LenIn, Out, LenOut);
  return Err;
}

STATIC
INTN
EspiWren (
  IN UINTN  Base,
  IN UINTN  Line
  )
{
  INTN   Err;
  UINT8  Status;

  Err = EspiExec (Base, Line, CMD_FLASH_WREN, 0, 0, 0);
  if (Err) {
    return Err;
  }

  Err = EspiExec (Base, Line, CMD_FLASH_RDSR, 0, &Status, 1);
  if (Err) {
    return Err;
  }

  return !(Status & SPI_FLASH_SR_WEL);
}

STATIC
INTN
EspiWait (
  IN UINTN  Base,
  IN UINTN  Line
  )
{
  INTN   Err;
  INTN   Timeout;
  UINT8  Status;

  Timeout = 1000;
  do {
    Err = EspiExec (Base, Line, CMD_FLASH_RDSR, 0, &Status, 1);
    if (Err) {
      return Err;
    }

    gBS->Stall (1000);
    if (Timeout-- == 0) {
      return -1;
    }
  } while (Status & SPI_FLASH_SR_WIP);

  return 0;
}

INTN
EspiDetect (
  IN UINTN  Base,
  IN UINTN  Line
  )
{
  INTN   Err;
  INTN   Try = 3;
  UINT8  Id[SPI_NOR_MAX_ID_LEN];

  while (Try--) {
    Err = EspiExec (Base, Line, CMD_FLASH_RDID, 0, Id, sizeof (Id));
    if (Err) {
      return Err;
    }

    if ((Id[0] == 0x00 && Id[1] == 0x00 && Id[2] == 0x00) ||
        (Id[0] == 0xFF && Id[1] == 0xFF && Id[2] == 0xFF)) {
      return -1;
    }
  }

  DEBUG ((
    EFI_D_INFO,
    "%a: JEDEC ID = 0x%02x%02x%02x\n",
    __func__,
    Id[0],
    Id[1],
    Id[2]
    ));
  return 0;
}

INTN
EspiInfo (
  IN UINTN  Base,
  IN UINTN  Line,
  IN UINTN  *SectorSize,
  IN UINTN  *SectorCount
  )
{
  if (SectorSize == NULL || SectorCount == NULL) {
    return -1;
  }

  // Default
  *SectorSize  = 4 * 1024;
  *SectorCount = 4 * 1024;

  EspiMode3 (Base, Line);
  return 0;
}

INTN
EspiRead (
  IN UINTN  Base,
  IN UINTN  Line,
  IN UINTN  Addr,
  IN VOID   *Data_,
  IN UINTN  Size
  )
{
  UINT8  *Data = Data_;
  INTN    Err;

  while (Size) {
    INTN  Part = MIN (Size, SPI_MAX_READ);

    Err = EspiExec (Base, Line, CMD_FLASH_READ, Addr, Data, Part);
    if (Err) {
      return Err;
    }

    Addr += Part;
    Data += Part;
    Size -= Part;
  }

  return 0;
}

INTN
EspiWrite (
  IN UINTN  Base,
  IN UINTN  Line,
  IN UINTN  Addr,
  IN VOID   *Data_,
  IN UINTN  Size
  )
{
  UINT8  *Data = Data_;
  INTN   Err;

  while (Size) {
    INTN  Part = MIN (Size, SPI_MAX_WRITE);
    INTN  Page1 = Addr / SPI_MAX_WRITE;
    INTN  Page2 = (Addr + Part) / SPI_MAX_WRITE;

    if (Page1 != Page2) {
      Part = Page2 * SPI_MAX_WRITE - Addr;
    }

    Err = EspiWren (Base, Line);
    if (Err) {
      return Err;
    }

    Err = EspiExec (Base, Line, CMD_FLASH_PP, Addr, Data, Part);
    if (Err) {
      return Err;
    }

    Err = EspiWait (Base, Line);
    if (Err) {
      return Err;
    }

    Addr += Part;
    Data += Part;
    Size -= Part;
  }

  return 0;
}

INTN
EspiErase (
  IN UINTN  Base,
  IN UINTN  Line,
  IN UINTN  Addr,
  IN UINTN  Size
  )
{
  INTN  Err;

  if (Size % SPI_SUBSECTOR) {
    return -1;
  }

  while (Size) {
    Err = EspiWren (Base, Line);
    if (Err) {
      return Err;
    }

    Err = EspiExec (Base, Line, CMD_FLASH_SSE, Addr, 0, 0);
    if (Err) {
      return Err;
    }

    Err = EspiWait (Base, Line);
    if (Err) {
      return Err;
    }

    Addr += SPI_SUBSECTOR;
    Size -= SPI_SUBSECTOR;
  }

  return 0;
}

INTN
EspiMode3 (
  IN UINTN  Base,
  IN UINTN  Line
  )
{
  INTN  Err;

  Err = EspiWren (Base, Line);
  if (Err) {
    return Err;
  }

  Err = EspiExec (Base, Line, CMD_FLASH_EX4BYTEADDR, 0, 0, 0);
  if (Err) {
    return Err;
  }

  AdrMode = ADR_MODE_3BYTE;
  return 0;
}

INTN
EspiMode4 (
  IN UINTN  Base,
  IN UINTN  Line
  )
{
  INTN  Err;

  Err = EspiWren (Base, Line);
  if (Err) {
    return Err;
  }

  Err = EspiExec (Base, Line, CMD_FLASH_EN4BYTEADDR, 0, 0, 0);
  if (Err) {
    return Err;
  }

  AdrMode = ADR_MODE_4BYTE;
  return 0;
}
