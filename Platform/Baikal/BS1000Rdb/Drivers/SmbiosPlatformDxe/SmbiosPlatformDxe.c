/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/BaikalMemoryRangeLib.h>
#include <Library/BaikalSmbiosLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CrcLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>
#include <Protocol/Smbios.h>
#include <Protocol/SpdClient.h>

#include <BS1000.h>

#ifdef BAIKAL_MBS_1S
STATIC UINT8  PcieSlots[] = {
  SlotTypePciExpressGen4,
  SlotTypePciExpressGen4,
  SlotTypePciExpressGen4,
  SlotTypePciExpressGen4,
  SlotTypePciExpressGen4,
  SlotTypeM2Socket3,
  SlotTypePciExpressGen4,
  SlotTypeOther,
  SlotTypePciExpressGen4
};
#elif defined(BAIKAL_MBS_2S)
STATIC UINT8  PcieSlots[] = {
  /* Chip 0 */
  SlotTypePciExpressGen4,
  SlotTypePciExpressGen4,
  SlotTypeM2Socket3,
  SlotTypeM2Socket3,
  SlotTypeOther,
  SlotTypePciExpressGen4,
  /* Chip 1 */
  SlotTypePciExpressGen4,
  SlotTypePciExpressGen4,
  SlotTypePciExpressGen4X16,
  SlotTypePciExpressGen4X16
};
#else
STATIC UINT8  PcieSlots[1] = {
  SlotTypePciExpressGen4
};
#endif

STATIC CHAR8  BaikalModel[] = BAIKAL_BOARD_NAME;

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
  IN  CONST CHAR8 * CONST       *StringPack,
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
      __func__,
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
        __func__,
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
      __func__,
      Status
      ));
  }

  if (StringPack != NULL) {
    FreePool (Record);
  }

  return Status;
}

// BIOS Information table, type 0

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
  (UINT8) (FixedPcdGet32 (PcdFdSize) - 1) / SIZE_64KB, // BIOS ROM size
  {},
  {
    BIT0, // AcpiIsSupported
    BIT3  // UefiSpecificationSupported
  },
  (UINT8) ((FixedPcdGet32 (PcdFirmwareRevision) >> 4) & 0xF), // System BIOS major revision
  (UINT8) ((FixedPcdGet32 (PcdFirmwareRevision) >> 0) & 0xF), // System BIOS minor revision
  0xFF,
  0xFF,
  {}
};

STATIC CHAR8         BiosDate[sizeof (BAIKAL_BIOS_DATE)];
STATIC CONST CHAR8  *SmbiosTable0Strings[] = {
  NULL,
  NULL,
  BiosDate
};
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable0Init (
  VOID
  )
{
  CONST UINTN  Size1 = StrSize (FixedPcdGetPtr (PcdFirmwareVendor));
  CONST UINTN  Size2 = StrSize (FixedPcdGetPtr (PcdFirmwareVersionString));
  EFI_STATUS   Status;
  CHAR8       *Str1;
  CHAR8       *Str2;

  *(UINT32 *) &SmbiosTable0.BiosCharacteristics = BIT11 | BIT7;

  Str1 = AllocateZeroPool (Size1);
  if (Str1 == NULL) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to allocate memory for SMBIOS table strings, Status = %r\n",
      __func__,
      EFI_OUT_OF_RESOURCES
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  Str2 = AllocateZeroPool (Size2);
  if (Str2 == NULL) {
    FreePool (Str1);
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to allocate memory for SMBIOS table strings, Status = %r\n",
      __func__,
      EFI_OUT_OF_RESOURCES
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  UnicodeStrToAsciiStrS (FixedPcdGetPtr (PcdFirmwareVendor), Str1, Size1);
  UnicodeStrToAsciiStrS (FixedPcdGetPtr (PcdFirmwareVersionString), Str2, Size2);

  SmbiosTable0Strings[0] = Str1;
  SmbiosTable0Strings[1] = Str2;

  AsciiSPrint (
    BiosDate,
    sizeof (BAIKAL_BIOS_DATE),
    "%02u/%02u/%04u",
    TIME_BUILD_MONTH, TIME_BUILD_DAY, TIME_BUILD_YEAR
    );

  Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable0, SmbiosTable0Strings, ARRAY_SIZE (SmbiosTable0Strings));
  FreePool (Str1);
  FreePool (Str2);

  return Status;
}

// System Information table, type 1

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

STATIC CONST CHAR8  *SmbiosTable1Strings[] = {
  NULL,
  BaikalModel
};
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable1Init (
  VOID
  )
{
  CONST UINTN  Size = StrSize (FixedPcdGetPtr (PcdFirmwareVendor));
  EFI_STATUS   Status;
  CHAR8       *Str;

  Str = AllocateZeroPool (Size);
  if (Str == NULL) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to allocate memory for SMBIOS table strings, Status = %r\n",
      __func__,
      EFI_OUT_OF_RESOURCES
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  UnicodeStrToAsciiStrS (FixedPcdGetPtr (PcdFirmwareVendor), Str, Size);
  SmbiosTable1Strings[0] = Str;

  Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable1, SmbiosTable1Strings, ARRAY_SIZE (SmbiosTable1Strings));
  FreePool (Str);

  return Status;
}

