/** BaikalI2cDxe.c
  I2C controller driver APIs for read, write, initialize, set speed and reset

  Copyright 2020 Baikal Electronics

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/I2cMaster.h>
#include <Protocol/NonDiscoverableDevice.h>

#include "BaikalI2cDxe.h"

#define I2C_SPEED_SM       100000
#define I2C_SPEED_FM       400000
#define I2C_SPEED_HM      1000000

#if !defined(I2C_FLAG_WRITE)
#define I2C_FLAG_WRITE 0
#endif

/* Speed Selection */
#define IC_SPEED_MODE_STANDARD  1
#define IC_SPEED_MODE_FAST      2
#define IC_SPEED_MODE_MAX       3

/* i2c enable register definitions */
#define IC_ENABLE_0B		0x0001
#define IC_STOP_DET		0x0200	/* (1<<9) R_STOP_DET [ic_raw_intr_stat] */
#define IC_TX_ABRT              0x0040  /* (1<<6) TX Abort [ic_raw_intr_stat] */
#define IC_CON_MM		0x0001	/* (1<<0) master enable [ic_con] */
#define IC_CON_SPD		0x0006	/* (3<<1) speed */
#define IC_CON_SPD_SS		0x0002	/* (1<<1) speed */
#define IC_CON_SPD_FS		0x0004	/* (2<<1) speed */
#define IC_CON_SPD_HS		0x0006	/* (3<<1) speed */
#define IC_CON_SD		0x0040	/* (1<<6) slave disable */
#define IC_CLK			100     /* 100MHz for Baikal-M */
#define IC_TL0			0x00
#define IC_RX_TL		IC_TL0
#define IC_TX_TL		IC_TL0
#define CONFIG_SYS_HZ		1200
#define NANO_TO_MICRO		1000
#define MIN_SS_SCL_HIGHTIME	4000
#define MIN_SS_SCL_LOWTIME	4700
#define MIN_FS_SCL_HIGHTIME	600
#define MIN_FS_SCL_LOWTIME	1300
#define MIN_HS_SCL_HIGHTIME	200
#define MIN_HS_SCL_LOWTIME	500

#define IRQ_MASK		0

#define IC_CMD			0x0100	/* (1<<8) read=1,write=0 */
#define IC_STOP			0x0200	/* (1<<9) ? */

#define BIT(n)			(1 << (n))

// ic_status
#define IC_STATUS_SA		BIT(6)
#define IC_STATUS_MA		BIT(5)
#define IC_STATUS_RFF		BIT(4)
#define IC_STATUS_RFNE		BIT(3)
#define IC_STATUS_TFE		BIT(2)
#define IC_STATUS_TFNF          BIT(1)
#define IC_STATUS_ACT	        BIT(0)

#define I2C_TIMEOUT             1000000

#define BAIKAL_I2C_SIGNATURE      SIGNATURE_32 ('B', 'I', '2', 'C')
#define BAIKAL_I2C_FROM_THIS(a)   CR ((a), BAIKAL_I2C_MASTER, \
                                    I2cMaster, BAIKAL_I2C_SIGNATURE)

typedef struct {
  unsigned int IcCon;          /* adr type [7,10 bit] */
  unsigned int IcTar;          /* target address */
  unsigned int IcSar;          /* self  address */
  unsigned int IcHsMaddr;
  unsigned int IcCmdData;      /* cmd[8] : data[7-0]  //cmd= 1-read,0-write */
  unsigned int IcSsSclHcnt;
  unsigned int IcSsSclLcnt;
  unsigned int IcFsSclHcnt;
  unsigned int IcFsSclLcnt;
  unsigned int IcHsSclHcnt;
  unsigned int IcHsSclLcnt;
  unsigned int IcIntrStat;
  unsigned int IcIntrMask;
  unsigned int IcRawIntrStat;
  unsigned int IcRxTl;
  unsigned int IcTxTl;
  unsigned int IcClrIntr;
  unsigned int IcClrRxUnder;
  unsigned int IcClrRxOver;
  unsigned int IcClrTxOver;
  unsigned int IcClrRdReq;
  unsigned int IcClrTxAbrt;
  unsigned int IcClrRxDone;
  unsigned int IcClrActivity;
  unsigned int IcClrStopDet;
  unsigned int IcClrStartDet;
  unsigned int IcClrGenCall;
  unsigned int IcEnable;
  unsigned int IcStatus;
  unsigned int IcTxFlr;
  unsigned int IcRxFlr;
  unsigned int IcSdaHold;
  unsigned int IcTxAbrtSource;
} I2C_REGS;

