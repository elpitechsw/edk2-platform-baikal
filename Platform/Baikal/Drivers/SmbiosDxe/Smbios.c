/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/SmBios.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/Smbios.h>

#define BAIKAL_FIRMWARE_VENDOR                      "Baikal Electronics"

/* Type 4 */
#define BAIKAL_PROCESSOR_SOCKET_DESIGNATION         "Cortex-A57"
#define BAIKAL_PROCESSOR_MANUFACTURER               "Baikal Electronics"

#define BAIKAL_PROCESSOR_VERSION                    " "
#define BAIKAL_PROCESSOR_SERIAL_NUMBER              " "
#define BAIKAL_PROCESSOR_ASSET_TAG                  " "
#define BAIKAL_PROCESSOR_PART_NUMBER                " "
#define BAIKAL_PROCESSOR_VOLTAGE                    {}

/* Type 0 */
#define BAIKAL_BIOS_VENDOR                          BAIKAL_FIRMWARE_VENDOR
#define BAIKAL_BIOS_VERSION                         " "
#define BAIKAL_BIOS_DATE                            "02/23/2021"
#define BAIKAL_BIOS_MAJOR_VERSION                   0xFF
#define BAIKAL_BIOS_MINOR_VERSION                   0xFF
#define BAIKAL_EMBEDDED_CONTROLLER_FW_MAJOR_VERSION 0xFF
#define BAIKAL_EMBEDDED_CONTROLLER_FW_MINOR_VERSION 0xFF
#define BAIKAL_ROM_SIZE                             0
#define BAIKAL_ROM_EXTENDED_SIZE                    {}

/* Type 1 */
#define BAIKAL_SYSTEM_MANUFACTURER                  BAIKAL_FIRMWARE_VENDOR
#define BAIKAL_SYSTEM_PRODUCT_NAME                  " "
#define BAIKAL_SYSTEM_VERSION                       " "
#define BAIKAL_SYSTEM_SERIAL_NUMBER                 " "
#define BAIKAL_SYSTEM_SKU_NUMBER                    " "
#define BAIKAL_SYSTEM_FAMILY                        " "
#define BAIKAL_SYSTEM_GUID                          {}

/* Type 3 */
#define BAIKAL_ENCLOSURE_MANUFACTURER               BAIKAL_FIRMWARE_VENDOR
#define BAIKAL_ENCLOSURE_VERSION                    " "
#define BAIKAL_ENCLOSURE_SERIAL_NUMBER              " "
#define BAIKAL_ENCLOSURE_ASSET_TAG                  " "
#define BAIKAL_ENCLOSURE_SKU_NUMBER                 " "

/* Type 17 */
#define BAIKAL_DDR3_SIZE                            0x100000000ULL
#define BAIKAL_DDR3_RANK                            1
#define BAIKAL_DDR3_MANUFACTURER                    " "
#define BAIKAL_DDR3_SERIAL_NUMBER                   " "
#define BAIKAL_DDR3_ASSET_TAG                       " "
#define BAIKAL_DDR3_PART_NUMBER                     " "
#define BAIKAL_DDR3_FIRMWARE_VERSION                " "

#define BAIKAL_DDR4_SIZE                            0x100000000ULL
#define BAIKAL_DDR4_RANK                            1
#define BAIKAL_DDR4_MANUFACTURER                    " "
#define BAIKAL_DDR4_SERIAL_NUMBER                   " "
#define BAIKAL_DDR4_ASSET_TAG                       " "
#define BAIKAL_DDR4_PART_NUMBER                     " "
#define BAIKAL_DDR4_FIRMWARE_VERSION                " "

/* Type 9 */
#define BAIKAL_PCIE_X4_0_DEV_FUNC_NUM               0
#define BAIKAL_PCIE_X4_1_DEV_FUNC_NUM               0
#define BAIKAL_PCIE_X8_DEV_FUNC_NUM                 0

