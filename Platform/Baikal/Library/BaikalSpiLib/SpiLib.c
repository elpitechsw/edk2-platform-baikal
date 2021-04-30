/** @file
 *
 * Spi Lib
 * Copyright (C) 2014 Baikal Electronics.
 *
 */

#include <stdint.h>
#include <Library/BaikalDebug.h>
#include <Platform/BaikalFlashMap.h>
#include <Library/BaikalSpiLib.h>
#include "Spi.h"


#define SPI_MAX_READ         SPI_AUTO_READ_SIZE
int en4byte = 0;


int timeout;
#define INIT()  timeout = 1000000
#define TRY()   if(!timeout--) goto exit

/*
------------------------------
GPIO
------------------------------
*/
#define GPIO_BASE  0x20200000
#define GPIO_DATA  *(volatile uint32_t*) ((intptr_t) (GPIO_BASE + 0x00))
#define GPIO_DIR   *(volatile uint32_t*) ((intptr_t) (GPIO_BASE + 0x04))

#define CS_BOOT   20
#define CS_NORMAL 24


void gpio_dir (int line, int output)
{
    if (output)
        GPIO_DIR |=  (1<<line);
    else
        GPIO_DIR &= ~(1<<line);
}

void gpio_data (int line, int value)
{
    if (value)
        GPIO_DATA |=  (1<<line);
    else
        GPIO_DATA &= ~(1<<line);
}


/*
------------------------------
low-level
------------------------------
*/

static int transfer (int port, int line,
    void *cmd_, uint32_t cmd_len,
    void *tx_ , uint32_t tx_len ,
    void *rx_ , uint32_t rx_len )
{
    int err = -1;
    uint8_t *cmd = cmd_;
    uint8_t *tx = tx_;
    uint8_t *rx = rx_;
    uint8_t *cmdend = (void*) ((intptr_t)cmd + (intptr_t)cmd_len);
    uint8_t *txend  = (void*) ((intptr_t)tx  + (intptr_t)tx_len);
    uint8_t *rxend  = (void*) ((intptr_t)rx  + (intptr_t)rx_len);
    int timeout;

    line = (1<<line);

    // bad command
    if(!cmd || !cmd_len){
        return -1;
    }

    gpio_data(CS_NORMAL,0);

    SPI_SSIENR(port) = SSIENR_SSI_DE;
    SPI_SER(port) = 0;                              /* disable all line's */
    SPI_CTRLR0(port) &= ~SPI_TMOD_MASK;

    if(rx_len) SPI_CTRLR0(port) |= (SPI_TMOD_EPROMREAD << SPI_TMOD_OFFSET);  /* mode: read  */
    else       SPI_CTRLR0(port) |= (SPI_TMOD_TO        << SPI_TMOD_OFFSET);  /* mode: write */

    switch ((SPI_CTRLR0(port) & SPI_TMOD_MASK) >> SPI_TMOD_OFFSET)
    {
        case SPI_TMOD_TO:
            SPI_SSIENR (port) = SSIENR_SSI_EN;      /* ebable fifo */
            INIT();
            while ((cmd != cmdend) && (SPI_SR(port) & SPI_SR_TFNF)){           /* push cmd */
                TRY();
                SPI_DR(port) = *cmd;
                cmd++;
            }

            INIT();
            while (tx != txend && (SPI_SR(port) & SPI_SR_TFNF)){             /* push tx */
                TRY();
                SPI_DR(port) = *tx;
                tx++;
            }

            SPI_SER(port) = line;                   /* start sending */

            INIT();
            while ((tx != txend)){
                TRY();
                if (SPI_SR(port) & SPI_SR_TFNF){
                    SPI_DR(port) = *tx;
                    tx++;
                    SPI_SER(port) = line;           /* restart if dropped */
                }
            }

            /* wait */
            INIT();
            while (!(SPI_SR(port) & SPI_SR_TFE))  TRY();
            while ( (SPI_SR(port) & SPI_SR_BUSY)) TRY();
            break;

        case SPI_TMOD_EPROMREAD:
            if (!rx  || !rx_len || rx_len > SPI_AUTO_READ_SIZE){
                goto exit;
            }

            SPI_CTRLR1 (port) = rx_len - 1;         /* set read size */
            SPI_SSIENR (port) = SSIENR_SSI_EN;      /* ebable fifo */

            INIT();
            while ((cmd != cmdend) && (SPI_SR(port) & SPI_SR_TFNF)) {          /* push cmd */
                TRY();
                SPI_DR(port) = *cmd;
                cmd++;
            }
            SPI_SER(port) = line;                   /* start sending */

            INIT();
            while ((rx != rxend)) {    /* read incoming data */
                TRY();
                if (SPI_SR(port) & SPI_SR_RFNE){
                    *rx++ = SPI_DR(port);
                }
            }

            /* wait */
            INIT();
            while (!(SPI_SR(port) & SPI_SR_TFE))  TRY();
            while ( (SPI_SR(port) & SPI_SR_BUSY)) TRY();
            break;

        case SPI_TMOD_TR:
        case SPI_TMOD_RO:
        default:
            goto exit;
    }
    err = 0;

exit:
    gpio_data(CS_NORMAL,1);
    return err;
}

