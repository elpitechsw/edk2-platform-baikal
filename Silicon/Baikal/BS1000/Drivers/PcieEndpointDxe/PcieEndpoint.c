/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>

#include <BS1000.h>

#define BS1000_PCIE_APB_PE_GEN_CTRL3           0x58
#define BS1000_PCIE_APB_PE_GEN_CTRL3_LTSSM_EN  BIT0

#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG               0x080
#define BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_RETRAIN_LINK  BIT5

STATIC CONST EFI_PHYSICAL_ADDRESS  mPcieApbBaseList[] = {
  BS1000_PCIE0_APB_P0_CTRL_BASE,
  BS1000_PCIE0_APB_P1_CTRL_BASE,
  BS1000_PCIE1_APB_P0_CTRL_BASE,
  BS1000_PCIE1_APB_P1_CTRL_BASE,
  BS1000_PCIE2_APB_P0_CTRL_BASE,
  BS1000_PCIE2_APB_P1_CTRL_BASE,
  BS1000_PCIE3_APB_P0_CTRL_BASE,
  BS1000_PCIE3_APB_P1_CTRL_BASE,
  BS1000_PCIE3_APB_P2_CTRL_BASE,
  BS1000_PCIE3_APB_P3_CTRL_BASE,
  BS1000_PCIE4_APB_P0_CTRL_BASE,
  BS1000_PCIE4_APB_P1_CTRL_BASE,
  BS1000_PCIE4_APB_P2_CTRL_BASE,
  BS1000_PCIE4_APB_P3_CTRL_BASE
};

STATIC CONST EFI_PHYSICAL_ADDRESS  mPcieDbiBaseList[] = {
  BS1000_PCIE0_P0_DBI_BASE,
  BS1000_PCIE0_P1_DBI_BASE,
  BS1000_PCIE1_P0_DBI_BASE,
  BS1000_PCIE1_P1_DBI_BASE,
  BS1000_PCIE2_P0_DBI_BASE,
  BS1000_PCIE2_P1_DBI_BASE,
  BS1000_PCIE3_P0_DBI_BASE,
  BS1000_PCIE3_P1_DBI_BASE,
  BS1000_PCIE3_P2_DBI_BASE,
  BS1000_PCIE3_P3_DBI_BASE,
  BS1000_PCIE4_P0_DBI_BASE,
  BS1000_PCIE4_P1_DBI_BASE,
  BS1000_PCIE4_P2_DBI_BASE,
  BS1000_PCIE4_P3_DBI_BASE
};

#if !defined(MDEPKG_NDEBUG)
STATIC CONST UINTN  mPcieDbiSizeList[] = {
  BS1000_PCIE0_P0_DBI_SIZE,
  BS1000_PCIE0_P1_DBI_SIZE,
  BS1000_PCIE1_P0_DBI_SIZE,
  BS1000_PCIE1_P1_DBI_SIZE,
  BS1000_PCIE2_P0_DBI_SIZE,
  BS1000_PCIE2_P1_DBI_SIZE,
  BS1000_PCIE3_P0_DBI_SIZE,
  BS1000_PCIE3_P1_DBI_SIZE,
  BS1000_PCIE3_P2_DBI_SIZE,
  BS1000_PCIE3_P3_DBI_SIZE,
  BS1000_PCIE4_P0_DBI_SIZE,
  BS1000_PCIE4_P1_DBI_SIZE,
  BS1000_PCIE4_P2_DBI_SIZE,
  BS1000_PCIE4_P3_DBI_SIZE
};
#endif

STATIC_ASSERT (
  ARRAY_SIZE (mPcieApbBaseList) == ARRAY_SIZE (mPcieDbiBaseList),
  "ARRAY_SIZE (mPcieApbBaseList) != ARRAY_SIZE (mPcieDbiBaseList)"
  );
#if !defined(MDEPKG_NDEBUG)
STATIC_ASSERT (
  ARRAY_SIZE (mPcieDbiSizeList) == ARRAY_SIZE (mPcieDbiBaseList),
  "ARRAY_SIZE (mPcieDbiSizeList) != ARRAY_SIZE (mPcieDbiBaseList)"
  );
#endif

EFI_STATUS
EFIAPI
PcieEndpointEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  FDT_CLIENT_PROTOCOL  *FdtClient;
  INT32                 Node = 0;
  EFI_STATUS            Status;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);

  while (TRUE) {
    INTN         ApbRegIdx = -1, DbiRegIdx = -1;
    CONST VOID  *Prop;
    UINT32       PropSize;

    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bs1000-pcie-ep", Node, &Node);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reg-names", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize > 0) {
      UINTN         RegIdx;
      CONST CHAR8  *StrPtr = Prop;

      for (RegIdx = 0; PropSize > 0; ++RegIdx) {
        CONST UINTN  StrSize = AsciiStrSize (StrPtr);

        ASSERT (StrSize <= PropSize);

        if (AsciiStrCmp (StrPtr, "dbi") == 0) {
          DbiRegIdx = RegIdx;
        } else if (AsciiStrCmp (StrPtr, "apb") == 0) {
          ApbRegIdx = RegIdx;
        }

        PropSize -= StrSize;
        StrPtr   += StrSize;
      }

      if (ApbRegIdx == -1 || DbiRegIdx == -1) {
        continue;
      }
    } else {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize >= 48 && (PropSize % 16) == 0) {
      CONST EFI_PHYSICAL_ADDRESS  ApbBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + ApbRegIdx * 2));
      CONST EFI_PHYSICAL_ADDRESS  DbiBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + DbiRegIdx * 2));
#if !defined(MDEPKG_NDEBUG)
      CONST UINTN                 ApbSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + ApbRegIdx * 2 + 1));
      CONST UINTN                 DbiSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + DbiRegIdx * 2 + 1));
#endif
      UINTN  ListIdx;

      for (ListIdx = 0; ListIdx < ARRAY_SIZE (mPcieDbiBaseList); ++ListIdx) {
        if (PLATFORM_ADDR_IN_CHIP(DbiBase) == mPcieDbiBaseList[ListIdx]) {
          ASSERT (PLATFORM_ADDR_IN_CHIP(ApbBase) == mPcieApbBaseList[ListIdx]);
#if !defined(MDEPKG_NDEBUG)
          ASSERT (ApbSize == 0x100);
          ASSERT (DbiSize == mPcieDbiSizeList[ListIdx]);
#endif
          MmioOr32 (
            ApbBase +
            BS1000_PCIE_APB_PE_GEN_CTRL3,
            BS1000_PCIE_APB_PE_GEN_CTRL3_LTSSM_EN
            );

          MmioOr32 (
            DbiBase +
            BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG,
            BS1000_PCIE_PF0_PCIE_CAP_LINK_CONTROL_LINK_STATUS_REG_RETRAIN_LINK
            );

          break;
        }
      }
    } else {
      continue;
    }
  }

  return EFI_SUCCESS;
}