#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH              Vendor;
  UINT64                          MmioBase;
  EFI_DEVICE_PATH_PROTOCOL        End;
} BAIKAL_I2C_DEVICE_PATH;
#pragma pack()

typedef struct {
  UINT32                          Signature;
  EFI_I2C_MASTER_PROTOCOL         I2cMaster;
  BAIKAL_I2C_DEVICE_PATH          DevicePath;
  NON_DISCOVERABLE_DEVICE         *Dev;
  UINT32                          SpeedSel;
} BAIKAL_I2C_MASTER;

STATIC CONST EFI_I2C_CONTROLLER_CAPABILITIES mI2cControllerCapabilities = {
  0,
  0,
  0,
  0
};

/**
  Function to set I2C bus frequency

  @param   This            Pointer to I2C master protocol
  @param   BusClockHertz   value to be set

  @retval EFI_SUCCESS      Operation successfull
**/
STATIC
EFI_STATUS
EFIAPI
SetBusFrequency (
  IN CONST EFI_I2C_MASTER_PROTOCOL   *This,
  IN OUT UINTN                       *BusClockHertz
 )
{
  BAIKAL_I2C_MASTER        *I2c;

  I2c = BAIKAL_I2C_FROM_THIS (This);

  if (BusClockHertz == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*BusClockHertz <= I2C_SPEED_SM) {
    I2c->SpeedSel = IC_SPEED_MODE_STANDARD;
    *BusClockHertz = I2C_SPEED_SM;
  } else if (*BusClockHertz <= I2C_SPEED_FM) {
    I2c->SpeedSel = IC_SPEED_MODE_FAST;
    *BusClockHertz = I2C_SPEED_FM;
  } else {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Function used to disable I2C controller

  @param   I2cRegs        Pointer to I2C registers

**/
STATIC
VOID
I2cDisable (
  IN  I2C_REGS            *I2cRegs
  )
{
  UINT32 IcEnable;
  IcEnable = MmioRead32 ((UINTN)&I2cRegs->IcEnable);
  IcEnable &= ~IC_ENABLE_0B;
  MmioWrite32 ((UINTN)&I2cRegs->IcEnable, IcEnable);
}

/**
  Function used to enable I2C controller

  @param   I2cRegs        Pointer to I2C registers

**/
STATIC
VOID
I2cEnable (
  IN  I2C_REGS            *I2cRegs
  )
{
  UINT32 IcEnable;
  IcEnable = MmioRead32 ((UINTN)&I2cRegs->IcEnable);
  IcEnable |= IC_ENABLE_0B;
  MmioWrite32 ((UINTN)&I2cRegs->IcEnable, IcEnable);
}

/**
  Function used to initialise I2C controller registers

  @param   I2cRegs        Pointer to I2C registers

**/
STATIC
VOID
I2cInitHw (
  IN  I2C_REGS            *I2cRegs
  )
{
  I2cDisable(I2cRegs);
  MmioWrite32 ((UINTN)&I2cRegs->IcCon, IC_CON_SD | IC_CON_SPD_FS | IC_CON_MM);
  MmioWrite32 ((UINTN)&I2cRegs->IcSsSclHcnt,
               (IC_CLK * MIN_SS_SCL_HIGHTIME) / NANO_TO_MICRO);
  MmioWrite32 ((UINTN)&I2cRegs->IcSsSclLcnt,
               (IC_CLK * MIN_SS_SCL_LOWTIME) / NANO_TO_MICRO);
  MmioWrite32 ((UINTN)&I2cRegs->IcFsSclHcnt,
               (IC_CLK * MIN_FS_SCL_HIGHTIME) / NANO_TO_MICRO);
  MmioWrite32 ((UINTN)&I2cRegs->IcFsSclLcnt,
               (IC_CLK * MIN_FS_SCL_LOWTIME) / NANO_TO_MICRO);
  MmioWrite32 ((UINTN)&I2cRegs->IcIntrMask, IRQ_MASK);
  MmioWrite32 ((UINTN)&I2cRegs->IcRxTl, IC_RX_TL);
  MmioWrite32 ((UINTN)&I2cRegs->IcTxTl, IC_TX_TL);
}

/**
  Function used to check if I2C bus is ready for next transaction

  @param   I2cRegs        Pointer to I2C registers

  @retval  EFI_TIMEOUT    Timeout occured
  @retval  EFI_SUCCESS    Bus is ready

**/
STATIC
EFI_STATUS
WaitForBusReady (
  IN  I2C_REGS            *I2cRegs
  )
{
  UINT32                  Status;
  UINTN                   Count;

  for (Count = 0; Count < I2C_TIMEOUT; Count++) {
    Status = MmioRead32 ((UINTN)&I2cRegs->IcStatus);
    if (!(Status & IC_STATUS_MA)) {
      return EFI_SUCCESS;
    }
  }

  return EFI_TIMEOUT;
}

/**
  Function used to wait for I2C bus transfer to finish

  @param   I2cRegs        Pointer to I2C registers

  @retval  EFI_TIMEOUT    Timeout occured
  @retval  EFI_SUCCESS    Bus is ready

**/
STATIC
EFI_STATUS
I2cXferFinish (
  IN  I2C_REGS            *I2cRegs
  )
{
  EFI_STATUS              Status;
  UINT32                  RawIntrStat;
  UINTN                   Count;

  for (Count = 0; Count < I2C_TIMEOUT; Count++) {
    RawIntrStat = MmioRead32 ((UINTN)&I2cRegs->IcRawIntrStat);
    if (RawIntrStat & IC_STOP_DET) {
      MmioRead32 ((UINTN)&I2cRegs->IcClrStopDet);
      break;
    }
    if (RawIntrStat & IC_TX_ABRT) {
      DEBUG ((EFI_D_ERROR, "I2cXferFinish: TX_ABRT (%x)!\n",
              MmioRead32 ((UINTN)&I2cRegs->IcTxAbrtSource)));
      MmioRead32 ((UINTN)&I2cRegs->IcClrTxAbrt);
      return EFI_TIMEOUT;
    }
  }
  MmioRead32 ((UINTN)&I2cRegs->IcClrIntr);

  if (Count < I2C_TIMEOUT) {
    Status = WaitForBusReady (I2cRegs);
    return Status;
  }

  return EFI_TIMEOUT;
}

/**
  Function to read data using I2C bus

  @param   I2cRegs         Pointer to I2C registers
  @param   Size            Size of data to be read
  @param   Buffer          A pointer to the destination buffer for the data

  @retval  EFI_TIMEOUT     Arbitration lost or
                           ACK was not recieved or
                           read operation timed out
  @retval  EFI_SUCCESS     Read was successful

**/
STATIC
EFI_STATUS
I2cDataRead (
  IN  I2C_REGS             *I2cRegs,
  IN  UINT32               Size,
  IN  UINT8                *Buffer,
  IN  BOOLEAN              Last
  )
{
  EFI_STATUS               Status;
  UINTN                    Timeout;
  UINT32                   BytesRead;
  UINT32                   CmdsWritten;
  UINT32                   CmdData;
  UINT32                   IcStatus;
  
  if (Size > 0) {
    BytesRead = 0;
    CmdsWritten = 0;
    Timeout = 0;
    while ((Timeout < I2C_TIMEOUT) &&
           (BytesRead < Size)) {
      IcStatus = MmioRead32 ((UINTN)&I2cRegs->IcStatus);
      if (CmdsWritten < Size &&
          (IcStatus & IC_STATUS_TFNF)) {
        CmdData = IC_CMD;
        if (Last && CmdsWritten == (Size - 1)) {
          CmdData |= IC_STOP;
        }
        MmioWrite32 ((UINTN)&I2cRegs->IcCmdData, CmdData);
        CmdsWritten++;
      }
      if (IcStatus & IC_STATUS_RFNE) {
        Buffer[BytesRead] = MmioRead32 ((UINTN)&I2cRegs->IcCmdData);
        BytesRead++;
        Timeout = 0;
      } else {
        Timeout++;
        IcStatus = MmioRead32((UINTN)&I2cRegs->IcRawIntrStat);
        if (IcStatus & IC_TX_ABRT) {
          DEBUG ((EFI_D_ERROR, "I2cDataRead: TX_ABRT (%x)!\n",
                  MmioRead32 ((UINTN)&I2cRegs->IcTxAbrtSource)));
          MmioRead32 ((UINTN)&I2cRegs->IcClrTxAbrt);
          return EFI_TIMEOUT;
        }
      }
    }
    Status = (BytesRead < Size) ? EFI_TIMEOUT : EFI_SUCCESS;
    return Status;
  } else {
    return EFI_INVALID_PARAMETER;
  }
}

/**
  Function to write data using I2C bus

  @param   I2cRegs         Pointer to I2C registers
  @param   Size            Size of data to be written
  @param   Buffer          A pointer to the source buffer for the data

  @retval  EFI_TIMEOUT     Arbitration lost or
                           ACK was not recieved or
                           write operation timed out
  @retval  EFI_SUCCESS     Read was successful

**/
STATIC
EFI_STATUS
I2cDataWrite (
  IN  I2C_REGS             *I2cRegs,
  IN  INT32                Size,
  IN  UINT8                *Buffer,
  IN  BOOLEAN              Last
  )
{
  EFI_STATUS               Status;
  UINTN                    Timeout;
  UINT32                   BytesWritten;
  UINT32                   CmdData;
  UINT32                   IcStatus;
  
  if (Size > 0) {
    BytesWritten = 0;
    Timeout = 0;
    while ((Timeout < I2C_TIMEOUT) &&
           (BytesWritten < Size)) {
      IcStatus = MmioRead32 ((UINTN)&I2cRegs->IcStatus);
      if (IcStatus & IC_STATUS_TFNF) {
        CmdData = Buffer[BytesWritten];
        if (Last && BytesWritten == (Size - 1)) {
          CmdData |= IC_STOP;
        }
        MmioWrite32 ((UINTN)&I2cRegs->IcCmdData, CmdData);
        BytesWritten++;
        Timeout = 0;
      } else {
        Timeout++;
        IcStatus = MmioRead32((UINTN)&I2cRegs->IcRawIntrStat);
        if (IcStatus & IC_TX_ABRT) {
          DEBUG ((EFI_D_ERROR, "I2cDataWrite: TX_ABRT (%x)!\n",
                  MmioRead32 ((UINTN)&I2cRegs->IcTxAbrtSource)));
          MmioRead32 ((UINTN)&I2cRegs->IcClrTxAbrt);
          return EFI_TIMEOUT;
        }
      }
    }
    Status = (BytesWritten < Size) ? EFI_TIMEOUT : EFI_SUCCESS;
    return Status;
  } else {
    return EFI_INVALID_PARAMETER;
  }
}

/**
  Function to reset I2C Controller

  @param  This             Pointer to I2C master protocol

  @return EFI_SUCCESS      Operation successfull
**/
STATIC
EFI_STATUS
EFIAPI
Reset (
  IN CONST EFI_I2C_MASTER_PROTOCOL *This
  )
{
  I2C_REGS                         *I2cRegs;
  BAIKAL_I2C_MASTER                *I2c;

  I2c = BAIKAL_I2C_FROM_THIS (This);
  I2cRegs = (I2C_REGS *)(I2c->Dev->Resources[0].AddrRangeMin);
  I2cInitHw (I2cRegs);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
StartRequest (
  IN CONST EFI_I2C_MASTER_PROTOCOL *This,
  IN UINTN                         SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET        *RequestPacket,
  IN EFI_EVENT                     Event            OPTIONAL,
  OUT EFI_STATUS                   *I2cStatus       OPTIONAL
  )
{
  BAIKAL_I2C_MASTER                *I2c;
  I2C_REGS                         *I2cRegs;
  UINT32                           Count;
  EFI_STATUS                       Status;
  UINT32                           Size;
  UINT8                            *Buffer;
  UINT32                           Flag;
  UINT32                           Reg;

  I2c = BAIKAL_I2C_FROM_THIS (This);
  I2cRegs = (I2C_REGS *)(I2c->Dev->Resources[0].AddrRangeMin);

  if (!RequestPacket || RequestPacket->OperationCount == 0) {
    DEBUG ((EFI_D_ERROR,"%a: Operation count is not valid %u\n",
           __FUNCTION__, RequestPacket->OperationCount));
    return EFI_INVALID_PARAMETER;
  }

  MmioWrite32 ((UINTN)&I2cRegs->IcTar, SlaveAddress);
  Reg = MmioRead32 ((UINTN)&I2cRegs->IcCon);
  Reg &= ~IC_CON_SPD;
  if (I2c->SpeedSel == IC_SPEED_MODE_FAST) {
    Reg |= IC_CON_SPD_FS;
  } else {
    Reg |= IC_CON_SPD_SS;
  }
  MmioWrite32 ((UINTN)&I2cRegs->IcCon, Reg);
  I2cEnable (I2cRegs);

  Status = EFI_SUCCESS;
  for (Count = 0;
       (Count < RequestPacket->OperationCount) &&
       !EFI_ERROR(Status);
       Count++) {
    Status = WaitForBusReady (I2cRegs);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Flag = RequestPacket->Operation[Count].Flags;
    Size = RequestPacket->Operation[Count].LengthInBytes;
    Buffer = RequestPacket->Operation[Count].Buffer;

    if (Flag == I2C_FLAG_READ) {
      Status = I2cDataRead (I2cRegs, Size, Buffer, Count == (RequestPacket->OperationCount - 1));
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR,"%a: I2c read operation failed (error %d)\n",
               __FUNCTION__, Status));
      }
    } else if (Flag == I2C_FLAG_WRITE) {
      Status = I2cDataWrite (I2cRegs, Size, Buffer, Count == (RequestPacket->OperationCount - 1));
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR,"%a: I2c write operation failed (error %d)\n",
               __FUNCTION__, Status));
      }
    } else {
      DEBUG ((EFI_D_ERROR,"%a: Invalid Flag %08X\n", __FUNCTION__, Flag));
      Status = EFI_INVALID_PARAMETER;
    }
  }

  Status |= I2cXferFinish (I2cRegs);
  I2cDisable(I2cRegs);
  DEBUG ((EFI_D_INFO, "StartRequest: Status %d\n", Status));

  if (I2cStatus != NULL) {
    *I2cStatus = Status;
  }
  if (Event != NULL) {
    gBS->SignalEvent(Event);
  }
  return Status;
}

