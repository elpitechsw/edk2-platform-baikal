/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>

#include "AcpiPlatform.h"

#define BAIKAL_PMTT_DIMM_COUNT  12

#define BAIKAL_PMTT_NODE(SocketId, SmbiosId)  {                  \
  /* EFI_ACPI_6_4_PMTT_COMMON_MEMORY_DEVICE  SocketHeader     */ \
  {                                                              \
    /* UINT8   Type      */                                      \
    0,                                                           \
    /* UINT8   Reserved  */                                      \
    EFI_ACPI_RESERVED_BYTE,                                      \
    /* UINT16  Length    */                                      \
    16,                                                          \
    /* UINT16  Flags     */                                      \
    BIT0,                                                        \
    /* UINT16  Reserved1 */                                      \
    EFI_ACPI_RESERVED_WORD,                                      \
    /* UINT32  NumberOfMemoryDevices */                          \
    1                                                            \
  },                                                             \
  /* UINT16                                  SocketIdentifier */ \
  SocketId,                                                      \
  /* UINT16                                  Reserved         */ \
  EFI_ACPI_RESERVED_WORD,                                        \
  /* EFI_ACPI_6_4_PMTT_COMMON_MEMORY_DEVICE  DimmHeader       */ \
  {                                                              \
    /* UINT8   Type      */                                      \
    2,                                                           \
    /* UINT8   Reserved  */                                      \
    EFI_ACPI_RESERVED_BYTE,                                      \
    /* UINT16  Length    */                                      \
    16,                                                          \
    /* UINT16  Flags     */                                      \
    BIT1,                                                        \
    /* UINT16  Reserved1 */                                      \
    EFI_ACPI_RESERVED_WORD,                                      \
    /* UINT32  NumberOfMemoryDevices */                          \
    1                                                            \
  },                                                             \
  /* UINT32                                  SmbiosHandle     */ \
  SmbiosId & 0x0000FFFF                                          \
}

#pragma pack(1)
typedef struct {
  EFI_ACPI_6_4_PMTT_COMMON_MEMORY_DEVICE  SocketHeader;
  UINT16                                  SocketIdentifier;
  UINT16                                  Reserved;
  EFI_ACPI_6_4_PMTT_COMMON_MEMORY_DEVICE  DimmHeader;
  UINT32                                  SmbiosHandle;
} BAIKAL_ACPI_PMTT_SOCKET_DIMM;

typedef struct {
  EFI_ACPI_6_4_PLATFORM_MEMORY_TOPOLOGY_TABLE  Header;
  BAIKAL_ACPI_PMTT_SOCKET_DIMM  Device[BAIKAL_PMTT_DIMM_COUNT];
} BAIKAL_ACPI_PMTT;

STATIC BAIKAL_ACPI_PMTT  Pmtt = {
  {
    /* EFI_ACPI_DESCRIPTION_HEADER  Header                */
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_4_PLATFORM_MEMORY_TOPOLOGY_TABLE_SIGNATURE,
      BAIKAL_ACPI_PMTT,
      EFI_ACPI_6_4_MEMORY_TOPOLOGY_TABLE_REVISION,
      0x54544D50
      ),
    /* UINT32                       NumberOfMemoryDevices */
    BAIKAL_PMTT_DIMM_COUNT,
  },
  {
    BAIKAL_PMTT_NODE (0, 0x1100),
    BAIKAL_PMTT_NODE (1, 0x1101),
    BAIKAL_PMTT_NODE (2, 0x1102),
    BAIKAL_PMTT_NODE (3, 0x1103),
    BAIKAL_PMTT_NODE (4, 0x1104),
    BAIKAL_PMTT_NODE (5, 0x1105),
    BAIKAL_PMTT_NODE (6, 0x1106),
    BAIKAL_PMTT_NODE (7, 0x1107),
    BAIKAL_PMTT_NODE (8, 0x1108),
    BAIKAL_PMTT_NODE (9, 0x1109),
    BAIKAL_PMTT_NODE (10, 0x110a),
    BAIKAL_PMTT_NODE (11, 0x110b)
  }
};
#pragma pack()

EFI_STATUS
PmttInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Pmtt;
  return EFI_SUCCESS;
}
