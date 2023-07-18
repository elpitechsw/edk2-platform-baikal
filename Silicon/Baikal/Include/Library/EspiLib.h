/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef ESPI_LIB_H_
#define ESPI_LIB_H_

#define SPI_NOR_MAX_ID_LEN  20

#define ADR_MODE_3BYTE     3
#define SPI_ADR_LEN_3BYTE  3
#define SPI_SET_ADDRESS_3BYTE(a, b) \
({  UINT8 *_b = (VOID *)(b);        \
  _b[1] = (((a) >> 8 * 2) & 0xFF);  \
  _b[2] = (((a) >> 8 * 1) & 0xFF);  \
  _b[3] = (((a) >> 8 * 0) & 0xFF);  \
})

#define ADR_MODE_4BYTE     4
#define SPI_ADR_LEN_4BYTE  4
#define SPI_SET_ADDRESS_4BYTE(a, b) \
({  UINT8 *_b = (VOID *)(b);        \
  _b[1] = (((a) >> 8 * 3) & 0xFF);  \
  _b[2] = (((a) >> 8 * 2) & 0xFF);  \
  _b[3] = (((a) >> 8 * 1) & 0xFF);  \
  _b[4] = (((a) >> 8 * 0) & 0xFF);  \
})

#define SPI_CMD_LEN     (1 + SPI_ADR_LEN_4BYTE)
#define SPI_MAX_READ    0x10000
#define SPI_MAX_WRITE   256 // (3, 256) Page Program
#define SPI_SECTOR      (64 * 1024)
#define SPI_SUBSECTOR   (4 * 1024)

// SPI Flash commands
#define CMD_FLASH_RDID         0x9F // (0, 1 .. 20) Read identification
#define CMD_FLASH_READ         0x03 // (3, 1 .. inf) Read Data Bytes
#define CMD_FLASH_WREN         0x06 // (0, 0) Write Enable
#define CMD_FLASH_WRDI         0x04 // (0, 0) Write Disable
#define CMD_FLASH_PP           0x02 // (3, 256) Page Program
#define CMD_FLASH_SSE          0x20 // (3, 0) SubSector Erase
#define CMD_FLASH_SE           0xD8 // (3, 0) Sector Erase
#define CMD_FLASH_RDSR         0x05 // (0, 1) Read Status Register
#define CMD_FLASH_WRSR         0x01 // (0, 1 .. inf) Write Status Register
#define CMD_FLASH_RFSR         0x70 // (1 .. inf) Read Flag Status Register
#define CMD_FLASH_BE           0xC7 // (0) Bulk Erase
#define CMD_FLASH_EN4BYTEADDR  0xB7 // Enter 4-byte address mode
#define CMD_FLASH_EX4BYTEADDR  0xE9 // Exit 4-byte address mode
#define CMD_FLASH_CLFSR        0x50 // Clear Flag Status Register
#define CMD_FLASH_RDNVCR       0xB5 // Read Non Volatile Configuration Register
#define CMD_FLASH_WRNVCR       0xB1 // Write Non Volatile Configuration Register
#define CMD_FLASH_RDVCR        0x85 // Read Volatile Configuration Register
#define CMD_FLASH_WRVCR        0x81 // Write Volatile Configuration Register
#define CMD_FLASH_RDVECR       0x65 // Read Volatile Enhanced Configuration Register
#define CMD_FLASH_WRVECR       0x61 // Write Volatile Enhanced Configuration Register
#define CMD_FLASH_WREAR        0xC5 // Write Extended Address Register
#define CMD_FLASH_RDEAR        0xC8 // Read Extended Address Register

// SPI Flash status register
#define SPI_FLASH_SR_WIP  BIT0 // Write In Progress
#define SPI_FLASH_SR_WEL  BIT1 // Write Enable Latch

//
// Bus functions
//
INTN
EspiInit (
  IN UINTN   Base
  );

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
  );

//
// JEDEC-specific functions
//
INTN
EspiDetect (
  IN UINTN   Base,
  IN UINTN   Line
  );

INTN
EspiInfo (
  IN UINTN   Base,
  IN UINTN   Line,
  IN UINTN  *SectorSize,
  IN UINTN  *SectorCount
  );

INTN
EspiMode3 (
  IN UINTN   Base,
  IN UINTN   Line
  );

INTN
EspiMode4 (
  IN UINTN   Base,
  IN UINTN   Line
  );

INTN
EspiRead (
  IN UINTN   Base,
  IN UINTN   Line,
  IN UINTN   Adr,
  IN VOID   *Data,
  IN UINTN   Size
  );

INTN
EspiWrite (
  IN UINTN   Base,
  IN UINTN   Line,
  IN UINTN   Adr,
  IN VOID   *Data,
  IN UINTN   Size
  );

INTN
EspiErase (
  IN UINTN   Base,
  IN UINTN   Line,
  IN UINTN   Adr,
  IN UINTN   Size
  );

#endif // ESPI_LIB_H_