static int exec (int port, int line,
    uint8_t cmd_op,
    uint32_t address,
    void *buf,
    uint32_t lenbuf)
{
    uint8_t cmd [SPI_CMD_LEN];
    uint8_t *in =0, *out =0;
    uint32_t lencmd =0, lenin =0, lenout =0;

    /* Save the SPI flash instruction. */
    cmd[0] = cmd_op;
    lencmd += 1;

    /* Prepare arguments for the SPI transaction. */
    switch (cmd_op)
    {
        case SPI_FLASH_RDID:    /* Read identification. */
        case SPI_FLASH_RDSR:    /* Read Status Register */
        case SPI_FLASH_RDEAR:   /* Read Extended Address Register */
            out = buf;
            lenout = lenbuf;
            break;

        case SPI_FLASH_READ:    /* Read Data Bytes */
            out = buf;
            lenout = lenbuf;
            if(en4byte){
                SPI_SET_ADDRESS_4BYTE(address, cmd);
                lencmd += SPI_ADR_LEN_4BYTE;
            }else{
                SPI_SET_ADDRESS_3BYTE(address, cmd);
                lencmd += SPI_ADR_LEN_3BYTE;
            }
            break;

        case SPI_FLASH_WREAR:
        case SPI_FLASH_WRSR:    /* Write Status Register */
            in = buf;
            lenin = 1;
            break;

        case SPI_FLASH_SSE:     /* SubSector Erase */
        case SPI_FLASH_SE:      /* Sector Erase */
            if(en4byte){
                SPI_SET_ADDRESS_4BYTE(address, cmd);
                lencmd += SPI_ADR_LEN_4BYTE;
            }else{
                SPI_SET_ADDRESS_3BYTE(address, cmd);
                lencmd += SPI_ADR_LEN_3BYTE;
            }
            break;

        case SPI_FLASH_EN4BYTEADDR:
        case SPI_FLASH_EX4BYTEADDR:
        case SPI_FLASH_WRDI:    /* Write Disable */
        case SPI_FLASH_WREN:    /* Write Enable */
        case SPI_FLASH_BE:      /* Bulk Erase */
            break;

        case SPI_FLASH_PP:      /* Page Program */
            if(lenbuf > SPI_PAGE_SIZE){
                return 1;
            }
            in = buf;
            lenin = lenbuf;
            if(en4byte){
                SPI_SET_ADDRESS_4BYTE(address, cmd);
                lencmd += SPI_ADR_LEN_4BYTE;
            }else{
                SPI_SET_ADDRESS_3BYTE(address, cmd);
                lencmd += SPI_ADR_LEN_3BYTE;
            }
            break;

        default:
            return 1;
    }

    /* Execute the SPI transaction */
    return transfer(port, line,
        cmd, lencmd,
        in,  lenin,
        out, lenout
    );
}


/*
------------------------------
internal
------------------------------
*/
static int status (int port, int line, void *status_)
{
    return exec (port, line, SPI_FLASH_RDSR, 0, status_, 1);
}