// Baseboard Information table, type 2

#pragma pack(1)
STATIC SMBIOS_TABLE_TYPE2 SmbiosTable2 = {
  {
    SMBIOS_TYPE_BASEBOARD_INFORMATION,
    0xF,
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
  0,
  {}
};

STATIC CONST CHAR8  *SmbiosTable2Strings[] = {
  NULL,
  BaikalModel
};
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable2Init (
  VOID
  )
{
  CONST UINTN  Size = StrSize (FixedPcdGetPtr (PcdFirmwareVendor));
  EFI_STATUS   Status;
  CHAR8       *Str;

  *(UINT8 *)&SmbiosTable2.FeatureFlag = BIT0;

  Str = AllocateZeroPool (Size);
  if (Str == NULL) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to allocate memory for SMBIOS table strings, Status = %r\n",
      __func__,
      EFI_OUT_OF_RESOURCES
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  UnicodeStrToAsciiStrS (FixedPcdGetPtr (PcdFirmwareVendor), Str, Size);
  SmbiosTable2Strings[0] = Str;

  Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable2, SmbiosTable2Strings, ARRAY_SIZE (SmbiosTable2Strings));
  FreePool (Str);

  return Status;
}

// System Enclosure or Chassis table, type 3

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

// Processor Information table, type 4

#define BAIKAL_PROCESSOR_MANUFACTURER        "Baikal Electronics"
#define BAIKAL_PROCESSOR_VERSION             "BE-S1000"
#define BAIKAL_PROCESSOR_CORE_COUNT          BS1000_CORE_COUNT
#define BAIKAL_PROCESSOR_CLUSTER_COUNT       BS1000_CLUSTER_COUNT

#pragma pack(1)
STATIC SMBIOS_TABLE_TYPE4 SmbiosTable4 = {
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
};

STATIC CHAR8         SmbiosTable4SocketDesignation[] = "CPU0";
STATIC CONST CHAR8  *SmbiosTable4Strings[] = {
  SmbiosTable4SocketDesignation,
  BAIKAL_PROCESSOR_MANUFACTURER,
  BAIKAL_PROCESSOR_VERSION
};
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable4Init (
  VOID
  )
{
  UINTN       ArmSmcParam;
  UINTN       ChipIdx;
  EFI_STATUS  Status;

  ArmSmcParam = SMCCC_ARCH_SOC_ID;
  if (ArmCallSmc1 (SMCCC_ARCH_FEATURES, &ArmSmcParam, NULL, NULL) == SMC_ARCH_CALL_SUCCESS) {
    INT32  Jep106Code;
    INT32  SocRevision;

    ArmSmcParam = 0;
    Jep106Code  = ArmCallSmc1 (SMCCC_ARCH_SOC_ID, &ArmSmcParam, NULL, NULL);
    ArmSmcParam = 1;
    SocRevision = ArmCallSmc1 (SMCCC_ARCH_SOC_ID, &ArmSmcParam, NULL, NULL);

    ((PROCESSOR_CHARACTERISTIC_FLAGS *) &SmbiosTable4.ProcessorCharacteristics)->ProcessorArm64SocId = 1;
    *(UINT64 *) &SmbiosTable4.ProcessorId = ((UINT64)SocRevision << 32) | Jep106Code;
  } else {
    ((PROCESSOR_CHARACTERISTIC_FLAGS *) &SmbiosTable4.ProcessorCharacteristics)->ProcessorArm64SocId = 0;
    *(UINT64 *) &SmbiosTable4.ProcessorId = ArmReadMidr ();
  }

  for (ChipIdx = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable4)->Handle = BAIKAL_SMBIOS_TABLE_HANDLE (4, ChipIdx);
    *(UINT16 *) &SmbiosTable4.L1CacheHandle = BAIKAL_SMBIOS_TABLE_HANDLE (7, ChipIdx * 5 + 1);
    *(UINT16 *) &SmbiosTable4.L2CacheHandle = BAIKAL_SMBIOS_TABLE_HANDLE (7, ChipIdx * 5 + 2);
    *(UINT16 *) &SmbiosTable4.L3CacheHandle = BAIKAL_SMBIOS_TABLE_HANDLE (7, ChipIdx * 5 + 3);
    SmbiosTable4SocketDesignation[3] += ChipIdx;
    Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable4,
                                SmbiosTable4Strings, ARRAY_SIZE (SmbiosTable4Strings));
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return Status;
}

// Cache Information tables, type 7

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
  UINTN       ChipIdx;
  UINTN       Idx;
  UINT16      Num;
  EFI_STATUS  Status;

  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < ARRAY_SIZE (SmbiosTable7); ++Idx, ++Num) {
      ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable7[Idx])->Handle = BAIKAL_SMBIOS_TABLE_HANDLE (7, Num);
      *(UINT16 *) &((SMBIOS_TABLE_TYPE7 *) SmbiosTable7[Idx])->SupportedSRAMType = BIT1;
      *(UINT16 *) &((SMBIOS_TABLE_TYPE7 *) SmbiosTable7[Idx])->CurrentSRAMType = BIT1;
      Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable7[Idx], NULL, 0);
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
  }

  return Status;
}

