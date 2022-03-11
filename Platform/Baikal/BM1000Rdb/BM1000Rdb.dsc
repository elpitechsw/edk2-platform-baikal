## @file
#
#  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  PLATFORM_NAME           = BM1000Rdb
  PLATFORM_GUID           = 983E1AB6-1B12-4225-8F73-03F0E83E14A1
  PLATFORM_VERSION        = 1.0
  DSC_SPECIFICATION       = 0x0001001C
  OUTPUT_DIRECTORY        = Build/Baikal
  SUPPORTED_ARCHITECTURES = AARCH64
  BUILD_TARGETS           = DEBUG|RELEASE
  SKUID_IDENTIFIER        = DEFAULT
  FLASH_DEFINITION        = Platform/Baikal/BM1000Rdb/BM1000Rdb.fdf

  # Network definition
  DEFINE NETWORK_ALLOW_HTTP_CONNECTIONS = TRUE
  DEFINE NETWORK_IP6_ENABLE             = FALSE
  DEFINE NETWORK_ISCSI_ENABLE           = FALSE
  DEFINE NETWORK_SNP_ENABLE             = FALSE
  DEFINE NETWORK_TLS_ENABLE             = FALSE
  DEFINE NETWORK_VLAN_ENABLE            = FALSE

[BuildOptions]
  GCC:*_*_*_CC_FLAGS = -DDISABLE_NEW_DEPRECATED_INTERFACES
  GCC:*_*_*_PLATFORM_FLAGS = -march=armv8-a -fno-stack-protector
  GCC:RELEASE_*_*_CC_FLAGS = -DMDEPKG_NDEBUG -DNDEBUG
  GCC:*_*_*_DLINK_FLAGS = -Wl,--no-eh-frame -Wl,--no-eh-frame-hdr
!if $(BAIKAL_DBM10)
  *_*_*_CC_FLAGS = -DBAIKAL_DBM10
  *_*_*_ASLPP_FLAGS = -DBAIKAL_DBM10
!elseif $(BAIKAL_DBM20)
  *_*_*_CC_FLAGS = -DBAIKAL_DBM20
  *_*_*_ASLPP_FLAGS = -DBAIKAL_DBM20
!elseif $(BAIKAL_MBM10)
  *_*_*_CC_FLAGS = -DBAIKAL_MBM10
  *_*_*_ASLPP_FLAGS = -DBAIKAL_MBM10
!elseif $(BAIKAL_MBM20)
  *_*_*_CC_FLAGS = -DBAIKAL_MBM20
  *_*_*_ASLPP_FLAGS = -DBAIKAL_MBM20
!elseif $(BAIKAL_ELP)
  *_*_*_CC_FLAGS = -DELP_$(BOARD_VER) -DELPITECH
  *_*_*_ASLPP_FLAGS = -DELP_$(BOARD_VER) -DELPITECH
!endif

[BuildOptions.AARCH64.EDKII.DXE_CORE, BuildOptions.AARCH64.EDKII.DXE_DRIVER, BuildOptions.AARCH64.EDKII.UEFI_DRIVER, BuildOptions.AARCH64.EDKII.UEFI_APPLICATION]
  GCC:*_*_AARCH64_DLINK_FLAGS = -z common-page-size=0x1000

[BuildOptions.AARCH64.EDKII.DXE_RUNTIME_DRIVER]
  GCC:*_*_AARCH64_DLINK_FLAGS = -z common-page-size=0x10000

