
#ifndef __BAIKAL_SPI_H__
#define __BAIKAL_SPI_H__

#include <stdint.h>

#define SPI_BASE                0x20210000
#define SPI_OFFSET              0x00001000

#define SPI_PORT(p)             (SPI_BASE + (p) * SPI_OFFSET)
#define SPI_CTRLR0(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x00) )
#define SPI_CTRLR1(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x04) )
#define SPI_SSIENR(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x08) )
#define SPI_MWCR(p)            *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x0C) )
#define SPI_SER(p)             *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x10) )
#define SPI_BAUDR(p)           *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x14) )
#define SPI_TXFTLR(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x18) )
#define SPI_RXFTLR(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x1C) )
#define SPI_TXFLR(p)           *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x20) )
#define SPI_RXFLR(p)           *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x24) )
#define SPI_SR(p)              *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x28) )
#define SPI_IMR(p)             *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x2C) )
#define SPI_IS(p)              *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x30) )
#define SPI_RISR(p)            *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x34) )
#define SPI_TXOICR(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x38) )
#define SPI_RXOICR(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x3C) )
#define SPI_RXUICR(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x40) )
#define SPI_MSTICR(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x44) )
#define SPI_ICR(p)             *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x48) )
#define SPI_DMACR(p)           *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x4C) )
#define SPI_DMATDLR(p)         *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x50) )
#define SPI_DMARDLR(p)         *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x54) )
#define SPI_IDR(p)             *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x58) )
#define SPI_SSI_VERSION_ID(p)  *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x5C) )
#define SPI_DR(p)              *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0x60) )
#define SPI_DR35(p)            *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0xEC) )
#define SPI_RX_SAMPLE_DLY(p)   *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0xF0) )
#define SPI_RSVD_0(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0xF4) )
#define SPI_RSVD_1(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0xF8) )
#define SPI_RSVD_2(p)          *(volatile uint32_t*) ( (intptr_t) (SPI_PORT(p) + 0xFC) )

#define SSIENR_SSI_DE (0)
#define SSIENR_SSI_EN (1)

/* CTRLR0 */
#define SPI_SLV_OE_OFFSET       10
#define SPI_SLV_MASK            (1 << SPI_SLV_OE_OFFSET)
#define SPI_SLV_OE              1

#define SPI_TMOD_OFFSET         8
#define SPI_TMOD_MASK           (3 << SPI_TMOD_OFFSET)
#define SPI_TMOD_TR             0
#define SPI_TMOD_TO             1
#define SPI_TMOD_RO             2
#define SPI_TMOD_EPROMREAD      3

#define SPI_SCPOL_OFFSET        7
#define SPI_SCPOL_MASK          (3 << SPI_SCPOL_OFFSET)
#define SPI_SCPOL_LOW           0
#define SPI_SCPOL_HIGH          1

#define SPI_SCPH_OFFSET         6
#define SPI_SCPH_MASK           (3 << SPI_SCPH_OFFSET)
#define SPI_SCPH_MIDDLE         0
#define SPI_SCPH_START          1

#define SPI_FRF_OFFSET          4
#define SPI_FRF_MASK            (3 << SPI_FRF_OFFSET)
#define SPI_FRF_SPI             0
#define SPI_FRF_SSP             1
#define SPI_FRF_MICROWIRE       2
#define SPI_FRF_RESV            3

/* Data Frame Size */
#define SPI_DFS_OFFSET          0
#define SPI_DFS_MASK            (3 << SPI_DFS_OFFSET)
#define SPI_DFS(x)              (x - 1)

/* SR */
#define SPI_SR_DCOL             (1 << 6)
#define SPI_SR_TXE              (1 << 5)
#define SPI_SR_RFF              (1 << 4)
#define SPI_SR_RFNE             (1 << 3)
#define SPI_SR_TFE              (1 << 2)
#define SPI_SR_TFNF             (1 << 1)
#define SPI_SR_BUSY             (1 << 0)

/* SPI Flash status register */
#define SPI_FLASH_SR_WIP        (1 << 0)    /* Write In Progress */
#define SPI_FLASH_SR_WEL        (1 << 1)    /* Write Enable Latch */