// Port Connector Information tables, type 8

STATIC UINT8  SmbiosTable8Idx;
STATIC SMBIOS_TABLE_TYPE8  SmbiosTable8 = {
  {
    8,
    9,
    0
  },
  0,
  0,
  1,
  0,
  0
};

STATIC
VOID
SmbiosTable8Add (
  IN  FDT_CLIENT_PROTOCOL  *FdtClient,
  IN  CONST CHAR8          *Compatible,
  IN  CONST CHAR8          *PropString,
  IN  UINT64                Value,
  IN  CONST CHAR8          *Name,
  IN  CONST CHAR8          *Str,
  IN  UINT8                 ConnectorType,
  IN  UINT8                 PortType
  )
{
  UINT32        Addr;
  BOOLEAN       IsAdd;
  INT32         Node;
  CONST VOID   *Prop;
  INT32         PrevNode;
  UINT32        PropSize;
  EFI_STATUS    Status;
  CHAR8        *Str2;
  CONST CHAR8  *StrPtr;

  if (!FdtClient) {
    IsAdd = TRUE;
  } else {
    IsAdd = FALSE;
    PrevNode = -1;
    while (FdtClient) {
      Status = FdtClient->FindNextCompatibleNode (FdtClient, Compatible, PrevNode, &Node);
      if (EFI_ERROR (Status)) {
        break;
      }
      PrevNode = Node;

      Status = FdtClient->GetNodeProperty (FdtClient, Node, "status", &Prop, &PropSize);
      if (EFI_ERROR (Status)) {
        continue;
      }

      if (PropSize < 5 || AsciiStrCmp (Prop, "okay")) {
        continue;
      }

      Status = FdtClient->GetNodeProperty (FdtClient, Node, PropString, &Prop, &PropSize);
      if (EFI_ERROR (Status)) {
        continue;
      }

      if (PropSize >= sizeof (UINT64)) {
        Addr = SwapBytes64 (ReadUnaligned64 (Prop));
      } else {
        continue;
      }

      if (Value < 0x10) {
        Addr &= 0xF;
      }

      if (Addr == Value) {
        IsAdd = TRUE;
        break;
      }
    }
  }

  if (IsAdd) {
    SmbiosTable8.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (8, SmbiosTable8Idx++);
    SmbiosTable8.ExternalConnectorType = ConnectorType;
    SmbiosTable8.PortType = PortType;

    if (Str) {
      Str2 = AllocatePool (AsciiStrLen(Name) + AsciiStrSize(Str));
      if (Str2 == NULL) {
        DEBUG ((
          EFI_D_ERROR,
          "%a: failed to allocate memory for SMBIOS table strings, Status = %r\n",
          __func__,
          EFI_OUT_OF_RESOURCES
          ));
        return;
      }

      CopyMem (Str2, Name, AsciiStrLen(Name));
      CopyMem (Str2 + AsciiStrLen(Name), Str, AsciiStrSize(Str));
      StrPtr = Str2;
    } else {
      Str2 = NULL;
      StrPtr = Name;
    }

    CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable8, &StrPtr, 1);
    if (Str2) {
      FreePool (Str2);
    }
  }

  return;
}

