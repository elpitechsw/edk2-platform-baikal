/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/BaikalSmbiosLib.h>
#include <Library/BaikalSpdLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/SmcEfuseLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>
#include <Protocol/FruClient.h>
#include <Protocol/Smbios.h>

#include <BM1000.h>

#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
STATIC UINT8  PcieSlots[] = {
  SlotTypeM2Socket3,
  0,
  SlotTypePciExpressGen3
};
#else
STATIC UINT8  PcieSlots[1] = {
  SlotTypePciExpressGen3
};
#endif

STATIC CHAR8  BaikalModel[] = BAIKAL_BOARD_NAME;

STATIC
INTN
IsMbm (
  VOID
  )
{
#ifndef ELPITECH
  return BaikalModel[0] == 'M' ? 1 : 0;
#else
  return 1;
#endif
}

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

  *(UINT32 *) &SmbiosTable0.BiosCharacteristics = BIT15 | BIT11 | BIT7;

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
  3,
  4,
  { 0x678B81D2, 0x8118, 0x11EB, { 0xB0, 0xDD, 0x00, 0x25, 0x90, 0x87, 0x21, 0x6F }},
  SystemWakeupTypeUnknown,
  0,
  0
};

STATIC CHAR8  *SmbiosTable1FruStrings[] = {
  (CHAR8[FRU_TYPLENSTR_MAX_SIZE]){},
  (CHAR8[FRU_TYPLENSTR_MAX_SIZE]){},
  (CHAR8[FRU_TYPLENSTR_MAX_SIZE]){},
  (CHAR8[FRU_TYPLENSTR_MAX_SIZE]){}
};

STATIC CONST CHAR8  *SmbiosTable1Strings[ARRAY_SIZE (SmbiosTable1FruStrings)];
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable1Init (
  VOID
  )
{
  FRU_CLIENT_PROTOCOL  *FruClient;
  EFI_STATUS            Status;

  Status = gBS->LocateProtocol (&gFruClientProtocolGuid, NULL, (VOID **) &FruClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to locate FruClientProtocol, Status = %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  if (FruClient->ReadProductManufacturer (SmbiosTable1FruStrings[0], FRU_TYPLENSTR_MAX_SIZE) == 0) {
    UnicodeStrToAsciiStrS (
      FixedPcdGetPtr (PcdFirmwareVendor),
      SmbiosTable1FruStrings[0],
      FRU_TYPLENSTR_MAX_SIZE
      );
  }
  SmbiosTable1Strings[0] = SmbiosTable1FruStrings[0];

  if (FruClient->ReadProductName (SmbiosTable1FruStrings[1], FRU_TYPLENSTR_MAX_SIZE) == 0) {
    SmbiosTable1Strings[1] = BaikalModel;
  } else {
    SmbiosTable1Strings[1] = SmbiosTable1FruStrings[1];
  }

  if (FruClient->ReadProductVersion (SmbiosTable1FruStrings[2], FRU_TYPLENSTR_MAX_SIZE) == 0) {
    SmbiosTable1.Version = 0;
    SmbiosTable1.SerialNumber--;
  } else {
    SmbiosTable1Strings[2] = SmbiosTable1FruStrings[2];
  }

  if (FruClient->ReadProductSerialNumber (SmbiosTable1FruStrings[3], FRU_TYPLENSTR_MAX_SIZE) == 0) {
    SmbiosTable1.SerialNumber = 0;
  } else {
    SmbiosTable1Strings[3] = SmbiosTable1FruStrings[3];
  }

  return CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable1, SmbiosTable1Strings, ARRAY_SIZE (SmbiosTable1Strings));
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
  3,
  0,
  {},
  0,
  BAIKAL_SMBIOS_TABLE_HANDLE (3, 0),
  BaseBoardTypeMotherBoard,
  0,
  {}
};

STATIC CHAR8  *SmbiosTable2FruStrings[] = {
  (CHAR8[FRU_TYPLENSTR_MAX_SIZE]){},
  (CHAR8[FRU_TYPLENSTR_MAX_SIZE]){},
  (CHAR8[FRU_TYPLENSTR_MAX_SIZE]){}
};