EFI_STATUS
BaikalI2cInit (
  IN EFI_HANDLE             DriverBindingHandle,
  IN EFI_HANDLE             ControllerHandle
  )
{
  EFI_STATUS                RetVal;
  NON_DISCOVERABLE_DEVICE   *Dev;
  BAIKAL_I2C_MASTER         *I2c;

  RetVal = gBS->OpenProtocol (ControllerHandle,
                              &gEdkiiNonDiscoverableDeviceProtocolGuid,
                              (VOID **)&Dev, DriverBindingHandle,
                              ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR (RetVal)) {
    return RetVal;
  }

  I2c = AllocateZeroPool (sizeof (BAIKAL_I2C_MASTER));
  if (I2c == NULL) {
    gBS->CloseProtocol (ControllerHandle,
                        &gEdkiiNonDiscoverableDeviceProtocolGuid,
                        DriverBindingHandle,
                        ControllerHandle);
    return EFI_OUT_OF_RESOURCES;
  }

  I2c->Signature                            = BAIKAL_I2C_SIGNATURE;
  I2c->I2cMaster.SetBusFrequency            = SetBusFrequency;
  I2c->I2cMaster.Reset                      = Reset;
  I2c->I2cMaster.StartRequest               = StartRequest;
  I2c->I2cMaster.I2cControllerCapabilities  = &mI2cControllerCapabilities;
  I2c->Dev                                  = Dev;
  I2c->SpeedSel                             = IC_SPEED_MODE_FAST;

  I2c->DevicePath.Vendor.Header.Type = HARDWARE_DEVICE_PATH;
  I2c->DevicePath.Vendor.Header.SubType = HW_VENDOR_DP;
  CopyGuid (&I2c->DevicePath.Vendor.Guid, &gEfiCallerIdGuid);
  I2c->DevicePath.MmioBase = I2c->Dev->Resources[0].AddrRangeMin;
  SetDevicePathNodeLength (&I2c->DevicePath.Vendor,
    sizeof (I2c->DevicePath) - sizeof (I2c->DevicePath.End));
  SetDevicePathEndNode (&I2c->DevicePath.End);

  I2cInitHw ((I2C_REGS *)(I2c->Dev->Resources[0].AddrRangeMin));

  RetVal = gBS->InstallMultipleProtocolInterfaces (&ControllerHandle,
                  &gEfiI2cMasterProtocolGuid, (VOID**)&I2c->I2cMaster,
                  &gEfiDevicePathProtocolGuid, &I2c->DevicePath,
                  NULL);

  if (EFI_ERROR (RetVal)) {
    FreePool (I2c);
    gBS->CloseProtocol (ControllerHandle,
                        &gEdkiiNonDiscoverableDeviceProtocolGuid,
                        DriverBindingHandle,
                        ControllerHandle);
  }

  return RetVal;
}