STATIC
EFI_STATUS
SmbiosTable8Init (
  VOID
  )
{
  UINT8                 ChipIdx;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  EFI_STATUS            Status;
  CHAR8                 Str[] = " (Chip 0)";
  CONST CHAR8          *StrPtr;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FdtClientProtocol, Status: %r\n", __func__, Status));
    return Status;
  }

  if (PLATFORM_CHIP_COUNT > 1) {
    StrPtr = Str;
  } else {
    StrPtr = NULL;
  }

  for (ChipIdx = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    SmbiosTable8Add (FdtClient, "generic-ehci", "reg", PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_EHCI_BASE), "USB 2.0", StrPtr, PortConnectorTypeUsb, PortTypeUsb);
    SmbiosTable8Add (FdtClient, "baikal,bs1000-gmac", "reg", PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_GMAC0_BASE), "GMAC0", StrPtr, PortConnectorTypeRJ45, PortTypeNetworkPort);
    SmbiosTable8Add (FdtClient, "baikal,bs1000-gmac", "reg", PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_GMAC1_BASE), "GMAC1", StrPtr, PortConnectorTypeRJ45, PortTypeNetworkPort);
    SmbiosTable8Add (FdtClient, "baikal,bs1000-lsp-mux-channel", "mux-controls", ChipIdx << 2, "GPIO8_1/UART_S + SMBUS_I2C5 + SMBUS_I2C6", StrPtr, PortConnectorTypeOther, PortTypeOther);
    SmbiosTable8Add (FdtClient, "baikal,bs1000-lsp-mux-channel", "mux-controls", (ChipIdx << 2) | 1, "GPIO8_2/QSPI2", StrPtr, PortConnectorTypeOther, PortTypeOther);
    SmbiosTable8Add (FdtClient, "baikal,bs1000-lsp-mux-channel", "mux-controls", (ChipIdx << 2) | 2, "GPIO16/eSPI", StrPtr, PortConnectorTypeOther, PortTypeOther);
    SmbiosTable8Add (FdtClient, "snps,dw-apb-gpio", "reg", PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_GPIO32_BASE), "GPIO32", StrPtr, PortConnectorTypeOther, PortTypeOther);
    SmbiosTable8Add (FdtClient, "snps,designware-i2c", "reg", PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_I2C2_BASE), "SMBUS_I2C2", StrPtr, PortConnectorTypeOther, PortTypeOther);
    SmbiosTable8Add (FdtClient, "snps,designware-i2c", "reg", PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_I2C3_BASE), "SMBUS_I2C3", StrPtr, PortConnectorTypeOther, PortTypeOther);
    SmbiosTable8Add (FdtClient, "snps,designware-i2c", "reg", PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_I2C4_BASE), "SMBUS_I2C4", StrPtr, PortConnectorTypeOther, PortTypeOther);
    SmbiosTable8Add (FdtClient, "snps,dw-apb-ssi", "reg", PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_QSPI1_BASE), "QSPI1", StrPtr, PortConnectorTypeOther, PortTypeOther);
    SmbiosTable8Add (FdtClient, "arm,pl011", "reg", PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_UART_A1_BASE), "UART_A1", StrPtr, PortConnectorTypeOther, PortTypeOther);
    SmbiosTable8Add (FdtClient, "arm,pl011", "reg", PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_UART_A2_BASE), "UART_A2", StrPtr, PortConnectorTypeOther, PortTypeOther);
    SmbiosTable8Add (NULL, NULL, NULL, 0, "SW/JTAG", StrPtr, PortConnectorTypeOther, PortTypeOther);

    if (Str) {
      ++Str[7];
    }
  }

  return EFI_SUCCESS;
}

// System Slots tables, type 9

STATIC UINT8  SmbiosTable9Idx;
STATIC UINT8  SmbiosTable9Pcie4xIdx;
STATIC UINT8  SmbiosTable9Pcie8xIdx;
STATIC UINT8  SmbiosTable9Pcie16xIdx;

#pragma pack(1)
typedef struct {
  SMBIOS_STRUCTURE     Hdr;
  SMBIOS_TABLE_STRING  SlotDesignation;
  UINT8                SlotType;
  UINT8                SlotDataBusWidth;
  UINT8                CurrentUsage;
  UINT8                SlotLength;
  UINT16               SlotID;
  UINT8                SlotCharacteristics1;
  UINT8                SlotCharacteristics2;
  UINT16               SegmentGroupNum;
  UINT8                BusNum;
  UINT8                DevFuncNum;
  UINT8                DataBusWidth;
  UINT8                PeerGroupingCount;
} SMBIOS_TABLE_TYPE9_BAIKAL;

STATIC SMBIOS_TABLE_TYPE9_BAIKAL  SmbiosTable9 = {
  {
    9,
    sizeof (SMBIOS_TABLE_TYPE9_BAIKAL),
    0
  },
  1,
  0,
  0,
  SlotUsageInUse,
  SlotLengthUnknown,
  0,
  BIT2,
  BIT1,
  0,
  0,
  0,
  0,
  0
};
#pragma pack()

STATIC
BOOLEAN
SmbiosTable9IsValidAddr (
  IN  UINT8   ChipIdx,
  IN  UINT64  Addr
  )
{
  if (Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE0_P0_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE0_P1_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE1_P0_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE1_P1_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE2_P0_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE2_P1_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE3_P0_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE3_P1_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE3_P2_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE3_P3_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE4_P0_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE4_P1_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE4_P2_DBI_BASE) ||
      Addr == PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_PCIE4_P3_DBI_BASE)) {
    return TRUE;
  }

  return FALSE;
}

