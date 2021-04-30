
#ifndef __BAIKAL_DEBUG_H__
#define __BAIKAL_DEBUG_H__

    #include <stdint.h>
    #include <Library/PrintLib.h>
    #include <Library/SerialPortLib.h>

    #if !defined(MDEPKG_NDEBUG)
        #define EARLY_PRINT(...) ({                \
            CHAR8 early_print_buf[512];            \
            UINTN early_print_cnt = AsciiSPrint (  \
                early_print_buf,                   \
                sizeof(early_print_buf),           \
                __VA_ARGS__);                      \
            SerialPortWrite (                      \
                (UINT8*)early_print_buf,           \
                early_print_cnt);                  \
        })

        #define RAW_STR(s)          ({EARLY_PRINT("%20a: \t%a\n",         __func__, s           );})
        #define RAW_DEC(d)          ({EARLY_PRINT("%20a: \t%a = %d\n",    __func__, #d, (int)d  );})
        #define RAW_HEX(h)          ({EARLY_PRINT("%20a: \t%a = 0x%x\n",  __func__, #h, (int)h  );})
        #define RAW_PTR(h)          ({EARLY_PRINT("%20a: \t%a = 0x%lx\n", __func__, #h, (long)h );})
        #define RAW_ERR(e)          ({EARLY_PRINT("%20a: %d\n",           __func__, e           );})
        #define RAW_ENTER()         RAW_ERR(0)
        #define RAW_EXIT()          RAW_ERR(1)

        #define RAW_MEM(p,n)  ({                            \
            UINT8 *a = (void*)(p);                          \
            UINT32 cnt = (n);                               \
            UINT32 i = 0;                                   \
            while(cnt--) {                                  \
                EARLY_PRINT(" %02x", *a++);                 \
                if(++i % 0x10 == 0)                         \
                    EARLY_PRINT("\n");                      \
            }                                               \
            EARLY_PRINT("\n");                              \
        })

    #else
        #define EARLY_PRINT(...)
        #define RAW_STR(s)
        #define RAW_DEC(d)
        #define RAW_HEX(h)
        #define RAW_PTR(h)
        #define RAW_ERR(e)
        #define RAW_ENTER()
        #define RAW_EXIT()
        #define RAW_MEM(p,n)
    #endif

#endif
