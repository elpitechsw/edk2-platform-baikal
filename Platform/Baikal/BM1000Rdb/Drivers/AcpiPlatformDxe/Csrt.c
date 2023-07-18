/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>

#include "AcpiPlatform.h"

#define ACPI_CSRT_TYPE_DMA        0x3
#define ACPI_CSRT_DMA_CHANNEL     0x0
#define ACPI_CSRT_DMA_CONTROLLER  0x1

#define BAIKAL_CSRT_CHANNEL_DESCRIPTOR(Id)  { \
    /* UINT32  Length  */                     \
    sizeof (BAIKAL_CSRT_DESCRIPTOR),          \
    /* UINT16  Type    */                     \
    ACPI_CSRT_TYPE_DMA,                       \
    /* UINT16  Subtype */                     \
    ACPI_CSRT_DMA_CHANNEL,                    \
    /* UINT32  Uid     */                     \
    Id                                        \
}

#pragma pack(1)
typedef struct {
  UINT32  Length;
  UINT32  VendorId;
  UINT32  SubvendorId;
  UINT16  DeviceId;
  UINT16  SubdeviceId;
  UINT16  Revision;
  UINT16  Reserved;
  UINT32  SharedInfoLength;
} BAIKAL_CSRT_GROUP;

typedef struct {
  UINT16  MajorVersion;
  UINT16  MinorVersion;
  UINT32  MmioBaseLow;
  UINT32  MmioBaseHigh;
  UINT32  GsiInterrupt;
  UINT8   InterruptPolarity;
  UINT8   InterruptMode;
  UINT8   NumChannels;
  UINT8   DmaAddressWidth;
  UINT16  BaseRequestLine;
  UINT16  NumHandshakeSignals;
  UINT32  MaxBlockSize;
} BAIKAL_CSRT_SHARED_INFO;

typedef struct {
  UINT32  Length;
  UINT16  Type;
  UINT16  Subtype;
  UINT32  Uid;
} BAIKAL_CSRT_DESCRIPTOR;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER  Header;
  BAIKAL_CSRT_GROUP            Group;
  BAIKAL_CSRT_SHARED_INFO      SharedInfo;
  BAIKAL_CSRT_DESCRIPTOR       Descriptor[9];
} BAIKAL_ACPI_CSRT;

STATIC BAIKAL_ACPI_CSRT  Csrt = {
  BAIKAL_ACPI_HEADER (
    EFI_ACPI_6_4_CORE_SYSTEM_RESOURCE_TABLE_SIGNATURE,
    BAIKAL_ACPI_CSRT,
    0,
    0x54525343
    ),
  {
    /* UINT32  Length           */
    sizeof (BAIKAL_CSRT_GROUP) +
      sizeof (BAIKAL_CSRT_SHARED_INFO) +
      sizeof (((BAIKAL_ACPI_CSRT *)0)->Descriptor),
    /* UINT32  VendorId         */
    0x454C4B42,
    /* UINT32  SubvendorId      */
    0,
    /* UINT16  DeviceId         */
    0,
    /* UINT16  SubdeviceId      */
    0,
    /* UINT16  Revision         */
    1,
    /* UINT16  Reserved         */
    EFI_ACPI_RESERVED_WORD,
    /* UINT32  SharedInfoLength */
    sizeof (BAIKAL_CSRT_SHARED_INFO)
  },
  {
    /* UINT16  MajorVersion        */
    0,
    /* UINT16  MinorVersion        */
    0,
    /* UINT32  MmioBaseLow         */
    0x202B0000,
    /* UINT32  MmioBaseHigh        */
    0,
    /* UINT32  GsiInterrupt        */
    80,
    /* UINT8   InterruptPolarity   */
    0,
    /* UINT8   InterruptMode       */
    0,
    /* UINT8   NumChannels         */
    8,
    /* UINT8   DmaAddressWidth     */
    32,
    /* UINT16  BaseRequestLine     */
    0,
    /* UINT16  NumHandshakeSignals */
    10,
    /* UINT32  MaxBlockSize        */
    0xFF
  },
  {
    /* DMAC LSP */
    {
      /* UINT32  Length  */
      sizeof (BAIKAL_CSRT_DESCRIPTOR),
      /* UINT16  Type    */
      ACPI_CSRT_TYPE_DMA,
      /* UINT16  Subtype */
      ACPI_CSRT_DMA_CONTROLLER,
      /* UINT32  Uid     */
      0
    },
    BAIKAL_CSRT_CHANNEL_DESCRIPTOR (1),
    BAIKAL_CSRT_CHANNEL_DESCRIPTOR (2),
    BAIKAL_CSRT_CHANNEL_DESCRIPTOR (3),
    BAIKAL_CSRT_CHANNEL_DESCRIPTOR (4),
    BAIKAL_CSRT_CHANNEL_DESCRIPTOR (5),
    BAIKAL_CSRT_CHANNEL_DESCRIPTOR (6),
    BAIKAL_CSRT_CHANNEL_DESCRIPTOR (7),
    BAIKAL_CSRT_CHANNEL_DESCRIPTOR (8)
  }
};
#pragma pack()

EFI_STATUS
CsrtInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Csrt;
  return EFI_SUCCESS;
}