STATIC
EFI_STATUS
SmbiosTable9Init (
  VOID
  )
{
  UINT64                Addr;
  UINT32                ChipIdx;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  CHAR8                 Name[] = "PCIe xXXXXX";
  INT32                 Node;
  UINT32                NumLanes;
  CONST VOID           *Prop;
  INT32                 PrevNode;
  UINT32                PropSize;
  EFI_STATUS            Status;
  CHAR8                 Str[] = " (Chip 0)";
  CHAR8                *Str2;
  CONST CHAR8          *StrPtr;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FdtClientProtocol, Status: %r\n", __func__, Status));
    return Status;
  }

  PrevNode = -1;
  while (1) {
    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bs1000-pcie", PrevNode, &Node);
    if (EFI_ERROR (Status)) {
      break;
    }
    PrevNode = Node;

    Status = FdtClient->GetNodeProperty (FdtClient, Node, "status", &Prop, &PropSize);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (PropSize < 5 || AsciiStrCmp (Prop, "okay")) {
      continue;
    }

    Status = FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (PropSize >= sizeof (UINT64)) {
      Addr = SwapBytes64 (ReadUnaligned64 (Prop));
    } else {
      continue;
    }

    Status = FdtClient->GetNodeProperty (FdtClient, Node, "numa-node-id", &Prop, &PropSize);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (PropSize >= sizeof (UINT32)) {
      ChipIdx = SwapBytes32 (ReadUnaligned32 (Prop));
    } else {
      continue;
    }

    Status = FdtClient->GetNodeProperty (FdtClient, Node, "num-lanes", &Prop, &PropSize);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (PropSize >= sizeof (UINT32)) {
      NumLanes = SwapBytes32 (ReadUnaligned32 (Prop));
    } else {
      continue;
    }

    if (SmbiosTable9IsValidAddr(ChipIdx, Addr)) {
      if (NumLanes == 4) {
        Name[6] = '4';
        Name[7] = '_';
        if (SmbiosTable9Pcie4xIdx > 9) {
          Name[8] = '0' + (SmbiosTable9Pcie4xIdx / 10);
          Name[9] = '0' + (SmbiosTable9Pcie4xIdx % 10);
          Name[10] = 0;
        } else {
          Name[8] = '0' + SmbiosTable9Pcie4xIdx;
          Name[9] = 0;
        }
        ++SmbiosTable9Pcie4xIdx;
        SmbiosTable9.SlotDataBusWidth = SlotDataBusWidth4X;
      } else if (NumLanes == 8) {
        Name[6] = '8';
        Name[7] = '_';
        if (SmbiosTable9Pcie8xIdx > 9) {
          Name[8] = '0' + (SmbiosTable9Pcie8xIdx / 10);
          Name[9] = '0' + (SmbiosTable9Pcie8xIdx % 10);
          Name[10] = 0;
        } else {
          Name[8] = '0' + SmbiosTable9Pcie8xIdx;
          Name[9] = 0;
        }
        ++SmbiosTable9Pcie8xIdx;
        SmbiosTable9.SlotDataBusWidth = SlotDataBusWidth8X;
      } else if (NumLanes == 16) {
        Name[6] = '1';
        Name[7] = '6';
        Name[8] = '_';
        if (SmbiosTable9Pcie16xIdx > 9) {
          Name[9] = '0' + (SmbiosTable9Pcie16xIdx / 10);
          Name[10] = '0' + (SmbiosTable9Pcie16xIdx % 10);
        } else {
          Name[9] = '0' + SmbiosTable9Pcie16xIdx;
          Name[10] = 0;
        }
        ++SmbiosTable9Pcie16xIdx;
        SmbiosTable9.SlotDataBusWidth = SlotDataBusWidth16X;
      } else {
        continue;
      }

      SmbiosTable9.DataBusWidth = SmbiosTable9.SlotDataBusWidth;
      SmbiosTable9.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (9, SmbiosTable9Idx);
      SmbiosTable9.SlotType = SmbiosTable9Idx < ARRAY_SIZE (PcieSlots) ? PcieSlots[SmbiosTable9Idx] : SlotTypePciExpressGen4;
      SmbiosTable9.SlotID = SmbiosTable9.SegmentGroupNum = SmbiosTable9Idx++;

      if (PLATFORM_CHIP_COUNT > 1) {
        Str2 = AllocatePool (AsciiStrLen(Name) + AsciiStrSize(Str));
        if (Str2 == NULL) {
          DEBUG ((
            EFI_D_ERROR,
            "%a: failed to allocate memory for SMBIOS table strings, Status = %r\n",
            __func__,
            EFI_OUT_OF_RESOURCES
            ));
          return EFI_OUT_OF_RESOURCES;
        }

        Str[7] = '0' + ChipIdx;
        CopyMem (Str2, Name, AsciiStrLen(Name));
        CopyMem (Str2 + AsciiStrLen(Name), Str, AsciiStrSize(Str));
        StrPtr = Str2;
      } else {
        Str2 = NULL;
        StrPtr = Name;
      }

      CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable9, &StrPtr, 1);
      if (Str2) {
        FreePool (Str2);
      }
    }
  }

  return EFI_SUCCESS;
}

// Physical Memory Array table, type 16
// Memory Device tables, type 17
// Memory Array Mapped Address tables, type 19
// Memory Device Mapped Address tables, type 20

#pragma pack(1)
STATIC VOID  *SmbiosTable16 = BAIKAL_SMBIOS_TABLE (
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
    0,
    SMBIOS_HANDLE_PI_RESERVED,
    0,
    0
  }
);