#pragma pack(1)
typedef struct {
  SMBIOS_STRUCTURE            Hdr;
  SMBIOS_TABLE_STRING         Manufacturer;
  UINT8                       Type;
  SMBIOS_TABLE_STRING         Version;
  SMBIOS_TABLE_STRING         SerialNumber;
  SMBIOS_TABLE_STRING         AssetTag;
  UINT8                       BootupState;
  UINT8                       PowerSupplyState;
  UINT8                       ThermalState;
  UINT8                       SecurityStatus;
  UINT8                       OemDefined[4];
  UINT8                       Height;
  UINT8                       NumberofPowerCords;
  UINT8                       ContainedElementCount;
  UINT8                       ContainedElementRecordLength;
  SMBIOS_TABLE_STRING         SKUNumber;
} SMBIOS_TABLE_TYPE3_BAIKAL;

typedef struct {
  SMBIOS_STRUCTURE            Hdr;
  SMBIOS_TABLE_STRING         SlotDesignation;
  UINT8                       SlotType;
  UINT8                       SlotDataBusWidth;
  UINT8                       CurrentUsage;
  UINT8                       SlotLength;
  UINT16                      SlotID;
  MISC_SLOT_CHARACTERISTICS1  SlotCharacteristics1;
  MISC_SLOT_CHARACTERISTICS2  SlotCharacteristics2;
  UINT16                      SegmentGroupNum;
  UINT8                       BusNum;
  UINT8                       DevFuncNum;
  UINT8                       DataBusWidth;
  UINT8                       PeerGroupingCount;
} SMBIOS_TABLE_TYPE9_BAIKAL;
#pragma pack()

#define BAIKAL_SMBIOS_STRING(Str) Str "\0"

#define BAIKAL_SMBIOS_TABLE(Num, Str, ...) (CHAR8 *) &(struct { \
  SMBIOS_TABLE_TYPE##Num Table; \
  CHAR8 Strings[sizeof (Str)]; \
}) { \
  __VA_ARGS__, \
  Str \
}

#define BAIKAL_SMBIOS_TABLE_HANDLE(Type, Num) (Type << 8 | Num)

#define BAIKAL_SMBIOS_PROCESSOR_INFORMATION_TABLE(Num) BAIKAL_SMBIOS_TABLE ( \
    4, \
    BAIKAL_SMBIOS_STRING (BAIKAL_PROCESSOR_SOCKET_DESIGNATION) \
    BAIKAL_SMBIOS_STRING (BAIKAL_PROCESSOR_MANUFACTURER) \
    BAIKAL_SMBIOS_STRING (BAIKAL_PROCESSOR_VERSION) \
    BAIKAL_SMBIOS_STRING (BAIKAL_PROCESSOR_SERIAL_NUMBER) \
    BAIKAL_SMBIOS_STRING (BAIKAL_PROCESSOR_ASSET_TAG) \
    BAIKAL_SMBIOS_STRING (BAIKAL_PROCESSOR_PART_NUMBER), \
    { \
      { \
        SMBIOS_TYPE_PROCESSOR_INFORMATION, \
        sizeof (SMBIOS_TABLE_TYPE4), \
        BAIKAL_SMBIOS_TABLE_HANDLE (4, Num) \
      }, \
      1, \
      CentralProcessor, \
      ProcessorFamilyIndicatorFamily2, \
      2, \
      {}, \
      3, \
      BAIKAL_PROCESSOR_VOLTAGE, \
      0, \
      1500, \
      1500, \
      0x41, \
      ProcessorUpgradeNone, \
      BAIKAL_SMBIOS_TABLE_HANDLE (7, 1), \
      BAIKAL_SMBIOS_TABLE_HANDLE (7, 2), \
      0, \
      4, \
      5, \
      6, \
      2, \
      2, \
      2, \
      0xEC, \
      ProcessorFamilyARMv8, \
      2, \
      2, \
      2 \
    } \
)