static int wren (int port, int line)
{
    int err;
    err = exec (port, line, SPI_FLASH_WREN, 0, 0, 0);
    if(err){
        return err;
    }

    uint8_t st;
    err = status(port, line, &st);
    if(err){
        return err;
    }
    return (st & SPI_FLASH_SR_WEL)? 0:1;
}

static int wait (int port, int line)
{
    uint8_t st;
    int err = -1;

    INIT();
    do {
        TRY();
        if(status(port, line, &st))
            goto exit;
    } while (st & SPI_FLASH_SR_WIP);
    err = 0;

exit:
    return err;
}

static void init_regs (int port)
{
    /* Disable device. */
    SPI_SSIENR (port) = SSIENR_SSI_DE;

    /* crt0 */
    SPI_CTRLR0 (port) = 0;
    SPI_CTRLR0 (port) |= (SPI_DFS(8) << SPI_DFS_OFFSET);

    /* other reg's */
    SPI_CTRLR1 (port) = 0;
    SPI_BAUDR  (port) = SPI_BAUDR_DEFAULT;
    SPI_TXFTLR (port) = 0;
    SPI_RXFTLR (port) = 0;
    SPI_IMR    (port) = 0;
    SPI_SER    (port) = 0;
}


/*
------------------------------
external
------------------------------
*/

static const flash_info_t *get_info (int port, int line);

const flash_info_t * llenv32_spi_init (int port, int line)
{
    const flash_info_t *info;

    gpio_dir(CS_NORMAL, 1);
    init_regs(port);
    info = get_info(port,line);

    if(info){
        uint32_t size = info->sector_size * info->n_sectors;
        if(size > 16*1024*1024){
            llenv32_enter_4byte(port,line);
        }
    }

    return info;
}

int llenv32_spi_erase (int port, int line, uint32_t adr, uint32_t size, uint32_t sector_size)
{
    int err;
    while (size) {

        err = wren(port,line);
        if(err){
            return err;
        }

        err = exec(port,line,SPI_FLASH_SE,adr,0,0);
        if(err){
            return err;
        }

        err = wait(port,line);
        if(err){
            return err;
        }

        adr  += sector_size;
        size -= (size > sector_size)? sector_size : size;
    }
    return 0;
}

int llenv32_spi_read (int port, int line, uint32_t adr, void *data, uint32_t size)
{
    int err;
    int part;
    char *pdata = data;
    while (size) {

        part = (size > SPI_MAX_READ)? SPI_MAX_READ : size;

        err = exec(port, line, SPI_FLASH_READ, adr, pdata, part);
        if(err){
            return err;
        }

        adr   += part;
        pdata += part;
        size  -= part;
    }
    return 0;
}

int llenv32_spi_write (int port, int line, uint32_t adr, void *data, uint32_t size)
{
    int err;
    int part;
    char *pdata = data;

    while (size) {

        part = (size > SPI_PAGE_SIZE)? SPI_PAGE_SIZE : size;

        /* fix size */
        int p1 = adr/SPI_PAGE_SIZE;            /* page number */
        int p2 = (adr+part)/SPI_PAGE_SIZE;
        if (p1 != p2){                         /* page overflow ? */
            p2 *= SPI_PAGE_SIZE;               /* page base address */
            part = p2 - adr;
        }

        err = wren(port, line);
        if(err){
            return err;
        }

        err = exec(port, line, SPI_FLASH_PP, adr, pdata, part);
        if(err){
            return err;
        }

        err = wait(port, line);
        if(err){
            return err;
        }

        adr   += part;
        pdata += part;
        size  -= part;
    }
    return 0;
}

int llenv32_enter_4byte (int port, int line)
{
    int err = wren(port,line);
    if(err){
        return err;
    }
    err = exec(port, line, SPI_FLASH_EN4BYTEADDR, 0, 0, 0);
    if(err){
        return err;
    }
    en4byte = 1;
    return 0;
}

int llenv32_exit_4byte (int port, int line)
{
    int err = wren(port,line);
    if(err) {
        return err;
    }
    err = exec(port, line, SPI_FLASH_EX4BYTEADDR, 0, 0, 0);
    if(err){
        return err;
    }
    en4byte = 0;
    return 0;
}