STATIC SMBIOS_TABLE_TYPE17  SmbiosTable17 = {
  .Hdr = {
    SMBIOS_TYPE_MEMORY_DEVICE,
    sizeof (SMBIOS_TABLE_TYPE17)
  },
  .MemoryArrayHandle = BAIKAL_SMBIOS_TABLE_HANDLE (16, 0),
  .MemoryErrorInformationHandle = SMBIOS_HANDLE_PI_RESERVED,
  .FormFactor = MemoryFormFactorDimm,
  .DeviceLocator = 1,
  .BankLocator = 2,
  .MemoryType = MemoryTypeDdr4,
  .TypeDetail = {
    .Synchronous = 1
  },
  .MemoryTechnology = MemoryTechnologyDram,
  .MemoryOperatingModeCapability = {
    .Bits = {
      .VolatileMemory = 1
    }
  },
};

STATIC VOID  *SmbiosTable19 = BAIKAL_SMBIOS_TABLE (
  19,
  BAIKAL_SMBIOS_STRING (""),
  {
    {
      SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS,
      sizeof (SMBIOS_TABLE_TYPE19),
      BAIKAL_SMBIOS_TABLE_HANDLE (19, 0)
    },
    0,
    0,
    BAIKAL_SMBIOS_TABLE_HANDLE (16, 0),
    0,
    0,
    0
  }
);
#pragma pack()

STATIC SPD_CLIENT_PROTOCOL  *SpdClient;

STATIC
UINT16
SpdGetCrc16 (
  IN  CONST UINT8 * CONST  SpdBuf
  )
{
  return SpdBuf[0] | (SpdBuf[1] << 8);
}

STATIC
INTN
SpdIsValid (
  IN  CONST UINT8 * CONST  SpdBuf,
  IN  CONST UINTN          Size
  )
{
  INTN  Result = 0;

  if (!SpdBuf || Size < 128) {
    return 0;
  }

  if (Size >= 128) {
    Result = SpdGetCrc16 (SpdBuf + 126) == Crc16 (SpdBuf, 126, 0);
  }

  if (Size >= 256) {
    Result &= SpdGetCrc16 (SpdBuf + 254) == Crc16 (SpdBuf + 128, 126, 0);
  }

  if (Size >= 320) {
    Result &= SpdGetCrc16 (SpdBuf + 318) == Crc16 (SpdBuf + 256, 62, 0);
  }

  return Result;
}

