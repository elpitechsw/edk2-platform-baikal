
#ifndef __BAIKAL_SPI_LIB_H__
#define __BAIKAL_SPI_LIB_H__

#include <stdint.h>
#include <Platform/BaikalFlashMap.h>



/* ----------- */
/* PROTOTYPE'S */
/* ----------- */
const flash_info_t * llenv32_spi_init (int port, int line);
int  llenv32_spi_erase   (int port, int line, uint32_t adr, uint32_t size, uint32_t sectore_size);
int  llenv32_spi_read    (int port, int line, uint32_t adr, void *data, uint32_t size);
int  llenv32_spi_write   (int port, int line, uint32_t adr, void *data, uint32_t size);
int  llenv32_spi_ping    (int port, int line);
int  llenv32_enter_4byte (int port, int line);
int  llenv32_exit_4byte  (int port, int line);



#endif
