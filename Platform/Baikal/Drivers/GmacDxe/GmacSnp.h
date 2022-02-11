/** @file
  Copyright (c) 2019 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef GMAC_SNP_H_
#define GMAC_SNP_H_

EFI_STATUS
GmacSnpInstanceCtor (
  IN   volatile GMAC_REGS           *GmacRegs,
  IN   CONST EFI_PHYSICAL_ADDRESS    ResetGpioBase,
  IN   CONST INTN                    ResetGpioPin,
  IN   CONST INTN                    ResetPolarity,
  IN   EFI_MAC_ADDRESS              *MacAddr,
  OUT  VOID                        **Snp,
  OUT  EFI_HANDLE                  **Handle
  );

EFI_STATUS
GmacSnpInstanceDtor (
  IN  VOID  *Snp
  );

#endif // GMAC_SNP_H_