STATIC CONST CHAR8  *SmbiosTable2Strings[ARRAY_SIZE (SmbiosTable2FruStrings)];
#pragma pack()

STATIC
EFI_STATUS
SmbiosTable2Init (
  VOID
  )
{
  FRU_CLIENT_PROTOCOL  *FruClient;
  EFI_STATUS            Status;

  *(UINT8 *)&SmbiosTable2.FeatureFlag = BIT0;

  Status = gBS->LocateProtocol (&gFruClientProtocolGuid, NULL, (VOID **) &FruClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to locate FruClientProtocol, Status = %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  if (FruClient->ReadBoardManufacturer (SmbiosTable2FruStrings[0], FRU_TYPLENSTR_MAX_SIZE) == 0) {
    SmbiosTable2.Manufacturer = 0;
    SmbiosTable2.ProductName--;
    SmbiosTable2.SerialNumber--;
  } else {
    SmbiosTable2Strings[0] = SmbiosTable2FruStrings[0];
  }

  if (FruClient->ReadBoardName (SmbiosTable2FruStrings[1], FRU_TYPLENSTR_MAX_SIZE) == 0) {
    SmbiosTable2Strings[1] = BaikalModel;
  } else {
    SmbiosTable2Strings[1] = SmbiosTable2FruStrings[1];
  }

  if (FruClient->ReadBoardSerialNumber (SmbiosTable2FruStrings[2], FRU_TYPLENSTR_MAX_SIZE) == 0) {
    SmbiosTable2.SerialNumber = 0;
  } else {
    SmbiosTable2Strings[2] = SmbiosTable2FruStrings[2];
  }

  return CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable2, SmbiosTable2Strings, ARRAY_SIZE (SmbiosTable2Strings));
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

#define BAIKAL_PROCESSOR_SOCKET_DESIGNATION  "CPU0"
#define BAIKAL_PROCESSOR_MANUFACTURER        "Baikal Electronics"
#define BAIKAL_PROCESSOR_VERSION             "BE-M1000"
#define BAIKAL_PROCESSOR_CORE_COUNT          8
#define BAIKAL_PROCESSOR_CLUSTER_COUNT       4
#define BAIKAL_PROCESSOR_SERIAL_STR_SIZE     9
#define BAIKAL_PROCESSOR_PART_STR_SIZE       7

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
  1500,
  1500,
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

STATIC CHAR8         SmbiosTable4Serial[BAIKAL_PROCESSOR_SERIAL_STR_SIZE];
STATIC CHAR8         SmbiosTable4Part[BAIKAL_PROCESSOR_PART_STR_SIZE];
STATIC CONST CHAR8  *SmbiosTable4Strings[] = {
  BAIKAL_PROCESSOR_SOCKET_DESIGNATION,
  BAIKAL_PROCESSOR_MANUFACTURER,
  BAIKAL_PROCESSOR_VERSION,
  SmbiosTable4Serial,
  SmbiosTable4Part
};
#pragma pack()

#define BAIKAL_SMC_CMU_CMD           0xC2000000
#define BAIKAL_SMC_CMU_PLL_GET_RATE  1
#define BM1000_CA57_0_BASE           0x28000000

STATIC
EFI_STATUS
SmbiosTable4Init (
  VOID
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;
  UINTN         ArmSmcParam;
  INTN          PartNumber;
  INTN          SerialNumber;

  ArmSmcArgs.Arg0 = BAIKAL_SMC_CMU_CMD;
  ArmSmcArgs.Arg1 = BM1000_CA57_0_BASE;
  ArmSmcArgs.Arg2 = BAIKAL_SMC_CMU_PLL_GET_RATE;
  ArmSmcArgs.Arg4 = 0;
  ArmCallSmc (&ArmSmcArgs);
  SmbiosTable4.CurrentSpeed = ArmSmcArgs.Arg0 / 1000000;

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

  if (AsciiStrCmp (BaikalModel, "QEMU-M")) {
    SerialNumber = SmcEfuseGetSerial ();
    if (SerialNumber > 0) {
      AsciiSPrint (
        SmbiosTable4Serial,
        BAIKAL_PROCESSOR_SERIAL_STR_SIZE,
        "%d",
        SerialNumber & 0xFFFFFF
        );

      SmbiosTable4.SerialNumber = 4;
    }

    PartNumber = SmcEfuseGetLot ();
    if (PartNumber > 0) {
      AsciiSPrint (
        SmbiosTable4Part,
        BAIKAL_PROCESSOR_PART_STR_SIZE,
        "%c%c%c%c%c%c",
        (PartNumber >>  0) & 0xFF,
        (PartNumber >>  8) & 0xFF,
        (PartNumber >> 16) & 0xFF,
        (PartNumber >> 24) & 0xFF,
        (PartNumber >> 32) & 0xFF,
        (PartNumber >> 40) & 0xFF
        );

      if (SmbiosTable4.SerialNumber == 0) {
        SmbiosTable4.PartNumber = 4;
      } else {
        SmbiosTable4.PartNumber = 5;
      }
    }
  }

  return CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable4, SmbiosTable4Strings, ARRAY_SIZE (SmbiosTable4Strings));
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
      48 * BAIKAL_PROCESSOR_CORE_COUNT,
      48 * BAIKAL_PROCESSOR_CORE_COUNT,
      {},
      {},
      0,
      CacheErrorParity,
      CacheTypeInstruction,
      CacheAssociativityOther,
      48 * BAIKAL_PROCESSOR_CORE_COUNT,
      48 * BAIKAL_PROCESSOR_CORE_COUNT
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
      32 * BAIKAL_PROCESSOR_CORE_COUNT,
      32 * BAIKAL_PROCESSOR_CORE_COUNT,
      {},
      {},
      0,
      CacheErrorSingleBit,
      CacheTypeData,
      CacheAssociativity2Way,
      32 * BAIKAL_PROCESSOR_CORE_COUNT,
      32 * BAIKAL_PROCESSOR_CORE_COUNT
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
      1024 * BAIKAL_PROCESSOR_CLUSTER_COUNT,
      1024 * BAIKAL_PROCESSOR_CLUSTER_COUNT,
      {},
      {},
      0,
      CacheErrorSingleBit,
      CacheTypeUnified,
      CacheAssociativity16Way,
      1024 * BAIKAL_PROCESSOR_CLUSTER_COUNT,
      1024 * BAIKAL_PROCESSOR_CLUSTER_COUNT
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
      0x2A2,
      8192,
      8192,
      {},
      {},
      0,
      CacheErrorSingleBit,
      CacheTypeUnified,
      CacheAssociativityFully,
      8192,
      8192
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

// Port Connector Information tables, type 8

#pragma pack(1)
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
#pragma pack()

STATIC
VOID
SmbiosTable8Add (
  IN  FDT_CLIENT_PROTOCOL  *FdtClient,
  IN  CONST CHAR8          *Compatible,
  IN  CONST CHAR8          *PropString,
  IN  UINT64                Value,
  IN  CONST CHAR8          *Name,
  IN  UINT8                 ConnectorType,
  IN  UINT8                 PortType
  )
{
  UINT32       Addr;
  BOOLEAN      IsAdd;
  INT32        Node;
  CONST VOID  *Prop;
  INT32        PrevNode;
  UINT32       PropSize;
  EFI_STATUS   Status;

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

      if (!AsciiStrCmp (PropString, "reg")) {
        /* LVDS */
        IsAdd = TRUE;
        break;
      } else if (PropSize >= sizeof (UINT64)) {
        Addr = SwapBytes64 (ReadUnaligned64 (Prop));
      } else {
        continue;
      }

      if (Addr == Value) {
        IsAdd = TRUE;
        break;
      }
    }
  }

  if (IsAdd) {
    if (Name) {
      SmbiosTable8.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (8, SmbiosTable8Idx++);
      SmbiosTable8.ExternalConnectorType = ConnectorType;
      SmbiosTable8.PortType = PortType;
      CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable8, &Name, 1);
    } else {
      /* USB */
      CHAR8         UsbName[] = "USB2_0";
      CONST CHAR8  *StrPtr;

      SmbiosTable8.ExternalConnectorType = ConnectorType;
      SmbiosTable8.PortType = PortType;
      StrPtr = UsbName;

      if (Value == BM1000_USB2_BASE) {
        SmbiosTable8.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (8, SmbiosTable8Idx++);
        CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable8, &StrPtr, 1);

        ++UsbName[5];
        SmbiosTable8.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (8, SmbiosTable8Idx++);
        CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable8, &StrPtr, 1);
      }

      if (Value == BM1000_USB3_BASE) {
        UsbName[5] = '2';
        SmbiosTable8.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (8, SmbiosTable8Idx++);
        CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable8, &StrPtr, 1);

        ++UsbName[5];
        SmbiosTable8.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (8, SmbiosTable8Idx++);
        CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable8, &StrPtr, 1);

        ++UsbName[3];
        UsbName[5] = '0';
        SmbiosTable8.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (8, SmbiosTable8Idx++);
        CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable8, &StrPtr, 1);

        ++UsbName[5];
        SmbiosTable8.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (8, SmbiosTable8Idx++);
        CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable8, &StrPtr, 1);
      }
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
  FDT_CLIENT_PROTOCOL  *FdtClient;
  EFI_STATUS            Status;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FdtClientProtocol, Status: %r\n", __func__, Status));
    return Status;
  }

  SmbiosTable8Add (FdtClient, "snps,dwc3", "reg", BM1000_USB2_BASE, NULL, PortConnectorTypeUsb, PortTypeUsb);
  SmbiosTable8Add (FdtClient, "snps,dwc3", "reg", BM1000_USB3_BASE, NULL, PortConnectorTypeUsb, PortTypeUsb);

  SmbiosTable8Add (FdtClient, "baikal,bm1000-gmac", "reg", BM1000_GMAC0_BASE, "GMAC0", PortConnectorTypeRJ45, PortTypeNetworkPort);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-gmac", "reg", BM1000_GMAC1_BASE, "GMAC1", PortConnectorTypeRJ45, PortTypeNetworkPort);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-xgmac", "reg", BM1000_XGMAC0_BASE, "XGMAC0", PortConnectorTypeRJ45, PortTypeNetworkPort);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-xgmac", "reg", BM1000_XGMAC1_BASE, "XGMAC1", PortConnectorTypeRJ45, PortTypeNetworkPort);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-ahci", "reg", BM1000_SATA0_BASE, "SATA0", PortConnectorTypeSasSata, PortTypeSata);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-ahci", "reg", BM1000_SATA1_BASE, "SATA1", PortConnectorTypeSasSata, PortTypeSata);
  SmbiosTable8Add (FdtClient, "baikal,dwcmshc-sdhci", "reg", BM1000_MMC_BASE, "eMMC/SD", PortConnectorTypeOther, PortTypeOther);

  SmbiosTable8Add (FdtClient, "snps,dw-apb-gpio", "reg", BM1000_GPIO32_BASE, "GPIO", PortConnectorTypeOther, PortTypeOther);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-uart", "reg", BM1000_UART1_BASE, "UART1", PortConnectorTypeOther, PortTypeSerial16550Compatible);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-uart", "reg", BM1000_UART2_BASE, "UART2", PortConnectorTypeOther, PortTypeSerial16550Compatible);
  SmbiosTable8Add (FdtClient, "snps,dw-apb-ssi", "reg", BM1000_SPI_BASE, "SPI", PortConnectorTypeOther, PortTypeOther);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-espi", "reg", BM1000_ESPI_BASE, "eSPI", PortConnectorTypeOther, PortTypeOther);
  SmbiosTable8Add (FdtClient, "snps,designware-i2c", "reg", BM1000_I2C1_BASE, "I2C1", PortConnectorTypeOther, PortTypeOther);
  SmbiosTable8Add (FdtClient, "snps,designware-i2c", "reg", BM1000_I2C2_BASE, "I2C2", PortConnectorTypeOther, PortTypeOther);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-smbus", "reg", BM1000_SMBUS1_BASE, "SMBUS1", PortConnectorTypeOther, PortTypeOther);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-smbus", "reg", BM1000_SMBUS2_BASE, "SMBUS2", PortConnectorTypeOther, PortTypeOther);

  SmbiosTable8Add (FdtClient, "baikal,bm1000-i2s", "reg", BM1000_I2S_BASE, "I2S", PortConnectorTypeOther, PortTypeAudioPort);
  SmbiosTable8Add (FdtClient, "baikal,bm1000-hda", "reg", BM1000_HDA_BASE, "HDA", PortConnectorTypeOther, PortTypeAudioPort);
  SmbiosTable8Add (FdtClient, "baikal,hdmi", "reg", BM1000_HDMI_CTRLR_BASE, "HDMI", PortConnectorTypeOther, PortTypeVideoPort);
  SmbiosTable8Add (FdtClient, "baikal,vdu", "lvds-lanes", 0, "LVDS", PortConnectorTypeOther, PortTypeVideoPort);

  SmbiosTable8Add (NULL, NULL, NULL, 0, "SW/JTAG", PortConnectorTypeOther, PortTypeOther);
  SmbiosTable8Add (NULL, NULL, NULL, 0, "MIPI PTI", PortConnectorTypeOther, PortTypeOther);

  return EFI_SUCCESS;
}

