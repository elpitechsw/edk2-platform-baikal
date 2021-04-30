// Copyright (c) 2019-2021 Baikal Electronics JSC
// Author: Mikhail Ivanov <michail.ivanov@baikalelectronics.ru>

#include <Library/ArmSmcLib.h>
#include <Library/DebugLib.h>
#include <Library/NetLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleNetwork.h>
#include "BaikalEthSnp.h"

#define BM1000_MMXGBE_BASE                  0x30000000
#define BM1000_MMXGBE_GMAC0_BASE            0x30240000
#define BM1000_MMXGBE_GMAC0_TX2CLKCH        10
#define BM1000_MMXGBE_GMAC1_TX2CLKCH        13
#define BAIKAL_SMC_LCRU_ID                  0x82000000
#define BAIKAL_SMC_PLAT_CMU_CLKCH_SET_RATE  6
#define BAIKAL_SMC_PLAT_CMU_CLKCH_GET_RATE  7

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
} BAIKAL_ETH_GMAC_REGS;

#define MAC_CONFIG_RE               (1 <<  2)
#define MAC_CONFIG_TE               (1 <<  3)
#define MAC_CONFIG_ACS              (1 <<  7)
#define MAC_CONFIG_IPC              (1 << 10)
#define MAC_CONFIG_DM               (1 << 11)
#define MAC_CONFIG_DO               (1 << 13)
#define MAC_CONFIG_FES              (1 << 14)
#define MAC_CONFIG_PS               (1 << 15)
#define MAC_CONFIG_DCRS             (1 << 16)
#define MAC_CONFIG_SARC_POS         28

#define MAC_FRAMEFILTER_PR          (1 <<  0)
#define MAC_FRAMEFILTER_PM          (1 <<  4)
#define MAC_FRAMEFILTER_DBF         (1 <<  5)

#define MAC_MIISTATUS_LNKSPEED_POS   1
#define MAC_MIISTATUS_LNKSTS        (1 <<  3)

#define MAC_GPIO_GPO                (1 <<  8)

#define DMA_BUSMODE_SWR             (1 <<  0)
#define DMA_BUSMODE_ATDS            (1 <<  7)

#define DMA_STATUS_TI               (1 <<  0)
#define DMA_STATUS_RI               (1 <<  6)
#define DMA_STATUS_RU               (1 <<  7)

#define DMA_OPERATIONMODE_SR        (1 <<  1)
#define DMA_OPERATIONMODE_ST        (1 << 13)
#define DMA_OPERATIONMODE_TSF       (1 << 21)
#define DMA_OPERATIONMODE_RSF       (1 << 25)

#define DMA_AXISTATUS_AXIWHSTS      (1 <<  0)
#define DMA_AXISTATUS_AXIRDSTS      (1 <<  1)

typedef struct {
  UINT32  Rdes0;
  UINT32  Rdes1;
  UINT32  Rdes2;
  UINT32  Rdes3;
  UINT32  Rdes4;
  UINT32  Rdes5;
  UINT32  Rdes6;
  UINT32  Rdes7;
} BAIKAL_ETH_GMAC_RDESC;

#define RDES0_LS        (1 <<  8)
#define RDES0_FS        (1 <<  9)
#define RDES0_FL_POS    16
#define RDES0_FL_MSK    0x3FFF
#define RDES0_OWN       (1 << 31)

#define RDES1_RBS1_POS  0
#define RDES1_RCH       (1 << 14)

typedef struct {
  UINT32  Tdes0;
  UINT32  Tdes1;
  UINT32  Tdes2;
  UINT32  Tdes3;
  UINT32  Tdes4;
  UINT32  Tdes5;
  UINT32  Tdes6;
  UINT32  Tdes7;
} BAIKAL_ETH_GMAC_TDESC;

#define TDES0_TCH       (1 << 20)
#define TDES0_FS        (1 << 28)
#define TDES0_LS        (1 << 29)
#define TDES0_IC        (1 << 30)
#define TDES0_OWN       (1 << 31)

#define TDES1_TBS1_POS  0

#define RX_BUF_SIZE  2048
#define RX_DESC_NUM  64
#define TX_DESC_NUM  64

