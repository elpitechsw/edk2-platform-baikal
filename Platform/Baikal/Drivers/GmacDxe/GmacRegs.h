/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef GMAC_REGS_H_
#define GMAC_REGS_H_

typedef struct {
  UINT32  MacConfig;
  UINT32  MacFrameFilter;
  UINT32  MacHashHi;
  UINT32  MacHashLo;
  UINT32  MacMiiAddr;
  UINT32  MacMiiData;
  UINT32  MacFlowCtrl;
  UINT32  MacVlanTag;
  UINT32  MacVersion;
  UINT32  MacDebug;
  UINT32  MacWakeupFilter;
  UINT32  MacPmtCtrlStatus;
  UINT32  MacLpiCtrlStatus;
  UINT32  MacLpiTimersCtrl;
  UINT32  MacInt;
  UINT32  MacIntMsk;
  UINT32  MacAddr0Hi;
  UINT32  MacAddr0Lo;
  UINT32  MacAddr1Hi;
  UINT32  MacAddr1Lo;
  UINT32  MacAddr2Hi;
  UINT32  MacAddr2Lo;
  UINT32  MacAddr3Hi;
  UINT32  MacAddr3Lo;
  UINT32  MacAddr4Hi;
  UINT32  MacAddr4Lo;
  UINT32  MacAddr5Hi;
  UINT32  MacAddr5Lo;
  UINT32  MacAddr6Hi;
  UINT32  MacAddr6Lo;
  UINT32  MacAddr7Hi;
  UINT32  MacAddr7Lo;
  UINT32  MacAddr8Hi;
  UINT32  MacAddr8Lo;
  UINT32  MacReserved[20];
  UINT32  MacMiiStatus;
  UINT32  MacWdtTimeout;
  UINT32  MacGpio;
  UINT32  Reserved[967];
  UINT32  DmaBusMode;
  UINT32  DmaTxPollDemand;
  UINT32  DmaRxPollDemand;
  UINT32  DmaRxDescBaseAddr;
  UINT32  DmaTxDescBaseAddr;
  UINT32  DmaStatus;
  UINT32  DmaOperationMode;
  UINT32  DmaIntEn;
  UINT32  DmaMissedFrameCntr;
  UINT32  DmaRxWatchdog;
  UINT32  DmaAxiBusMode;
  UINT32  DmaAxiStatus;
  UINT32  DmaReserved[6];
  UINT32  DmaCurTxDescAddr;
  UINT32  DmaCurRxDescAddr;
  UINT32  DmaCurTxBufAddr;
  UINT32  DmaCurRxBufAddr;
  UINT32  DmaHwFeature;
} GMAC_REGS;

#endif // GMAC_REGS_H_