// System Slots tables, type 9

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
EFI_STATUS
SmbiosTable9Init (
  VOID
  )
{
  UINT64                Addr;
  FDT_CLIENT_PROTOCOL  *FdtClient;
  UINT8                 Id;
  CHAR8                 Name[] = "PCIe xXXX";
  INT32                 Node;
  CONST VOID           *Prop;
  INT32                 PrevNode;
  UINT32                PropSize;
  EFI_STATUS            Status;
  CONST CHAR8          *StrPtr = Name;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: unable to locate FdtClientProtocol, Status: %r\n", __func__, Status));
    return Status;
  }

  PrevNode = -1;
  while (1) {
    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bm1000-pcie", PrevNode, &Node);
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

    if (Addr == BM1000_PCIE0_DBI_BASE) {
      Name[6] = '4';
      Name[7] = '_';
      Name[8] = '0';
      SmbiosTable9.SlotDataBusWidth = SlotDataBusWidth4X;
      Id = 0;
    } else if (Addr == BM1000_PCIE1_DBI_BASE) {
      Name[6] = '4';
      Name[7] = '_';
      Name[8] = '1';
      SmbiosTable9.SlotDataBusWidth = SlotDataBusWidth4X;
      Id = 1;
    } else if (Addr == BM1000_PCIE2_DBI_BASE) {
      Name[6] = '8';
      Name[7] = 0;
      SmbiosTable9.SlotDataBusWidth = SlotDataBusWidth8X;
      Id = 2;
    } else {
      continue;
    }

    SmbiosTable9.DataBusWidth = SmbiosTable9.SlotDataBusWidth;
    SmbiosTable9.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (9, Id);
    SmbiosTable9.SlotType = Id < ARRAY_SIZE (PcieSlots) ? PcieSlots[Id] : SlotTypePciExpressGen3;
    SmbiosTable9.SlotID = SmbiosTable9.SegmentGroupNum = Id;

    CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTable9, &StrPtr, 1);
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
    0x8000000, // DDR4 max 64 GiB (x2)
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
  }
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

