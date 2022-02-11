/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/Smbios.h>

STATIC CHAR8  BaikalModel[] = "DBS";

#define BAIKAL_SMBIOS_STRING(Str)  Str "\0"
#define BAIKAL_SMBIOS_TABLE_HANDLE(Type, Num)  ((Type) << 8 | (Num))
#define BAIKAL_SMBIOS_TABLE(Num, Str, ...)  (CHAR8 *) &(struct { \
  SMBIOS_TABLE_TYPE##Num Table; \
  CHAR8 Strings[sizeof (Str)]; \
}) { \
  __VA_ARGS__, \
  Str \
}

STATIC
EFI_STATUS
CreateSmbiosTable (
  IN  EFI_SMBIOS_TABLE_HEADER   *Template,
  IN  CHAR8                    **StringPack,
  IN  CONST UINTN                StringPackSize
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL      *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER  *Record;
  UINTN                     Idx;
  UINTN                     StringSize;
  UINTN                     Size;
  CHAR8                    *Str;

  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **) &Smbios);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to locate SMBIOS Protocol, Status = %r\n",
      __FUNCTION__,
      Status
      ));
    return Status;
  }

  // Calculate the size of the fixed record and optional string pack
  Size = Template->Length;

  if (StringPack != NULL) {
    for (Idx = 0; Idx < StringPackSize; ++Idx) {
      if (StringPack[Idx] != NULL) {
        StringSize = AsciiStrSize (StringPack[Idx]);
        if (StringSize > 1) {
          Size += StringSize;
        }
      }
    }

    if (Size == Template->Length) {
      // At least a double null is required
      Size += 1;
    }

    // Don't forget the terminating double null
    Size += 1;

    // Copy over Template
    Record = (EFI_SMBIOS_TABLE_HEADER *) AllocateZeroPool (Size);
    if (Record == NULL) {
      DEBUG ((
        EFI_D_ERROR,
        "%a: failed to allocate memory for SMBIOS table, Status = %r\n",
        __FUNCTION__,
        EFI_OUT_OF_RESOURCES
        ));
      return EFI_OUT_OF_RESOURCES;
    }

    CopyMem (Record, Template, Template->Length);

    // Append string pack
    Str = ((CHAR8 *) Record) + Template->Length;

    for (Idx = 0; Idx < StringPackSize; ++Idx) {
      if (StringPack[Idx] != NULL) {
        StringSize = AsciiStrSize (StringPack[Idx]);
        if (StringSize > 1) {
          CopyMem (Str, StringPack[Idx], StringSize);
          Str += StringSize;
        }
      }
    }
  } else {
    Record = Template;
  }

  SmbiosHandle = Record->Handle;
  Status = Smbios->Add (Smbios, gImageHandle, &SmbiosHandle, Record);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to add SMBIOS table, Status = %r\n",
      __FUNCTION__,
      Status
      ));
  }

  if (StringPack != NULL) {
    FreePool (Record);
  }

  return Status;
}

/* BIOS Information table, type 0 */

#define BAIKAL_BIOS_DATE  "00/00/0000"

#pragma pack(1)
STATIC SMBIOS_TABLE_TYPE0 SmbiosTable0 = {
  {
    SMBIOS_TYPE_BIOS_INFORMATION,
    sizeof (SMBIOS_TABLE_TYPE0),
    BAIKAL_SMBIOS_TABLE_HANDLE (0, 0)
  },
  1,
  2,
  0,
  3,
  (UINT8) (FixedPcdGet32 (PcdFdSize) - 1) / SIZE_64KB, /* BIOS ROM size */
  {},
  {
    BIT0, // AcpiIsSupported
    BIT3  // UefiSpecificationSupported
  },
  (UINT8) ((FixedPcdGet32 (PcdFirmwareRevision) >> 4) & 0xF), /* System BIOS major revision */
  (UINT8) ((FixedPcdGet32 (PcdFirmwareRevision) >> 0) & 0xF), /* System BIOS minor revision */
  0xFF,
  0xFF,
  {}
};