/* Flag Status Register */
#define SPI_FLAG_PE             (1 << 7)    /* P/E Controller (not WIP) */
#define SPI_FLAG_ER_SUSPEND     (1 << 6)    /* Erase Suspend */
#define SPI_FLAG_ERASE          (1 << 5)    /* Erase */
#define SPI_FLAG_PROGRAM        (1 << 4)    /* Program */
#define SPI_FLAG_VPP            (1 << 3)    /* VPP */
#define SPI_FLAG_PR_SUSPEND     (1 << 2)    /* Program Suspend */
#define SPI_FLAG_PROTECTION     (1 << 1)    /* Protection */
#define SPI_FLAG_               (1 << 0)    /* RESERVED */

/* ISR, IMR, RISR */
#define SPI_INT_TXEI            (1 << 0)
#define SPI_INT_TXOI            (1 << 1)
#define SPI_INT_RXUI            (1 << 2)
#define SPI_INT_RXOI            (1 << 3)
#define SPI_INT_RXFI            (1 << 4)
#define SPI_INT_MSTI            (1 << 5)


/* Commands */
#define SPI_FLASH_RDID          0x9F /* (0, 1-20) Read identification. */
#define SPI_FLASH_READ          0x03 /* (3, 1-∞ ) Read Data Bytes */
#define SPI_FLASH_WREN          0x06 /* (0, 0   ) Write Enable */
#define SPI_FLASH_WRDI          0x04 /* (0, 0   ) Write Disable */
#define SPI_FLASH_PP            0x02 /* (3, 256 ) Page Program */
#define SPI_FLASH_SSE           0x20 /* (3, 0   ) SubSector Erase */
#define SPI_FLASH_SE            0xD8 /* (3, 0   ) Sector Erase */
#define SPI_FLASH_RDSR          0x05 /* (0, 1   ) Read Status Register */
#define SPI_FLASH_WRSR          0x01 /* (0, 1-∞ ) Write Status Register */
#define SPI_FLASH_RDLR          0xE8 /* (3, 1-∞ ) Read Lock Register */
#define SPI_FLASH_WRLR          0xE5 /* (3, 1   ) Write Lock Register */
#define SPI_FLASH_RFSR          0x70 /* (1 to ∞)  Read Flag Status Register */
#define SPI_FLASH_CLFSR         0x50 /* (0) Clear Flag Status Register */
#define SPI_FLASH_BE            0xC7 /* (0) Bulk Erase */
#define SPI_FLASH_RSTEN         0x66 /* Reset Enable */
#define SPI_FLASH_RST           0x99 /* Reset Memory */

/* 256 MBit */
#define SPI_FLASH_EN4BYTEADDR   0xB7 /* Enter 4-byte address mode */
#define SPI_FLASH_EX4BYTEADDR   0xE9 /* Exit 4-byte address mode */
#define SPI_FLASH_WREAR         0xC5 /* Write Extended Address Register */
#define SPI_FLASH_RDEAR         0xC8 /* Read Extended Address Register */

/* const */
#define SPI_ID_SIZE             20
#define SPI_FIFO_SIZE           64
#define SPI_AUTO_READ_SIZE      0x10000
#define SPI_PAGE_SIZE           256 /* (3, 256 ) Page Program */
#define SPI_BAUDR_DEFAULT       12
#define SPI_BAUDR_MAX           0xFFFE

/* put address */
#define SPI_ADR_LEN_4BYTE 4
#define SPI_SET_ADDRESS_4BYTE(a, b)      \
    ({                                   \
        uint8_t* _b = (void*)(b);        \
        _b[1] = (((a) >> 8 * 3) & 0xFF); \
        _b[2] = (((a) >> 8 * 2) & 0xFF); \
        _b[3] = (((a) >> 8 * 1) & 0xFF); \
        _b[4] = (((a) >> 8 * 0) & 0xFF); \
    })
#define SPI_ADR_LEN_3BYTE 3
#define SPI_SET_ADDRESS_3BYTE(a, b)      \
    ({                                   \
        uint8_t* _b = (void*)(b);        \
        _b[1] = (((a) >> 8 * 2) & 0xFF); \
        _b[2] = (((a) >> 8 * 1) & 0xFF); \
        _b[3] = (((a) >> 8 * 0) & 0xFF); \
    })
#define SPI_CMD_LEN (1 + SPI_ADR_LEN_4BYTE)

#endif