typedef struct {
  EFI_HANDLE                      Handle;
  EFI_SIMPLE_NETWORK_PROTOCOL     Snp;
  EFI_SIMPLE_NETWORK_MODE         SnpMode;
  volatile BAIKAL_ETH_GMAC_REGS  *GmacRegs;
  UINTN                           GmacTx2ClkFreq;
  volatile UINT8                  RxBufs[RX_DESC_NUM][RX_BUF_SIZE] __attribute__((aligned(16)));
  volatile BAIKAL_ETH_GMAC_RDESC  RxDescs[RX_DESC_NUM] __attribute__((aligned(sizeof (BAIKAL_ETH_GMAC_RDESC))));
  volatile BAIKAL_ETH_GMAC_TDESC  TxDescs[TX_DESC_NUM] __attribute__((aligned(sizeof (BAIKAL_ETH_GMAC_TDESC))));
  UINTN                           RxDescReadIdx;
  UINTN                           TxDescWriteIdx;
  UINTN                           TxDescReleaseIdx;
} BAIKAL_ETH_INSTANCE;

STATIC
UINT32
EFIAPI
BaikalEthSnpGetReceiveFilterSetting (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpGetStatus (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  OUT    UINT32                       *InterruptStatus,  OPTIONAL
  OUT    VOID                        **TxBuf             OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpInitialize (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN     UINTN                         ExtraRxBufSize,  OPTIONAL
  IN     UINTN                         ExtraTxBufSize   OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpMCastIpToMac (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN     BOOLEAN                       Ipv6,
  IN     EFI_IP_ADDRESS               *Ip,
  OUT    EFI_MAC_ADDRESS              *McastMacAddr
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpNvData (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN     BOOLEAN                       ReadWrite,
  IN     UINTN                         Offset,
  IN     UINTN                         BufSize,
  IN OUT VOID                         *Buf
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpReset (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN     BOOLEAN                       ExtendedVerification
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpReceiveFilters (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN     UINT32                        Enable,
  IN     UINT32                        Disable,
  IN     BOOLEAN                       ResetMCastFilter,
  IN     UINTN                         MCastFilterCnt,   OPTIONAL
  IN     EFI_MAC_ADDRESS              *MCastFilter       OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpShutdown (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpStart (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpStationAddress (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN     BOOLEAN                       Reset,
  IN     EFI_MAC_ADDRESS              *NewMacAddr  OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpStatistics (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN     BOOLEAN                       Reset,
  IN OUT UINTN                        *StatisticsSize,  OPTIONAL
  OUT    EFI_NETWORK_STATISTICS       *StatisticsTable  OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpStop (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpReceive (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  OUT    UINTN                        *HdrSize,  OPTIONAL
  IN OUT UINTN                        *BufSize,
  OUT    VOID                         *Buf,
  OUT    EFI_MAC_ADDRESS              *SrcAddr,  OPTIONAL
  OUT    EFI_MAC_ADDRESS              *DstAddr,  OPTIONAL
  OUT    UINT16                       *Protocol  OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpTransmit (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN     UINTN                         HdrSize,
  IN     UINTN                         BufSize,
  IN     VOID                         *Buf,
  IN     EFI_MAC_ADDRESS              *SrcAddr,  OPTIONAL
  IN     EFI_MAC_ADDRESS              *DstAddr,  OPTIONAL
  IN     UINT16                       *Protocol  OPTIONAL
  );

STATIC
UINT32
BaikalEthSnpGetReceiveFilterSetting (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  )
{
  CONST BAIKAL_ETH_INSTANCE *CONST  EthInst = BASE_CR (Snp, BAIKAL_ETH_INSTANCE, Snp);
  UINT32  ReceiveFilterSetting = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST;

  if (!(EthInst->GmacRegs->MacFrameFilter & MAC_FRAMEFILTER_DBF)) {
    ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST;
  }

  if (EthInst->GmacRegs->MacFrameFilter & MAC_FRAMEFILTER_PR) {
    ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS;
  }

  if (EthInst->GmacRegs->MacFrameFilter & MAC_FRAMEFILTER_PM) {
    ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;
  }

  return ReceiveFilterSetting;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpGetStatus (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL   *Snp,
  OUT  UINT32                        *InterruptStatus,  OPTIONAL
  OUT  VOID                         **TxBuf             OPTIONAL
  )
{
  BAIKAL_ETH_INSTANCE *CONST  EthInst = BASE_CR (Snp, BAIKAL_ETH_INSTANCE, Snp);
  UINTN    MacMiiStatus;
  BOOLEAN  MediaPresent;
  EFI_TPL  SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpGetStatus: SNP not initialized\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpGetStatus: SNP not started\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpGetStatus: SNP invalid state = %u\n", EthInst->GmacRegs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  MacMiiStatus = EthInst->GmacRegs->MacMiiStatus;
  MediaPresent = MacMiiStatus & MAC_MIISTATUS_LNKSTS;

  if (Snp->Mode->MediaPresent != MediaPresent) {
    Snp->Mode->MediaPresent    = MediaPresent;
  }

  if (MediaPresent) {
    ARM_SMC_ARGS  ArmSmcArgs;
    CONST UINTN  LnkSpeed = (MacMiiStatus >> MAC_MIISTATUS_LNKSPEED_POS) & 0x3;

    if (!EthInst->GmacTx2ClkFreq) {
      ArmSmcArgs.Arg0 = BAIKAL_SMC_LCRU_ID;
      if (EthInst->GmacRegs == (VOID *) BM1000_MMXGBE_GMAC0_BASE) {
        ArmSmcArgs.Arg1 = BM1000_MMXGBE_GMAC0_TX2CLKCH;
      } else {
        ArmSmcArgs.Arg1 = BM1000_MMXGBE_GMAC1_TX2CLKCH;
      }

      ArmSmcArgs.Arg2 = BAIKAL_SMC_PLAT_CMU_CLKCH_GET_RATE;
      ArmSmcArgs.Arg4 = BM1000_MMXGBE_BASE;
      ArmCallSmc(&ArmSmcArgs);
      EthInst->GmacTx2ClkFreq = ArmSmcArgs.Arg0;
    }

    if ((!LnkSpeed     && EthInst->GmacTx2ClkFreq != 5000000)  ||
        (LnkSpeed == 1 && EthInst->GmacTx2ClkFreq != 50000000) ||
        (LnkSpeed == 2 && EthInst->GmacTx2ClkFreq != 250000000)) {
      if (!LnkSpeed) {
        EthInst->GmacTx2ClkFreq = 5000000;
      } else if (LnkSpeed == 1) {
        EthInst->GmacTx2ClkFreq = 50000000;
      } else {
        EthInst->GmacTx2ClkFreq = 250000000;
      }

      ArmSmcArgs.Arg0 = BAIKAL_SMC_LCRU_ID;
      if (EthInst->GmacRegs == (VOID *) BM1000_MMXGBE_GMAC0_BASE) {
        ArmSmcArgs.Arg1 = BM1000_MMXGBE_GMAC0_TX2CLKCH;
      } else {
        ArmSmcArgs.Arg1 = BM1000_MMXGBE_GMAC1_TX2CLKCH;
      }

      ArmSmcArgs.Arg2 = BAIKAL_SMC_PLAT_CMU_CLKCH_SET_RATE;
      ArmSmcArgs.Arg3 = EthInst->GmacTx2ClkFreq;
      ArmSmcArgs.Arg4 = BM1000_MMXGBE_BASE;
      ArmCallSmc(&ArmSmcArgs);
    }

    if (!LnkSpeed) {
      EthInst->GmacRegs->MacConfig = (EthInst->GmacRegs->MacConfig & ~MAC_CONFIG_FES) | MAC_CONFIG_PS;
    } else if (LnkSpeed == 1) {
      EthInst->GmacRegs->MacConfig |= MAC_CONFIG_PS | MAC_CONFIG_FES;
    } else if (LnkSpeed == 2) {
      EthInst->GmacRegs->MacConfig &= ~(MAC_CONFIG_PS | MAC_CONFIG_FES);
    }
  }

  if (TxBuf != NULL && EthInst->TxDescReleaseIdx != EthInst->TxDescWriteIdx) {
    *((UINT32 *) TxBuf) = EthInst->TxDescs[EthInst->TxDescReleaseIdx].Tdes2;
    EthInst->TxDescReleaseIdx = (EthInst->TxDescReleaseIdx + 1) % TX_DESC_NUM;
  }

  if (InterruptStatus) {
    *InterruptStatus =
      (EthInst->GmacRegs->DmaStatus & DMA_STATUS_RI ? EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT  : 0) |
      (EthInst->GmacRegs->DmaStatus & DMA_STATUS_TI ? EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT : 0);

    EthInst->GmacRegs->DmaStatus =
      (*InterruptStatus & EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT  ? DMA_STATUS_RI : 0) |
      (*InterruptStatus & EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT ? DMA_STATUS_TI : 0);
  }

  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

EFI_STATUS
BaikalEthSnpInstanceCtor (
  IN   VOID              *GmacRegs,
  IN   EFI_MAC_ADDRESS   *MacAddr,
  OUT  VOID             **Snp,
  OUT  EFI_HANDLE       **Handle
  )
{
  BAIKAL_ETH_INSTANCE   *EthInst;
  EFI_PHYSICAL_ADDRESS   PhysicalAddr;
  EFI_STATUS             Status;

  PhysicalAddr = (EFI_PHYSICAL_ADDRESS) (SIZE_4GB - 1);
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiBootServicesData,
                  EFI_SIZE_TO_PAGES (sizeof (BAIKAL_ETH_INSTANCE)),
                  &PhysicalAddr
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "BaikalEth(%p)SnpInstaceCtor: unable to allocate EthInst, Status = %r\n", GmacRegs, Status));
    return Status;
  }

  EthInst = (VOID *) PhysicalAddr;
  EthInst->Handle = NULL;
  EthInst->GmacRegs = GmacRegs;
  EthInst->GmacTx2ClkFreq = 0;

  EthInst->RxDescReadIdx    = 0;
  EthInst->TxDescWriteIdx   = 0;
  EthInst->TxDescReleaseIdx = 0;

  EthInst->Snp.Revision       = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
  EthInst->Snp.Start          = BaikalEthSnpStart;
  EthInst->Snp.Stop           = BaikalEthSnpStop;
  EthInst->Snp.Initialize     = BaikalEthSnpInitialize;
  EthInst->Snp.Reset          = BaikalEthSnpReset;
  EthInst->Snp.Shutdown       = BaikalEthSnpShutdown;
  EthInst->Snp.ReceiveFilters = BaikalEthSnpReceiveFilters;
  EthInst->Snp.StationAddress = BaikalEthSnpStationAddress;
  EthInst->Snp.Statistics     = BaikalEthSnpStatistics;
  EthInst->Snp.MCastIpToMac   = BaikalEthSnpMCastIpToMac;
  EthInst->Snp.NvData         = BaikalEthSnpNvData;
  EthInst->Snp.GetStatus      = BaikalEthSnpGetStatus;
  EthInst->Snp.Transmit       = BaikalEthSnpTransmit;
  EthInst->Snp.Receive        = BaikalEthSnpReceive;
  EthInst->Snp.WaitForPacket  = NULL;
  EthInst->Snp.Mode           = &EthInst->SnpMode;

  EthInst->SnpMode.State                 = EfiSimpleNetworkStopped;
  EthInst->SnpMode.HwAddressSize         = NET_ETHER_ADDR_LEN;
  EthInst->SnpMode.MediaHeaderSize       = sizeof (ETHER_HEAD);
  EthInst->SnpMode.MaxPacketSize         = 1500;
  EthInst->SnpMode.NvRamSize             = 0;
  EthInst->SnpMode.NvRamAccessSize       = 0;
  EthInst->SnpMode.ReceiveFilterMask     = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST     |
                                           EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST   |
                                           EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS |
                                           EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;

  EthInst->SnpMode.ReceiveFilterSetting  = BaikalEthSnpGetReceiveFilterSetting (&EthInst->Snp);
  EthInst->SnpMode.MaxMCastFilterCount   = MAX_MCAST_FILTER_CNT;
  EthInst->SnpMode.MCastFilterCount      = 0;
  EthInst->SnpMode.IfType                = NET_IFTYPE_ETHERNET;
  EthInst->SnpMode.MacAddressChangeable  = TRUE;
  EthInst->SnpMode.MultipleTxSupported   = FALSE;
  EthInst->SnpMode.MediaPresentSupported = TRUE;
  EthInst->SnpMode.MediaPresent          = FALSE;

  gBS->CopyMem (&EthInst->SnpMode.CurrentAddress,   MacAddr, sizeof (EFI_MAC_ADDRESS));
  gBS->CopyMem (&EthInst->SnpMode.PermanentAddress, MacAddr, sizeof (EFI_MAC_ADDRESS));
  gBS->SetMem (&EthInst->SnpMode.MCastFilter, MAX_MCAST_FILTER_CNT * sizeof (EFI_MAC_ADDRESS), 0);
  gBS->SetMem (&EthInst->SnpMode.BroadcastAddress, sizeof (EFI_MAC_ADDRESS), 0xFF);

  *Handle = &EthInst->Handle;
  *Snp    = &EthInst->Snp;

  /* TODO: create event to reset (DmaBusMode.SWR = 1) the EthInst when exit boot service
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  BaikalEthSnpExitBootService,
                  EthInst,
                  &gEfiEventExitBootServicesGuid,
                  &EthInst->ExitBootServiceEvent
                  );
  */

  return EFI_SUCCESS;
}

EFI_STATUS
BaikalEthSnpInstanceDtor (
  IN  VOID  *Snp
  )
{
  BAIKAL_ETH_INSTANCE *CONST  EthInst = BASE_CR (Snp, BAIKAL_ETH_INSTANCE, Snp);
  EFI_STATUS                  Status;

  Status = gBS->FreePages ((EFI_PHYSICAL_ADDRESS) EthInst, EFI_SIZE_TO_PAGES (sizeof (BAIKAL_ETH_INSTANCE)));
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpInitialize (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINTN                         ExtraRxBufSize,  OPTIONAL
  IN  UINTN                         ExtraTxBufSize   OPTIONAL
  )
{
  BAIKAL_ETH_INSTANCE *CONST  EthInst = BASE_CR (Snp, BAIKAL_ETH_INSTANCE, Snp);
  UINTN                       DescIdx;
  UINTN                       Limit;
  EFI_TPL                     SavedTpl;

  ASSERT (Snp != NULL);

  if (ExtraRxBufSize != 0 || ExtraTxBufSize != 0) {
    return EFI_UNSUPPORTED;
  }

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkStarted:
    break;
  case EfiSimpleNetworkInitialized:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpInitialize: SNP already initialized\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_SUCCESS;
  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpInitialize: SNP not started\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpInitialize: SNP invalid state = %u\n", EthInst->GmacRegs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  EthInst->GmacRegs->DmaBusMode = DMA_BUSMODE_SWR;
  gBS->Stall (10);

  for (Limit = 3000; Limit; --Limit) {
    EthInst->GmacRegs->MacGpio |= MAC_GPIO_GPO;
    if (!(EthInst->GmacRegs->DmaBusMode & DMA_BUSMODE_SWR)) {
      break;
    }

    gBS->Stall (1000);
  }

  if (!Limit) {
    DEBUG ((EFI_D_ERROR, "BaikalEth(%p)SnpInitialize: GMAC reset not completed\n", EthInst->GmacRegs));
    return EFI_DEVICE_ERROR;
  }

  for (Limit = 3000; Limit; --Limit) {
    if (!(EthInst->GmacRegs->DmaAxiStatus & (DMA_AXISTATUS_AXIRDSTS | DMA_AXISTATUS_AXIWHSTS))) {
      break;
    }

    gBS->Stall (1000);
  }

  EthInst->GmacRegs->MacAddr0Hi =
    Snp->Mode->CurrentAddress.Addr[4] |
    Snp->Mode->CurrentAddress.Addr[5] << 8;

  EthInst->GmacRegs->MacAddr0Lo =
    Snp->Mode->CurrentAddress.Addr[0]       |
    Snp->Mode->CurrentAddress.Addr[1] << 8  |
    Snp->Mode->CurrentAddress.Addr[2] << 16 |
    Snp->Mode->CurrentAddress.Addr[3] << 24;

  EthInst->GmacRegs->DmaBusMode       |= DMA_BUSMODE_ATDS;
  EthInst->GmacRegs->DmaRxDescBaseAddr = (UINTN) EthInst->RxDescs;
  EthInst->GmacRegs->DmaTxDescBaseAddr = (UINTN) EthInst->TxDescs;

  for (DescIdx = 0; DescIdx < RX_DESC_NUM; ++DescIdx) {
    EthInst->RxDescs[DescIdx].Rdes0 = RDES0_OWN;
    EthInst->RxDescs[DescIdx].Rdes1 = RDES1_RCH | (RX_BUF_SIZE << RDES1_RBS1_POS);
    EthInst->RxDescs[DescIdx].Rdes2 = (UINTN) EthInst->RxBufs[DescIdx];
    EthInst->RxDescs[DescIdx].Rdes3 = (UINTN) &EthInst->RxDescs[(DescIdx + 1) % RX_DESC_NUM];
  }

  for (DescIdx = 0; DescIdx < TX_DESC_NUM; ++DescIdx) {
    EthInst->TxDescs[DescIdx].Tdes0 = 0;
    EthInst->TxDescs[DescIdx].Tdes3 = (UINTN) &EthInst->TxDescs[(DescIdx + 1) % TX_DESC_NUM];
  }

  EthInst->GmacRegs->DmaStatus = 0xFFFFFFFF;
  EthInst->GmacRegs->MacConfig = (3 << MAC_CONFIG_SARC_POS) |
                                 MAC_CONFIG_DCRS |
                                 MAC_CONFIG_DO   |
                                 MAC_CONFIG_DM   |
                                 MAC_CONFIG_IPC  |
                                 MAC_CONFIG_ACS;

  EthInst->GmacRegs->DmaOperationMode = DMA_OPERATIONMODE_TSF |
                                        DMA_OPERATIONMODE_RSF |
                                        DMA_OPERATIONMODE_ST  |
                                        DMA_OPERATIONMODE_SR;

  EthInst->GmacRegs->MacConfig       |= MAC_CONFIG_TE | MAC_CONFIG_RE;
  EthInst->GmacRegs->DmaRxPollDemand  = 0;

  Snp->Mode->State = EfiSimpleNetworkInitialized;
  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpMCastIpToMac (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN   BOOLEAN                       Ipv6,
  IN   EFI_IP_ADDRESS               *Ip,
  OUT  EFI_MAC_ADDRESS              *McastMacAddr
  )
{
  EFI_TPL  SavedTpl;

  ASSERT (Snp != NULL);

  if (Ip == NULL || McastMacAddr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStopped:
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  gBS->RestoreTPL (SavedTpl);
  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpNvData (
  IN      EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN      BOOLEAN                       ReadWrite,
  IN      UINTN                         Offset,
  IN      UINTN                         BufSize,
  IN OUT  VOID                         *Buf
  )
{
  EFI_TPL  SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStopped:
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  gBS->RestoreTPL (SavedTpl);
  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpReset (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  BOOLEAN                       ExtendedVerification
  )
{
  EFI_TPL  SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStopped:
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpReceiveFilters (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINT32                        Enable,
  IN  UINT32                        Disable,
  IN  BOOLEAN                       ResetMCastFilter,
  IN  UINTN                         MCastFilterCnt,   OPTIONAL
  IN  EFI_MAC_ADDRESS              *MCastFilter       OPTIONAL
  )
{
  BAIKAL_ETH_INSTANCE *CONST  EthInst = BASE_CR (Snp, BAIKAL_ETH_INSTANCE, Snp);
  CONST UINT32  ResultingMsk = Enable & ~Disable;
  EFI_TPL       SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpReceiveFilters: SNP not initialized\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpReceiveFilters: SNP not started\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpReceiveFilters: SNP invalid state = %u\n", EthInst->GmacRegs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (ResultingMsk & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST) {
    EthInst->GmacRegs->MacFrameFilter &= ~MAC_FRAMEFILTER_DBF;
  } else {
    EthInst->GmacRegs->MacFrameFilter |=  MAC_FRAMEFILTER_DBF;
  }

  if (ResultingMsk & (EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST |
                      EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST)) {
    EthInst->GmacRegs->MacFrameFilter |=  MAC_FRAMEFILTER_PM;
  } else {
    EthInst->GmacRegs->MacFrameFilter &= ~MAC_FRAMEFILTER_PM;
  }

  if (ResultingMsk & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS) {
    EthInst->GmacRegs->MacFrameFilter |=  MAC_FRAMEFILTER_PR;
  } else {
    EthInst->GmacRegs->MacFrameFilter &= ~MAC_FRAMEFILTER_PR;
  }

  Snp->Mode->ReceiveFilterSetting = BaikalEthSnpGetReceiveFilterSetting (Snp);
  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpShutdown (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  )
{
  EFI_TPL  SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStopped:
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpStart (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  )
{
  EFI_TPL  SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkStopped:
    break;
  case EfiSimpleNetworkStarted:
  case EfiSimpleNetworkInitialized:
    gBS->RestoreTPL (SavedTpl);
    return EFI_ALREADY_STARTED;
  default:
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  Snp->Mode->State = EfiSimpleNetworkStarted;
  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpStationAddress (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  BOOLEAN                       Reset,
  IN  EFI_MAC_ADDRESS              *NewMacAddr  OPTIONAL
  )
{
  BAIKAL_ETH_INSTANCE *CONST  EthInst = BASE_CR (Snp, BAIKAL_ETH_INSTANCE, Snp);
  EFI_TPL  SavedTpl;

  ASSERT (Snp != NULL);

  DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpStationAddress()\n", EthInst->GmacRegs));
  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpStationAddress: SNP not initialized\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpStationAddress: SNP not started\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpStationAddress: SNP invalid state = %u\n", EthInst->GmacRegs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (Reset) {
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpStationAddress: reset MAC address\n", EthInst->GmacRegs));
    Snp->Mode->CurrentAddress = Snp->Mode->PermanentAddress;
  } else {
    if (NewMacAddr == NULL) {
      gBS->RestoreTPL (SavedTpl);
      return EFI_INVALID_PARAMETER;
    }

    gBS->CopyMem (&Snp->Mode->CurrentAddress, NewMacAddr, sizeof (EFI_MAC_ADDRESS));
  }

  EthInst->GmacRegs->MacAddr0Hi =
    Snp->Mode->CurrentAddress.Addr[4] |
    Snp->Mode->CurrentAddress.Addr[5] << 8;

  EthInst->GmacRegs->MacAddr0Lo =
    Snp->Mode->CurrentAddress.Addr[0]       |
    Snp->Mode->CurrentAddress.Addr[1] << 8  |
    Snp->Mode->CurrentAddress.Addr[2] << 16 |
    Snp->Mode->CurrentAddress.Addr[3] << 24;

  DEBUG ((
    EFI_D_NET,
    "BaikalEth(%p)SnpStationAddress: current MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
    EthInst->GmacRegs,
    Snp->Mode->CurrentAddress.Addr[0],
    Snp->Mode->CurrentAddress.Addr[1],
    Snp->Mode->CurrentAddress.Addr[2],
    Snp->Mode->CurrentAddress.Addr[3],
    Snp->Mode->CurrentAddress.Addr[4],
    Snp->Mode->CurrentAddress.Addr[5]
    ));

  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpStatistics (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN   BOOLEAN                       Reset,
  IN   OUT UINTN                    *StatisticsSize,  OPTIONAL
  OUT  EFI_NETWORK_STATISTICS       *StatisticsTable  OPTIONAL
  )
{
  EFI_TPL  SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStopped:
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  gBS->RestoreTPL (SavedTpl);
  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpStop (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  )
{
  EFI_TPL  SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkStarted:
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStopped:
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  Snp->Mode->State = EfiSimpleNetworkStopped;
  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpReceive ( // Receive a packet from a network interface
  IN      EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  OUT     UINTN                        *HdrSize,  OPTIONAL
  IN OUT  UINTN                        *BufSize,
  OUT     VOID                         *Buf,
  OUT     EFI_MAC_ADDRESS              *SrcAddr,  OPTIONAL
  OUT     EFI_MAC_ADDRESS              *DstAddr,  OPTIONAL
  OUT     UINT16                       *Protocol  OPTIONAL
  )
{
  BAIKAL_ETH_INSTANCE *CONST  EthInst = BASE_CR (Snp, BAIKAL_ETH_INSTANCE, Snp);
  EFI_TPL                     SavedTpl;

  ASSERT (Snp != NULL);

  if (BufSize == NULL || Buf == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpReceive: SNP not initialized\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpReceive: SNP not started\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpReceive: SNP invalid state = %u\n", EthInst->GmacRegs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  EFI_STATUS Status = EFI_NOT_READY;

  while (!(EthInst->RxDescs[EthInst->RxDescReadIdx].Rdes0 & RDES0_OWN) && (Status == EFI_NOT_READY)) {
    if (((RDES0_FS | RDES0_LS) & EthInst->RxDescs[EthInst->RxDescReadIdx].Rdes0) ==
         (RDES0_FS | RDES0_LS)) {
      CONST UINTN FrameLen = (EthInst->RxDescs[EthInst->RxDescReadIdx].Rdes0 >> RDES0_FL_POS) & RDES0_FL_MSK;

      if (*BufSize < FrameLen) {
        DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpReceive: receive BufSize(%u) < FrameLen(%u)\n", EthInst->GmacRegs, *BufSize, FrameLen));
        Status = EFI_BUFFER_TOO_SMALL;
      }

      *BufSize = FrameLen;

      if (Status != EFI_BUFFER_TOO_SMALL) {
        gBS->CopyMem (Buf, (VOID *) EthInst->RxBufs[EthInst->RxDescReadIdx], FrameLen);

        if (HdrSize != NULL) {
          *HdrSize = Snp->Mode->MediaHeaderSize;
        }

        if (DstAddr != NULL) {
          gBS->CopyMem (DstAddr, Buf, NET_ETHER_ADDR_LEN);
        }

        if (SrcAddr != NULL) {
          gBS->CopyMem (SrcAddr, (UINT8*) Buf + 6, NET_ETHER_ADDR_LEN);
        }

        if (Protocol != NULL) {
          *Protocol = NTOHS (*(UINT16*)((UINT8*) Buf + 12));
        }

        Status = EFI_SUCCESS;
      }
    }

    EthInst->RxDescs[EthInst->RxDescReadIdx].Rdes0 = RDES0_OWN;
    EthInst->RxDescReadIdx = (EthInst->RxDescReadIdx + 1) % RX_DESC_NUM;

    if (EthInst->GmacRegs->DmaStatus & DMA_STATUS_RU) {
      EthInst->GmacRegs->DmaStatus   = DMA_STATUS_RU;
      EthInst->GmacRegs->DmaRxPollDemand = 0;
    }
  }

  gBS->RestoreTPL (SavedTpl);
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
BaikalEthSnpTransmit (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINTN                         HdrSize,
  IN  UINTN                         BufSize,
  IN  VOID                         *Buf,
  IN  EFI_MAC_ADDRESS              *SrcAddr,  OPTIONAL
  IN  EFI_MAC_ADDRESS              *DstAddr,  OPTIONAL
  IN  UINT16                       *Protocol  OPTIONAL
  )
{
  BAIKAL_ETH_INSTANCE *CONST  EthInst = BASE_CR (Snp, BAIKAL_ETH_INSTANCE, Snp);
  EFI_TPL                     SavedTpl;
  EFI_STATUS                  Status;

  ASSERT (Snp != NULL);

  if (Buf == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpTransmit: SNP not initialized\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpTransmit: SNP not started\n", EthInst->GmacRegs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;
  default:
    DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpTransmit: SNP invalid state = %u\n", EthInst->GmacRegs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (HdrSize != 0) {
    if (HdrSize != Snp->Mode->MediaHeaderSize) {
      DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpTransmit: HdrSize(%u) != Snp->Mode->MediaHeaderSize(%u)\n", EthInst->GmacRegs, HdrSize, Snp->Mode->MediaHeaderSize));
      gBS->RestoreTPL (SavedTpl);
      return EFI_INVALID_PARAMETER;
    }

    if (DstAddr == NULL || Protocol == NULL) {
      DEBUG ((EFI_D_NET, "BaikalEth(%p)SnpTransmit: Hdr DstAddr(%p) or Protocol(%p) is NULL\n", EthInst->GmacRegs, DstAddr, Protocol));
      gBS->RestoreTPL (SavedTpl);
      return EFI_INVALID_PARAMETER;
    }
  }

  if (!Snp->Mode->MediaPresent) {
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_READY;
  }

  if (HdrSize != 0) {
    UINT16 EtherType = HTONS (*Protocol);

    gBS->CopyMem (Buf, DstAddr, NET_ETHER_ADDR_LEN);

    if (SrcAddr != NULL) {
      gBS->CopyMem ((UINT8 *) Buf + NET_ETHER_ADDR_LEN, SrcAddr, NET_ETHER_ADDR_LEN);
    } else {
      gBS->CopyMem ((UINT8 *) Buf + NET_ETHER_ADDR_LEN, &Snp->Mode->CurrentAddress, NET_ETHER_ADDR_LEN);
    }

    gBS->CopyMem ((UINT8 *) Buf + NET_ETHER_ADDR_LEN * 2, &EtherType, 2);
  }

  if (!(EthInst->TxDescs[EthInst->TxDescWriteIdx].Tdes0 & TDES0_OWN) &&
      ((EthInst->TxDescWriteIdx + 1) % TX_DESC_NUM) != EthInst->TxDescWriteIdx) {
    EthInst->TxDescs[EthInst->TxDescWriteIdx].Tdes2 = (UINTN) Buf;
    EthInst->TxDescs[EthInst->TxDescWriteIdx].Tdes1 = BufSize << TDES1_TBS1_POS;
    EthInst->TxDescs[EthInst->TxDescWriteIdx].Tdes0 = TDES0_OWN | TDES0_IC | TDES0_LS | TDES0_FS | TDES0_TCH;
    EthInst->TxDescWriteIdx = (EthInst->TxDescWriteIdx + 1) % TX_DESC_NUM;
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NOT_READY;
  }

  EthInst->GmacRegs->DmaOperationMode |= DMA_OPERATIONMODE_ST;
  EthInst->GmacRegs->MacConfig        |= MAC_CONFIG_TE;
  EthInst->GmacRegs->DmaTxPollDemand   = 0;
  gBS->RestoreTPL (SavedTpl);
  return Status;
}