STATIC CHAR8 *SmbiosTable0Strings[] = {
  FixedPcdGetPtr (PcdFirmwareVendor),
  FixedPcdGetPtr (PcdFirmwareVersionString),
  (CHAR8[sizeof (BAIKAL_BIOS_DATE)]){}
};
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable0Init (
  VOID
  )
{
  EFI_STATUS   Status;
  CONST UINTN  Year   = TIME_BUILD_YEAR;
  CONST UINTN  Month  = TIME_BUILD_MONTH;
  CONST UINTN  Day    = TIME_BUILD_DAY;
  CONST UINTN  Size1  = StrSize ((CHAR16 *) SmbiosTable0Strings[0]);
  CONST UINTN  Size2  = StrSize ((CHAR16 *) SmbiosTable0Strings[1]);

  *(UINT32 *) &SmbiosTable0.BiosCharacteristics = BIT11 | BIT7;

  CHAR8  *Str = (CHAR8 *) AllocateZeroPool (Size1 + Size2);
  if (Str == NULL) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to allocate memory for SMBIOS table strings, Status = %r\n",
      __FUNCTION__,
      EFI_OUT_OF_RESOURCES
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  UnicodeStrToAsciiStrS ((CHAR16 *) SmbiosTable0Strings[0], Str, Size1);
  UnicodeStrToAsciiStrS ((CHAR16 *) SmbiosTable0Strings[1], Str + Size1, Size2);

  SmbiosTable0Strings[0] = Str;
  SmbiosTable0Strings[1] = Str + Size1;

  AsciiSPrint (
    SmbiosTable0Strings[2],
    sizeof (BAIKAL_BIOS_DATE),
    "%02u/%02u/%04u",
    Month, Day, Year
    );

  Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable0, SmbiosTable0Strings, ARRAY_SIZE (SmbiosTable0Strings));
  FreePool (Str);

  return Status;
}

/* System Information table, type 1 */

#pragma pack(1)
STATIC SMBIOS_TABLE_TYPE1 SmbiosTable1 = {
  {
    SMBIOS_TYPE_SYSTEM_INFORMATION,
    sizeof (SMBIOS_TABLE_TYPE1),
    BAIKAL_SMBIOS_TABLE_HANDLE (1, 0)
  },
  1,
  2,
  0,
  0,
  { 0x21029EC3, 0x82A8, 0x47DC, { 0xA5, 0xBB, 0x5C, 0x00, 0x77, 0xF8, 0xE3, 0x2D }},
  SystemWakeupTypeUnknown,
  0,
  0
};

STATIC CHAR8 *SmbiosTable1Strings[] = {
  (CHAR8[FixedPcdGetSize (PcdFirmwareVendor)]){},
  BaikalModel
};
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable1Init (
  VOID
  )
{
  UnicodeStrToAsciiStrS (
    (CHAR16 *) FixedPcdGetPtr (PcdFirmwareVendor),
    SmbiosTable1Strings[0],
    FixedPcdGetSize (PcdFirmwareVendor)
    );

  SmbiosTable1Strings[1] = BaikalModel;

  return CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable1, SmbiosTable1Strings, ARRAY_SIZE (SmbiosTable1Strings));
}

/* Baseboard Information table, type 2 */

#pragma pack(1)
STATIC SMBIOS_TABLE_TYPE2 SmbiosTable2 = {
  {
    SMBIOS_TYPE_BASEBOARD_INFORMATION,
    sizeof (SMBIOS_TABLE_TYPE2),
    BAIKAL_SMBIOS_TABLE_HANDLE (2, 0)
  },
  1,
  2,
  0,
  0,
  0,
  {},
  0,
  BAIKAL_SMBIOS_TABLE_HANDLE (3, 0),
  BaseBoardTypeMotherBoard,
  1,
  { BAIKAL_SMBIOS_TABLE_HANDLE (4, 0) }
};

STATIC CHAR8 *SmbiosTable2Strings[] = {
  (CHAR8[FixedPcdGetSize (PcdFirmwareVendor)]){},
  BaikalModel
};
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable2Init (
  VOID
  )
{
  UnicodeStrToAsciiStrS (
    (CHAR16 *) FixedPcdGetPtr (PcdFirmwareVendor),
    SmbiosTable2Strings[0],
    FixedPcdGetSize (PcdFirmwareVendor)
    );

  *(UINT8 *)&SmbiosTable2.FeatureFlag = BIT0;

  return CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable2, SmbiosTable2Strings, ARRAY_SIZE (SmbiosTable2Strings));
}

