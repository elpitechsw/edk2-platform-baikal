
#ifndef __BAIKAL_SMC_LIB_H__
#define __BAIKAL_SMC_LIB_H__

INTN smc_info  (UINT32 *psize, UINT32* pcnt);
INTN smc_erase (UINT32 adr, UINT32 size);
INTN smc_write (UINT32 adr, VOID *data, UINT32 size);
INTN smc_read  (UINT32 adr, VOID* data, UINT32 size);
VOID smc_convert_pointer (VOID);

#endif
