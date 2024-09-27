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
  IN DDR_SPD_CONFIG  *Struc
  )
{
  INTN  Err;

  Err = SmcFlashRead (
          FixedPcdGet32 (PcdFlashNvStorageDdrCfgBase),
          Struc,
          sizeof (DDR_SPD_CONFIG)
          );
  if (Err) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
RamStrucFlashSave (
  IN DDR_SPD_CONFIG *Struc
  )
{
  INTN  Err;

  DEBUG ((
    DEBUG_INFO,
    "RamMenu: saving DDR SDRAM configuration to Flash @ 0x%x - 0x%x\n",
    FixedPcdGet32 (PcdFlashNvStorageDdrCfgBase),
    FixedPcdGet32 (PcdFlashNvStorageDdrCfgBase) + BAIKAL_STR_PORT_OFFS - 1
    ));

  SmcFlashLock (0);

  Err = SmcFlashErase (
          FixedPcdGet32 (PcdFlashNvStorageDdrCfgBase),
          BAIKAL_STR_PORT_OFFS
          );
  if (Err) {
    goto ExitLock;
  }

  Err = SmcFlashWrite (
          FixedPcdGet32 (PcdFlashNvStorageDdrCfgBase),
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
  UINTN               ChanId;
  EFI_STATUS          Status;
  DDR_SPD_CONFIG      Struc;
  DDR_SPD_SETTINGS   *Channel;
  RAM_SETTINGS_STRUC *VarChannel;

  Status = RamStrucFlashFind (&Struc);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  for (ChanId = 0; ChanId < FixedPcdGet32 (PcdDdrChannelsNumber); ChanId++) {
    Channel = ((VOID *)&Struc) + sizeof (DDR_SPD_SETTINGS) * ChanId;
    VarChannel = ((VOID *)Variable) + sizeof (RAM_SETTINGS_STRUC) * ChanId;

    VarChannel->Speedbin = Channel->SpeedBin;
    VarChannel->Dic      = Channel->Dic;
    VarChannel->RttWr    = Channel->RttWr;
    VarChannel->RttNom   = Channel->RttNom;
    VarChannel->RttPark  = Channel->RttPark;
    VarChannel->Cl       = Channel->Cl;
    VarChannel->tRCD     = Channel->tRCD;
    VarChannel->tRP      = Channel->tRP;
    VarChannel->tRAS     = Channel->tRAS;
    VarChannel->tFAW     = Channel->tFAW;
    VarChannel->t1T2T    = Channel->t1T2T;
    VarChannel->HostVref = Channel->HostVref;
    VarChannel->DramVref = Channel->DramVref;
    VarChannel->VrefFlag = Channel->VrefFlagU;
    if (Channel->EccDis == BAIKAL_ECC_DISABLE) {
      VarChannel->EccDis = TRUE;
    } else {
      VarChannel->EccDis = FALSE;
    }
    if (Channel->CrcEn == BAIKAL_CRC_ENABLE) {
      VarChannel->CrcEn = TRUE;
    } else {
      VarChannel->CrcEn = FALSE;
    }
    if (Channel->ParEn == BAIKAL_PARITY_EN) {
      VarChannel->ParEn = TRUE;
    } else {
      VarChannel->ParEn = FALSE;
    }

    if (Channel->SpdFlagU == BAIKAL_FLASH_USE_STR) {
      VarChannel->Flash = 1;
    }
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
  UINTN               ChanId;
  EFI_STATUS          Status;
  DDR_SPD_CONFIG      Struc;
  DDR_SPD_SETTINGS   *Channel;
  RAM_SETTINGS_STRUC *VarChannel;

  Status = RamStrucFlashFind (&Struc);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  for (ChanId = 0; ChanId < FixedPcdGet32 (PcdDdrChannelsNumber); ChanId++) {
    Channel = ((VOID *)&Struc) + sizeof (DDR_SPD_SETTINGS) * ChanId;
    VarChannel = ((VOID *)Variable) + sizeof (RAM_SETTINGS_STRUC) * ChanId;

    Channel->SpeedBin  = VarChannel->Speedbin;
    Channel->Dic       = VarChannel->Dic;
    Channel->RttWr     = VarChannel->RttWr;
    Channel->RttNom    = VarChannel->RttNom;
    Channel->RttPark   = VarChannel->RttPark;
    Channel->Cl        = VarChannel->Cl;
    Channel->tRCD      = VarChannel->tRCD;
    Channel->tRP       = VarChannel->tRP;
    Channel->tRAS      = VarChannel->tRAS;
    Channel->tFAW      = VarChannel->tFAW;
    Channel->t1T2T     = VarChannel->t1T2T;
    Channel->HostVref  = VarChannel->HostVref;
    Channel->DramVref  = VarChannel->DramVref;
    Channel->VrefFlagU = VarChannel->VrefFlag;
    if (VarChannel->EccDis) {
      Channel->EccDis = BAIKAL_ECC_DISABLE;
    } else {
      Channel->EccDis = 0;
    }
    if (VarChannel->CrcEn) {
      Channel->CrcEn = BAIKAL_CRC_ENABLE;
    } else {
      Channel->CrcEn = 0;
    }
    if (VarChannel->ParEn) {
      Channel->ParEn = BAIKAL_PARITY_EN;
    } else {
      Channel->ParEn = 0;
    }

    if (VarChannel->Flash) {
      Channel->SpdFlagU = BAIKAL_FLASH_USE_STR;
    } else {
      Channel->SpdFlagU = BAIKAL_FLASH_USE_SPD;
    }
  }

  Status = RamStrucFlashSave (&Struc);

Exit:
  return Status;
}

EFI_STATUS
EFIAPI
RamStrucConfigurationIsGood (
  IN RAM_MENU_STRUC *Variable
  )
{
  UINTN               ChanId;
  RAM_SETTINGS_STRUC *VarChannel;

  for (ChanId = 0; ChanId < FixedPcdGet32 (PcdDdrChannelsNumber); ChanId++) {
    VarChannel = ((VOID *)Variable) + sizeof (RAM_SETTINGS_STRUC) * ChanId;
    if (VarChannel->Speedbin == 0x0) {
      goto ErrorExit;
    }
  }

  return EFI_SUCCESS;

ErrorExit:
  return EFI_VOLUME_CORRUPTED;
}