/* System Enclosure or Chassis table, type 3 */

#pragma pack(1)
typedef struct {
  SMBIOS_STRUCTURE     Hdr;
  SMBIOS_TABLE_STRING  Manufacturer;
  UINT8                Type;
  SMBIOS_TABLE_STRING  Version;
  SMBIOS_TABLE_STRING  SerialNumber;
  SMBIOS_TABLE_STRING  AssetTag;
  UINT8                BootupState;
  UINT8                PowerSupplyState;
  UINT8                ThermalState;
  UINT8                SecurityStatus;
  UINT8                OemDefined[4];
  UINT8                Height;
  UINT8                NumberofPowerCords;
  UINT8                ContainedElementCount;
  UINT8                ContainedElementRecordLength;
  SMBIOS_TABLE_STRING  SKUNumber;
} SMBIOS_TABLE_TYPE3_BAIKAL;

STATIC VOID  *SmbiosTable3 = BAIKAL_SMBIOS_TABLE (
  3_BAIKAL,
  BAIKAL_SMBIOS_STRING (""),
  {
    {
      SMBIOS_TYPE_SYSTEM_ENCLOSURE,
      sizeof (SMBIOS_TABLE_TYPE3_BAIKAL),
      BAIKAL_SMBIOS_TABLE_HANDLE (3, 0)
    },
    0,
    MiscChassisTypeUnknown,
    0,
    0,
    0,
    ChassisStateUnknown,
    ChassisStateUnknown,
    ChassisStateUnknown,
    ChassisSecurityStatusNone,
    {},
    0,
    0,
    0,
    0,
    0
  }
);
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable3Init (
  VOID
  )
{
  return CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable3, NULL, 0);
}

/* Processor Information table, type 4 */

#define BAIKAL_PROCESSOR_SOCKET_DESIGNATION  "CPU0"
#define BAIKAL_PROCESSOR_MANUFACTURER        "Baikal Electronics"
#define BAIKAL_PROCESSOR_VERSION             "BE-S1000"
#define BAIKAL_PROCESSOR_CORE_COUNT          48
#define BAIKAL_PROCESSOR_CLUSTER_COUNT       12

#pragma pack(1)
STATIC VOID  *SmbiosTable4 = BAIKAL_SMBIOS_TABLE (
  4,
  BAIKAL_SMBIOS_STRING (BAIKAL_PROCESSOR_SOCKET_DESIGNATION)
  BAIKAL_SMBIOS_STRING (BAIKAL_PROCESSOR_MANUFACTURER)
  BAIKAL_SMBIOS_STRING (BAIKAL_PROCESSOR_VERSION),
  {
    {
      SMBIOS_TYPE_PROCESSOR_INFORMATION,
      sizeof (SMBIOS_TABLE_TYPE4),
      BAIKAL_SMBIOS_TABLE_HANDLE (4, 0)
    },
    1,
    CentralProcessor,
    ProcessorFamilyIndicatorFamily2,
    2,
    {},
    3,
    {},
    0,
    2000,
    2000,
    BIT6 | 1,
    ProcessorUpgradeNone,
    BAIKAL_SMBIOS_TABLE_HANDLE (7, 1),
    BAIKAL_SMBIOS_TABLE_HANDLE (7, 2),
    BAIKAL_SMBIOS_TABLE_HANDLE (7, 3),
    0,
    0,
    0,
    BAIKAL_PROCESSOR_CORE_COUNT,
    BAIKAL_PROCESSOR_CORE_COUNT,
    BAIKAL_PROCESSOR_CORE_COUNT,
    BIT2 | BIT3 | BIT5 | BIT6 | BIT7,
    ProcessorFamilyARMv8,
    BAIKAL_PROCESSOR_CORE_COUNT,
    BAIKAL_PROCESSOR_CORE_COUNT,
    BAIKAL_PROCESSOR_CORE_COUNT
  }
);
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable4Init (
  VOID
  )
{
  *(UINT64 *) &((SMBIOS_TABLE_TYPE4 *) SmbiosTable4)->ProcessorId = ArmReadMidr ();
  return CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable4, NULL, 0);
}