EFI_STATUS
BaikalI2cRelease (
  IN EFI_HANDLE                 DriverBindingHandle,
  IN EFI_HANDLE                 ControllerHandle
  )
{
  EFI_I2C_MASTER_PROTOCOL       *I2cMaster;
  EFI_STATUS                    RetVal;
  BAIKAL_I2C_MASTER             *I2c;

  RetVal = gBS->HandleProtocol (ControllerHandle,
                                &gEfiI2cMasterProtocolGuid,
                                (VOID **)&I2cMaster);
  ASSERT_EFI_ERROR (RetVal);
  if (EFI_ERROR (RetVal)) {
    return RetVal;
  }

  I2c = BAIKAL_I2C_FROM_THIS (I2cMaster);

  RetVal = gBS->UninstallMultipleProtocolInterfaces (ControllerHandle,
                  &gEfiI2cMasterProtocolGuid, I2cMaster,
                  &gEfiDevicePathProtocolGuid, &I2c->DevicePath,
                  NULL);
  if (EFI_ERROR (RetVal)) {
    return RetVal;
  }

  I2cDisable ((I2C_REGS *)(I2c->Dev->Resources[0].AddrRangeMin));

  RetVal = gBS->CloseProtocol (ControllerHandle,
                               &gEdkiiNonDiscoverableDeviceProtocolGuid,
                               DriverBindingHandle,
                               ControllerHandle);
  ASSERT_EFI_ERROR (RetVal);
  if (EFI_ERROR (RetVal)) {
    return RetVal;
  }

  gBS->FreePool (I2c);

  return EFI_SUCCESS;
}
