#include "I2cPmBus.h"
#include "PmBus.h"

#define PMBUS_IO_TYPE(write_type, read_type) ((write_type) | ((read_type) << 4))
#define PMBUS_IO_WRITE(io_type) ((io_type) & 0xf)
#define PMBUS_IO_READ(io_type) (((io_type) >> 4) & 0xf)

STATIC CONST PMBUS_CMD_IO PmBusCmdIo[] = {
#define _PMBUS_CMD_IO(cmd_code, cmd_name, write_type, read_type, data_size) \
  [cmd_code] = {                                                        \
    .Type = PMBUS_IO_TYPE(PMB_IO_##write_type, PMB_IO_##read_type),     \
    .Size = data_size,                                                  \
  },
  PMB_CMD_DEF(_PMBUS_CMD_IO)
#undef _PMBUS_CMD_IO
};

STATIC
EFI_STATUS
I2cPmBusIo (
  IN     VOID          *IoCtx,
  IN     BOOLEAN        Dir,
  IN OUT PMB_IO_TRANS  *Trans
  )
{
  I2C_PMBUS_IO         *Io = IoCtx;
  CONST PMBUS_CMD_IO   *CmdIo;
  UINT8                 TransType;
  UINTN                 TxLen;
  UINTN                 RxLen;
  UINTN                 RxedLen;

  ASSERT(Trans->Req.Cmd < sizeof(PmBusCmdIo) / sizeof(PMBUS_CMD_IO));

  CmdIo = &PmBusCmdIo[Trans->Req.Cmd];

  switch (Dir) {
  case PMB_IO_READ:
    TransType = PMBUS_IO_READ(CmdIo->Type);

    switch (TransType) {
    case PMB_IO_RECV_BYTE:
    case PMB_IO_READ_BYTE:
    case PMB_IO_READ_WORD:
    case PMB_IO_READ_32:
    case PMB_IO_READ_64:
      TxLen = 1;
      RxLen = CmdIo->Size;
      break;
    case PMB_IO_BLOCK_READ:
      TxLen = 1;
      RxLen = CmdIo->Size < 0 ? PMB_IO_MAX_PKT_LEN : CmdIo->Size;
      break;
    default:
      DEBUG ((EFI_D_ERROR, "%a: Invalid read transaction (txn: %02x, cmd: %02x)\n",
              __FUNCTION__, TransType, Trans->Req.Cmd));
      return EFI_INVALID_PARAMETER;
    }
    break;
  case PMB_IO_WRITE:
    TransType = PMBUS_IO_WRITE(CmdIo->Type);

    switch (TransType) {
    case PMB_IO_SEND_BYTE:
    case PMB_IO_WRITE_BYTE:
    case PMB_IO_WRITE_WORD:
    case PMB_IO_WRITE_32:
    case PMB_IO_WRITE_64:
      TxLen = 1 + CmdIo->Size;
      RxLen = 0;
      break;
    case PMB_IO_BLOCK_WRITE:
      TxLen = 2 + (CmdIo->Size < 0 ? Trans->Req.Block.Size : CmdIo->Size);
      RxLen = 0;
      break;
    default:
      DEBUG ((EFI_D_ERROR, "%a: Invalid write transaction (txn: %02x, cmd: %02x)\n",
              __FUNCTION__, TransType, Trans->Req.Cmd));
      return EFI_INVALID_PARAMETER;
    }
    break;

  default:
    DEBUG ((EFI_D_ERROR, "%a: Invalid direction %u\n", __FUNCTION__, Dir));
    return EFI_INVALID_PARAMETER;
  }

  RxedLen = I2cTxRx (Io->I2cBus, Io->I2cAddr,
                     Trans->Req.Raw, TxLen,
                     Trans->Res.Raw, RxLen);

  if (RxedLen != RxLen) {
    DEBUG ((EFI_D_WARN, "%a: Error when sending PMBUS command 0x%x\n",
            __FUNCTION__, Trans->Req.Cmd));

    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

I2C_PMBUS_DEV  *mI2cPmBusDevice = NULL;
EFI_HANDLE      mHandle         = NULL;

I2C_PMBUS_DEVICE_PATH mDevicePath = {
  {
    {MESSAGING_DEVICE_PATH, MSG_VENDOR_DP,
     {sizeof(VENDOR_DEVICE_PATH), 0}},
    SIMPLE_SENSOR_PROTOCOL_GUID,
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {sizeof(EFI_DEVICE_PATH_PROTOCOL), 0}
  }
};

STATIC
VOID
I2cPmBusPollHandler (
  IN EFI_EVENT     Event,
  IN VOID         *Context
  )
{
  I2C_PMBUS_DEV   *Device = Context;

  (VOID)PmBusPollSensors (&Device->IoCtx, I2cPmBusIo, &Device->Context);

  PmBusNotifyListeners (&Device->Context);
}

/**
  Starts the PMBUS device with this driver.

  This function produces Simple Sensor Protocol, and starts queue to poll this PMBUS device.

  @param  Handle                 Handle of device to bind driver to.
  @param  I2cBus                 I2C Bus address
  @param  I2cAddr                I2C Device address

  @retval EFI_SUCCESS            The driver started successfully.
  @retval Other                  This driver failed to start.
**/
EFI_STATUS
EFIAPI
I2cPmBusDriverStart (
  IN EFI_HANDLE                 Handle,
  IN EFI_PHYSICAL_ADDRESS       I2cBus,
  IN UINT8                      I2cAddr
  )
{
  EFI_STATUS                    Status;
  I2C_PMBUS_IO                  IoCtx;
  PMBUS_SCAN_DATA               ScanData;
  PMB_IO_TRANS                  Trans;
  I2C_PMBUS_DEV                *Device;
  EFI_TPL                       OldTpl;

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

  IoCtx.I2cBus = I2cBus;
  IoCtx.I2cAddr = I2cAddr;

  //
  // Request capabilities
  //
  Trans.Req.Cmd = PMB_CMD_CAPABILITY;
  Status = I2cPmBusIo (&IoCtx, PMB_IO_READ, &Trans);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Unable to request PMBUS capabilities: %r\n",
            __FUNCTION__, Status));
    goto ErrorExit;
  }
  IoCtx.Capability = PMB_READ_BYTE(&Trans.Res, 0);

  //
  // Scan pages and sensors
  //
  Status = PmBusScanSensors (&IoCtx, I2cPmBusIo, &ScanData);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Error when scanning PMBUS: %r\n",
            __FUNCTION__, Status));
    goto ErrorExit;
  }

  Device = AllocateZeroPool (sizeof(I2C_PMBUS_DEV) +
                             PMBUS_CONTEXT_MEMORY_SIZE(&ScanData));
  ASSERT(Device != NULL);

  Device->IoCtx = IoCtx;
  PmBusInitSensors ((UINT8 *)Device + sizeof(I2C_PMBUS_DEV),
                    &ScanData, &Device->Context);

  //
  // Do initial sensors polling
  //
  Status = PmBusPollSensors (&Device->IoCtx, I2cPmBusIo,
                             &Device->Context);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Error when polling PMBUS: %r\n",
            __FUNCTION__, Status));
    goto ErrorExit;
  }

  Status = gBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL,
                             TPL_CALLBACK, I2cPmBusPollHandler,
                             Device, &Device->TimerEvent);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Error when creating PMBUS poll event: %r\n",
            __FUNCTION__, Status));
    goto ErrorExit;
  }

  Status = gBS->SetTimer (Device->TimerEvent, TimerPeriodic,
                          I2C_PMBUS_POLL_INTERVAL);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Error when starting PMBUS poll timer: %r\n",
            __FUNCTION__, Status));
    goto ErrorExit;
  }