/* Cache Information tables, type 7 */

#pragma pack(1)
STATIC VOID  *SmbiosTable7[] = {
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
      64 * BAIKAL_PROCESSOR_CORE_COUNT,
      64 * BAIKAL_PROCESSOR_CORE_COUNT,
      {},
      {},
      0,
      CacheErrorParity,
      CacheTypeInstruction,
      CacheAssociativity4Way,
      64 * BAIKAL_PROCESSOR_CORE_COUNT,
      64 * BAIKAL_PROCESSOR_CORE_COUNT
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
      64 * BAIKAL_PROCESSOR_CORE_COUNT,
      64 * BAIKAL_PROCESSOR_CORE_COUNT,
      {},
      {},
      0,
      CacheErrorMultiBit,
      CacheTypeData,
      CacheAssociativity16Way,
      64 * BAIKAL_PROCESSOR_CORE_COUNT,
      64 * BAIKAL_PROCESSOR_CORE_COUNT
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
      0x281,
      512 * BAIKAL_PROCESSOR_CORE_COUNT,
      512 * BAIKAL_PROCESSOR_CORE_COUNT,
      {},
      {},
      0,
      CacheErrorMultiBit,
      CacheTypeUnified,
      CacheAssociativity8Way,
      512 * BAIKAL_PROCESSOR_CORE_COUNT,
      512 * BAIKAL_PROCESSOR_CORE_COUNT
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    7,
    BAIKAL_SMBIOS_STRING ("L3"),
    {
      {
        SMBIOS_TYPE_CACHE_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE7),
        BAIKAL_SMBIOS_TABLE_HANDLE (7, 3)
      },
      1,
      0x282,
      2048 * BAIKAL_PROCESSOR_CLUSTER_COUNT,
      2048 * BAIKAL_PROCESSOR_CLUSTER_COUNT,
      {},
      {},
      0,
      CacheErrorMultiBit,
      CacheTypeUnified,
      CacheAssociativityUnknown,
      2048 * BAIKAL_PROCESSOR_CLUSTER_COUNT,
      2048 * BAIKAL_PROCESSOR_CLUSTER_COUNT,
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    7,
    BAIKAL_SMBIOS_STRING ("L4"),
    {
      {
        SMBIOS_TYPE_CACHE_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE7),
        BAIKAL_SMBIOS_TABLE_HANDLE (7, 4)
      },
      1,
      0x2A3,
      32 * 1024,
      32 * 1024,
      {},
      {},
      0,
      CacheErrorMultiBit,
      CacheTypeUnified,
      CacheAssociativityUnknown,
      32 * 1024,
      32 * 1024
    }
  )
};
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable7Init (
  VOID
  )
{
  UINTN       Idx;
  EFI_STATUS  Status;

  for (Idx = 0; Idx < ARRAY_SIZE (SmbiosTable7); ++Idx) {
    *(UINT16 *) &((SMBIOS_TABLE_TYPE7 *) SmbiosTable7[Idx])->SupportedSRAMType = BIT1;
    *(UINT16 *) &((SMBIOS_TABLE_TYPE7 *) SmbiosTable7[Idx])->CurrentSRAMType = BIT1;
    Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable7[Idx], NULL, 0);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return Status;
}

/* Port Connector Information tables, type 8 */