#pragma pack(1)
STATIC CHAR8 *SmbiosTables[] = {
  /* BIOS Information table, type 0 */
  BAIKAL_SMBIOS_TABLE (
    0,
    BAIKAL_SMBIOS_STRING (BAIKAL_BIOS_VENDOR)
    BAIKAL_SMBIOS_STRING (BAIKAL_BIOS_VERSION)
    BAIKAL_SMBIOS_STRING (BAIKAL_BIOS_DATE),
    {
      {
        SMBIOS_TYPE_BIOS_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE0),
        BAIKAL_SMBIOS_TABLE_HANDLE (0, 0)
      },
      1,
      2,
      0,
      3,
      BAIKAL_ROM_SIZE,
      {.BiosCharacteristicsNotSupported = 1},
      {},
      BAIKAL_BIOS_MAJOR_VERSION,
      BAIKAL_BIOS_MINOR_VERSION,
      BAIKAL_EMBEDDED_CONTROLLER_FW_MAJOR_VERSION,
      BAIKAL_EMBEDDED_CONTROLLER_FW_MINOR_VERSION,
      BAIKAL_ROM_EXTENDED_SIZE
    }
  ),

  /* System Information table, type 1 */
  BAIKAL_SMBIOS_TABLE (
    1,
    BAIKAL_SMBIOS_STRING (BAIKAL_SYSTEM_MANUFACTURER)
    BAIKAL_SMBIOS_STRING (BAIKAL_SYSTEM_PRODUCT_NAME)
    BAIKAL_SMBIOS_STRING (BAIKAL_SYSTEM_VERSION)
    BAIKAL_SMBIOS_STRING (BAIKAL_SYSTEM_SERIAL_NUMBER)
    BAIKAL_SMBIOS_STRING (BAIKAL_SYSTEM_SKU_NUMBER)
    BAIKAL_SMBIOS_STRING (BAIKAL_SYSTEM_FAMILY),
    {
      {
        SMBIOS_TYPE_SYSTEM_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE1),
        BAIKAL_SMBIOS_TABLE_HANDLE (1, 0)
      },
      1,
      2,
      3,
      4,
      BAIKAL_SYSTEM_GUID,
      SystemWakeupTypeUnknown,
      5,
      6
    }
  ),

  /* System Enclosure or Chassis table, type 3 */
  BAIKAL_SMBIOS_TABLE (
    3_BAIKAL,
    BAIKAL_SMBIOS_STRING (BAIKAL_ENCLOSURE_MANUFACTURER)
    BAIKAL_SMBIOS_STRING (BAIKAL_ENCLOSURE_VERSION)
    BAIKAL_SMBIOS_STRING (BAIKAL_ENCLOSURE_SERIAL_NUMBER)
    BAIKAL_SMBIOS_STRING (BAIKAL_ENCLOSURE_ASSET_TAG)
    BAIKAL_SMBIOS_STRING (BAIKAL_ENCLOSURE_SKU_NUMBER),
    {
      {
        SMBIOS_TYPE_SYSTEM_ENCLOSURE,
        sizeof (SMBIOS_TABLE_TYPE3_BAIKAL),
        BAIKAL_SMBIOS_TABLE_HANDLE (3, 0)
      },
      1,
      0x22,
      2,
      3,
      4,
      ChassisStateUnknown,
      ChassisStateUnknown,
      ChassisStateUnknown,
      ChassisSecurityStatusNone,
      {},
      0,
      0,
      0,
      0,
      5
    }
  ),

  /* Cache Information tables, type 7 */
  BAIKAL_SMBIOS_TABLE (
    7,
    BAIKAL_SMBIOS_STRING ("L1 Instruction"),
    {
      {
        SMBIOS_TYPE_CACHE_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE7),
        BAIKAL_SMBIOS_TABLE_HANDLE (7, 0)
      },
      1,
      0x180,
      0x30,
      0x30,
      {.Unknown = 1},
      {.Unknown = 1},
      0,
      CacheErrorParity,
      CacheTypeInstruction,
      CacheAssociativityOther,
      0x30,
      0x30
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    7,
    BAIKAL_SMBIOS_STRING ("L1 Data"),
    {
      {
        SMBIOS_TYPE_CACHE_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE7),
        BAIKAL_SMBIOS_TABLE_HANDLE (7, 1)
      },
      1,
      0x180,
      0x20,
      0x20,
      {.Unknown = 1},
      {.Unknown = 1},
      0,
      CacheErrorSingleBit,
      CacheTypeData,
      CacheAssociativity2Way,
      0x20,
      0x20
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    7,
    BAIKAL_SMBIOS_STRING ("L2"),
    {
      {
        SMBIOS_TYPE_CACHE_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE7),
        BAIKAL_SMBIOS_TABLE_HANDLE (7, 2)
      },
      1,
      0x181,
      0x800,
      0x800,
      {.Unknown = 1},
      {.Unknown = 1},
      0,
      CacheErrorSingleBit,
      CacheTypeUnified,
      CacheAssociativity16Way,
      0x800,
      0x800
    }
  ),

  /* Processor Information tables, type 4 */
  BAIKAL_SMBIOS_PROCESSOR_INFORMATION_TABLE (0),
  BAIKAL_SMBIOS_PROCESSOR_INFORMATION_TABLE (1),
  BAIKAL_SMBIOS_PROCESSOR_INFORMATION_TABLE (2),
  BAIKAL_SMBIOS_PROCESSOR_INFORMATION_TABLE (3),

  /* System Boot Information table, type 32 */
  BAIKAL_SMBIOS_TABLE (
    32,
    BAIKAL_SMBIOS_STRING (""),
    {
      {
        SMBIOS_TYPE_SYSTEM_BOOT_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE32),
        BAIKAL_SMBIOS_TABLE_HANDLE (32, 0)
      },
      {},
      0
    }
  ),

  /* Physical Memory Array table, type 16 */
  BAIKAL_SMBIOS_TABLE (
    16,
    BAIKAL_SMBIOS_STRING (""),
    {
      {
        SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY,
        sizeof (SMBIOS_TABLE_TYPE16),
        BAIKAL_SMBIOS_TABLE_HANDLE (16, 0)
      },
      MemoryArrayLocationSystemBoard,
      MemoryArrayUseSystemMemory,
      MemoryErrorCorrectionMultiBitEcc,
      0x5000000, /* DDR3 max 16Gb + DDR4 max 64Gb */
      SMBIOS_HANDLE_PI_RESERVED,
      2,
      0
    }
  ),

  /* Memory Device tables, type 17 */
  BAIKAL_SMBIOS_TABLE (
    17,
    BAIKAL_SMBIOS_STRING ("DIMM 1")
    BAIKAL_SMBIOS_STRING ("Bank A")
    BAIKAL_SMBIOS_STRING (BAIKAL_DDR3_MANUFACTURER)
    BAIKAL_SMBIOS_STRING (BAIKAL_DDR3_SERIAL_NUMBER)
    BAIKAL_SMBIOS_STRING (BAIKAL_DDR3_ASSET_TAG)
    BAIKAL_SMBIOS_STRING (BAIKAL_DDR3_PART_NUMBER)
    BAIKAL_SMBIOS_STRING (BAIKAL_DDR3_FIRMWARE_VERSION),
    {
      {
        SMBIOS_TYPE_MEMORY_DEVICE,
        sizeof (SMBIOS_TABLE_TYPE17),
        BAIKAL_SMBIOS_TABLE_HANDLE (17, 0)
      },
      BAIKAL_SMBIOS_TABLE_HANDLE (16, 0),
      SMBIOS_HANDLE_PI_RESERVED,
      72,
      64,
      BAIKAL_DDR3_SIZE >> 20,
      MemoryFormFactorDimm,
      0,
      1,
      2,
      MemoryTypeDdr3,
      {.Synchronous = 1},
      0,
      3,
      4,
      5,
      6,
      BAIKAL_DDR3_RANK,
      0,
      0,
      0,
      0,
      0,
      MemoryTechnologyDram,
      {.Bits = {.VolatileMemory = 1}},
      7,
      0,
      0,
      0,
      0,
      0,
      BAIKAL_DDR3_SIZE,
      0,
      0,
      0,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    17,
    BAIKAL_SMBIOS_STRING ("DIMM 2")
    BAIKAL_SMBIOS_STRING ("Bank B")
    BAIKAL_SMBIOS_STRING (BAIKAL_DDR4_MANUFACTURER)
    BAIKAL_SMBIOS_STRING (BAIKAL_DDR4_SERIAL_NUMBER)
    BAIKAL_SMBIOS_STRING (BAIKAL_DDR4_ASSET_TAG)
    BAIKAL_SMBIOS_STRING (BAIKAL_DDR4_PART_NUMBER)
    BAIKAL_SMBIOS_STRING (BAIKAL_DDR4_FIRMWARE_VERSION),
    {
      {
        SMBIOS_TYPE_MEMORY_DEVICE,
        sizeof (SMBIOS_TABLE_TYPE17),
        BAIKAL_SMBIOS_TABLE_HANDLE (17, 1)
      },
      BAIKAL_SMBIOS_TABLE_HANDLE (16, 0),
      SMBIOS_HANDLE_PI_RESERVED,
      72,
      64,
      BAIKAL_DDR4_SIZE >> 20,
      MemoryFormFactorDimm,
      0,
      1,
      2,
      MemoryTypeDdr4,
      {.Synchronous = 1},
      0,
      3,
      4,
      5,
      6,
      BAIKAL_DDR4_RANK,
      0,
      0,
      0,
      0,
      0,
      MemoryTechnologyDram,
      {.Bits = {.VolatileMemory = 1}},
      7,
      0,
      0,
      0,
      0,
      0,
      BAIKAL_DDR4_SIZE,
      0,
      0,
      0,
      0
    }
  ),

  /* Memory Array Mapped Address table, type 19 */
  BAIKAL_SMBIOS_TABLE (
    19,
    BAIKAL_SMBIOS_STRING (""),
    {
      {
        SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS,
        sizeof (SMBIOS_TABLE_TYPE19),
        BAIKAL_SMBIOS_TABLE_HANDLE (19, 0)
      },
      0,
      (BAIKAL_DDR3_SIZE >> 10) + (BAIKAL_DDR4_SIZE >> 10) - 1,
      BAIKAL_SMBIOS_TABLE_HANDLE (16, 0),
      2,
      0,
      0
    }
  ),

  /* System Slots tables, type 9 */
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x4_0"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        BAIKAL_SMBIOS_TABLE_HANDLE (9, 0)
      },
      1,
      SlotTypePciExpress,
      SlotDataBusWidth4X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {.Provides33Volts = 1},
      {.HotPlugDevicesSupported = 1},
      0,
      0,
      BAIKAL_PCIE_X4_0_DEV_FUNC_NUM,
      4,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x4_1"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        BAIKAL_SMBIOS_TABLE_HANDLE (9, 1)
      },
      1,
      SlotTypePciExpress,
      SlotDataBusWidth4X,
      SlotUsageInUse,
      SlotLengthUnknown,
      1,
      {.Provides33Volts = 1},
      {.HotPlugDevicesSupported = 1},
      0,
      1,
      BAIKAL_PCIE_X4_1_DEV_FUNC_NUM,
      4,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x8"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        BAIKAL_SMBIOS_TABLE_HANDLE (9, 2)
      },
      1,
      SlotTypePciExpress,
      SlotDataBusWidth8X,
      SlotUsageInUse,
      SlotLengthUnknown,
      2,
      {.Provides33Volts = 1},
      {.HotPlugDevicesSupported = 1},
      0,
      2,
      BAIKAL_PCIE_X8_DEV_FUNC_NUM,
      8,
      0
    }
  ),

  /* Port Connector Information tables, type 8 */
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("USB3_0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 0)
      },
      0,
      0,
      1,
      PortConnectorTypeUsb,
      PortTypeUsb
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("USB3_1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 1)
      },
      0,
      0,
      1,
      PortConnectorTypeUsb,
      PortTypeUsb
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("USB2_0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 2)
      },
      0,
      0,
      1,
      PortConnectorTypeUsb,
      PortTypeUsb
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("USB2_1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 3)
      },
      0,
      0,
      1,
      PortConnectorTypeUsb,
      PortTypeUsb
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("USB2_2"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 4)
      },
      0,
      0,
      1,
      PortConnectorTypeUsb,
      PortTypeUsb
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("USB2_3"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 5)
      },
      0,
      0,
      1,
      PortConnectorTypeUsb,
      PortTypeUsb
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("SATA0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 6)
      },
      0,
      0,
      1,
      PortConnectorTypeSasSata,
      PortTypeSata
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("SATA1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 7)
      },
      0,
      0,
      1,
      PortConnectorTypeSasSata,
      PortTypeSata
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("GMAC0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 8)
      },
      0,
      0,
      1,
      PortConnectorTypeRJ45,
      PortTypeNetworkPort
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("GMAC1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 9)
      },
      0,
      0,
      1,
      PortConnectorTypeRJ45,
      PortTypeNetworkPort
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("XGMAC0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 10)
      },
      0,
      0,
      1,
      PortConnectorTypeRJ45,
      PortTypeNetworkPort
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("XGMAC1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 11)
      },
      0,
      0,
      1,
      PortConnectorTypeRJ45,
      PortTypeNetworkPort
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("eMMC/SD"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 12)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeOther
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("GPIO"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 13)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeOther
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("UART1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 14)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeSerial16550Compatible
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("UART2"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 15)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeSerial16550Compatible
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("SPI"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 16)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeOther
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("eSPI"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 17)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeOther
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("I2C0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 18)},
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeOther
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("I2C1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 19)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeOther
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("SMBUS0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 20)},
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeOther
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("SMBUS1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 21)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeOther
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("LVDS"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 22)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeVideoPort
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("HDMI"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 23)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeVideoPort
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("I2S"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 24)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeAudioPort
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("SW/JTAG"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 25)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeOther
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("MIPI PTI"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        BAIKAL_SMBIOS_TABLE_HANDLE (8, 26)
      },
      0,
      0,
      1,
      PortConnectorTypeOther,
      PortTypeOther
    }
  )
};
#pragma pack()

EFI_STATUS
EFIAPI
SmbiosDxeInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN                    Idx;
  EFI_SMBIOS_TABLE_HEADER  *Record;
  EFI_SMBIOS_PROTOCOL      *Smbios;
  EFI_SMBIOS_HANDLE        SmbiosHandle;
  EFI_STATUS               Status;

  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **) &Smbios);

  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: failed to locate SMBIOS Protocol, Status = %r\n",
      __FUNCTION__,
      Status
      ));

    return Status;
  }

  for (Idx = 0; Idx < ARRAY_SIZE (SmbiosTables); ++Idx) {
    Record = (EFI_SMBIOS_TABLE_HEADER *) SmbiosTables[Idx];
    if (Record->Type == SMBIOS_TYPE_PROCESSOR_INFORMATION) {
      *(UINT32 *)(((CHAR8 *) Record) + OFFSET_OF (SMBIOS_TABLE_TYPE4, ProcessorId)) = 0x411FD073;
    }

    SmbiosHandle = Record->Handle;
    Status = Smbios->Add (Smbios, gImageHandle, &SmbiosHandle, Record);

    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: failed to add SMBIOS table, Status = %r\n",
        __FUNCTION__,
        Status
        ));

      return Status;
    }
  }

  return Status;
}
