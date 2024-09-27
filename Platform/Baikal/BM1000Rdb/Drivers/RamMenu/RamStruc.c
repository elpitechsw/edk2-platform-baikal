/** @file
  Copyright (c) 2023 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/SmcFlashLib.h>
#include <Library/UefiLib.h>
#include "RamStruc.h"

STATIC
EFI_STATUS
EFIAPI
RamStrucFlashFind (
  IN UINTN            Port,
  IN DDR_SPD_CONFIG  *Struc
  )
{
  INTN  Err;

  Err = SmcFlashRead (
          FixedPcdGet32 (PcdFlashNvStorageDdrCfgBase) + (BAIKAL_STR_PORT_OFFS * Port),
          Struc,
          sizeof (DDR_SPD_CONFIG)
          );
  if (Err) {
    return EFI_DEVICE_ERROR;
  }

  if (Struc->SpdFlagU == BAIKAL_FLASH_USE_SPD ||
      Struc->SpdFlagU == BAIKAL_FLASH_USE_STR) {
    return EFI_SUCCESS;
  } else {
    return EFI_INVALID_PARAMETER;
  }
}

STATIC
EFI_STATUS
EFIAPI
RamStrucFlashSave (
  IN UINTN           Port,
  IN DDR_SPD_CONFIG *Struc
  )
{
  INTN  Err;

  DEBUG ((
    DEBUG_INFO,
    "RamMenu: saving DDR SDRAM configuration to Flash @ 0x%x - 0x%x\n",
    FixedPcdGet32 (PcdFlashNvStorageDdrCfgBase) + (BAIKAL_STR_PORT_OFFS * Port),
    FixedPcdGet32 (PcdFlashNvStorageDdrCfgBase) + (BAIKAL_STR_PORT_OFFS * (Port + 1)) - 1
    ));

  SmcFlashLock (0);

  Err = SmcFlashErase (
          FixedPcdGet32 (PcdFlashNvStorageDdrCfgBase) + (BAIKAL_STR_PORT_OFFS * Port),
          BAIKAL_STR_PORT_OFFS
          );
  if (Err) {
    goto ExitLock;
  }

  Err = SmcFlashWrite (
          FixedPcdGet32 (PcdFlashNvStorageDdrCfgBase) + (BAIKAL_STR_PORT_OFFS * Port),
          Struc,
          sizeof (DDR_SPD_CONFIG)
          );
  if (Err) {
    goto ExitLock;
  }

  SmcFlashLock (1);

  return EFI_SUCCESS;

ExitLock:
  SmcFlashLock (1);

  return EFI_DEVICE_ERROR;
}

EFI_STATUS
EFIAPI
RamStrucCurrentSettings (
  IN RAM_MENU_STRUC  *Variable
  )
{
  EFI_STATUS      Status;
  DDR_SPD_CONFIG  Struc;

  Status = RamStrucFlashFind (0, &Struc);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Variable->Speedbin0 = Struc.SpeedBin;
  Variable->Dic0      = Struc.Dic;
  Variable->RttWr0    = Struc.RttWr;
  Variable->RttNom0   = Struc.RttNom;
  Variable->RttPark0  = Struc.RttPark;
  Variable->Cl0       = Struc.Cl;
  Variable->tRCD0     = Struc.tRCD;
  Variable->tRP0      = Struc.tRP;
  Variable->tRAS0     = Struc.tRAS;
  Variable->tFAW0     = Struc.tFAW;
  Variable->t1T2T0    = Struc.t1T2T;
  Variable->HostVref0 = Struc.HostVref;
  Variable->DramVref0 = Struc.DramVref;
  Variable->VrefFlag0 = Struc.VrefFlagU;

  if (Struc.SpdFlagU == BAIKAL_FLASH_USE_STR) {
    Variable->Flash0 = 1;
  }

  Status = RamStrucFlashFind (1, &Struc);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Variable->Speedbin1 = Struc.SpeedBin;
  Variable->Dic1      = Struc.Dic;
  Variable->RttWr1    = Struc.RttWr;
  Variable->RttNom1   = Struc.RttNom;
  Variable->RttPark1  = Struc.RttPark;
  Variable->Cl1       = Struc.Cl;
  Variable->tRCD1     = Struc.tRCD;
  Variable->tRP1      = Struc.tRP;
  Variable->tRAS1     = Struc.tRAS;
  Variable->tFAW1     = Struc.tFAW;
  Variable->t1T2T1    = Struc.t1T2T;
  Variable->HostVref1 = Struc.HostVref;
  Variable->DramVref1 = Struc.DramVref;
  Variable->VrefFlag1 = Struc.VrefFlagU;

  if (Struc.SpdFlagU == BAIKAL_FLASH_USE_STR) {
    Variable->Flash1 = 1;
  }

Exit:
  return Status;
}

EFI_STATUS
EFIAPI
RamStrucSaveCurrentSettings (
  IN RAM_MENU_STRUC  *Variable
  )
{
  EFI_STATUS      Status;
  DDR_SPD_CONFIG  Struc;

  Struc.SpeedBin  = Variable->Speedbin0;
  Struc.Dic       = Variable->Dic0;
  Struc.RttWr     = Variable->RttWr0;
  Struc.RttNom    = Variable->RttNom0;
  Struc.RttPark   = Variable->RttPark0;
  Struc.Cl        = Variable->Cl0;
  Struc.tRCD      = Variable->tRCD0;
  Struc.tRP       = Variable->tRP0;
  Struc.tRAS      = Variable->tRAS0;
  Struc.tFAW      = Variable->tFAW0;
  Struc.t1T2T     = Variable->t1T2T0;
  Struc.HostVref  = Variable->HostVref0;
  Struc.DramVref  = Variable->DramVref0;
  Struc.VrefFlagU = Variable->VrefFlag0;

  if (Variable->Flash0) {
    Struc.SpdFlagU = BAIKAL_FLASH_USE_STR;
  } else {
    Struc.SpdFlagU = BAIKAL_FLASH_USE_SPD;
  }

  Status = RamStrucFlashSave (0, &Struc);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Struc.SpeedBin  = Variable->Speedbin1;
  Struc.Dic       = Variable->Dic1;
  Struc.RttWr     = Variable->RttWr1;
  Struc.RttNom    = Variable->RttNom1;
  Struc.RttPark   = Variable->RttPark1;
  Struc.Cl        = Variable->Cl1;
  Struc.tRCD      = Variable->tRCD1;
  Struc.tRP       = Variable->tRP1;
  Struc.tRAS      = Variable->tRAS1;
  Struc.tFAW      = Variable->tFAW1;
  Struc.t1T2T     = Variable->t1T2T1;
  Struc.HostVref  = Variable->HostVref1;
  Struc.DramVref  = Variable->DramVref1;
  Struc.VrefFlagU = Variable->VrefFlag1;

  if (Variable->Flash1) {
    Struc.SpdFlagU = BAIKAL_FLASH_USE_STR;
  } else {
    Struc.SpdFlagU = BAIKAL_FLASH_USE_SPD;
  }

  Status = RamStrucFlashSave (1, &Struc);

  return Status;
}
