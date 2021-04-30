// Copyright (c) 2020 Baikal Electronics JSC
// Author: Mikhail Ivanov <michail.ivanov@baikalelectronics.ru>

#ifndef BAIKAL_FRU_LIB_H_
#define BAIKAL_FRU_LIB_H_

#define FRU_SIZE  4096

EFI_STATUS
EFIAPI
BaikalFruGetMacAddr (
  IN   UINT8            *Buf,
  IN   UINTN             BufSize,
  IN   UINTN             MacAddrIdx,
  OUT  EFI_MAC_ADDRESS  *MacAddr
  );

EFI_STATUS
EFIAPI
BaikalFruParseBoardArea (
  IN   UINT8  *Buf,
  IN   UINTN   BufSize,
  OUT  UINT8  *BoardMfg   OPTIONAL,
  OUT  UINT8  *BoardName  OPTIONAL,
  OUT  UINT8  *BoardSn    OPTIONAL
  );

#endif // BAIKAL_FRU_LIB_H_
