/** @file
  Copyright (c) 2019 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef GMAC_SNP_H_
#define GMAC_SNP_H_

EFI_STATUS
GmacSnpInstanceConstructor (
  IN   volatile GMAC_REGS           *GmacRegs,
  IN   CONST BOOLEAN                 DmaCoherent,
  IN   CONST EFI_PHYSICAL_ADDRESS    Tx2ClkChCtlAddr,
  IN   CONST BOOLEAN                 Tx2AddDiv2,
  IN   CONST EFI_PHYSICAL_ADDRESS    ResetGpioBase,
  IN   CONST INTN                    ResetGpioPin,
  IN   CONST INTN                    ResetPolarity,
  IN   CONST INTN                    PhyAddr,
  IN   CONST UINTN                   ClkCsr,
  IN   CONST BOOLEAN                 RgmiiRxid,
  IN   CONST BOOLEAN                 RgmiiTxid,
  IN   EFI_MAC_ADDRESS              *MacAddr,
  OUT  VOID                        **Snp,
  OUT  EFI_HANDLE                  **Handle
  );

EFI_STATUS
GmacSnpInstanceDestructor (
  IN  VOID  *Snp
  );

#endif // GMAC_SNP_H_