STATIC VOID  *SmbiosTable20 = BAIKAL_SMBIOS_TABLE (
  20,
  BAIKAL_SMBIOS_STRING (""),
  {
    {
      SMBIOS_TYPE_MEMORY_DEVICE_MAPPED_ADDRESS,
      sizeof (SMBIOS_TABLE_TYPE20),
      BAIKAL_SMBIOS_TABLE_HANDLE (20, 0)
    },
    0,
    0,
    BAIKAL_SMBIOS_TABLE_HANDLE (17, 0),
    BAIKAL_SMBIOS_TABLE_HANDLE (19, 0),
    0xFF,
    0xFF,
    0xFF,
    0,
    0
  }
);
#pragma pack()

STATIC
EFI_STATUS
GetMemoryRanges (
  OUT  CONST UINT32 ** CONST  Reg,
  OUT  UINTN * CONST          Amount,
  OUT  UINTN * CONST          AddressCells,
  OUT  UINTN * CONST          SizeCells
  )
{
  FDT_CLIENT_PROTOCOL  *FdtClient;
  INT32                 Node;
  UINT32                RegSize;
  EFI_STATUS            Status;

  if (Reg == NULL || Amount == NULL || AddressCells == NULL || SizeCells == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to locate FdtClientProtocol, Status = %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  Status = FdtClient->FindMemoryNodeReg (
                        FdtClient,
                        &Node,
                        (CONST VOID **) Reg,
                        AddressCells,
                        SizeCells,
                        &RegSize
                        );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to find MemoryNodeReg, Status = %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  if (*AddressCells > 2) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid AddressCells(%u) of the MemoryNodeReg\n",
      __func__,
      *AddressCells
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (*SizeCells > 2) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid SizeCells(%u) of the MemoryNodeReg\n",
      __func__,
      *SizeCells
      ));
    return EFI_INVALID_PARAMETER;
  }

  if ((RegSize <  (*AddressCells + *SizeCells) * sizeof (UINT32)) ||
      (RegSize % ((*AddressCells + *SizeCells) * sizeof (UINT32)))) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: invalid RegSize(%u) of the MemoryNodeReg\n",
      __func__,
      RegSize
      ));
    return EFI_INVALID_PARAMETER;
  }

  *Amount = RegSize / ((*AddressCells + *SizeCells) * sizeof (UINT32));

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SmbiosTable16_17_19_20Init (
  VOID
  )
{
  UINTN                    AddressCells;
  CHAR8                    BankLocator[] = "Bank A";
  UINT16                   DdrAmount;
  BAIKAL_SMBIOS_DDR_INFO  *DdrInfo;
  UINT8                    DdrPresence = 0;
  CHAR8                    DeviceLocator[] = "DDR4_CH0";
  UINT8                    Idx;
  CONST CHAR8             *PtrBuf[5];
  CONST UINT32            *Reg;
  UINT64                  *RegAddr;
  UINTN                    RegAmount;
  UINT64                  *RegArray;
  UINT64                  *RegSize;
  UINTN                    SizeCells;
  CONST UINT8             *SpdBuf;
  INTN                     SpdSize;
  EFI_STATUS               Status;

  if (IsMbm ()) {
    DdrAmount = 2;
  } else {
    DdrAmount = 4;
  }

  // Getting information about DDR
  DdrInfo = (BAIKAL_SMBIOS_DDR_INFO *) AllocateZeroPool (DdrAmount * sizeof (BAIKAL_SMBIOS_DDR_INFO));
  if (DdrInfo == NULL) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to allocate memory for SPD info structure, Status = %r\n",
      __func__,
      EFI_OUT_OF_RESOURCES
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  for (Idx = 0; Idx < BAIKAL_SPD_PORT_COUNT; ++Idx) {
    SpdBuf = SpdGetBuf (Idx);
    SpdSize = SpdGetSize (Idx);

    if (SpdSize == 0) {
      continue;
    }

    if (SmbiosSetDdrInfo (SpdBuf, SpdSize, DdrInfo + DdrAmount / 2 * Idx, NULL, NULL)) {
      DEBUG ((
        EFI_D_ERROR,
        "%a: DDR4 DIMM%d SPD info is invalid\n",
        __func__,
        Idx
        ));
      FreePool (DdrInfo);
      return EFI_INVALID_PARAMETER;
    } else {
      if (SpdIsDualChannel (Idx)) {
        UINT32  Serial = SpdGetSerial (Idx);

        SmbiosSetDdrInfo (SpdBuf, SpdSize, DdrInfo + DdrAmount / 2 * Idx + 1,
                          &Serial,
                          SpdGetPart (Idx));
        DdrPresence |= 1 << (DdrAmount / 2 * Idx + 1);
        ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->PartitionWidth++;
      }
      DdrPresence |= 1 << (DdrAmount / 2 * Idx);
      ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->PartitionWidth++;
    }
  }

  if (DdrPresence == 0) {
    FreePool (DdrInfo);
    return EFI_INVALID_PARAMETER;
  }

  Status = GetMemoryRanges (&Reg, &RegAmount, &AddressCells, &SizeCells);
  if (EFI_ERROR (Status)) {
    FreePool (DdrInfo);
    return Status;
  }

  RegArray = (UINT64 *) AllocateZeroPool (2 * RegAmount * sizeof (UINT64));
  if (RegArray == NULL) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to allocate memory for memory region info, Status = %r\n",
      __func__,
      EFI_OUT_OF_RESOURCES
      ));
    FreePool (DdrInfo);
    return EFI_OUT_OF_RESOURCES;
  }

  RegAddr = RegArray;
  RegSize = RegArray + 1;
  for (Idx = 0; Idx < RegAmount; ++Idx) {
    *RegAddr = SwapBytes32 (*Reg++);
    if (AddressCells > 1) {
      *RegAddr = (*RegAddr << 32) | SwapBytes32 (*Reg++);
    }

    *RegSize = SwapBytes32 (*Reg++);
    if (SizeCells > 1) {
      *RegSize = (*RegSize << 32) | SwapBytes32 (*Reg++);
    }

    RegAddr += 2;
    RegSize += 2;
  }

  // Init and load table of type 16
  ((SMBIOS_TABLE_TYPE16 *) SmbiosTable16)->NumberOfMemoryDevices = DdrAmount;
  if (!IsMbm ()) {
    ((SMBIOS_TABLE_TYPE16 *) SmbiosTable16)->MaximumCapacity <<= 1;
  }
  Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable16, NULL, 0);
  if (EFI_ERROR (Status)) {
    FreePool (DdrInfo);
    FreePool (RegArray);
    return Status;
  }

  PtrBuf[0] = DeviceLocator;
  PtrBuf[1] = BankLocator;

  for (Idx = 0; Idx < DdrAmount; ++Idx) {
    PtrBuf[2] = PtrBuf[3] = PtrBuf[4] = NULL;
    SmbiosTable17.Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (17, Idx);

    if ((DdrPresence >> Idx) & 1) {
      CHAR8        *Buf;
      UINT8         IsExtendedSize;
      CHAR8         Manufacturer[] = "Bank: 0x00, Id: 0x00";
      CONST CHAR8  *PtrString = NULL;

      IsExtendedSize = DdrInfo[Idx].Size >= (SIZE_32GB - 1) ? 1 : 0;

      SmbiosTable17.TotalWidth = DdrInfo[Idx].DataWidth + DdrInfo[Idx].ExtensionWidth;
      SmbiosTable17.DataWidth = DdrInfo[Idx].DataWidth;
      if (IsExtendedSize) {
        SmbiosTable17.ExtendedSize = DdrInfo[Idx].Size / SIZE_1MB;
        SmbiosTable17.Size = 0x7FFF;
      } else {
        SmbiosTable17.Size = DdrInfo[Idx].Size / SIZE_1MB;
        SmbiosTable17.ExtendedSize = 0;
      }
      SmbiosTable17.Speed = DdrInfo[Idx].Speed;

      UINT8 Cc = DdrInfo[Idx].ManufacturerId & 0x7F;
      UINT8 Id = (DdrInfo[Idx].ManufacturerId >> 8) & 0x7F;
      if (Cc == 5 && Id == 27) {
        PtrString = "Crucial Technology";
      } else if (Cc == 1 && Id == 24) {
        PtrString = "Kingston";
      } else if (Cc || Id) {
        CHAR8  HexValue[3];
        AsciiValueToStringS (HexValue, 3, RADIX_HEX, DdrInfo[Idx].ManufacturerId & 0xFF, 2);
        CopyMem (Manufacturer + 8, HexValue, 2);
        AsciiValueToStringS (HexValue, 3, RADIX_HEX, (DdrInfo[Idx].ManufacturerId >> 8) & 0xFF, 2);
        CopyMem (Manufacturer + 18, HexValue, 2);
        PtrString = Manufacturer;
      }

      if (PtrString) {
        Buf = AllocatePool (AsciiStrSize (PtrString));
        if (Buf == NULL) {
          FreePool (DdrInfo);
          FreePool (RegArray);
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

      if (DdrInfo[Idx].SerialNumber == 0) {
        SmbiosTable17.SerialNumber = 0;
        SmbiosTable17.PartNumber = SmbiosTable17.Manufacturer == 0 ? 3 : 4;
      } else {
        Buf = AllocatePool (11);
        if (Buf == NULL) {
          FreePool (DdrInfo);
          FreePool (RegArray);
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

        AsciiValueToStringS (Buf, 11, 0, DdrInfo[Idx].SerialNumber, 10);
        PtrBuf[3] = Buf;
        SmbiosTable17.SerialNumber = SmbiosTable17.Manufacturer == 0 ? 3 : 4;
        SmbiosTable17.PartNumber = SmbiosTable17.Manufacturer == 0 ? 4 : 5;
      }

      if (DdrInfo[Idx].PartNumber[0] != '\0') {
        Buf = AllocatePool (sizeof (DdrInfo[Idx].PartNumber));
        if (Buf == NULL) {
          FreePool (DdrInfo);
          FreePool (RegArray);
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

        CopyMem (Buf, DdrInfo[Idx].PartNumber, sizeof (DdrInfo[Idx].PartNumber));
        PtrBuf[4] = Buf;
      } else {
        SmbiosTable17.PartNumber = 0;
      }

      SmbiosTable17.Attributes = DdrInfo[Idx].Rank;
      SmbiosTable17.ConfiguredMemoryClockSpeed = SpdGetConfiguredSpeed (Idx * 2 / DdrAmount);
      SmbiosTable17.MinimumVoltage = DdrInfo[Idx].Voltage;
      SmbiosTable17.MaximumVoltage = DdrInfo[Idx].Voltage;
      SmbiosTable17.ConfiguredVoltage = DdrInfo[Idx].Voltage;
      SmbiosTable17.ModuleManufacturerID = DdrInfo[Idx].ManufacturerId;
      SmbiosTable17.VolatileSize = DdrInfo[Idx].Size;

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
        FreePool (RegArray);
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
        FreePool (RegArray);
        return Status;
      }
    }

    BankLocator[5]++;
    DeviceLocator[7]++;
  }

  // Init and load tables of type 19
  RegAddr = RegArray;
  RegSize = RegArray + 1;
  for (Idx = 0; Idx < RegAmount; ++Idx) {
    ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (19, Idx);
    ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->StartingAddress = *RegAddr / SIZE_1KB;
    ((SMBIOS_TABLE_TYPE19 *) SmbiosTable19)->EndingAddress = (*RegAddr + *RegSize - 1) / SIZE_1KB;

    Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable19, NULL, 0);
    if (EFI_ERROR (Status)) {
      FreePool (DdrInfo);
      FreePool (RegArray);
      return Status;
    }

    RegAddr += 2;
    RegSize += 2;
  }

  // Init and load tables of type 20
  UINT8  Idx2;
  UINT8  Idx3 = 0;
  RegAddr = RegArray;
  RegSize = RegArray + 1;
  for (Idx = 0; Idx < RegAmount; ++Idx) {
    ((SMBIOS_TABLE_TYPE20 *) SmbiosTable20)->StartingAddress = *RegAddr / SIZE_1KB;
    ((SMBIOS_TABLE_TYPE20 *) SmbiosTable20)->EndingAddress = (*RegAddr + *RegSize - 1) / SIZE_1KB;
    ((SMBIOS_TABLE_TYPE20 *) SmbiosTable20)->MemoryArrayMappedAddressHandle = BAIKAL_SMBIOS_TABLE_HANDLE (19, Idx);

    for (Idx2 = 0; Idx2 < DdrAmount; ++Idx2) {
      if ((DdrPresence >> Idx2) & 1) {
        ((SMBIOS_TABLE_TYPE20 *) SmbiosTable20)->Hdr.Handle = BAIKAL_SMBIOS_TABLE_HANDLE (20, Idx3++);
        ((SMBIOS_TABLE_TYPE20 *) SmbiosTable20)->MemoryDeviceHandle = BAIKAL_SMBIOS_TABLE_HANDLE (17, Idx2);

        Status = CreateSmbiosTable ((EFI_SMBIOS_TABLE_HEADER *) SmbiosTable20, NULL, 0);
        if (EFI_ERROR (Status)) {
          FreePool (DdrInfo);
          FreePool (RegArray);
          return Status;
        }
      }
    }

    RegAddr += 2;
    RegSize += 2;
  }

  FreePool (DdrInfo);
  FreePool (RegArray);

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
  &SmbiosTable16_17_19_20Init,
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