[LibraryClasses.AARCH64.SEC, LibraryClasses.AARCH64.DXE_CORE, LibraryClasses.AARCH64.DXE_DRIVER, LibraryClasses.AARCH64.DXE_RUNTIME_DRIVER, LibraryClasses.AARCH64.UEFI_APPLICATION, LibraryClasses.AARCH64.UEFI_DRIVER]
  ArmDisassemblerLib|ArmPkg/Library/ArmDisassemblerLib/ArmDisassemblerLib.inf
  ArmGenericTimerCounterLib|ArmPkg/Library/ArmGenericTimerPhyCounterLib/ArmGenericTimerPhyCounterLib.inf
  ArmGicArchLib|ArmPkg/Library/ArmGicArchLib/ArmGicArchLib.inf
  ArmGicLib|ArmPkg/Drivers/ArmGic/ArmGicLib.inf
  ArmLib|ArmPkg/Library/ArmLib/ArmBaseLib.inf
  ArmMmuLib|ArmPkg/Library/ArmMmuLib/ArmMmuBaseLib.inf
  ArmPlatformLib|Platform/Baikal/BM1000Rdb/Library/PlatformLib/PlatformLib.inf
  ArmPlatformStackLib|ArmPlatformPkg/Library/ArmPlatformStackLib/ArmPlatformStackLib.inf
  ArmSmcLib|ArmPkg/Library/ArmSmcLib/ArmSmcLib.inf
  AuthVariableLib|MdeModulePkg/Library/AuthVariableLibNull/AuthVariableLibNull.inf
  BaikalSmcLib|Platform/Baikal/Library/BaikalSmcLib/BaikalSmcLib.inf
  BaikalSpdLib|Platform/Baikal/Library/BaikalSpdLib/BaikalSpdLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf
  BmpSupportLib|MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  BootLogoLib|MdeModulePkg/Library/BootLogoLib/BootLogoLib.inf
  CacheMaintenanceLib|ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  CmuLib|Platform/Baikal/BM1000Rdb/Library/CmuLib/CmuLib.inf
  CpuExceptionHandlerLib|ArmPkg/Library/ArmExceptionLib/ArmExceptionLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  CrcLib|Platform/Baikal/Library/CrcLib/CrcLib.inf
  CustomizedDisplayLib|MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  DwUartLib|Silicon/Baikal/BM1000/Library/DwUartLib/DwUartLib.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf
  FileExplorerLib|MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  GpioLib|Silicon/Baikal/Library/DwGpioLib/DwGpioLib.inf
  HdmiLib|Platform/Baikal/Library/BaikalVduLib/BaikalHdmiLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  I2cLib|Silicon/Baikal/Library/DwI2cLib/DwI2cLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  LcdHwLib|Platform/Baikal/Library/BaikalVduLib/BaikalVduHwLib.inf
  LcdPlatformLib|Platform/Baikal/Library/BaikalVduLib/BaikalVduLib.inf
  LzmaDecompressLib|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  MemoryInitPeiLib|Platform/Baikal/Library/BaikalMemoryInitPeiLib/BaikalMemoryInitPeiLib.inf
  NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciHostBridgeLib|Silicon/Baikal/BM1000/Library/PciHostBridgeLib/PciHostBridgeLib.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciSegmentLib|Silicon/Baikal/BM1000/Library/PciSegmentLib/PciSegmentLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PlatformBootManagerLib|Platform/Baikal/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf
  PlatformHookLib|MdeModulePkg/Library/BasePlatformHookLibNull/BasePlatformHookLibNull.inf
  PlatformPeiLib|Platform/Baikal/Library/PlatformPeiLib/PlatformPeiLib.inf
  PrePiHobListPointerLib|ArmPlatformPkg/Library/PrePiHobListPointerLib/PrePiHobListPointerLib.inf
  PrePiLib|EmbeddedPkg/Library/PrePiLib/PrePiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
!if $(BAIKAL_QEMU_M) == FALSE
  RealTimeClockLib|Platform/Baikal/Library/BaikalRtcLib/BaikalRtcLib.inf
!else
  RealTimeClockLib|EmbeddedPkg/Library/TemplateRealTimeClockLib/TemplateRealTimeClockLib.inf
!endif
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  ResetSystemLib|ArmPkg/Library/ArmSmcPsciResetSystemLib/ArmSmcPsciResetSystemLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  SerialPortLib|MdeModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.inf
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  TimeBaseLib|EmbeddedPkg/Library/TimeBaseLib/TimeBaseLib.inf
  TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf
  TpmMeasurementLib|MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootManagerLib|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  VarCheckLib|MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
!if $(TARGET) == RELEASE
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  #PeCoffExtraActionLib|ArmPkg/Library/DebugPeCoffExtraActionLib/DebugPeCoffExtraActionLib.inf
!endif

[LibraryClasses.AARCH64.DXE_CORE]
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf

[LibraryClasses.AARCH64.DXE_DRIVER]
  NonDiscoverableDeviceRegistrationLib|MdeModulePkg/Library/NonDiscoverableDeviceRegistrationLib/NonDiscoverableDeviceRegistrationLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf

[LibraryClasses.AARCH64.DXE_RUNTIME_DRIVER]
!if $(TARGET) == DEBUG
  DebugLib|MdePkg/Library/DxeRuntimeDebugLibSerialPort/DxeRuntimeDebugLibSerialPort.inf
!endif

[LibraryClasses.AARCH64.UEFI_APPLICATION]
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf

[Components.AARCH64]
  # SEC modules
  Platform/Baikal/PrePi/PrePiUniCoreRelocatable.inf {
    <LibraryClasses>
      BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
      ExtractGuidedSectionLib|EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
      HobLib|EmbeddedPkg/Library/PrePiHobLib/PrePiHobLib.inf
      LzmaDecompressLib|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
      MemoryAllocationLib|EmbeddedPkg/Library/PrePiMemoryAllocationLib/PrePiMemoryAllocationLib.inf
  }

  # DXE
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
!if $(TARGET) == DEBUG
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x802B550F
!endif
  }
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  # Architectural protocols
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/VarCheckUefiLib/VarCheckUefiLib.inf
  }
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  MdeModulePkg/Universal/EbcDxe/EbcDxe.inf

  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }

  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }

  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  MdeModulePkg/Universal/SerialDxe/SerialDxe.inf {
    <LibraryClasses>
      NULL|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  ArmPlatformPkg/Drivers/LcdGraphicsOutputDxe/LcdGraphicsOutputDxe.inf {
!if $(TARGET) == DEBUG
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x807B55BF
!endif
  }

  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  Platform/Baikal/Drivers/BaikalSpiFvDxe/BaikalSpiFvDxe.inf
  Platform/Baikal/Drivers/BaikalSpiBlockDxe/BaikalSpiBlockDxe.inf
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  Platform/Baikal/BM1000Rdb/Drivers/EuiClientDxe/EuiClientDxe.inf
  Platform/Baikal/BM1000Rdb/Drivers/UidClientDxe/UidClientDxe.inf
  Platform/Baikal/Drivers/FdtClientDxe/FdtClientDxe.inf
  Platform/Baikal/Drivers/FruClientDxe/FruClientDxe.inf
  Platform/Baikal/Drivers/HighMemDxe/HighMemDxe.inf

  # GPT/MBR partitioning + filesystems
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf
  MdeModulePkg/Universal/Disk/UdfDxe/UdfDxe.inf

  # SMBIOS Support
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  Platform/Baikal/BM1000Rdb/Drivers/SmbiosPlatformDxe/SmbiosPlatformDxe.inf

  # ACPI support
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf {
    <PcdsFixedAtBuild>
      gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemTableId|{ 0x42, 0x4B, 0x4C, 0x45, 0x42, 0x47, 0x52, 0x54 }
  }
  Platform/Baikal/BM1000Rdb/Drivers/AcpiPlatformDxe/AcpiPlatformDxe.inf

  # BDS
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/DisplayEngineDxe/DisplayEngineDxe.inf
  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf
  Platform/Baikal/BM1000Rdb/Drivers/ConfigDxe/ConfigDxe.inf
  Platform/Baikal/Logo/LogoDxe.inf
  MdeModulePkg/Application/UiApp/UiApp.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/DeviceManagerUiLib/DeviceManagerUiLib.inf
      NULL|MdeModulePkg/Library/BootManagerUiLib/BootManagerUiLib.inf
      NULL|MdeModulePkg/Library/BootMaintenanceManagerUiLib/BootMaintenanceManagerUiLib.inf
  }

  # Disk driver
  Silicon/Baikal/BM1000/Drivers/NonDiscoverableAhciDxe/NonDiscoverableAhciDxe.inf
  MdeModulePkg/Bus/Ata/AtaAtapiPassThru/AtaAtapiPassThru.inf
  MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf
  MdeModulePkg/Bus/Pci/NonDiscoverablePciDeviceDxe/NonDiscoverablePciDeviceDxe.inf
  MdeModulePkg/Bus/Pci/SataControllerDxe/SataControllerDxe.inf

  # SCSI
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf

  # Network
!include NetworkPkg/Network.dsc.inc
  Platform/Baikal/Drivers/GmacDxe/GmacDxe.inf

  # PCI
  ArmPkg/Drivers/ArmPciCpuIo2Dxe/ArmPciCpuIo2Dxe.inf
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  MdeModulePkg/Bus/Pci/PciHostBridgeDxe/PciHostBridgeDxe.inf

  # NVMe
  MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf

  # SD
  Platform/Baikal/Drivers/SdBlockDxe/SdBlock.inf

  # USB
  Silicon/Baikal/BM1000/Drivers/NonDiscoverableXhciDxe/NonDiscoverableXhciDxe.inf
  MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf
  MdeModulePkg/Bus/Usb/UsbMouseDxe/UsbMouseDxe.inf

  # PS/2
  Platform/Baikal/Drivers/Ps2MultDxe/Ps2MultDxe.inf

  # UEFI application (Shell Embedded Boot Loader)
  ShellPkg/Application/Shell/Shell.inf {
    <LibraryClasses>
      ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
      NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf
      HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
      BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf

    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|8000
      gEfiShellPkgTokenSpaceGuid.PcdShellFileOperationSize|0x200000
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  }
  ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf {
    <PcdsFixedAtBuild>
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  }

