/** @file
  Copyright (c) 2019 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/ArmSmcLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/NetLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleNetwork.h>
#include "GmacSnp.h"

#define BM1000_GPIO32_BASE  0x20200000
#define BM1000_GPIO32_DATA  (BM1000_GPIO32_BASE + 0x00)
#define BM1000_GPIO32_DIR   (BM1000_GPIO32_BASE + 0x04)

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
} GMAC_REGS;

#define MAC_CONFIG_RE               BIT2
#define MAC_CONFIG_TE               BIT3
#define MAC_CONFIG_ACS              BIT7
#define MAC_CONFIG_IPC              BIT10
#define MAC_CONFIG_DM               BIT11
#define MAC_CONFIG_DO               BIT13
#define MAC_CONFIG_FES              BIT14
#define MAC_CONFIG_PS               BIT15
#define MAC_CONFIG_DCRS             BIT16
#define MAC_CONFIG_SARC_POS         28

#define MAC_FRAMEFILTER_PR          BIT0
#define MAC_FRAMEFILTER_PM          BIT4
#define MAC_FRAMEFILTER_DBF         BIT5

#define MAC_MIISTATUS_LNKSPEED_POS  1
#define MAC_MIISTATUS_LNKSTS        BIT3

#define MAC_GPIO_GPO                BIT8

#define DMA_BUSMODE_SWR             BIT0
#define DMA_BUSMODE_ATDS            BIT7

#define DMA_STATUS_TI               BIT0
#define DMA_STATUS_RI               BIT6
#define DMA_STATUS_RU               BIT7

#define DMA_OPERATIONMODE_SR        BIT1
#define DMA_OPERATIONMODE_ST        BIT13
#define DMA_OPERATIONMODE_TSF       BIT21
#define DMA_OPERATIONMODE_RSF       BIT25

#define DMA_AXISTATUS_AXIWHSTS      BIT0
#define DMA_AXISTATUS_AXIRDSTS      BIT1

typedef struct {
  UINT32  Rdes0;
  UINT32  Rdes1;
  UINT32  Rdes2;
  UINT32  Rdes3;
  UINT32  Rdes4;
  UINT32  Rdes5;
  UINT32  Rdes6;
  UINT32  Rdes7;
} GMAC_RDESC;

#define RDES0_LS        BIT8
#define RDES0_FS        BIT9
#define RDES0_FL_POS    16
#define RDES0_FL_MSK    0x3FFF
#define RDES0_OWN       BIT31

#define RDES1_RBS1_POS  0
#define RDES1_RCH       BIT14

typedef struct {
  UINT32  Tdes0;
  UINT32  Tdes1;
  UINT32  Tdes2;
  UINT32  Tdes3;
  UINT32  Tdes4;
  UINT32  Tdes5;
  UINT32  Tdes6;
  UINT32  Tdes7;
} GMAC_TDESC;

#define TDES0_TCH       BIT20
#define TDES0_FS        BIT28
#define TDES0_LS        BIT29
#define TDES0_IC        BIT30
#define TDES0_OWN       BIT31

#define TDES1_TBS1_POS  0

#define RX_BUF_SIZE  2048
#define RX_DESC_NUM  64
#define TX_BUF_SIZE  2048
#define TX_DESC_NUM  64

typedef struct {
  EFI_HANDLE                   Handle;
  EFI_EVENT                    ExitBootServicesEvent;
  EFI_SIMPLE_NETWORK_PROTOCOL  Snp;
  EFI_SIMPLE_NETWORK_MODE      SnpMode;
  volatile GMAC_REGS          *Regs;
  UINTN                        Tx2ClkFreq;
  INTN                         ResetGpioPin;
  INTN                         ResetPolarity;
  volatile UINT8               RxBufs[RX_DESC_NUM][RX_BUF_SIZE] __attribute__((aligned(16)));
  volatile GMAC_RDESC          RxDescs[RX_DESC_NUM] __attribute__((aligned(sizeof (GMAC_RDESC))));
  volatile UINT8               TxBufs[TX_DESC_NUM][TX_BUF_SIZE] __attribute__((aligned(16)));
  volatile GMAC_TDESC          TxDescs[TX_DESC_NUM] __attribute__((aligned(sizeof (GMAC_TDESC))));
  EFI_PHYSICAL_ADDRESS         TxBufPtrs[TX_DESC_NUM];
  UINTN                        RxDescReadIdx;
  UINTN                        TxDescWriteIdx;
  UINTN                        TxDescReleaseIdx;
} GMAC_INSTANCE;

STATIC
VOID
EFIAPI
GmacSnpExitBootServices (
  IN  EFI_EVENT  Event,
  IN  VOID      *Context
  );

STATIC
UINT32
EFIAPI
GmacSnpGetReceiveFilterSetting (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpGetStatus (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  OUT  UINT32                       *InterruptStatus,  OPTIONAL
  OUT  VOID                        **TxBuf             OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpInitialize (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINTN                         ExtraRxBufSize,  OPTIONAL
  IN  UINTN                         ExtraTxBufSize   OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpMCastIpToMac (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN   BOOLEAN                       Ipv6,
  IN   EFI_IP_ADDRESS               *Ip,
  OUT  EFI_MAC_ADDRESS              *McastMacAddr
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpNvData (
  IN      EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN      BOOLEAN                       ReadWrite,
  IN      UINTN                         Offset,
  IN      UINTN                         BufSize,
  IN OUT  VOID                         *Buf
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpReset (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  BOOLEAN                       ExtendedVerification
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpReceiveFilters (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINT32                        Enable,
  IN  UINT32                        Disable,
  IN  BOOLEAN                       ResetMCastFilter,
  IN  UINTN                         MCastFilterCnt,   OPTIONAL
  IN  EFI_MAC_ADDRESS              *MCastFilter       OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpShutdown (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpStart (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpStationAddress (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN     BOOLEAN                       Reset,
  IN     EFI_MAC_ADDRESS              *NewMacAddr  OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpStatistics (
  IN      EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN      BOOLEAN                       Reset,
  IN OUT  UINTN                        *StatisticsSize,  OPTIONAL
  OUT     EFI_NETWORK_STATISTICS       *StatisticsTable  OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpStop (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpReceive (
  IN      EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  OUT     UINTN                        *HdrSize,  OPTIONAL
  IN OUT  UINTN                        *BufSize,
  OUT     VOID                         *Buf,
  OUT     EFI_MAC_ADDRESS              *SrcAddr,  OPTIONAL
  OUT     EFI_MAC_ADDRESS              *DstAddr,  OPTIONAL
  OUT     UINT16                       *Protocol  OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpTransmit (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINTN                         HdrSize,
  IN  UINTN                         BufSize,
  IN  VOID                         *Buf,
  IN  EFI_MAC_ADDRESS              *SrcAddr,  OPTIONAL
  IN  EFI_MAC_ADDRESS              *DstAddr,  OPTIONAL
  IN  UINT16                       *Protocol  OPTIONAL
  );

STATIC
UINT32
GmacSnpGetReceiveFilterSetting (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  )
{
  CONST GMAC_INSTANCE *CONST  GmacInst = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  UINT32  ReceiveFilterSetting = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST;

  if (!(GmacInst->Regs->MacFrameFilter & MAC_FRAMEFILTER_DBF)) {
    ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST;
  }

  if (GmacInst->Regs->MacFrameFilter & MAC_FRAMEFILTER_PR) {
    ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS;
  }

  if (GmacInst->Regs->MacFrameFilter & MAC_FRAMEFILTER_PM) {
    ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;
  }

  return ReceiveFilterSetting;
}

STATIC
EFI_STATUS
EFIAPI
GmacSnpGetStatus (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  OUT  UINT32                       *InterruptStatus,  OPTIONAL
  OUT  VOID                        **TxBuf             OPTIONAL
  )
{
  GMAC_INSTANCE *CONST  GmacInst = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  UINTN                 MacMiiStatus;
  EFI_TPL               SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpGetStatus: SNP not initialized\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;

  case EfiSimpleNetworkStopped:
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpGetStatus: SNP invalid state = %u\n", GmacInst->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  MacMiiStatus = GmacInst->Regs->MacMiiStatus;
  Snp->Mode->MediaPresent = MacMiiStatus & MAC_MIISTATUS_LNKSTS;

  if (Snp->Mode->MediaPresent) {
    ARM_SMC_ARGS  ArmSmcArgs;
    CONST UINTN   LnkSpeed = (MacMiiStatus >> MAC_MIISTATUS_LNKSPEED_POS) & 0x3;

    if (!GmacInst->Tx2ClkFreq) {
      ArmSmcArgs.Arg0 = BAIKAL_SMC_LCRU_ID;
      if (GmacInst->Regs == (VOID *) BM1000_MMXGBE_GMAC0_BASE) {
        ArmSmcArgs.Arg1 = BM1000_MMXGBE_GMAC0_TX2CLKCH;
      } else {
        ArmSmcArgs.Arg1 = BM1000_MMXGBE_GMAC1_TX2CLKCH;
      }

      ArmSmcArgs.Arg2 = BAIKAL_SMC_PLAT_CMU_CLKCH_GET_RATE;
      ArmSmcArgs.Arg4 = BM1000_MMXGBE_BASE;
      ArmCallSmc (&ArmSmcArgs);
      GmacInst->Tx2ClkFreq = ArmSmcArgs.Arg0;
    }

    if ((!LnkSpeed     && GmacInst->Tx2ClkFreq != 5000000)  ||
        (LnkSpeed == 1 && GmacInst->Tx2ClkFreq != 50000000) ||
        (LnkSpeed == 2 && GmacInst->Tx2ClkFreq != 250000000)) {
      if (!LnkSpeed) {
        GmacInst->Tx2ClkFreq = 5000000;
      } else if (LnkSpeed == 1) {
        GmacInst->Tx2ClkFreq = 50000000;
      } else {
        GmacInst->Tx2ClkFreq = 250000000;
      }

      ArmSmcArgs.Arg0 = BAIKAL_SMC_LCRU_ID;
      if (GmacInst->Regs == (VOID *) BM1000_MMXGBE_GMAC0_BASE) {
        ArmSmcArgs.Arg1 = BM1000_MMXGBE_GMAC0_TX2CLKCH;
      } else {
        ArmSmcArgs.Arg1 = BM1000_MMXGBE_GMAC1_TX2CLKCH;
      }

      ArmSmcArgs.Arg2 = BAIKAL_SMC_PLAT_CMU_CLKCH_SET_RATE;
      ArmSmcArgs.Arg3 = GmacInst->Tx2ClkFreq;
      ArmSmcArgs.Arg4 = BM1000_MMXGBE_BASE;
      ArmCallSmc (&ArmSmcArgs);
    }

    if (!LnkSpeed) {
      GmacInst->Regs->MacConfig = (GmacInst->Regs->MacConfig & ~MAC_CONFIG_FES) | MAC_CONFIG_PS;
    } else if (LnkSpeed == 1) {
      GmacInst->Regs->MacConfig |=   MAC_CONFIG_PS | MAC_CONFIG_FES;
    } else if (LnkSpeed == 2) {
      GmacInst->Regs->MacConfig &= ~(MAC_CONFIG_PS | MAC_CONFIG_FES);
    }
  }

  if (TxBuf != NULL) {
    if (GmacInst->TxDescReleaseIdx != GmacInst->TxDescWriteIdx) {
      *((UINT32 *) TxBuf) = GmacInst->TxBufPtrs[GmacInst->TxDescReleaseIdx];
      GmacInst->TxDescReleaseIdx = (GmacInst->TxDescReleaseIdx + 1) % TX_DESC_NUM;
    } else {
      *TxBuf = NULL;
    }
  }

  if (InterruptStatus) {
    *InterruptStatus =
      (GmacInst->Regs->DmaStatus & DMA_STATUS_RI ? EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT  : 0) |
      (GmacInst->Regs->DmaStatus & DMA_STATUS_TI ? EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT : 0);

    GmacInst->Regs->DmaStatus =
      (*InterruptStatus & EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT  ? DMA_STATUS_RI : 0) |
      (*InterruptStatus & EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT ? DMA_STATUS_TI : 0);
  }

  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

EFI_STATUS
GmacSnpInstanceCtor (
  IN   VOID              *GmacRegs,
  IN   CONST INTN         ResetGpioPin,
  IN   CONST INTN         ResetPolarity,
  IN   EFI_MAC_ADDRESS   *MacAddr,
  OUT  VOID             **Snp,
  OUT  EFI_HANDLE       **Handle
  )
{
  GMAC_INSTANCE         *GmacInst;
  EFI_PHYSICAL_ADDRESS   PhysicalAddr;
  EFI_STATUS             Status;

  PhysicalAddr = (EFI_PHYSICAL_ADDRESS) (BASE_4GB - 1);
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiBootServicesData,
                  EFI_SIZE_TO_PAGES (sizeof (GMAC_INSTANCE)),
                  &PhysicalAddr
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Gmac(%p)SnpInstaceCtor: unable to allocate GmacInst, Status = %r\n", GmacRegs, Status));
    return Status;
  }

  GmacInst = (VOID *) PhysicalAddr;
  GmacInst->Handle         = NULL;
  GmacInst->Regs           = GmacRegs;
  GmacInst->Tx2ClkFreq     = 0;
  GmacInst->ResetGpioPin   = ResetGpioPin;
  GmacInst->ResetPolarity  = ResetPolarity;

  GmacInst->RxDescReadIdx    = 0;
  GmacInst->TxDescWriteIdx   = 0;
  GmacInst->TxDescReleaseIdx = 0;

  GmacInst->Snp.Revision       = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
  GmacInst->Snp.Start          = GmacSnpStart;
  GmacInst->Snp.Stop           = GmacSnpStop;
  GmacInst->Snp.Initialize     = GmacSnpInitialize;
  GmacInst->Snp.Reset          = GmacSnpReset;
  GmacInst->Snp.Shutdown       = GmacSnpShutdown;
  GmacInst->Snp.ReceiveFilters = GmacSnpReceiveFilters;
  GmacInst->Snp.StationAddress = GmacSnpStationAddress;
  GmacInst->Snp.Statistics     = GmacSnpStatistics;
  GmacInst->Snp.MCastIpToMac   = GmacSnpMCastIpToMac;
  GmacInst->Snp.NvData         = GmacSnpNvData;
  GmacInst->Snp.GetStatus      = GmacSnpGetStatus;
  GmacInst->Snp.Transmit       = GmacSnpTransmit;
  GmacInst->Snp.Receive        = GmacSnpReceive;
  GmacInst->Snp.WaitForPacket  = NULL;
  GmacInst->Snp.Mode           = &GmacInst->SnpMode;

  GmacInst->SnpMode.State                 = EfiSimpleNetworkStopped;
  GmacInst->SnpMode.HwAddressSize         = NET_ETHER_ADDR_LEN;
  GmacInst->SnpMode.MediaHeaderSize       = sizeof (ETHER_HEAD);
  GmacInst->SnpMode.MaxPacketSize         = 1500;
  GmacInst->SnpMode.NvRamSize             = 0;
  GmacInst->SnpMode.NvRamAccessSize       = 0;
  GmacInst->SnpMode.ReceiveFilterMask     = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST     |
                                            EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST   |
                                            EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS |
                                            EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;

  GmacInst->SnpMode.ReceiveFilterSetting  = GmacSnpGetReceiveFilterSetting (&GmacInst->Snp);
  GmacInst->SnpMode.MaxMCastFilterCount   = MAX_MCAST_FILTER_CNT;
  GmacInst->SnpMode.MCastFilterCount      = 0;
  GmacInst->SnpMode.IfType                = NET_IFTYPE_ETHERNET;
  GmacInst->SnpMode.MacAddressChangeable  = TRUE;
  GmacInst->SnpMode.MultipleTxSupported   = FALSE;
  GmacInst->SnpMode.MediaPresentSupported = TRUE;
  GmacInst->SnpMode.MediaPresent          = FALSE;

  gBS->CopyMem (&GmacInst->SnpMode.CurrentAddress,   MacAddr, sizeof (EFI_MAC_ADDRESS));
  gBS->CopyMem (&GmacInst->SnpMode.PermanentAddress, MacAddr, sizeof (EFI_MAC_ADDRESS));
  gBS->SetMem (&GmacInst->SnpMode.MCastFilter, MAX_MCAST_FILTER_CNT * sizeof (EFI_MAC_ADDRESS), 0);
  gBS->SetMem (&GmacInst->SnpMode.BroadcastAddress, sizeof (EFI_MAC_ADDRESS), 0xFF);

  *Handle = &GmacInst->Handle;
  *Snp    = &GmacInst->Snp;

  Status = gBS->CreateEvent (
                  EVT_SIGNAL_EXIT_BOOT_SERVICES,
                  TPL_NOTIFY,
                  GmacSnpExitBootServices,
                  GmacInst,
                  &GmacInst->ExitBootServicesEvent
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
GmacSnpInstanceDtor (
  IN  VOID  *Snp
  )
{
  GMAC_INSTANCE *CONST  GmacInst = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  EFI_STATUS            Status;

  Status = gBS->FreePages ((EFI_PHYSICAL_ADDRESS) GmacInst, EFI_SIZE_TO_PAGES (sizeof (GMAC_INSTANCE)));
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
GmacSnpInitialize (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINTN                         ExtraRxBufSize,  OPTIONAL
  IN  UINTN                         ExtraTxBufSize   OPTIONAL
  )
{
  GMAC_INSTANCE *CONST  GmacInst = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  UINTN                 DescIdx;
  UINTN                 Limit;
  EFI_TPL               SavedTpl;

  ASSERT (Snp != NULL);

  if (ExtraRxBufSize != 0 || ExtraTxBufSize != 0) {
    return EFI_UNSUPPORTED;
  }

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkStarted:
    break;

  case EfiSimpleNetworkInitialized:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpInitialize: SNP already initialized\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_SUCCESS;

  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpInitialize: SNP not started\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpInitialize: SNP invalid state = %u\n", GmacInst->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (GmacInst->ResetGpioPin >= 0  &&
      GmacInst->ResetGpioPin <= 31 &&
      GmacInst->ResetPolarity >= 0) {
    if (GmacInst->ResetPolarity) {
      MmioAnd32 (BM1000_GPIO32_DATA, ~(1 << GmacInst->ResetGpioPin));
    } else {
      MmioOr32 (BM1000_GPIO32_DATA, 1 << GmacInst->ResetGpioPin);
    }

    MmioOr32 (BM1000_GPIO32_DIR, 1 << GmacInst->ResetGpioPin);
  }

  GmacInst->Regs->DmaBusMode = DMA_BUSMODE_SWR;
  gBS->Stall (100);

  if (GmacInst->ResetGpioPin >= 0  &&
      GmacInst->ResetGpioPin <= 31 &&
      GmacInst->ResetPolarity >= 0) {
    if (GmacInst->ResetPolarity) {
      MmioOr32 (BM1000_GPIO32_DATA, 1 << GmacInst->ResetGpioPin);
    } else {
      MmioAnd32 (BM1000_GPIO32_DATA, ~(1 << GmacInst->ResetGpioPin));
    }
  }

  for (Limit = 3000; Limit; --Limit) {
    if (GmacInst->ResetGpioPin == 0xE0) {
      GmacInst->Regs->MacGpio |= MAC_GPIO_GPO;
    }

    if (!(GmacInst->Regs->DmaBusMode & DMA_BUSMODE_SWR)) {
      break;
    }

    gBS->Stall (1000);
  }

  if (!Limit) {
    DEBUG ((EFI_D_ERROR, "Gmac(%p)SnpInitialize: GMAC reset not completed\n", GmacInst->Regs));
    return EFI_DEVICE_ERROR;
  }

  for (Limit = 3000; Limit; --Limit) {
    if (!(GmacInst->Regs->DmaAxiStatus & (DMA_AXISTATUS_AXIRDSTS | DMA_AXISTATUS_AXIWHSTS))) {
      break;
    }

    gBS->Stall (1000);
  }

  GmacInst->Regs->MacAddr0Hi =
    Snp->Mode->CurrentAddress.Addr[4] |
    Snp->Mode->CurrentAddress.Addr[5] << 8;

  GmacInst->Regs->MacAddr0Lo =
    Snp->Mode->CurrentAddress.Addr[0]       |
    Snp->Mode->CurrentAddress.Addr[1] << 8  |
    Snp->Mode->CurrentAddress.Addr[2] << 16 |
    Snp->Mode->CurrentAddress.Addr[3] << 24;

  GmacInst->Regs->DmaBusMode        |= DMA_BUSMODE_ATDS;
  GmacInst->Regs->DmaRxDescBaseAddr  = (EFI_PHYSICAL_ADDRESS) GmacInst->RxDescs;
  GmacInst->Regs->DmaTxDescBaseAddr  = (EFI_PHYSICAL_ADDRESS) GmacInst->TxDescs;

  GmacInst->RxDescReadIdx    = 0;
  GmacInst->TxDescWriteIdx   = 0;
  GmacInst->TxDescReleaseIdx = 0;

  for (DescIdx = 0; DescIdx < RX_DESC_NUM; ++DescIdx) {
    GmacInst->RxDescs[DescIdx].Rdes0 = RDES0_OWN;
    GmacInst->RxDescs[DescIdx].Rdes1 = RDES1_RCH | (RX_BUF_SIZE << RDES1_RBS1_POS);
    GmacInst->RxDescs[DescIdx].Rdes2 = (EFI_PHYSICAL_ADDRESS) GmacInst->RxBufs[DescIdx];
    GmacInst->RxDescs[DescIdx].Rdes3 = (EFI_PHYSICAL_ADDRESS) &GmacInst->RxDescs[(DescIdx + 1) % RX_DESC_NUM];
  }

  for (DescIdx = 0; DescIdx < TX_DESC_NUM; ++DescIdx) {
    GmacInst->TxDescs[DescIdx].Tdes0 = 0;
    GmacInst->TxDescs[DescIdx].Tdes3 = (EFI_PHYSICAL_ADDRESS) &GmacInst->TxDescs[(DescIdx + 1) % TX_DESC_NUM];
  }

  GmacInst->Regs->DmaStatus = 0xFFFFFFFF;
  GmacInst->Regs->MacConfig = (3 << MAC_CONFIG_SARC_POS) |
                                    MAC_CONFIG_DCRS      |
                                    MAC_CONFIG_DO        |
                                    MAC_CONFIG_DM        |
                                    MAC_CONFIG_IPC       |
                                    MAC_CONFIG_ACS;

  // Wait for linkup if the link has already been established
  if (Snp->Mode->MediaPresent) {
    UINTN  Limit;

    // TODO: it may require to adjust Tx2ClkFreq, so the flow from SnpGetStatus() should be re-used
    for (Limit = 4000; ; --Limit) {
      if (GmacInst->Regs->MacMiiStatus & MAC_MIISTATUS_LNKSTS) {
        break;
      } else if (!Limit) {
        Snp->Mode->MediaPresent = FALSE;
        break;
      }

      gBS->Stall (1000);
    }
  }

  GmacInst->Regs->DmaOperationMode = DMA_OPERATIONMODE_TSF |
                                     DMA_OPERATIONMODE_RSF |
                                     DMA_OPERATIONMODE_ST  |
                                     DMA_OPERATIONMODE_SR;

  GmacInst->Regs->MacConfig       |= MAC_CONFIG_TE | MAC_CONFIG_RE;
  GmacInst->Regs->DmaRxPollDemand  = 0;

  Snp->Mode->State = EfiSimpleNetworkInitialized;
  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
GmacSnpMCastIpToMac (
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
VOID
EFIAPI
GmacSnpExitBootServices (
  IN  EFI_EVENT  Event,
  IN  VOID      *Context
  )
{
  GMAC_INSTANCE *CONST  GmacInst = Context;
  GmacInst->Regs->DmaBusMode = DMA_BUSMODE_SWR;

  if (GmacInst->ResetGpioPin >= 0  &&
      GmacInst->ResetGpioPin <= 31) {
    MmioAnd32 (BM1000_GPIO32_DIR, ~(1 << GmacInst->ResetGpioPin));
  }

  GmacInst->Snp.Mode->MediaPresent = FALSE;
}

STATIC
EFI_STATUS
EFIAPI
GmacSnpNvData (
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
GmacSnpReset (
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
GmacSnpReceiveFilters (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINT32                        Enable,
  IN  UINT32                        Disable,
  IN  BOOLEAN                       ResetMCastFilter,
  IN  UINTN                         MCastFilterCnt,   OPTIONAL
  IN  EFI_MAC_ADDRESS              *MCastFilter       OPTIONAL
  )
{
  GMAC_INSTANCE *CONST  GmacInst = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  CONST UINT32          ResultingMsk = Enable & ~Disable;
  EFI_TPL               SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceiveFilters: SNP not initialized\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;

  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceiveFilters: SNP not started\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceiveFilters: SNP invalid state = %u\n", GmacInst->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (ResultingMsk & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST) {
    GmacInst->Regs->MacFrameFilter &= ~MAC_FRAMEFILTER_DBF;
  } else {
    GmacInst->Regs->MacFrameFilter |=  MAC_FRAMEFILTER_DBF;
  }

  if (ResultingMsk & (EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST |
                      EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST)) {
    GmacInst->Regs->MacFrameFilter |=  MAC_FRAMEFILTER_PM;
  } else {
    GmacInst->Regs->MacFrameFilter &= ~MAC_FRAMEFILTER_PM;
  }

  if (ResultingMsk & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS) {
    GmacInst->Regs->MacFrameFilter |=  MAC_FRAMEFILTER_PR;
  } else {
    GmacInst->Regs->MacFrameFilter &= ~MAC_FRAMEFILTER_PR;
  }

  Snp->Mode->ReceiveFilterSetting = GmacSnpGetReceiveFilterSetting (Snp);
  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
GmacSnpShutdown (
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
GmacSnpStart (
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
GmacSnpStationAddress (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  BOOLEAN                       Reset,
  IN  EFI_MAC_ADDRESS              *NewMacAddr  OPTIONAL
  )
{
  GMAC_INSTANCE *CONST  GmacInst = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  EFI_TPL               SavedTpl;

  ASSERT (Snp != NULL);

  DEBUG ((EFI_D_NET, "Gmac(%p)SnpStationAddress()\n", GmacInst->Regs));
  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpStationAddress: SNP not initialized\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;

  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpStationAddress: SNP not started\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpStationAddress: SNP invalid state = %u\n", GmacInst->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (Reset) {
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpStationAddress: reset MAC address\n", GmacInst->Regs));
    Snp->Mode->CurrentAddress = Snp->Mode->PermanentAddress;
  } else {
    if (NewMacAddr == NULL) {
      gBS->RestoreTPL (SavedTpl);
      return EFI_INVALID_PARAMETER;
    }

    gBS->CopyMem (&Snp->Mode->CurrentAddress, NewMacAddr, sizeof (EFI_MAC_ADDRESS));
  }

  GmacInst->Regs->MacAddr0Hi =
    Snp->Mode->CurrentAddress.Addr[4] |
    Snp->Mode->CurrentAddress.Addr[5] << 8;

  GmacInst->Regs->MacAddr0Lo =
    Snp->Mode->CurrentAddress.Addr[0]       |
    Snp->Mode->CurrentAddress.Addr[1] << 8  |
    Snp->Mode->CurrentAddress.Addr[2] << 16 |
    Snp->Mode->CurrentAddress.Addr[3] << 24;

  DEBUG ((
    EFI_D_NET,
    "Gmac(%p)SnpStationAddress: current MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
    GmacInst->Regs,
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
GmacSnpStatistics (
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
GmacSnpStop (
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
GmacSnpReceive ( // Receive a packet from a network interface
  IN      EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  OUT     UINTN                        *HdrSize,  OPTIONAL
  IN OUT  UINTN                        *BufSize,
  OUT     VOID                         *Buf,
  OUT     EFI_MAC_ADDRESS              *SrcAddr,  OPTIONAL
  OUT     EFI_MAC_ADDRESS              *DstAddr,  OPTIONAL
  OUT     UINT16                       *Protocol  OPTIONAL
  )
{
  GMAC_INSTANCE *CONST  GmacInst = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  EFI_TPL               SavedTpl;

  ASSERT (Snp != NULL);

  if (BufSize == NULL || Buf == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceive: SNP not initialized\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;

  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceive: SNP not started\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceive: SNP invalid state = %u\n", GmacInst->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  EFI_STATUS Status = EFI_NOT_READY;

  while (!(GmacInst->RxDescs[GmacInst->RxDescReadIdx].Rdes0 & RDES0_OWN) && (Status == EFI_NOT_READY)) {
    if (((RDES0_FS | RDES0_LS) & GmacInst->RxDescs[GmacInst->RxDescReadIdx].Rdes0) ==
         (RDES0_FS | RDES0_LS)) {
      CONST UINTN  FrameLen = (GmacInst->RxDescs[GmacInst->RxDescReadIdx].Rdes0 >> RDES0_FL_POS) & RDES0_FL_MSK;

      if (*BufSize < FrameLen) {
        DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceive: receive BufSize(%u) < FrameLen(%u)\n", GmacInst->Regs, *BufSize, FrameLen));
        Status = EFI_BUFFER_TOO_SMALL;
      }

      *BufSize = FrameLen;

      if (Status != EFI_BUFFER_TOO_SMALL) {
        gBS->CopyMem (Buf, (VOID *) GmacInst->RxBufs[GmacInst->RxDescReadIdx], FrameLen);

        if (HdrSize != NULL) {
          *HdrSize = Snp->Mode->MediaHeaderSize;
        }

        if (DstAddr != NULL) {
          gBS->CopyMem (DstAddr, Buf, NET_ETHER_ADDR_LEN);
        }

        if (SrcAddr != NULL) {
          gBS->CopyMem (SrcAddr, (UINT8 *) Buf + 6, NET_ETHER_ADDR_LEN);
        }

        if (Protocol != NULL) {
          *Protocol = NTOHS (*(UINT16 *)((UINT8 *) Buf + 12));
        }

        Status = EFI_SUCCESS;
      }
    }

    GmacInst->RxDescs[GmacInst->RxDescReadIdx].Rdes0 = RDES0_OWN;
    GmacInst->RxDescReadIdx = (GmacInst->RxDescReadIdx + 1) % RX_DESC_NUM;

    if (GmacInst->Regs->DmaStatus & DMA_STATUS_RU) {
      GmacInst->Regs->DmaStatus   = DMA_STATUS_RU;
      GmacInst->Regs->DmaRxPollDemand = 0;
    }
  }

  gBS->RestoreTPL (SavedTpl);
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
GmacSnpTransmit (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp,
  IN  UINTN                         HdrSize,
  IN  UINTN                         BufSize,
  IN  VOID                         *Buf,
  IN  EFI_MAC_ADDRESS              *SrcAddr,  OPTIONAL
  IN  EFI_MAC_ADDRESS              *DstAddr,  OPTIONAL
  IN  UINT16                       *Protocol  OPTIONAL
  )
{
  GMAC_INSTANCE *CONST  GmacInst = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  EFI_TPL               SavedTpl;
  EFI_STATUS            Status;

  ASSERT (Snp != NULL);

  if (Buf == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpTransmit: SNP not initialized\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;

  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpTransmit: SNP not started\n", GmacInst->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpTransmit: SNP invalid state = %u\n", GmacInst->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (HdrSize != 0) {
    if (HdrSize != Snp->Mode->MediaHeaderSize) {
      DEBUG ((EFI_D_NET, "Gmac(%p)SnpTransmit: HdrSize(%u) != Snp->Mode->MediaHeaderSize(%u)\n", GmacInst->Regs, HdrSize, Snp->Mode->MediaHeaderSize));
      gBS->RestoreTPL (SavedTpl);
      return EFI_INVALID_PARAMETER;
    }

    if (DstAddr == NULL || Protocol == NULL) {
      DEBUG ((EFI_D_NET, "Gmac(%p)SnpTransmit: Hdr DstAddr(%p) or Protocol(%p) is NULL\n", GmacInst->Regs, DstAddr, Protocol));
      gBS->RestoreTPL (SavedTpl);
      return EFI_INVALID_PARAMETER;
    }
  }

  if (!Snp->Mode->MediaPresent) {
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_READY;
  }

  if (HdrSize != 0) {
    UINT16  EtherType = HTONS (*Protocol);

    gBS->CopyMem (Buf, DstAddr, NET_ETHER_ADDR_LEN);

    if (SrcAddr != NULL) {
      gBS->CopyMem ((UINT8 *) Buf + NET_ETHER_ADDR_LEN, SrcAddr, NET_ETHER_ADDR_LEN);
    } else {
      gBS->CopyMem ((UINT8 *) Buf + NET_ETHER_ADDR_LEN, &Snp->Mode->CurrentAddress, NET_ETHER_ADDR_LEN);
    }

    gBS->CopyMem ((UINT8 *) Buf + NET_ETHER_ADDR_LEN * 2, &EtherType, 2);
  }

  if (!(GmacInst->TxDescs[GmacInst->TxDescWriteIdx].Tdes0 & TDES0_OWN) &&
      ((GmacInst->TxDescWriteIdx + 1) % TX_DESC_NUM) != GmacInst->TxDescWriteIdx) {
    // Store the Buf address in order to release it later
    GmacInst->TxBufPtrs[GmacInst->TxDescWriteIdx] = (EFI_PHYSICAL_ADDRESS) Buf;
    // Buf address could be higher than BASE_4GB, so copy the Buf data to GmacInst->TxBuf
    gBS->CopyMem ((UINT8 *) GmacInst->TxBufs[GmacInst->TxDescWriteIdx], Buf, BufSize);
    GmacInst->TxDescs[GmacInst->TxDescWriteIdx].Tdes2 = (EFI_PHYSICAL_ADDRESS) GmacInst->TxBufs[GmacInst->TxDescWriteIdx];
    GmacInst->TxDescs[GmacInst->TxDescWriteIdx].Tdes1 = BufSize << TDES1_TBS1_POS;
    GmacInst->TxDescs[GmacInst->TxDescWriteIdx].Tdes0 = TDES0_OWN | TDES0_IC | TDES0_LS | TDES0_FS | TDES0_TCH;
    GmacInst->TxDescWriteIdx = (GmacInst->TxDescWriteIdx + 1) % TX_DESC_NUM;
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NOT_READY;
  }

  GmacInst->Regs->DmaOperationMode |= DMA_OPERATIONMODE_ST;
  GmacInst->Regs->MacConfig        |= MAC_CONFIG_TE;
  GmacInst->Regs->DmaTxPollDemand   = 0;
  gBS->RestoreTPL (SavedTpl);
  return Status;
}