#define INFO(_jedec_id, _sector_size, _n_sectors)  \
    .id = {                                                     \
        ((_jedec_id) >> 16) & 0xff,                             \
        ((_jedec_id) >>  8) & 0xff,                             \
        ((_jedec_id) >>  0) & 0xff,                             \
        },                                                      \
    .sector_size = (_sector_size),                              \
    .n_sectors = (_n_sectors),

static const flash_info_t spi_nor_ids[] = {

    /* Atmel */
    { "at25fs010",   INFO(0x1f6601,  32*1024,    4) },
    { "at25fs040",   INFO(0x1f6604,  64*1024,    8) },
    { "at25df041a",  INFO(0x1f4401,  64*1024,    8) },
    { "at25df321",   INFO(0x1f4700,  64*1024,   64) },
    { "at25df321a",  INFO(0x1f4701,  64*1024,   64) },
    { "at25df641",   INFO(0x1f4800,  64*1024,  128) },
    { "at25sl321",   INFO(0x1f4216,  64*1024,   64) },
    { "at26f004",    INFO(0x1f0400,  64*1024,    8) },
    { "at26df081a",  INFO(0x1f4501,  64*1024,   16) },
    { "at26df161a",  INFO(0x1f4601,  64*1024,   32) },
    { "at26df321",   INFO(0x1f4700,  64*1024,   64) },
    { "at45db081d",  INFO(0x1f2500,  64*1024,   16) },

    /* EON */
    { "en25f32",     INFO(0x1c3116,  64*1024,   64) },
    { "en25p32",     INFO(0x1c2016,  64*1024,   64) },
    { "en25q32b",    INFO(0x1c3016,  64*1024,   64) },
    { "en25p64",     INFO(0x1c2017,  64*1024,  128) },
    { "en25q64",     INFO(0x1c3017,  64*1024,  128) },
    { "en25q80a",    INFO(0x1c3014,  64*1024,   16) },
    { "en25qh16",    INFO(0x1c7015,  64*1024,   32) },
    { "en25qh32",    INFO(0x1c7016,  64*1024,   64) },
    { "en25qh64",    INFO(0x1c7017,  64*1024,  128) },
    { "en25qh128",   INFO(0x1c7018,  64*1024,  256) },
    { "en25qh256",   INFO(0x1c7019,  64*1024,  512) },
    { "en25s64",     INFO(0x1c3817,  64*1024,  128) },

    /* ESMT */
    { "f25l32pa",    INFO(0x8c2016,  64*1024,   64) },
    { "f25l32qa",    INFO(0x8c4116,  64*1024,   64) },
    { "f25l64qa",    INFO(0x8c4117,  64*1024,  128) },

    /* GigaDevice */
    { "gd25q16",     INFO(0xc84015,  64*1024,   32) },
    { "gd25q32",     INFO(0xc84016,  64*1024,   64) },
    { "gd25lq32",    INFO(0xc86016,  64*1024,   64) },
    { "gd25q64",     INFO(0xc84017,  64*1024,  128) },
    { "gd25lq64c",   INFO(0xc86017,  64*1024,  128) },
    { "gd25lq128d",  INFO(0xc86018,  64*1024,  256) },
    { "gd25q128",    INFO(0xc84018,  64*1024,  256) },
    { "gd25q256",    INFO(0xc84019,  64*1024,  512) },

    /* Fujitsu */
    { "mb85rs1mt",   INFO(0x047f27, 128*1024,    1) },

    /* Intel/Numonyx */
    { "160s33b",     INFO(0x898911,  64*1024,   32) },
    { "320s33b",     INFO(0x898912,  64*1024,   64) },
    { "640s33b",     INFO(0x898913,  64*1024,  128) },

    /* ISSI */
    { "is25cd512",   INFO(0x7f9d20,  32*1024,    2) },
    { "is25lq040b",  INFO(0x9d4013,  64*1024,    8) },
    { "is25lp016d",  INFO(0x9d6015,  64*1024,   32) },
    { "is25lp080d",  INFO(0x9d6014,  64*1024,   16) },
    { "is25lp032",   INFO(0x9d6016,  64*1024,   64) },
    { "is25lp064",   INFO(0x9d6017,  64*1024,  128) },
    { "is25lp128",   INFO(0x9d6018,  64*1024,  256) },
    { "is25lp256",   INFO(0x9d6019,  64*1024,  512) },
    { "is25wp032",   INFO(0x9d7016,  64*1024,   64) },
    { "is25wp064",   INFO(0x9d7017,  64*1024,  128) },
    { "is25wp128",   INFO(0x9d7018,  64*1024,  256) },
    { "is25wp256",   INFO(0x9d7019,  64*1024,  512) },

    /* Macronix */
    { "mx25l512e",   INFO(0xc22010,  64*1024,    1) },
    { "mx25l2005a",  INFO(0xc22012,  64*1024,    4) },
    { "mx25l4005a",  INFO(0xc22013,  64*1024,    8) },
    { "mx25l8005",   INFO(0xc22014,  64*1024,   16) },
    { "mx25l1606e",  INFO(0xc22015,  64*1024,   32) },
    { "mx25l3205d",  INFO(0xc22016,  64*1024,   64) },
    { "mx25l3255e",  INFO(0xc29e16,  64*1024,   64) },
    { "mx25l6405d",  INFO(0xc22017,  64*1024,  128) },
    { "mx25u2033e",  INFO(0xc22532,  64*1024,    4) },
    { "mx25u3235f",  INFO(0xc22536,  64*1024,   64) },
    { "mx25u4035",   INFO(0xc22533,  64*1024,    8) },
    { "mx25u8035",   INFO(0xc22534,  64*1024,   16) },
    { "mx25u6435f",  INFO(0xc22537,  64*1024,  128) },
    { "mx25l12805d", INFO(0xc22018,  64*1024,  256) },
    { "mx25l12855e", INFO(0xc22618,  64*1024,  256) },
    { "mx25r1635f",  INFO(0xc22815,  64*1024,   32) },
    { "mx25r3235f",  INFO(0xc22816,  64*1024,   64) },
    { "mx25u12835f", INFO(0xc22538,  64*1024,  256) },
    { "mx25l25635e", INFO(0xc22019,  64*1024,  512) },
    { "mx25u25635f", INFO(0xc22539,  64*1024,  512) },
    { "mx25u51245g", INFO(0xc2253a,  64*1024,  102) },
    { "mx25v8035f",  INFO(0xc22314,  64*1024,   16) },
    { "mx25l25655e", INFO(0xc22619,  64*1024,  512) },
    { "mx25l51245g", INFO(0xc2201a,  64*1024, 1024) },
    { "mx66l51235l", INFO(0xc2201a,  64*1024, 1024) },
    { "mx66u51235f", INFO(0xc2253a,  64*1024, 1024) },
    { "mx66l1g45g",  INFO(0xc2201b,  64*1024, 2048) },
    { "mx66l1g55g",  INFO(0xc2261b,  64*1024, 2048) },
    { "mx66u2g45g",  INFO(0xc2253c,  64*1024, 4096) },

    /* Micron */
    { "n25q016a",    INFO(0x20bb15,  64*1024,   32) },
    { "n25q032",     INFO(0x20ba16,  64*1024,   64) },
    { "n25q032a",    INFO(0x20bb16,  64*1024,   64) },
    { "n25q064",     INFO(0x20ba17,  64*1024,  128) },
    { "n25q064a",    INFO(0x20bb17,  64*1024,  128) },
    { "n25q128a11",  INFO(0x20bb18,  64*1024,  256) },
    { "n25q128a13",  INFO(0x20ba18,  64*1024,  256) },
    { "n25q256a",    INFO(0x20ba19,  64*1024,  512) },
    { "n25q512a",    INFO(0x20bb20,  64*1024, 1024) },
    { "n25q256ax1",  INFO(0x20bb19,  64*1024,  512) },
    { "n25q512ax3",  INFO(0x20ba20,  64*1024, 1024) },
    { "n25q00",      INFO(0x20ba21,  64*1024, 2048) },
    { "n25q00a",     INFO(0x20bb21,  64*1024, 2048) },
    { "mt25ql02g",   INFO(0x20ba22,  64*1024, 4096) },
    { "mt25qu02g",   INFO(0x20bb22,  64*1024, 4096) },
    { "mt35xu512aba",INFO(0x2c5b1a,  128*1024, 512) },
    { "mt35xu02g",   INFO(0x2c5b1c,  128*1024,2048) },

    /* PMC */
    { "pm25lq032",   INFO(0x7f9d46,  64*1024,   64) },

    /* SST */
    { "sst25vf040b", INFO(0xbf258d,  64*1024,    8) },
    { "sst25vf080b", INFO(0xbf258e,  64*1024,   16) },
    { "sst25vf016b", INFO(0xbf2541,  64*1024,   32) },
    { "sst25vf032b", INFO(0xbf254a,  64*1024,   64) },
    { "sst25vf064c", INFO(0xbf254b,  64*1024,  128) },
    { "sst25wf512",  INFO(0xbf2501,  64*1024,    1) },
    { "sst25wf010",  INFO(0xbf2502,  64*1024,    2) },
    { "sst25wf020",  INFO(0xbf2503,  64*1024,    4) },
    { "sst25wf020a", INFO(0x621612,  64*1024,    4) },
    { "sst25wf040b", INFO(0x621613,  64*1024,    8) },
    { "sst25wf040",  INFO(0xbf2504,  64*1024,    8) },
    { "sst25wf080",  INFO(0xbf2505,  64*1024,   16) },
    { "sst26wf016b", INFO(0xbf2651,  64*1024,   32) },
    { "sst26vf016b", INFO(0xbf2641,  64*1024,   32) },
    { "sst26vf064b", INFO(0xbf2643,  64*1024,  128) },

    /* ST Microelectronics */
    { "m25p05",      INFO(0x202010,  32*1024,    2) },
    { "m25p10",      INFO(0x202011,  32*1024,    4) },
    { "m25p20",      INFO(0x202012,  64*1024,    4) },
    { "m25p40",      INFO(0x202013,  64*1024,    8) },
    { "m25p80",      INFO(0x202014,  64*1024,   16) },
    { "m25p16",      INFO(0x202015,  64*1024,   32) },
    { "m25p32",      INFO(0x202016,  64*1024,   64) },
    { "m25p64",      INFO(0x202017,  64*1024,  128) },
    { "m25p128",     INFO(0x202018, 256*1024,   64) },
    { "m45pe10",     INFO(0x204011,  64*1024,    2) },
    { "m45pe80",     INFO(0x204014,  64*1024,   16) },
    { "m45pe16",     INFO(0x204015,  64*1024,   32) },
    { "m25pe20",     INFO(0x208012,  64*1024,    4) },
    { "m25pe80",     INFO(0x208014,  64*1024,   16) },
    { "m25pe16",     INFO(0x208015,  64*1024,   32) },
    { "m25px16",     INFO(0x207115,  64*1024,   32) },
    { "m25px32",     INFO(0x207116,  64*1024,   64) },
    { "m25px32-s0",  INFO(0x207316,  64*1024,   64) },
    { "m25px32-s1",  INFO(0x206316,  64*1024,   64) },
    { "m25px64",     INFO(0x207117,  64*1024,  128) },
    { "m25px80",     INFO(0x207114,  64*1024,   16) },

    /* Winbond */
    { "w25x05",      INFO(0xef3010,  64*1024,    1) },
    { "w25x10",      INFO(0xef3011,  64*1024,    2) },
    { "w25x20",      INFO(0xef3012,  64*1024,    4) },
    { "w25x40",      INFO(0xef3013,  64*1024,    8) },
    { "w25x80",      INFO(0xef3014,  64*1024,   16) },
    { "w25x16",      INFO(0xef3015,  64*1024,   32) },
    { "w25q16dw",    INFO(0xef6015,  64*1024,   32) },
    { "w25x32",      INFO(0xef3016,  64*1024,   64) },
    { "w25q20cl",    INFO(0xef4012,  64*1024,    4) },
    { "w25q20bw",    INFO(0xef5012,  64*1024,    4) },
    { "w25q20ew",    INFO(0xef6012,  64*1024,    4) },
    { "w25q32",      INFO(0xef4016,  64*1024,   64) },
    { "w25q32dw",    INFO(0xef6016,  64*1024,   64) },
    { "w25q32jv",    INFO(0xef7016,  64*1024,   64) },
    { "w25q32jwm",   INFO(0xef8016,  64*1024,   64) },
    { "w25x64",      INFO(0xef3017,  64*1024,  128) },
    { "w25q64",      INFO(0xef4017,  64*1024,  128) },
    { "w25q64dw",    INFO(0xef6017,  64*1024,  128) },
    { "w25q64jvm",   INFO(0xef7017,  64*1024,  128) },
    { "w25q128fw",   INFO(0xef6018,  64*1024,  256) },
    { "w25q128jv",   INFO(0xef7018,  64*1024,  256) },
    { "w25q80",      INFO(0xef5014,  64*1024,   16) },
    { "w25q80bl",    INFO(0xef4014,  64*1024,   16) },
    { "w25q128",     INFO(0xef4018,  64*1024,  256) },
    { "w25q256",     INFO(0xef4019,  64*1024,  512) },
    { "w25q256jvm",  INFO(0xef7019,  64*1024,  512) },
    { "w25q256jw",   INFO(0xef6019,  64*1024,  512) },
    { "w25m512jv",   INFO(0xef7119,  64*1024, 1024) },
    { "w25q16jv-im/jm", INFO(0xef7015, 64*1024,  32) },

    /* Spansion */
    { "s25sl004a",   INFO(0x010212,  64*1024,    8) },
    { "s25sl008a",   INFO(0x010213,  64*1024,   16) },
    { "s25sl016a",   INFO(0x010214,  64*1024,   32) },
    { "s25sl032a",   INFO(0x010215,  64*1024,   64) },
    { "s25sl064a",   INFO(0x010216,  64*1024,  128) },
    { "s25fl004k",   INFO(0xef4013,  64*1024,    8) },
    { "s25fl008k",   INFO(0xef4014,  64*1024,   16) },
    { "s25fl016k",   INFO(0xef4015,  64*1024,   32) },
    { "s25fl064k",   INFO(0xef4017,  64*1024,  128) },
    { "s25fl116k",   INFO(0x014015,  64*1024,   32) },
    { "s25fl132k",   INFO(0x014016,  64*1024,   64) },
    { "s25fl164k",   INFO(0x014017,  64*1024,  128) },
    { "s25fl204k",   INFO(0x014013,  64*1024,    8) },
    { "s25fl208k",   INFO(0x014014,  64*1024,   16) },
    { "s25fl064l",   INFO(0x016017,  64*1024,  128) },
    { "s25fl128l",   INFO(0x016018,  64*1024,  256) },
    { "s25fl256l",   INFO(0x016019,  64*1024,  512) },

    { },
};


static int compare (const uint8_t *s1, const uint8_t *s2, uint32_t count)
{
    while (count-- > 0) {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}


static const flash_info_t *get_info (int port, int line)
{
    const flash_info_t *info;
    uint8_t id [SPI_NOR_MAX_ID_LEN];

    // read
    if (exec (port, line, SPI_FLASH_RDID, 0, id, SPI_NOR_MAX_ID_LEN)){
        EARLY_PRINT("error while reading JEDEC ID\n");
        return NULL;
    }

    // find
    int k;
    for (k = 0; k < sizeof(spi_nor_ids)/sizeof(spi_nor_ids[0]) -1; k++){
        info = &spi_nor_ids[k];
        if (!compare(info->id, id, SPI_NOR_MAX_ID_LEN)){
            // EARLY_PRINT("JEDEC id bytes: %02x%02x%02x\n", id[0], id[1], id[2]);
            EARLY_PRINT("flash: sector %dKB, cnt %d, total %dMB\n", info->sector_size/1024, info->n_sectors, ((info->sector_size/1024)*info->n_sectors)/1024);
            return info;
        }
    }
    EARLY_PRINT("unrecognized JEDEC id bytes: %02x%02x%02x\n", id[0], id[1], id[2]);
    return NULL;
}