!if $(BUILD_UEFI_APPS) == TRUE
  Platform/Baikal/Application/SpiFlash/SpiFlash.inf
  Platform/Baikal/Application/SpiFlashImage/SpiFlashImage.inf
!endif

[PcdsFeatureFlag.common]
  gArmTokenSpaceGuid.PcdRelocateVectorTable|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutUgaSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdHiiOsRuntimeSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdInstallAcpiSdtProtocol|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdPciDegradeResourceForOptionRom|FALSE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnostics2Disable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnosticsDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdUgaConsumeSupport|FALSE
  gEmbeddedTokenSpaceGuid.PcdPrePiProduceMemoryTypeInformationHob|TRUE

[PcdsPatchableInModule.common]
  gArmTokenSpaceGuid.PcdFdBaseAddress|0
  gArmTokenSpaceGuid.PcdFvBaseAddress|0
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0
  gArmTokenSpaceGuid.PcdSystemMemorySize|0
  gBaikalTokenSpaceGuid.PcdDeviceTreeInitialBaseAddress|0x80000000

[PcdsFixedAtBuild.common]
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x4000
  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0x4007C000
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x2D000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x2D100000
  gArmTokenSpaceGuid.PcdVFPEnabled|1
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorId|{ 0x42, 0x4B, 0x4C, 0x45 }
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorRevision|1
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemId|{ 0x42, 0x41, 0x49, 0x4B, 0x41, 0x4C }
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|1
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemTableId|{ 0x42, 0x4B, 0x4C, 0x45 }
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiExposedTableVersions|0x20
  gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerMenuFile|{ 0x21, 0xAA, 0x2C, 0x46, 0x14, 0x76, 0x03, 0x45, 0x83, 0x6E, 0x8A, 0xB6, 0xF4, 0x66, 0x23, 0x31 }
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeNxMemoryProtectionPolicy|0xC000000000007FD1
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareRevision|$(FIRMWARE_REVISION)
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"$(FIRMWARE_VENDOR)"
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"$(FIRMWARE_VERSION_STRING)"
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxAuthVariableSize|0x2800
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x2000
  gEfiMdeModulePkgTokenSpaceGuid.PcdPciDisableBusEnumeration|FALSE
  #
  # In ECAM mode, the ACPI I/O aperture matches the PPB aperture for the exposed bus.
  # Unfortunately, Linux doesn't seem to handle correctly "small" apertures (i.e. 4K),
  # so round it out to the full 16-bit space.
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdPciIoApertureSizeAlignment|0x10000
  gEfiMdeModulePkgTokenSpaceGuid.PcdPlatformRecoverySupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialBaudRate|115200
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialClockRate|7361963
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|0x20230000
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterStride|4
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialUseHardwareFlowControl|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialUseMmio|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosDocRev|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosVersion|0x0300
  gEfiMdePkgTokenSpaceGuid.PcdDefaultTerminalType|4
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdPostCodePropertyMask|0
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07
  gEfiMdePkgTokenSpaceGuid.PcdSpinLockTimeout|10000000
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|320
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultPlatformLangCodes|"en-US"
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIMemoryNVS|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIReclaimMemory|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|300
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesData|2000
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderCode|20
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderData|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiReservedMemoryType|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|150
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|300
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|32
!if $(TARGET) == RELEASE
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0
!else
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2F
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x803B45FF
!endif

[PcdsDynamicDefault.common]
  gBaikalTokenSpaceGuid.PcdPcieCfg0FilteringWorks|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|0

[PcdsDynamicHii.common]
  gBaikalTokenSpaceGuid.PcdAcpiMsiMode|L"AcpiMsi"|gConfigDxeFormSetGuid|0x0|1
  gBaikalTokenSpaceGuid.PcdAcpiPcieMode|L"AcpiPcie"|gConfigDxeFormSetGuid|0x0|1
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|L"Timeout"|gEfiGlobalVariableGuid|0x0|5
