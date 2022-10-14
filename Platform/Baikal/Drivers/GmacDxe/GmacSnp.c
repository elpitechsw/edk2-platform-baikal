/** @file
  Copyright (c) 2019 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/ArmLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/CmuLib.h>
#include <Library/DebugLib.h>
#include <Library/GpioLib.h>
#include <Library/IoLib.h>
#include <Library/NetLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/Cpu.h>
#include <Protocol/SimpleNetwork.h>
#include "GmacRegs.h"
#include "GmacSnp.h"

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

#define MII_BUSY                    BIT0
#define MII_WRITE                   BIT1
#define MII_ADDR_SHIFT              11
#define MII_REG_SHIFT               6
#define MII_CLK_CSR_SHIFT           2
#define MII_DATA_MASK               0xFFFF

#define MII_BMCR                    0x00    // Basic mode control register
#define BMCR_ANRESTART              0x0200  // Auto negotiation restart
#define BMCR_ISOLATE                0x0400  // Isolate data paths from MII
#define BMCR_RESET                  0x8000  // Reset to default state
#define MII_PHY_ID1                 0x02    // PHY ID 1
#define MII_PHY_ID2                 0x03    // PHY ID 2

#define MII_MARVELL_COPPER_PAGE          0x00
#define MII_MARVELL_MSCR_PAGE            0x02
#define MII_88E1121_PHY_MSCR_REG         21
#define MII_88E1121_PHY_MSCR_RX_DELAY    BIT5
#define MII_88E1121_PHY_MSCR_TX_DELAY    BIT4
#define MII_88E1121_PHY_MSCR_DELAY_MASK  ((BIT5) | (BIT4))
#define MII_MARVELL_PHY_PAGE             22

#define MARVELL_PHY_ID_88E1510           0x01410DD0
#define MARVELL_PHY_ID_MASK              0xFFFFFFF0

typedef union {
  UINT16  Regs[2];
  UINT32  PhyId;
} PHY_ID;

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

#define TX2_CLKCH_RATE_10MBPS    5000000
#define TX2_CLKCH_RATE_100MBPS   50000000
#define TX2_CLKCH_RATE_1000MBPS  250000000

#define BAIKAL_SMC_GMAC_DIV2_ENABLE   0x82000500
#define BAIKAL_SMC_GMAC_DIV2_DISABLE  0x82000501

typedef struct {
  UINT8       RxBufs[RX_DESC_NUM][RX_BUF_SIZE] __attribute__((aligned(16)));
  GMAC_RDESC  RxDescs[RX_DESC_NUM] __attribute__((aligned(sizeof (GMAC_RDESC))));
  UINT8       TxBufs[TX_DESC_NUM][TX_BUF_SIZE] __attribute__((aligned(16)));
  GMAC_TDESC  TxDescs[TX_DESC_NUM] __attribute__((aligned(sizeof (GMAC_TDESC))));
} GMAC_DMA_BUFFERS;

typedef struct {
  EFI_HANDLE                    Handle;
  EFI_EVENT                     ExitBootServicesEvent;
  EFI_SIMPLE_NETWORK_PROTOCOL   Snp;
  EFI_SIMPLE_NETWORK_MODE       SnpMode;
  volatile GMAC_REGS           *Regs;
  volatile GMAC_DMA_BUFFERS    *Dma;
  UINTN                         RxDescReadIdx;
  EFI_PHYSICAL_ADDRESS          TxBufPtrs[TX_DESC_NUM];
  UINTN                         TxDescWriteIdx;
  UINTN                         TxDescReleaseIdx;
  EFI_PHYSICAL_ADDRESS          ResetGpioBase;
  INTN                          ResetGpioPin;
  INTN                          ResetPolarity;
  EFI_PHYSICAL_ADDRESS          Tx2ClkChCtlAddr;
  INTN                          Tx2ClkChRate;
  BOOLEAN                       Tx2AddDiv2;
  INTN                          PhyAddr;
  UINTN                         ClkCsr;
  BOOLEAN                       RgmiiRxId;
  BOOLEAN                       RgmiiTxId;
} GMAC_INSTANCE;

STATIC
VOID
EFIAPI
GmacSnpExitBootServices (
  IN  EFI_EVENT   Event,
  IN  VOID       *Context
  );

STATIC
UINT32
EFIAPI
GmacSnpGetReceiveFilterSetting (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL      *Snp
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpGetStatus (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL     *Snp,
  OUT  UINT32                          *InterruptStatus,  OPTIONAL
  OUT  VOID                           **TxBuf             OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpInitialize (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL      *Snp,
  IN  UINTN                             ExtraRxBufSize,  OPTIONAL
  IN  UINTN                             ExtraTxBufSize   OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpMCastIpToMac (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL     *Snp,
  IN   BOOLEAN                          Ipv6,
  IN   EFI_IP_ADDRESS                  *Ip,
  OUT  EFI_MAC_ADDRESS                 *McastMacAddr
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
  IN  EFI_SIMPLE_NETWORK_PROTOCOL      *Snp,
  IN  BOOLEAN                           ExtendedVerification
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpReceiveFilters (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL      *Snp,
  IN  UINT32                            Enable,
  IN  UINT32                            Disable,
  IN  BOOLEAN                           ResetMCastFilter,
  IN  UINTN                             MCastFilterCnt,   OPTIONAL
  IN  EFI_MAC_ADDRESS                  *MCastFilter       OPTIONAL
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpShutdown (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL      *Snp
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpStart (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL      *Snp
  );

STATIC
EFI_STATUS
EFIAPI
GmacSnpStationAddress (
  IN     EFI_SIMPLE_NETWORK_PROTOCOL   *Snp,
  IN     BOOLEAN                        Reset,
  IN     EFI_MAC_ADDRESS               *NewMacAddr  OPTIONAL
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
  IN  EFI_SIMPLE_NETWORK_PROTOCOL      *Snp
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
  IN  EFI_SIMPLE_NETWORK_PROTOCOL      *Snp,
  IN  UINTN                             HdrSize,
  IN  UINTN                             BufSize,
  IN  VOID                             *Buf,
  IN  EFI_MAC_ADDRESS                  *SrcAddr,  OPTIONAL
  IN  EFI_MAC_ADDRESS                  *DstAddr,  OPTIONAL
  IN  UINT16                           *Protocol  OPTIONAL
  );

STATIC
UINT32
GmacSnpGetReceiveFilterSetting (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL  *Snp
  )
{
  CONST GMAC_INSTANCE * CONST  Gmac = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  UINT32  ReceiveFilterSetting = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST;

  if (!(Gmac->Regs->MacFrameFilter & MAC_FRAMEFILTER_DBF)) {
    ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST;
  }

  if (Gmac->Regs->MacFrameFilter & MAC_FRAMEFILTER_PR) {
    ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS;
  }

  if (Gmac->Regs->MacFrameFilter & MAC_FRAMEFILTER_PM) {
    ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;
  }

  return ReceiveFilterSetting;
}

STATIC
EFI_STATUS
EFIAPI
GmacSnpGetStatus (
  IN   EFI_SIMPLE_NETWORK_PROTOCOL   *Snp,
  OUT  UINT32                        *InterruptStatus,  OPTIONAL
  OUT  VOID                         **TxBuf             OPTIONAL
  )
{
  GMAC_INSTANCE * CONST  Gmac = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  UINTN                  LnkSpeed;
  UINTN                  MacMiiStatus;
  EFI_TPL                SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpGetStatus: SNP not initialized\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;

  case EfiSimpleNetworkStopped:
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpGetStatus: SNP invalid state = %u\n", Gmac->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  MacMiiStatus = Gmac->Regs->MacMiiStatus;
  Snp->Mode->MediaPresent = MacMiiStatus & MAC_MIISTATUS_LNKSTS;
  LnkSpeed = (MacMiiStatus >> MAC_MIISTATUS_LNKSPEED_POS) & 0x3;

  if (Gmac->Tx2ClkChCtlAddr) {
    if (Gmac->Tx2ClkChRate < 0) {
      Gmac->Tx2ClkChRate = CmuClkChGetRate (Gmac->Tx2ClkChCtlAddr);
    }

    if ((LnkSpeed == 0 && Gmac->Tx2ClkChRate != TX2_CLKCH_RATE_10MBPS * (Gmac->Tx2AddDiv2 ? 2 : 1)) ||
        (LnkSpeed == 1 && Gmac->Tx2ClkChRate != TX2_CLKCH_RATE_100MBPS) ||
        (LnkSpeed == 2 && Gmac->Tx2ClkChRate != TX2_CLKCH_RATE_1000MBPS)) {
      if (LnkSpeed == 0) {
        Gmac->Tx2ClkChRate = TX2_CLKCH_RATE_10MBPS * (Gmac->Tx2AddDiv2 ? 2 : 1);
      } else if (LnkSpeed == 1) {
        Gmac->Tx2ClkChRate = TX2_CLKCH_RATE_100MBPS;
      } else {
        Gmac->Tx2ClkChRate = TX2_CLKCH_RATE_1000MBPS;
      }

      if (Gmac->Tx2AddDiv2) {
        ARM_SMC_ARGS  ArmSmcArgs;

        if (LnkSpeed == 0) {
          ArmSmcArgs.Arg0 = BAIKAL_SMC_GMAC_DIV2_ENABLE;
        } else {
          ArmSmcArgs.Arg0 = BAIKAL_SMC_GMAC_DIV2_DISABLE;
        }

        ArmSmcArgs.Arg1 = (EFI_PHYSICAL_ADDRESS) Gmac->Regs;
        ArmCallSmc (&ArmSmcArgs);
      }

      CmuClkChSetRate (Gmac->Tx2ClkChCtlAddr, Gmac->Tx2ClkChRate);
    }
  }

  if (LnkSpeed == 0) {
    Gmac->Regs->MacConfig = (Gmac->Regs->MacConfig & ~MAC_CONFIG_FES) | MAC_CONFIG_PS;
  } else if (LnkSpeed == 1) {
    Gmac->Regs->MacConfig |=   MAC_CONFIG_PS | MAC_CONFIG_FES;
  } else if (LnkSpeed == 2) {
    Gmac->Regs->MacConfig &= ~(MAC_CONFIG_PS | MAC_CONFIG_FES);
  }

  if (TxBuf != NULL) {
    if (Gmac->TxDescReleaseIdx != Gmac->TxDescWriteIdx) {
      *((UINT32 *) TxBuf) = Gmac->TxBufPtrs[Gmac->TxDescReleaseIdx];
      Gmac->TxDescReleaseIdx = (Gmac->TxDescReleaseIdx + 1) % TX_DESC_NUM;
    } else {
      *TxBuf = NULL;
    }
  }

  if (InterruptStatus) {
    *InterruptStatus =
      (Gmac->Regs->DmaStatus & DMA_STATUS_RI ? EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT  : 0) |
      (Gmac->Regs->DmaStatus & DMA_STATUS_TI ? EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT : 0);

    Gmac->Regs->DmaStatus =
      (*InterruptStatus & EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT  ? DMA_STATUS_RI : 0) |
      (*InterruptStatus & EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT ? DMA_STATUS_TI : 0);
  }

  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;
}

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
  IN   CONST BOOLEAN                 RgmiiRxId,
  IN   CONST BOOLEAN                 RgmiiTxId,
  IN   EFI_MAC_ADDRESS              *MacAddr,
  OUT  VOID                        **Snp,
  OUT  EFI_HANDLE                  **Handle
  )
{
  GMAC_INSTANCE         *Gmac;
  EFI_PHYSICAL_ADDRESS   GmacBufsAddr;
  EFI_STATUS             Status;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (GMAC_INSTANCE),
                  (VOID **) &Gmac
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "Gmac(%p)SnpInstanceConstructor: unable to allocate GMAC_INSTANCE, Status = %r\n",
      GmacRegs,
      Status
      ));
    return Status;
  }

  GmacBufsAddr = (EFI_PHYSICAL_ADDRESS) (BASE_4GB - 1);
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiBootServicesData,
                  EFI_SIZE_TO_PAGES (sizeof (GMAC_DMA_BUFFERS)),
                  &GmacBufsAddr
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "Gmac(%p)SnpInstanceConstructor: unable to allocate GMAC_DMA_BUFFERS, Status = %r\n",
      GmacRegs,
      Status
      ));
    return Status;
  }

  if (!DmaCoherent) {
    EFI_CPU_ARCH_PROTOCOL  *Cpu;
    Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **) &Cpu);
    if (EFI_ERROR (Status)) {
      DEBUG ((
        EFI_D_ERROR,
        "Gmac(%p)SnpInstanceConstructor: unable to locate CpuArchProtocol, Status: %r\n",
        GmacRegs,
        Status
        ));
      return Status;
    }

    Status = Cpu->SetMemoryAttributes (
                    Cpu,
                    GmacBufsAddr,
                    ALIGN_VALUE (sizeof (GMAC_DMA_BUFFERS), EFI_PAGE_SIZE),
                    EFI_MEMORY_WC | EFI_MEMORY_XP
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        EFI_D_ERROR,
        "Gmac(%p)SnpInstanceConstructor: unable to set memory attributes, Status: %r\n",
        GmacRegs,
        Status
        ));
      return Status;
    }
  }

  Gmac->Handle           = NULL;
  Gmac->Regs             = GmacRegs;
  Gmac->Dma              = (VOID *) GmacBufsAddr;
  Gmac->RxDescReadIdx    = 0;
  Gmac->TxDescWriteIdx   = 0;
  Gmac->TxDescReleaseIdx = 0;
  Gmac->ResetGpioBase    = ResetGpioBase;
  Gmac->ResetGpioPin     = ResetGpioPin;
  Gmac->ResetPolarity    = ResetPolarity;
  Gmac->Tx2ClkChCtlAddr  = Tx2ClkChCtlAddr;
  Gmac->Tx2ClkChRate     = -1;
  Gmac->Tx2AddDiv2       = Tx2AddDiv2;
  Gmac->ClkCsr           = ClkCsr;
  Gmac->RgmiiRxId        = RgmiiRxId;
  Gmac->RgmiiTxId        = RgmiiTxId;
  Gmac->PhyAddr          = PhyAddr;

  Gmac->Snp.Revision       = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
  Gmac->Snp.Start          = GmacSnpStart;
  Gmac->Snp.Stop           = GmacSnpStop;
  Gmac->Snp.Initialize     = GmacSnpInitialize;
  Gmac->Snp.Reset          = GmacSnpReset;
  Gmac->Snp.Shutdown       = GmacSnpShutdown;
  Gmac->Snp.ReceiveFilters = GmacSnpReceiveFilters;
  Gmac->Snp.StationAddress = GmacSnpStationAddress;
  Gmac->Snp.Statistics     = GmacSnpStatistics;
  Gmac->Snp.MCastIpToMac   = GmacSnpMCastIpToMac;
  Gmac->Snp.NvData         = GmacSnpNvData;
  Gmac->Snp.GetStatus      = GmacSnpGetStatus;
  Gmac->Snp.Transmit       = GmacSnpTransmit;
  Gmac->Snp.Receive        = GmacSnpReceive;
  Gmac->Snp.WaitForPacket  = NULL;
  Gmac->Snp.Mode           = &Gmac->SnpMode;

  Gmac->SnpMode.State                 = EfiSimpleNetworkStopped;
  Gmac->SnpMode.HwAddressSize         = NET_ETHER_ADDR_LEN;
  Gmac->SnpMode.MediaHeaderSize       = sizeof (ETHER_HEAD);
  Gmac->SnpMode.MaxPacketSize         = 1500;
  Gmac->SnpMode.NvRamSize             = 0;
  Gmac->SnpMode.NvRamAccessSize       = 0;
  Gmac->SnpMode.ReceiveFilterMask     = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST     |
                                        EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST   |
                                        EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS |
                                        EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;

  Gmac->SnpMode.ReceiveFilterSetting  = GmacSnpGetReceiveFilterSetting (&Gmac->Snp);
  Gmac->SnpMode.MaxMCastFilterCount   = MAX_MCAST_FILTER_CNT;
  Gmac->SnpMode.MCastFilterCount      = 0;
  Gmac->SnpMode.IfType                = NET_IFTYPE_ETHERNET;
  Gmac->SnpMode.MacAddressChangeable  = TRUE;
  Gmac->SnpMode.MultipleTxSupported   = FALSE;
  Gmac->SnpMode.MediaPresentSupported = TRUE;
  Gmac->SnpMode.MediaPresent          = FALSE;

  gBS->CopyMem (&Gmac->SnpMode.CurrentAddress,   MacAddr, sizeof (EFI_MAC_ADDRESS));
  gBS->CopyMem (&Gmac->SnpMode.PermanentAddress, MacAddr, sizeof (EFI_MAC_ADDRESS));
  gBS->SetMem (&Gmac->SnpMode.MCastFilter, MAX_MCAST_FILTER_CNT * sizeof (EFI_MAC_ADDRESS), 0);
  gBS->SetMem (&Gmac->SnpMode.BroadcastAddress, sizeof (EFI_MAC_ADDRESS), 0xFF);

  *Handle = &Gmac->Handle;
  *Snp    = &Gmac->Snp;

  Status = gBS->CreateEvent (
                  EVT_SIGNAL_EXIT_BOOT_SERVICES,
                  TPL_NOTIFY,
                  GmacSnpExitBootServices,
                  Gmac,
                  &Gmac->ExitBootServicesEvent
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
GmacSnpInstanceDestructor (
  IN  VOID  *Snp
  )
{
  GMAC_INSTANCE * CONST  Gmac = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  EFI_STATUS             Status;

  Status = gBS->FreePages ((EFI_PHYSICAL_ADDRESS) Gmac->Dma, EFI_SIZE_TO_PAGES (sizeof (GMAC_DMA_BUFFERS)));
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "Gmac(%p)SnpInstanceDestructor: unable to free GMAC_DMA_BUFFERS, Status = %r\n",
      Gmac->Regs,
      Status
      ));
    return Status;
  }

  Status = gBS->FreePool (Gmac);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to free GMAC_INSTANCE, Status = %r\n", __FUNCTION__, Status));
    return Status;
  }

  return Status;
}

STATIC
BOOLEAN
GmacPollRegister (
  IN  volatile UINT32  *Reg,
  IN  UINT32           *Val,
  IN  UINTN             Delay,
  IN  UINTN             Timeout
  )
{
  UINTN Time = 0;
  for (;;) {
    MicroSecondDelay (Delay);
    Time += Delay;
    *Val = *Reg;

    if (!(*Val & MII_BUSY)) {
      return FALSE;
    }

    if (Time >= Timeout) {
      return TRUE;
    }
  }
}

STATIC
EFI_STATUS
GmacPhyRegRead (
  IN   volatile GMAC_REGS  *GmacRegs,
  IN   UINTN                ClkCsr,
  IN   INTN                 PhyAddr,
  IN   UINTN                PhyReg,
  OUT  UINT16              *PhyData
  )
{
  UINT32  Data;

  ASSERT (PhyAddr <= 0x1F);
  ASSERT (PhyReg  <= 0x1F);
  ASSERT (ClkCsr  <= 5 || (ClkCsr >= 8 && ClkCsr <= 15));

  if (GmacPollRegister (&GmacRegs->MacMiiAddr, &Data, 100, 10000)) {
    return EFI_TIMEOUT;
  }

  GmacRegs->MacMiiAddr = (PhyAddr << MII_ADDR_SHIFT)    |
                         (PhyReg  << MII_REG_SHIFT)     |
                         (ClkCsr  << MII_CLK_CSR_SHIFT) |
                         MII_BUSY;

  if (GmacPollRegister (&GmacRegs->MacMiiAddr, &Data, 100, 10000)) {
    return EFI_TIMEOUT;
  }

  *PhyData = (UINT16) GmacRegs->MacMiiData;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GmacPhyRegWrite (
  IN  volatile GMAC_REGS  *GmacRegs,
  IN  UINTN                ClkCsr,
  IN  INTN                 PhyAddr,
  IN  UINTN                PhyReg,
  IN  UINT16               PhyData
  )
{
  UINT32 Data;

  ASSERT (PhyAddr <= 0x1F);
  ASSERT (PhyReg  <= 0x1F);
  ASSERT (ClkCsr  <= 5 || (ClkCsr >= 8 && ClkCsr <= 15));

  if (GmacPollRegister (&GmacRegs->MacMiiAddr, &Data, 100, 10000)) {
    return EFI_TIMEOUT;
  }

  GmacRegs->MacMiiData = PhyData;
  GmacRegs->MacMiiAddr = (PhyAddr << MII_ADDR_SHIFT)    |
                         (PhyReg  << MII_REG_SHIFT)     |
                         (ClkCsr  << MII_CLK_CSR_SHIFT) |
                         MII_WRITE | MII_BUSY;

  if (GmacPollRegister (&GmacRegs->MacMiiAddr, &Data, 100, 10000)) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

STATIC
VOID
GmacConfigurePhyMarvell88E1510 (
  IN  GMAC_INSTANCE * CONST  Gmac
  )
{
  UINT16  Reg;
  UINT16  OldReg;

  // Configure Tx and Rx delays
  GmacPhyRegWrite (Gmac->Regs, Gmac->ClkCsr, Gmac->PhyAddr, MII_MARVELL_PHY_PAGE, MII_MARVELL_MSCR_PAGE);
  GmacPhyRegRead (Gmac->Regs, Gmac->ClkCsr, Gmac->PhyAddr, MII_88E1121_PHY_MSCR_REG, &Reg);
  OldReg = Reg;
  Reg &= ~MII_88E1121_PHY_MSCR_DELAY_MASK;

  if (Gmac->RgmiiRxId) {
    Reg |= MII_88E1121_PHY_MSCR_RX_DELAY;
  }

  if (Gmac->RgmiiTxId) {
    Reg |= MII_88E1121_PHY_MSCR_TX_DELAY;
  }

  // Set MSCR register
  GmacPhyRegWrite (Gmac->Regs, Gmac->ClkCsr, Gmac->PhyAddr, MII_88E1121_PHY_MSCR_REG, Reg);
  GmacPhyRegWrite (Gmac->Regs, Gmac->ClkCsr, Gmac->PhyAddr, MII_MARVELL_PHY_PAGE, MII_MARVELL_COPPER_PAGE);

  if (OldReg != Reg) {
    UINTN  Retries;

    // Issue PHY software reset
    GmacPhyRegRead (Gmac->Regs, Gmac->ClkCsr, Gmac->PhyAddr, MII_BMCR, &Reg);
    Reg &= ~BMCR_ISOLATE;
    Reg |= BMCR_RESET | BMCR_ANRESTART;
    GmacPhyRegWrite (Gmac->Regs, Gmac->ClkCsr, Gmac->PhyAddr, MII_BMCR, Reg);

    Retries = 12;
    do {
      MicroSecondDelay (50000);
      GmacPhyRegRead (Gmac->Regs, Gmac->ClkCsr, Gmac->PhyAddr, MII_BMCR, &Reg);
    } while (Reg & BMCR_RESET && --Retries);
  }
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
  GMAC_INSTANCE * CONST  Gmac = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  UINTN                  DescIdx;
  UINTN                  Limit;
  EFI_TPL                SavedTpl;
  PHY_ID                 PhyId;

  ASSERT (Snp != NULL);

  if (ExtraRxBufSize != 0 || ExtraTxBufSize != 0) {
    return EFI_UNSUPPORTED;
  }

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkStarted:
    break;

  case EfiSimpleNetworkInitialized:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpInitialize: SNP already initialized\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_SUCCESS;

  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpInitialize: SNP not started\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpInitialize: SNP invalid state = %u\n", Gmac->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (Gmac->ResetGpioBase &&
      Gmac->ResetGpioBase != (EFI_PHYSICAL_ADDRESS) &Gmac->Regs->MacGpio &&
      Gmac->ResetGpioPin >= 0  &&
      Gmac->ResetGpioPin <= 31 &&
      Gmac->ResetPolarity >= 0) {
    if (Gmac->ResetPolarity) {
      GpioOutRst (Gmac->ResetGpioBase, Gmac->ResetGpioPin);
    } else {
      GpioOutSet (Gmac->ResetGpioBase, Gmac->ResetGpioPin);
    }

    GpioDirSet (Gmac->ResetGpioBase, Gmac->ResetGpioPin);
  }

  Gmac->Regs->DmaBusMode = DMA_BUSMODE_SWR;
  gBS->Stall (100);

  if (Gmac->ResetGpioBase      &&
      Gmac->ResetGpioBase != (EFI_PHYSICAL_ADDRESS) &Gmac->Regs->MacGpio &&
      Gmac->ResetGpioPin >= 0  &&
      Gmac->ResetGpioPin <= 31 &&
      Gmac->ResetPolarity >= 0) {
    if (Gmac->ResetPolarity) {
      GpioOutSet (Gmac->ResetGpioBase, Gmac->ResetGpioPin);
    } else {
      GpioOutRst (Gmac->ResetGpioBase, Gmac->ResetGpioPin);
    }
  }

  for (Limit = 3000; Limit; --Limit) {
    if (Gmac->ResetGpioBase == (EFI_PHYSICAL_ADDRESS) &Gmac->Regs->MacGpio &&
        Gmac->ResetGpioPin == 0) {
      Gmac->Regs->MacGpio |= MAC_GPIO_GPO;
    }

    if (!(Gmac->Regs->DmaBusMode & DMA_BUSMODE_SWR)) {
      break;
    }

    gBS->Stall (1000);
  }

  if (!Limit) {
    DEBUG ((EFI_D_ERROR, "Gmac(%p)SnpInitialize: GMAC reset not completed\n", Gmac->Regs));
    return EFI_DEVICE_ERROR;
  }

  for (Limit = 3000; Limit; --Limit) {
    if (!(Gmac->Regs->DmaAxiStatus & (DMA_AXISTATUS_AXIRDSTS | DMA_AXISTATUS_AXIWHSTS))) {
      break;
    }

    gBS->Stall (1000);
  }

  GmacPhyRegRead (Gmac->Regs, Gmac->ClkCsr, Gmac->PhyAddr, MII_PHY_ID2, &PhyId.Regs[0]);
  GmacPhyRegRead (Gmac->Regs, Gmac->ClkCsr, Gmac->PhyAddr, MII_PHY_ID1, &PhyId.Regs[1]);
  PhyId.PhyId &= MARVELL_PHY_ID_MASK;
  switch (PhyId.PhyId) {
  case MARVELL_PHY_ID_88E1510:
    GmacConfigurePhyMarvell88E1510 (Gmac);
    break;
    // TODO handle other types of PHYs
  }

  Gmac->Regs->MacAddr0Hi = Snp->Mode->CurrentAddress.Addr[4] |
                           Snp->Mode->CurrentAddress.Addr[5] << 8;

  Gmac->Regs->MacAddr0Lo = Snp->Mode->CurrentAddress.Addr[0]       |
                           Snp->Mode->CurrentAddress.Addr[1] << 8  |
                           Snp->Mode->CurrentAddress.Addr[2] << 16 |
                           Snp->Mode->CurrentAddress.Addr[3] << 24;

  Gmac->Regs->DmaBusMode        |= DMA_BUSMODE_ATDS;
  Gmac->Regs->DmaRxDescBaseAddr  = (EFI_PHYSICAL_ADDRESS) Gmac->Dma->RxDescs;
  Gmac->Regs->DmaTxDescBaseAddr  = (EFI_PHYSICAL_ADDRESS) Gmac->Dma->TxDescs;

  Gmac->RxDescReadIdx    = 0;
  Gmac->TxDescWriteIdx   = 0;
  Gmac->TxDescReleaseIdx = 0;

  for (DescIdx = 0; DescIdx < RX_DESC_NUM; ++DescIdx) {
    Gmac->Dma->RxDescs[DescIdx].Rdes0 = RDES0_OWN;
    Gmac->Dma->RxDescs[DescIdx].Rdes1 = RDES1_RCH | (RX_BUF_SIZE << RDES1_RBS1_POS);
    Gmac->Dma->RxDescs[DescIdx].Rdes2 = (EFI_PHYSICAL_ADDRESS) Gmac->Dma->RxBufs[DescIdx];
    Gmac->Dma->RxDescs[DescIdx].Rdes3 = (EFI_PHYSICAL_ADDRESS) &Gmac->Dma->RxDescs[(DescIdx + 1) % RX_DESC_NUM];
  }

  for (DescIdx = 0; DescIdx < TX_DESC_NUM; ++DescIdx) {
    Gmac->Dma->TxDescs[DescIdx].Tdes0 = 0;
    Gmac->Dma->TxDescs[DescIdx].Tdes3 = (EFI_PHYSICAL_ADDRESS) &Gmac->Dma->TxDescs[(DescIdx + 1) % TX_DESC_NUM];
  }

  Gmac->Regs->DmaStatus = 0xFFFFFFFF;
  Gmac->Regs->MacConfig = (3 << MAC_CONFIG_SARC_POS) |
                                MAC_CONFIG_DCRS      |
                                MAC_CONFIG_DO        |
                                MAC_CONFIG_DM        |
                                MAC_CONFIG_IPC       |
                                MAC_CONFIG_ACS;

  // Wait for linkup if the link has already been established
  if (Snp->Mode->MediaPresent) {
    UINTN  Limit;

    // TODO: it may require to adjust Tx2ClkChRate, so the flow from SnpGetStatus() should be re-used
    for (Limit = 4000; ; --Limit) {
      if (Gmac->Regs->MacMiiStatus & MAC_MIISTATUS_LNKSTS) {
        break;
      } else if (!Limit) {
        Snp->Mode->MediaPresent = FALSE;
        break;
      }

      gBS->Stall (1000);
    }
  }

  ArmDataSynchronizationBarrier ();
  Gmac->Regs->DmaOperationMode = DMA_OPERATIONMODE_TSF |
                                 DMA_OPERATIONMODE_RSF |
                                 DMA_OPERATIONMODE_ST  |
                                 DMA_OPERATIONMODE_SR;

  Gmac->Regs->MacConfig       |= MAC_CONFIG_TE | MAC_CONFIG_RE;
  Gmac->Regs->DmaRxPollDemand  = 0;

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
  GMAC_INSTANCE * CONST  Gmac = Context;
  Gmac->Regs->DmaBusMode = DMA_BUSMODE_SWR;

  if (Gmac->ResetGpioBase &&
      Gmac->ResetGpioBase != (EFI_PHYSICAL_ADDRESS) &Gmac->Regs->MacGpio &&
      Gmac->ResetGpioPin >= 0  &&
      Gmac->ResetGpioPin <= 31) {
    GpioDirClr (Gmac->ResetGpioBase, Gmac->ResetGpioPin);
  }

  Gmac->Snp.Mode->MediaPresent = FALSE;
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
  GMAC_INSTANCE * CONST  Gmac = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  CONST UINT32           ResultingMsk = Enable & ~Disable;
  EFI_TPL                SavedTpl;

  ASSERT (Snp != NULL);

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceiveFilters: SNP not initialized\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;

  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceiveFilters: SNP not started\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceiveFilters: SNP invalid state = %u\n", Gmac->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (ResultingMsk & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST) {
    Gmac->Regs->MacFrameFilter &= ~MAC_FRAMEFILTER_DBF;
  } else {
    Gmac->Regs->MacFrameFilter |=  MAC_FRAMEFILTER_DBF;
  }

  if (ResultingMsk & (EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST |
                      EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST)) {
    Gmac->Regs->MacFrameFilter |=  MAC_FRAMEFILTER_PM;
  } else {
    Gmac->Regs->MacFrameFilter &= ~MAC_FRAMEFILTER_PM;
  }

  if (ResultingMsk & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS) {
    Gmac->Regs->MacFrameFilter |=  MAC_FRAMEFILTER_PR;
  } else {
    Gmac->Regs->MacFrameFilter &= ~MAC_FRAMEFILTER_PR;
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
  GMAC_INSTANCE * CONST  Gmac = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  EFI_TPL                SavedTpl;

  ASSERT (Snp != NULL);

  DEBUG ((EFI_D_NET, "Gmac(%p)SnpStationAddress()\n", Gmac->Regs));
  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpStationAddress: SNP not initialized\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;

  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpStationAddress: SNP not started\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpStationAddress: SNP invalid state = %u\n", Gmac->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (Reset) {
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpStationAddress: reset MAC address\n", Gmac->Regs));
    Snp->Mode->CurrentAddress = Snp->Mode->PermanentAddress;
  } else {
    if (NewMacAddr == NULL) {
      gBS->RestoreTPL (SavedTpl);
      return EFI_INVALID_PARAMETER;
    }

    gBS->CopyMem (&Snp->Mode->CurrentAddress, NewMacAddr, sizeof (EFI_MAC_ADDRESS));
  }

  Gmac->Regs->MacAddr0Hi = Snp->Mode->CurrentAddress.Addr[4] |
                           Snp->Mode->CurrentAddress.Addr[5] << 8;

  Gmac->Regs->MacAddr0Lo = Snp->Mode->CurrentAddress.Addr[0]       |
                           Snp->Mode->CurrentAddress.Addr[1] << 8  |
                           Snp->Mode->CurrentAddress.Addr[2] << 16 |
                           Snp->Mode->CurrentAddress.Addr[3] << 24;

  DEBUG ((
    EFI_D_NET,
    "Gmac(%p)SnpStationAddress: current MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
    Gmac->Regs,
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
  GMAC_INSTANCE * CONST  Gmac = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  EFI_TPL                SavedTpl;

  ASSERT (Snp != NULL);

  if (BufSize == NULL || Buf == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceive: SNP not initialized\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;

  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceive: SNP not started\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceive: SNP invalid state = %u\n", Gmac->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  EFI_STATUS Status = EFI_NOT_READY;

  while (!(Gmac->Dma->RxDescs[Gmac->RxDescReadIdx].Rdes0 & RDES0_OWN) && (Status == EFI_NOT_READY)) {
    if (((RDES0_FS | RDES0_LS) & Gmac->Dma->RxDescs[Gmac->RxDescReadIdx].Rdes0) ==
         (RDES0_FS | RDES0_LS)) {
      CONST UINTN  FrameLen = (Gmac->Dma->RxDescs[Gmac->RxDescReadIdx].Rdes0 >> RDES0_FL_POS) & RDES0_FL_MSK;

      if (*BufSize < FrameLen) {
        DEBUG ((EFI_D_NET, "Gmac(%p)SnpReceive: receive BufSize(%u) < FrameLen(%u)\n", Gmac->Regs, *BufSize, FrameLen));
        Status = EFI_BUFFER_TOO_SMALL;
      }

      *BufSize = FrameLen;

      if (Status != EFI_BUFFER_TOO_SMALL) {
        gBS->CopyMem (Buf, (VOID *) Gmac->Dma->RxBufs[Gmac->RxDescReadIdx], FrameLen);

        if (HdrSize != NULL) {
          *HdrSize = Snp->Mode->MediaHeaderSize;
        }

        if (DstAddr != NULL) {
          gBS->CopyMem (DstAddr, Buf, NET_ETHER_ADDR_LEN);
        }

        if (SrcAddr != NULL) {
          gBS->CopyMem (SrcAddr, (UINT8 *) Buf + NET_ETHER_ADDR_LEN, NET_ETHER_ADDR_LEN);
        }

        if (Protocol != NULL) {
          *Protocol = NTOHS (*(UINT16 *)((UINT8 *) Buf + 2 * NET_ETHER_ADDR_LEN));
        }

        Status = EFI_SUCCESS;
      }
    }

    Gmac->Dma->RxDescs[Gmac->RxDescReadIdx].Rdes0 = RDES0_OWN;
    Gmac->RxDescReadIdx = (Gmac->RxDescReadIdx + 1) % RX_DESC_NUM;

    if (Gmac->Regs->DmaStatus & DMA_STATUS_RU) {
      ArmDataSynchronizationBarrier ();
      Gmac->Regs->DmaStatus   = DMA_STATUS_RU;
      Gmac->Regs->DmaRxPollDemand = 0;
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
  GMAC_INSTANCE * CONST  Gmac = BASE_CR (Snp, GMAC_INSTANCE, Snp);
  EFI_TPL                SavedTpl;
  EFI_STATUS             Status;

  ASSERT (Snp != NULL);

  if (Buf == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SavedTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStarted:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpTransmit: SNP not initialized\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;

  case EfiSimpleNetworkStopped:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpTransmit: SNP not started\n", Gmac->Regs));
    gBS->RestoreTPL (SavedTpl);
    return EFI_NOT_STARTED;

  default:
    DEBUG ((EFI_D_NET, "Gmac(%p)SnpTransmit: SNP invalid state = %u\n", Gmac->Regs, Snp->Mode->State));
    gBS->RestoreTPL (SavedTpl);
    return EFI_DEVICE_ERROR;
  }

  if (HdrSize != 0) {
    if (HdrSize != Snp->Mode->MediaHeaderSize) {
      DEBUG ((
        EFI_D_NET,
        "Gmac(%p)SnpTransmit: HdrSize(%u) != Snp->Mode->MediaHeaderSize(%u)\n",
        Gmac->Regs,
        HdrSize,
        Snp->Mode->MediaHeaderSize
        ));
      gBS->RestoreTPL (SavedTpl);
      return EFI_INVALID_PARAMETER;
    }

    if (DstAddr == NULL || Protocol == NULL) {
      DEBUG ((
        EFI_D_NET,
        "Gmac(%p)SnpTransmit: Hdr DstAddr(%p) or Protocol(%p) is NULL\n",
        Gmac->Regs,
        DstAddr,
        Protocol
        ));
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

    gBS->CopyMem ((UINT8 *) Buf + 2 * NET_ETHER_ADDR_LEN, &EtherType, 2);
  }

  if (!(Gmac->Dma->TxDescs[Gmac->TxDescWriteIdx].Tdes0 & TDES0_OWN) &&
      ((Gmac->TxDescWriteIdx + 1) % TX_DESC_NUM) != Gmac->TxDescWriteIdx) {
    // Store the Buf address in order to release it later
    Gmac->TxBufPtrs[Gmac->TxDescWriteIdx] = (EFI_PHYSICAL_ADDRESS) Buf;
    // Buf address could be higher than BASE_4GB, so copy the Buf data to Gmac->TxBuf
    gBS->CopyMem ((UINT8 *) Gmac->Dma->TxBufs[Gmac->TxDescWriteIdx], Buf, BufSize);
    Gmac->Dma->TxDescs[Gmac->TxDescWriteIdx].Tdes2 = (EFI_PHYSICAL_ADDRESS) Gmac->Dma->TxBufs[Gmac->TxDescWriteIdx];
    Gmac->Dma->TxDescs[Gmac->TxDescWriteIdx].Tdes1 = BufSize << TDES1_TBS1_POS;
    ArmDataSynchronizationBarrier ();
    Gmac->Dma->TxDescs[Gmac->TxDescWriteIdx].Tdes0 = TDES0_OWN | TDES0_IC | TDES0_LS | TDES0_FS | TDES0_TCH;
    ArmDataSynchronizationBarrier ();
    Gmac->TxDescWriteIdx = (Gmac->TxDescWriteIdx + 1) % TX_DESC_NUM;
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NOT_READY;
  }

  Gmac->Regs->DmaOperationMode |= DMA_OPERATIONMODE_ST;
  Gmac->Regs->MacConfig        |= MAC_CONFIG_TE;
  Gmac->Regs->DmaTxPollDemand   = 0;
  gBS->RestoreTPL (SavedTpl);
  return Status;
}