#pragma pack(1)
STATIC VOID  *SmbiosTable8[] = {
  BAIKAL_SMBIOS_TABLE (
    8,
    BAIKAL_SMBIOS_STRING ("USB2"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("ETH0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("ETH1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("GPIO8_0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("GPIO8_1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("GPIO16"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("GPIO32"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("I2C_0/SMBUS_0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("I2C_1/SMBUS_1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("I2C_2/SMBUS_2"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("I2C_3/SMBUS_3"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("I2C_4/SMBUS_4"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("UART_0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("UART_1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("UART_2"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("QSPI_0"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
    BAIKAL_SMBIOS_STRING ("QSPI_1"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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
        0
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
    BAIKAL_SMBIOS_STRING ("SW/JTAG"),
    {
      {
        SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,
        sizeof (SMBIOS_TABLE_TYPE8),
        0
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

STATIC
EFI_STATUS
SmbiosTable8Init (
  VOID
  )
{
  UINTN       Idx;
  UINT16      Num = 0;
  EFI_STATUS  Status;

  for (Idx = 0; Idx < ARRAY_SIZE (SmbiosTable8); ++Idx) {
    ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable8[Idx])->Handle = BAIKAL_SMBIOS_TABLE_HANDLE (8, Num++);
    Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable8[Idx], NULL, 0);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return Status;
}

/* System Slots tables, type 9 */

#pragma pack(1)
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

STATIC VOID  *SmbiosTable9[] = {
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x16_0"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth16X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth16X,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x16_1"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth16X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth16X,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x8_0"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth8X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth8X,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x8_1"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth8X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth8X,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x8_2"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth8X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth8X,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x8_3"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth8X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth8X,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x8_4"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth8X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth8X,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x4_0"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth4X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth4X,
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
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth4X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth4X,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x4_2"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth4X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth4X,
      0
    }
  ),
  BAIKAL_SMBIOS_TABLE (
    9_BAIKAL,
    BAIKAL_SMBIOS_STRING ("PCIe x4_3"),
    {
      {
        SMBIOS_TYPE_SYSTEM_SLOTS,
        sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
        0
      },
      1,
      SlotTypePciExpressGen4,
      SlotDataBusWidth4X,
      SlotUsageInUse,
      SlotLengthUnknown,
      0,
      {},
      {},
      0,
      0,
      0,
      SlotDataBusWidth4X,
      0
    }
  )
};
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable9Init (
  VOID
  )
{
  UINTN       Idx;
  UINT16      Num = 0;
  EFI_STATUS  Status;

  for (Idx = 0; Idx < ARRAY_SIZE (SmbiosTable9); ++Idx) {
    *(UINT8 *) &((SMBIOS_TABLE_TYPE9_BAIKAL *) SmbiosTable9[Idx])->SlotID = Num;
    *(UINT8 *) &((SMBIOS_TABLE_TYPE9_BAIKAL *) SmbiosTable9[Idx])->SegmentGroupNum = Num;
    ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable9[Idx])->Handle = BAIKAL_SMBIOS_TABLE_HANDLE (9, Num++);
    *(UINT8 *) &((SMBIOS_TABLE_TYPE9_BAIKAL *) SmbiosTable9[Idx])->SlotCharacteristics1 = BIT2;
    *(UINT8 *) &((SMBIOS_TABLE_TYPE9_BAIKAL *) SmbiosTable9[Idx])->SlotCharacteristics2 = BIT1;

    Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable9[Idx], NULL, 0);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return Status;
}

/* System Boot Information table, type 32 */

#pragma pack(1)
STATIC VOID  *SmbiosTable32 = BAIKAL_SMBIOS_TABLE (
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
);
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable32Init (
  VOID
  )
{
  return CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable32, NULL, 0);
}

typedef
EFI_STATUS
(*BAIKAL_SMBIOS_INIT_FUNCTION) (
  VOID
  );

STATIC BAIKAL_SMBIOS_INIT_FUNCTION SmbiosTableInit[] = {
  &SmbiosTable0Init,
  &SmbiosTable1Init,
  &SmbiosTable2Init,
  &SmbiosTable3Init,
  &SmbiosTable4Init,
  &SmbiosTable7Init,
  &SmbiosTable8Init,
  &SmbiosTable9Init,
  &SmbiosTable32Init
};

EFI_STATUS
EFIAPI
SmbiosPlatformDxeInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN       Idx;
  EFI_STATUS  Status;

  for (Idx = 0; Idx < ARRAY_SIZE (SmbiosTableInit); ++Idx) {
    Status = (*SmbiosTableInit[Idx]) ();
    if (EFI_ERROR (Status)) {
      continue;
    }
  }

  return EFI_SUCCESS;
}
