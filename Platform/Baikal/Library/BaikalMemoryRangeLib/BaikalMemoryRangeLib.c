/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>

#include <Library/BaikalMemoryRangeLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>

EFI_STATUS
GetMemoryRanges (
  IN OUT  INT32 * CONST       Node,
  OUT  CONST UINT32 ** CONST  Reg,
  OUT  UINTN * CONST          Amount,
  OUT  UINTN * CONST          AddressCells,
  OUT  UINTN * CONST          SizeCells
  )
{
  FDT_CLIENT_PROTOCOL  *FdtClient;
  UINT32                RegSize;
  EFI_STATUS            Status;

  if (Node == NULL || Reg == NULL || Amount == NULL || AddressCells == NULL || SizeCells == NULL) {
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

  Status = FdtClient->FindNextMemoryNodeReg (
                        FdtClient,
                        *Node,
                        Node,
                        (CONST VOID **) Reg,
                        AddressCells,
                        SizeCells,
                        &RegSize
                        );

  if (EFI_ERROR (Status)) {
    if (Status != EFI_NOT_FOUND) {
      DEBUG ((
        EFI_D_ERROR,
        "%a: failed to find MemoryNodeReg, Status = %r\n",
        __func__,
        Status
        ));
    }
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
