
#include <PiDxe.h>
#include <stdint.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/BaikalDebug.h>
#include <Library/DebugLib.h>
#include <Library/BaikalSmcLib.h>
#include <Platform/BaikalFlashMap.h>


#define BAIKAL_SMC_FLASH_DATA_SIZE    1024

/* PVT */
#define BAIKAL_SMC_PVT_ID         0x82000001

/* FLASH */
#define BAIKAL_SMC_FLASH          0x82000002
#define BAIKAL_SMC_FLASH_WRITE    (BAIKAL_SMC_FLASH +0)
#define BAIKAL_SMC_FLASH_READ     (BAIKAL_SMC_FLASH +1)
#define BAIKAL_SMC_FLASH_ERASE    (BAIKAL_SMC_FLASH +2)
#define BAIKAL_SMC_FLASH_PUSH     (BAIKAL_SMC_FLASH +3)
#define BAIKAL_SMC_FLASH_PULL     (BAIKAL_SMC_FLASH +4)
#define BAIKAL_SMC_FLASH_POSITION (BAIKAL_SMC_FLASH +5)
#define BAIKAL_SMC_FLASH_INFO     (BAIKAL_SMC_FLASH +6)

//---------------
// INTERNAL
//---------------
INTN
smc_position (
  IN UINT32 position
  )
{
  if (position >= BAIKAL_SMC_FLASH_DATA_SIZE){
    return -1;
  }
  ARM_SMC_ARGS arg = {BAIKAL_SMC_FLASH_POSITION, position, 0,0,0};
  ArmCallSmc(&arg);
  return arg.Arg0;
}

INTN
smc_write_buf (
  IN UINT32 adr,
  IN UINT32 size
  )
{
  if (!size){
    return -1;
  }
  ARM_SMC_ARGS arg = {BAIKAL_SMC_FLASH_WRITE, adr, size, 0,0};
  ArmCallSmc(&arg);
  return arg.Arg0;
}

INTN
smc_read_buf (
  IN UINT32 adr,
  IN UINT32 size
  )
{
  if (!size){
    return -1;
  }
  ARM_SMC_ARGS arg = {BAIKAL_SMC_FLASH_READ, adr, size, 0,0};
  ArmCallSmc(&arg);
  return arg.Arg0;
}

/* push 4 uint64_t word to arm-tf */
INTN
smc_push_4word (
  IN VOID *data
  )
{
  if (!data){
    return -1;
  }
  uint64_t *d = data;
  ARM_SMC_ARGS arg = {BAIKAL_SMC_FLASH_PUSH, d[0], d[1], d[2], d[3]};
  ArmCallSmc(&arg);
  return arg.Arg0;
}

/* pull 4 uint64_t word from arm-tf */
INTN
smc_pull_4word (
  IN VOID *data
  )
{
  if (!data){
    return -1;
  }
  ARM_SMC_ARGS arg = {BAIKAL_SMC_FLASH_PULL, 0,0,0,0};
  ArmCallSmc(&arg);

  uint64_t *d = data;
  d[0] = arg.Arg0;
  d[1] = arg.Arg1;
  d[2] = arg.Arg2;
  d[3] = arg.Arg3;

  return 0;
}

/* push buffer to arm-tf */
INTN
smc_push (
  IN VOID *data,
  IN UINT32 size
  )
{
  INTN ret;
  if (!data || !size || (size > BAIKAL_SMC_FLASH_DATA_SIZE)){
    return -1;
  }

  UINT8 *a = data;
  UINT8 *pb;
  UINT8  part;
  uint64_t b[4];

  ret = smc_position(0);
  if (ret){
    return ret;
  }

  while (size) {
    pb = (VOID*)b;
    part = size > sizeof(b)? sizeof(b) : size;
    size -= part;

    while (part--)
      *pb++ = *a++;

    ret = smc_push_4word(b);
    if (ret){
      return ret;
    }
  }
  return 0;
}

/* pull buffer from arm-tf */
INTN
smc_pull (
  IN VOID *data,
  IN UINT32 size
  )
{
  INTN ret;
  if (!data || !size || (size > BAIKAL_SMC_FLASH_DATA_SIZE)){
    return -1;
  }

  UINT8 *a = data;
  UINT8 *pb;
  UINT8  part;
  uint64_t b[4];

  ret = smc_position(0);
  if (ret){
    return ret;
  }

  while (size) {
    ret = smc_pull_4word(b);
    if (ret){
      return ret;
    }

    pb = (VOID*)b;
    part = size > sizeof(b)? sizeof(b) : size;
    size -= part;
    while (part--)
      *a++ = *pb++;
  }
  return 0;
}



//---------------
// EXTERNAL
//---------------
INTN
smc_erase (
  IN UINT32 adr,
  IN UINT32 size
  )
{
  if (!size){
    return -1;
  }
  ARM_SMC_ARGS arg = {BAIKAL_SMC_FLASH_ERASE, adr, size, 0,0};
  ArmCallSmc(&arg);
  return arg.Arg0;
}

INTN
smc_write (
  IN UINT32 adr,
  IN VOID  *data,
  IN UINT32 size
  )
{
  INTN ret;
  if (!data || !size){
    return -1;
  }

  UINT32 part;
  UINT8 *pdata = data;

  while (size) {

    part = (size > BAIKAL_SMC_FLASH_DATA_SIZE)? BAIKAL_SMC_FLASH_DATA_SIZE : size;

    ret = smc_push(pdata,part);
    if (ret){
      return ret;
    }

    ret = smc_write_buf(adr,part);
    if (ret){
      return ret;
    }

    adr   += part;
    pdata += part;
    size  -= part;
  }
  return 0;
}

INTN
smc_read (
  IN UINT32 adr,
  IN VOID  *data,
  IN UINT32 size
  )
{
  INTN ret;
  if (!data || !size){
    return -1;
  }

  UINT32 part;
  UINT8 *pdata = data;

  while (size) {

    part = (size > BAIKAL_SMC_FLASH_DATA_SIZE)? BAIKAL_SMC_FLASH_DATA_SIZE : size;

    ret = smc_read_buf(adr,part);
    if (ret){
      return ret;
    }

    ret = smc_pull(pdata,part);
    if (ret){
      return ret;
    }

    adr   += part;
    pdata += part;
    size  -= part;
  }
  return 0;
}

INTN
smc_info (
  IN UINT32 *sector_size,
  IN UINT32 *sector_cnt
  )
{
  ARM_SMC_ARGS arg;
  arg.Arg0 = BAIKAL_SMC_FLASH_INFO;
  ArmCallSmc(&arg);

  if (sector_size)
    *sector_size = arg.Arg1;
  if (sector_cnt)
    *sector_cnt  = arg.Arg2;

  return arg.Arg0;
}

VOID
smc_convert_pointer (
  VOID
  )
{
  EfiConvertPointer(0x0, (VOID**) &smc_info);
  EfiConvertPointer(0x0, (VOID**) &smc_erase);
  EfiConvertPointer(0x0, (VOID**) &smc_write);
  EfiConvertPointer(0x0, (VOID**) &smc_read);

  EfiConvertPointer(0x0, (VOID**) &smc_position);
  EfiConvertPointer(0x0, (VOID**) &smc_write_buf);
  EfiConvertPointer(0x0, (VOID**) &smc_read_buf);
  EfiConvertPointer(0x0, (VOID**) &smc_push_4word);
  EfiConvertPointer(0x0, (VOID**) &smc_pull_4word);
  EfiConvertPointer(0x0, (VOID**) &smc_push);
  EfiConvertPointer(0x0, (VOID**) &smc_pull);
}
