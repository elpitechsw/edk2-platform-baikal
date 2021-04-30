// Copyright (c) 2019 Baikal Electronics JSC
// Author: Mikhail Ivanov <michail.ivanov@baikalelectronics.ru>

#ifndef BAIKAL_ETH_SNP_H_
#define BAIKAL_ETH_SNP_H_

EFI_STATUS
BaikalEthSnpInstanceCtor (
  IN  VOID              *GmacRegs,
  IN  EFI_MAC_ADDRESS   *MacAddr,
  OUT VOID             **Snp,
  OUT EFI_HANDLE       **Handle
  );

EFI_STATUS
BaikalEthSnpInstanceDtor (
  IN  VOID  *Snp
  );

#endif // BAIKAL_ETH_SNP_H_