STATIC
EFI_STATUS
GetDdrInfo (
  OUT  BAIKAL_SMBIOS_DDR_INFO * CONST  DdrInfo,
  IN   CONST UINTN                     ChipIdx,
  IN   CONST INTN                      Num
  )
{
  CONST UINT8  *Spd;
  INTN          SpdSize;

  Spd = SpdClient->GetData (ChipIdx, Num);

  switch (*Spd & 0x07) {
  case 1:
    SpdSize = 128;
    break;
  case 2:
    SpdSize = 256;
    break;
  case 3:
    SpdSize = 384;
    break;
  case 4:
    SpdSize = 512;
    break;
  default:
    SpdSize = 0;
    break;
  }

  if (SpdSize == 0) {
    return EFI_NOT_FOUND;
  }

  if (!SpdIsValid (Spd, SpdSize)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: DDR4 DIMM%d SPD has invalid CRC\n",
      __func__,
      BS1000_DIMM_COUNT * ChipIdx + Num
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (SmbiosSetDdrInfo (Spd, SpdSize, DdrInfo, NULL, NULL)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: DDR4 DIMM%d SPD info is invalid\n",
      __func__,
      BS1000_DIMM_COUNT * ChipIdx + Num
      ));
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SmbiosTable16_17_19Init (
  VOID
  )
{
  UINTN                    AddressCells;
  CHAR8                    BankLocator[]   = "Bank 00";
  UINT64                   Capacity;
  UINTN                    ChipIdx;
  BAIKAL_SMBIOS_DDR_INFO  *DdrInfo;
  UINT64                   DdrPresence = 0;
  CHAR8                    DeviceLocator[] = "DDR4_CH00";
  UINTN                    Idx;
  INT32                    Node = 0;
  UINTN                    Num;
  CONST CHAR8             *PtrBuf[5];
  CONST UINT32            *Reg;
  UINT64                   RegAddr;
  UINTN                    RegAmount;
  UINT64                   RegSize;
  UINTN                    SizeCells;
  EFI_STATUS               Status;

  Status = gBS->LocateProtocol (&gSpdClientProtocolGuid, NULL, (VOID **) &SpdClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to locate SpdClientProtocol, Status: %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  // Getting information about DDR
  DdrInfo = (BAIKAL_SMBIOS_DDR_INFO *) AllocateZeroPool (PLATFORM_CHIP_COUNT * BS1000_DIMM_COUNT * sizeof (BAIKAL_SMBIOS_DDR_INFO));
  if (DdrInfo == NULL) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to allocate memory for SPD info structure, Status = %r\n",
      __func__,
      EFI_OUT_OF_RESOURCES
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_DIMM_COUNT; ++Idx, ++Num) {
      Status = GetDdrInfo (DdrInfo + Num, ChipIdx, Idx);
      if (EFI_ERROR (Status)) {
        if (Status == EFI_NOT_FOUND) {
          continue;
        } else {
          FreePool (DdrInfo);
          return Status;
        }
      } else {
        DdrPresence |= 1 << Num;
        ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->PartitionWidth++;
      }
    }
  }

  if (DdrPresence == 0) {
    FreePool (DdrInfo);
    return EFI_INVALID_PARAMETER;
  }

  // Init and load table of type 16
  ((SMBIOS_TABLE_TYPE16 *) SmbiosTable16)->NumberOfMemoryDevices = BS1000_DIMM_COUNT * PLATFORM_CHIP_COUNT;
  Capacity = SIZE_64GB * BS1000_DIMM_COUNT * PLATFORM_CHIP_COUNT;
  if (Capacity < SIZE_2TB) {
    ((SMBIOS_TABLE_TYPE16 *) SmbiosTable16)->MaximumCapacity = Capacity / SIZE_1KB;
  }
  else {
    ((SMBIOS_TABLE_TYPE16 *) SmbiosTable16)->MaximumCapacity = 0x80000000;
    ((SMBIOS_TABLE_TYPE16 *) SmbiosTable16)->ExtendedMaximumCapacity = Capacity;
  }
  Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable16, NULL, 0);
  if (EFI_ERROR (Status)) {
    FreePool (DdrInfo);
    return Status;
  }

  PtrBuf[0] = DeviceLocator;
  PtrBuf[1] = BankLocator;

  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_DIMM_COUNT; ++Idx, ++Num) {
      PtrBuf[2] = PtrBuf[3] = PtrBuf[4] = NULL;
      SmbiosTable17.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (17, Num);

      if ((DdrPresence >> Num) & 1) {
        CHAR8         *Buf;
        UINT8         IsExtendedSize;
        CHAR8         Manufacturer[] = "Bank: 0x00, Id: 0x00";
        CONST CHAR8  *PtrString = NULL;

        IsExtendedSize = DdrInfo[Num].Size >= (SIZE_32GB - 1) ? 1 : 0;

        SmbiosTable17.TotalWidth = DdrInfo[Num].DataWidth + DdrInfo[Num].ExtensionWidth;
        SmbiosTable17.DataWidth = DdrInfo[Num].DataWidth;
        if (IsExtendedSize) {
          SmbiosTable17.ExtendedSize = DdrInfo[Num].Size / SIZE_1MB;
          SmbiosTable17.Size = 0x7FFF;
        } else {
          SmbiosTable17.Size = DdrInfo[Num].Size / SIZE_1MB;
          SmbiosTable17.ExtendedSize = 0;
        }
        SmbiosTable17.Speed = DdrInfo[Num].Speed;

        UINT8 Cc = DdrInfo[Num].ManufacturerId & 0x7F;
        UINT8 Id = (DdrInfo[Num].ManufacturerId >> 8) & 0x7F;
        if (Cc == 5 && Id == 27) {
          PtrString = "Crucial Technology";
        } else if (Cc == 1 && Id == 24) {
          PtrString = "Kingston";
        } else if (Cc || Id) {
          CHAR8  HexValue[3];
          AsciiValueToStringS (HexValue, 3, RADIX_HEX, DdrInfo[Num].ManufacturerId & 0xFF, 2);
          CopyMem (Manufacturer + 8, HexValue, 2);
          AsciiValueToStringS (HexValue, 3, RADIX_HEX, (DdrInfo[Num].ManufacturerId >> 8) & 0xFF, 2);
          CopyMem (Manufacturer + 18, HexValue, 2);
          PtrString = Manufacturer;
        }

        if (PtrString) {
          Buf = AllocatePool (AsciiStrSize (PtrString));
          if (Buf == NULL) {
            FreePool (DdrInfo);
            DEBUG ((
              EFI_D_ERROR,
              "%a: failed to allocate memory, Status = %r\n",
              __func__,
              EFI_OUT_OF_RESOURCES
              ));
            return EFI_OUT_OF_RESOURCES;
          }

          CopyMem (Buf, PtrString, AsciiStrSize (PtrString));
          PtrBuf[2] = Buf;
          SmbiosTable17.Manufacturer = 3;
        } else {
          SmbiosTable17.Manufacturer = 0;
        }

        if (DdrInfo[Num].SerialNumber == 0) {
          SmbiosTable17.SerialNumber = 0;
          SmbiosTable17.PartNumber = SmbiosTable17.Manufacturer == 0 ? 3 : 4;
        } else {
          Buf = AllocatePool (11);
          if (Buf == NULL) {
            FreePool (DdrInfo);
            if (PtrBuf[2]) {
              FreePool ((VOID *) PtrBuf[2]);
            }
            DEBUG ((
              EFI_D_ERROR,
              "%a: failed to allocate memory, Status = %r\n",
              __func__,
              EFI_OUT_OF_RESOURCES
              ));
            return EFI_OUT_OF_RESOURCES;
          }

          AsciiValueToStringS (Buf, 11, 0, DdrInfo[Num].SerialNumber, 10);
          PtrBuf[3] = Buf;
          SmbiosTable17.SerialNumber = SmbiosTable17.Manufacturer == 0 ? 3 : 4;
          SmbiosTable17.PartNumber = SmbiosTable17.Manufacturer == 0 ? 4 : 5;
        }

        if (DdrInfo[Num].PartNumber[0] != '\0') {
          Buf = AllocatePool (sizeof (DdrInfo[Num].PartNumber));
          if (Buf == NULL) {
            FreePool (DdrInfo);
            if (PtrBuf[2]) {
              FreePool ((VOID *) PtrBuf[2]);
            }
            if (PtrBuf[3]) {
              FreePool ((VOID *) PtrBuf[3]);
            }
            DEBUG ((
              EFI_D_ERROR,
              "%a: failed to allocate memory, Status = %r\n",
              __func__,
              EFI_OUT_OF_RESOURCES
              ));
            return EFI_OUT_OF_RESOURCES;
          }

          CopyMem (Buf, DdrInfo[Num].PartNumber, sizeof (DdrInfo[Num].PartNumber));
          PtrBuf[4] = Buf;
        } else {
          SmbiosTable17.PartNumber = 0;
        }

        SmbiosTable17.Attributes = DdrInfo[Num].Rank;
        SmbiosTable17.ConfiguredMemoryClockSpeed = SmbiosTable17.Speed;
        SmbiosTable17.MinimumVoltage = DdrInfo[Num].Voltage;
        SmbiosTable17.MaximumVoltage = DdrInfo[Num].Voltage;
        SmbiosTable17.ConfiguredVoltage = DdrInfo[Num].Voltage;
        SmbiosTable17.ModuleManufacturerID = DdrInfo[Num].ManufacturerId;
        SmbiosTable17.VolatileSize = DdrInfo[Num].Size;

        Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable17, PtrBuf, ARRAY_SIZE (PtrBuf));

        if (PtrBuf[2]) {
          FreePool ((VOID *) PtrBuf[2]);
        }

        if (PtrBuf[3]) {
          FreePool ((VOID *) PtrBuf[3]);
        }

        if (PtrBuf[4]) {
          FreePool ((VOID *) PtrBuf[4]);
        }

        if (EFI_ERROR (Status)) {
          FreePool (DdrInfo);
          return Status;
        }
      } else {
        SmbiosTable17.TotalWidth = 0xFFFF;
        SmbiosTable17.DataWidth = 0xFFFF;
        SmbiosTable17.Size = 0;
        SmbiosTable17.Speed = 0;
        SmbiosTable17.SerialNumber = 0;
        SmbiosTable17.PartNumber = 0;
        SmbiosTable17.Attributes = 0;
        SmbiosTable17.ExtendedSize = 0;
        SmbiosTable17.ConfiguredMemoryClockSpeed = 0;
        SmbiosTable17.MinimumVoltage = 0;
        SmbiosTable17.MaximumVoltage = 0;
        SmbiosTable17.ConfiguredVoltage = 0;
        SmbiosTable17.ModuleManufacturerID = 0;
        SmbiosTable17.ModuleProductID = 0;
        SmbiosTable17.MemorySubsystemControllerManufacturerID = 0;
        SmbiosTable17.MemorySubsystemControllerProductID = 0;
        SmbiosTable17.VolatileSize = 0;

        Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable17, PtrBuf, 2);
        if (EFI_ERROR (Status)) {
          FreePool (DdrInfo);
          return Status;
        }
      }

      if (DeviceLocator[8] == '9') {
        DeviceLocator[7]++;
        DeviceLocator[8] = '0';
      } else {
        DeviceLocator[8]++;
      }
      BankLocator[5] = DeviceLocator[7];
      BankLocator[6] = DeviceLocator[8];
    }
  }

  // Init and load tables of type 19
  Num = 0;
  while ((Status = GetMemoryRanges (&Node, &Reg, &RegAmount, &AddressCells, &SizeCells)) != EFI_NOT_FOUND) {
    if (EFI_ERROR (Status)) {
      FreePool (DdrInfo);
      return Status;
    }

    for (Idx = 0; Idx < RegAmount; ++Idx, ++Num) {
      RegAddr = SwapBytes32 (*Reg++);
      if (AddressCells > 1) {
        RegAddr = (RegAddr << 32) | SwapBytes32 (*Reg++);
      }

      RegSize = SwapBytes32 (*Reg++);
      if (SizeCells > 1) {
        RegSize = (RegSize << 32) | SwapBytes32 (*Reg++);
      }

      ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (19, Num);
      ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->StartingAddress = 0xFFFFFFFF;
      ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->EndingAddress = 0xFFFFFFFF;
      ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->ExtendedStartingAddress = RegAddr;
      ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->ExtendedEndingAddress = RegAddr + RegSize - 1;

      Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable19, NULL, 0);
      if (EFI_ERROR (Status)) {
        FreePool (DdrInfo);
        return Status;
      }
    }
  }

  FreePool (DdrInfo);

  return EFI_SUCCESS;
}

// System Boot Information table, type 32

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
  &SmbiosTable16_17_19Init,
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