#if 0
  //
  // Get the Device Path Protocol on Controller's handle
  //
  Status = gBS->OpenProtocol (
    Handle,
    &gEfiDevicePathProtocolGuid,
    (VOID **) &Device->DevicePath,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Unable to get device path: %r\n", __FUNCTION__, Status));
    goto ErrorExit;
  }
#endif

  Status = gBS->InstallMultipleProtocolInterfaces (
      &Handle,
      &gSimpleSensorProtocolGuid,
      &Device->Context.SimpleSensor,
      NULL);

  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Unable to install SimpleSensor protocol: %r\n", __FUNCTION__,
            Status));
    goto ErrorExit;
  }

  mI2cPmBusDevice = Device;

  goto Exit;

//
// Error handler
//
ErrorExit:
  if (Device != NULL) {
    if (Device->TimerEvent != NULL) {
      gBS->CloseEvent (Device->TimerEvent);
    }

    FreePool (Device);
  }

Exit:
  gBS->RestoreTPL (OldTpl);

  return Status;
}

/**
  Stop the PMBUS device handled by this driver.

  @param  Handle             The handle to release.

  @retval EFI_SUCCESS            The device was stopped.
  @retval EFI_UNSUPPORTED        Simple Sensor Protocol is not installed on Controller.
  @retval EFI_DEVICE_ERROR       The device could not be stopped due to a device error.
  @retval Others                 Fail to uninstall protocols attached on the device.

**/
EFI_STATUS
EFIAPI
I2cPmBusDriverStop (
  IN  EFI_HANDLE                  Handle
  )
{
  I2C_PMBUS_DEV                  *Device;
  EFI_STATUS                      Status;

  Device = mI2cPmBusDevice;
  mI2cPmBusDevice = NULL;

  if (Device == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->UninstallMultipleProtocolInterfaces (
    &Handle,
    &gSimpleSensorProtocolGuid,
    &Device->Context.SimpleSensor,
    NULL);

  //
  // Free all resources.
  //
  gBS->CloseEvent (Device->TimerEvent);

  FreePool(Device);

  return Status;
}

/**
  Entrypoint of PMBUS Driver.

  This function is the entrypoint of I2C PMBUS Driver. It installs Driver Binding
  Protocols together with Component Name Protocols.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/
EFI_STATUS
EFIAPI
I2cPmBusDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  FDT_CLIENT_PROTOCOL    *FdtClient;
  INT32                   FdtNodeBus;
  INT32                   FdtNodeDev;
  CONST VOID             *FdtProp;
  UINT32                  FdtPropSize;
  UINT32                  I2cBus;
  UINT32                  I2cAddr;
  EFI_STATUS              Status;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FdtClientProtocol, Status: %r\n",
            __FUNCTION__, Status));
    return Status;
  }

  if (!(FdtClient->FindNodeByAlias (FdtClient, "hwmon", &FdtNodeDev) == EFI_SUCCESS &&
        FdtClient->IsNodeEnabled (FdtClient, FdtNodeDev) &&
        FdtClient->GetNodeProperty (FdtClient, FdtNodeDev, "compatible",
                                    &FdtProp, &FdtPropSize) == EFI_SUCCESS)) {
    DEBUG ((EFI_D_ERROR, "%a: PMBus HwMon node not found\n", __FUNCTION__));
    return EFI_NOT_FOUND;
  }

  if (AsciiStrCmp((CHAR8 *)FdtProp, "pmbus") != 0) {
    DEBUG ((EFI_D_ERROR, "%a: Unknown HwMon \"compatible\" property (%a)\n",
            __FUNCTION__, (UINT8 *)FdtProp));
    return EFI_NOT_FOUND;
  }

  if (!(FdtClient->GetNodeProperty (FdtClient, FdtNodeDev, "reg", &FdtProp, &FdtPropSize) == EFI_SUCCESS &&
        FdtPropSize == 4)) {
    DEBUG ((EFI_D_ERROR, "%a: Unable to get HwMon \"reg\" property\n", __FUNCTION__));
    return EFI_NOT_FOUND;
  }

  I2cAddr = SwapBytes32 (((CONST UINT32 *) FdtProp)[0]);

  if (!(FdtClient->FindParentNode  (FdtClient, FdtNodeDev, &FdtNodeBus) == EFI_SUCCESS &&
        FdtClient->IsNodeEnabled   (FdtClient, FdtNodeBus) &&
        FdtClient->GetNodeProperty (FdtClient, FdtNodeBus, "reg", &FdtProp, &FdtPropSize) == EFI_SUCCESS &&
        FdtPropSize == 16)) {
    DEBUG ((EFI_D_ERROR, "%a: Unable to get HwMon I2C bus \"reg\" property\n", __FUNCTION__));
    return EFI_NOT_FOUND;
  }

  I2cBus = SwapBytes64 (((CONST UINT64 *) FdtProp)[0]);

  /* create path */
  Status = gBS->InstallMultipleProtocolInterfaces (
    &mHandle,
    &gEfiDevicePathProtocolGuid,
    &mDevicePath,
    NULL
  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Unable to create device handle, %r\n", __FUNCTION__, Status));
    return Status;
  }

  /* start driver */
  Status = I2cPmBusDriverStart (mHandle, I2cBus, I2cAddr);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Unable to start driver, %r\n", __FUNCTION__, Status));
    return Status;
  }

  return EFI_SUCCESS;
}

/**
   Unload of PMBUS Driver.

   This function is the finalized of I2C PMBUS Driver.

   @param  ImageHandle       The firmware allocated handle for the EFI image.
   @param  SystemTable       A pointer to the EFI System Table.

   @retval EFI_SUCCESS       The finalizer is executed successfully.

**/
EFI_STATUS
EFIAPI
I2cPmBusDriverUnload(
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;

  Status = I2cPmBusDriverStop(mHandle);

  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Unable to stop PMBUS device, %r\n",
            __FUNCTION__, Status));
    return Status;
  }

  /* remove path */
  Status = gBS->UninstallMultipleProtocolInterfaces (
      &mHandle,
      &gEfiDevicePathProtocolGuid,
      &mDevicePath,
      NULL);

  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "%a: Unable to remove device handle, %r\n",
            __FUNCTION__, Status));
  }

  return Status;
}
