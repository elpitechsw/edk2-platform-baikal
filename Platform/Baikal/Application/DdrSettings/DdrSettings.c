/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ShellLib.h>
#include <Library/SmcFlashLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/ShellParameters.h>
#include "DdrSettings.h"

STATIC
EFI_STATUS
DdrProcessRttValue (
  IN BOOLEAN          DisplayHelpAndExit,
  IN DDR_SPD_CONFIG  *NewFlashBuf
  )
{
  if (DisplayHelpAndExit) {
    goto PrintHelp;
  }
  switch (NewFlashBuf->Dic) {
    case 0:
      NewFlashBuf->Dic = BAIKAL_DIC_RZQ_DIV_7;
      break;
    case 1:
      NewFlashBuf->Dic = BAIKAL_DIC_RZQ_DIV_5;
      break;
    case BAIKAL_FLASH_DUMMY:
      break;
    default:
      Print (L"ERROR: Wrong value for DIC.\n");
      goto PrintHelp;
  }
  switch (NewFlashBuf->RttWr) {
    case 0:
      NewFlashBuf->RttWr = BAIKAL_RTTWR_DYN_OFF;
      break;
    case 1:
      NewFlashBuf->RttWr = BAIKAL_RTTWR_RZQ_DIV_4;
      break;
    case 2:
      NewFlashBuf->RttWr = BAIKAL_RTTWR_RZQ_DIV_2;
      break;
    case 3:
      NewFlashBuf->RttWr = BAIKAL_RTTWR_HI_Z;
      break;
    case 4:
      NewFlashBuf->RttWr = BAIKAL_RTTWR_RZQ_DIV_3;
      break;
    case BAIKAL_FLASH_DUMMY:
      break;
    default:
      Print (L"ERROR: Wrong value for RTT WR\n");
      goto PrintHelp;
  }
  switch (NewFlashBuf->RttNom) {
    case 0:
      NewFlashBuf->RttNom = BAIKAL_RTTNOM_RZQ_DIS;
      break;
    case 1:
      NewFlashBuf->RttNom = BAIKAL_RTTNOM_RZQ_DIV_4;
      break;
    case 2:
      NewFlashBuf->RttNom = BAIKAL_RTTNOM_RZQ_DIV_2;
      break;
    case 3:
      NewFlashBuf->RttNom = BAIKAL_RTTNOM_RZQ_DIV_6;
      break;
    case 4:
      NewFlashBuf->RttNom = BAIKAL_RTTNOM_RZQ_DIV_1;
      break;
    case 5:
     NewFlashBuf->RttNom = BAIKAL_RTTNOM_RZQ_DIV_5;
      break;
    case 6:
      NewFlashBuf->RttNom = BAIKAL_RTTNOM_RZQ_DIV_3;
      break;
    case 7:
      NewFlashBuf->RttNom = BAIKAL_RTTNOM_RZQ_DIV_7;
      break;
    case BAIKAL_FLASH_DUMMY:
      break;
    default:
      Print (L"ERROR: Wrong value for RTT NOM\n");
      goto PrintHelp;
  }
  switch (NewFlashBuf->RttPark) {
    case 0:
      NewFlashBuf->RttPark = BAIKAL_RTTPARK_RZQ_DIS;
      break;
    case 1:
      NewFlashBuf->RttPark = BAIKAL_RTTPARK_RZQ_DIS;
      break;
    case 2:
      NewFlashBuf->RttPark = BAIKAL_RTTPARK_RZQ_DIS;
      break;
    case 3:
      NewFlashBuf->RttPark = BAIKAL_RTTPARK_RZQ_DIS;
      break;
    case 4:
      NewFlashBuf->RttPark = BAIKAL_RTTPARK_RZQ_DIS;
      break;
    case 5:
      NewFlashBuf->RttPark = BAIKAL_RTTPARK_RZQ_DIS;
      break;
    case 6:
      NewFlashBuf->RttPark = BAIKAL_RTTPARK_RZQ_DIS;
      break;
    case 7:
      NewFlashBuf->RttPark = BAIKAL_RTTPARK_RZQ_DIS;
      break;
    case BAIKAL_FLASH_DUMMY:
      break;
    default:
      Print (L"ERROR: Wrong value for RTT PARK\n");
      goto PrintHelp;
  }

  return EFI_SUCCESS;

PrintHelp:
  Print (L"Change ddr odt settings on a next start:\n");
  Print (L"\n");
  Print (L"  --dic\n");
  Print (L"    set a value for driver impedance control.\n");
  Print (L"    Address JEDEC DDR4 Standard for details.\n");
  Print (L"    Possible vlues are: 0 (RZQ/7) and 1 (RZQ/5)\n");
  Print (L"\n");
  Print (L"  --rttwr\n");
  Print (L"    set a value for rtt WR.\n");
  Print (L"    Address JEDEC DDR4 Standard for details.\n");
  Print (L"    Possible vlues are: 0 (DIS), 1 (RZQ/2), 2 (RZQ), 3 (Hi-Z), 4 (RZQ/3)\n");
  Print (L"\n");
  Print (L"  --rttnom\n");
  Print (L"    set a value for rtt NOM.\n");
  Print (L"    Address JEDEC DDR4 Standard for details.\n");
  Print (L"    Possible vlues are: 0 (DIS), 1 (RZQ/4), 2 (RZQ/2), 3 (RZQ/6), 4 (RZQ),\n");
  Print (L"                          5 (RZQ/5), 6 (RZQ/3), 7 (RZQ/7)\n");
  Print (L"\n");
  Print (L"  --rttpark\n");
  Print (L"    set a value for rtt PARK.\n");
  Print (L"    Address JEDEC DDR4 Standard for details.\n");
  Print (L"    Possible vlues are: 0 (DIS), 1 (RZQ/4), 2 (RZQ/2), 3 (RZQ/6), 4 (RZQ),\n");
  Print (L"                          5 (RZQ/5), 6 (RZQ/3), 7 (RZQ/7)\n");
  Print (L"\n");
  Print (L"Example:\n");
  Print (L"  DdrSettings.efi -p 1 --rttnom 3 --dic 0\n");
  Print (L"  DdrSettings.efi -p 0 --rttnom 1 --rttwr 4\n");

  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
EFIAPI
DdrSettingsAppMain (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  INTN                            Err;
  UINTN                           Index;
  UINTN                           Port;
  UINT16                          SettingWasUsed;
  DDR_SPD_CONFIG                  NewFlashBuf;
  DDR_SPD_CONFIG                  FlashBuf;
  EFI_STATUS                      Status;
  EFI_SHELL_PARAMETERS_PROTOCOL  *ShellParameters;
  EFI_SHELL_PROTOCOL             *ShellProtocol;

  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiShellParametersProtocolGuid,
                  (VOID **) &ShellParameters
                  );
  if (EFI_ERROR (Status)) {
    Print (L"Please use UEFI Shell to run this application.\n");
    return Status;
  }

  Status = gBS->LocateProtocol (
                  &gEfiShellProtocolGuid,
                  NULL,
                  (VOID **) &ShellProtocol
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Port   = BAIKAL_FLASH_DUMMY;
  SettingWasUsed = 0x0;
  SetMem (&NewFlashBuf, sizeof (DDR_SPD_CONFIG), (UINT8) BAIKAL_FLASH_DUMMY);

  for (Index = 1; Index < ShellParameters->Argc; Index++) {
    if (!StrCmp (ShellParameters->Argv[Index], L"-h") ||
        !StrCmp (ShellParameters->Argv[Index], L"--help")) {
      goto PrintHelp;
    }
    if (!StrCmp (ShellParameters->Argv[Index], L"--help-rtts")) {
      DdrProcessRttValue (TRUE, NULL);
      return EFI_SUCCESS;
    }
    if (!StrCmp (ShellParameters->Argv[Index], L"-p") ||
        !StrCmp (ShellParameters->Argv[Index], L"--port")) {
      Port = ShellStrToUintn (ShellParameters->Argv[++Index]);
      if (Port > 1) {
        Print (L"ERROR: port can be 0 or 1\n");
        Status = EFI_INVALID_PARAMETER;
        goto Exit;
      }
    }
    if (!StrCmp (ShellParameters->Argv[Index], L"-sb") ||
        !StrCmp (ShellParameters->Argv[Index], L"--speedbin")) {
      NewFlashBuf.SpeedBin = ShellStrToUintn (ShellParameters->Argv[++Index]);
    }
    if (!StrCmp (ShellParameters->Argv[Index], L"--dic")) {
      NewFlashBuf.Dic = ShellStrToUintn (ShellParameters->Argv[++Index]);
      if (NewFlashBuf.Dic > 1) {
        Print (L"ERROR: Wrong value for DIC.\n");
        Status = EFI_INVALID_PARAMETER;
        goto Exit;
      }
    }
    if (!StrCmp (ShellParameters->Argv[Index], L"--rttwr")) {
      NewFlashBuf.RttWr = ShellStrToUintn (ShellParameters->Argv[++Index]);
      if (NewFlashBuf.RttWr > 4) {
        Print (L"ERROR: Wrong value for RTT WR\n");
        Status = EFI_INVALID_PARAMETER;
        goto Exit;
      }
    }
    if (!StrCmp (ShellParameters->Argv[Index], L"--rttnom")) {
      NewFlashBuf.RttNom = ShellStrToUintn (ShellParameters->Argv[++Index]);
      if (NewFlashBuf.RttNom > 7) {
        Print (L"ERROR: Wrong value for RTT NOM\n");
        Status = EFI_INVALID_PARAMETER;
        goto Exit;
      }
    }
    if (!StrCmp (ShellParameters->Argv[Index], L"--rttpark")) {
      NewFlashBuf.RttPark = ShellStrToUintn (ShellParameters->Argv[++Index]);
      if (NewFlashBuf.RttPark > 7) {
        Print (L"ERROR: Wrong value for RTT PARK\n");
        Status = EFI_INVALID_PARAMETER;
        goto Exit;
      }
    }
    if (!StrCmp (ShellParameters->Argv[Index], L"flash")) {
      if (NewFlashBuf.SpdFlagU != BAIKAL_FLASH_USE_SPD) {
        NewFlashBuf.SpdFlagU = BAIKAL_FLASH_USE_STR;
      } else {
        Print (L"\nWrong usage!\n");
        goto PrintHelp;
      }
    }
    if (!StrCmp (ShellParameters->Argv[Index], L"spd")) {
      if (NewFlashBuf.SpdFlagU != BAIKAL_FLASH_USE_STR) {
        NewFlashBuf.SpdFlagU = BAIKAL_FLASH_USE_SPD;
      } else {
        Print (L"\nWrong usage!\n");
        goto PrintHelp;
      }
    }
  }

  if (Port == BAIKAL_FLASH_DUMMY) {
    Print (L"ERROR: Port wasn't specified.\n");
    goto Exit;
  }

  for (Index = 0; Index < sizeof(DDR_SPD_CONFIG) / sizeof (UINT16); Index++) {
    if (((UINT16 *)&NewFlashBuf)[Index] != BAIKAL_FLASH_DUMMY) {
      SettingWasUsed = 0xF;
      break;
    }
  }

  if (SettingWasUsed == 0x0) {
    Print (L"Provide new settings.\n");
    goto Exit;
  }

  switch (NewFlashBuf.SpeedBin) {
    case 2400:
      NewFlashBuf.SpeedBin = FLASH_SPEEDBIN_2400;
      break;
    case 2133:
      NewFlashBuf.SpeedBin = FLASH_SPEEDBIN_2133;
      break;
    case 1866:
      NewFlashBuf.SpeedBin = FLASH_SPEEDBIN_1866;
      break;
    case 1600:
      NewFlashBuf.SpeedBin = FLASH_SPEEDBIN_1600;
      break;
    case BAIKAL_FLASH_DUMMY:
      break;
    default:
      Print (L"\nWrong value of a SpeedBin!\n");
      goto PrintHelp;
  }

  Status = DdrProcessRttValue (FALSE, &NewFlashBuf);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Print (L"DdrSettingsApp: unlocking...\n");
  SmcFlashLock (0);

  Err = SmcFlashRead (BAIKAL_STORAGE_ADDR + (BAIKAL_STR_PORT_OFFS * Port),
                             &FlashBuf, sizeof (DDR_SPD_CONFIG));
  if (Err) {
    Print (L"DdrSettingsApp: error %d\n", Err);
    goto ExitLock;
  }

  for (Index = 0; Index < sizeof(DDR_SPD_CONFIG) / sizeof (UINT16); Index++) {
    if (((UINT16 *)&NewFlashBuf)[Index] != BAIKAL_FLASH_DUMMY) {
      ((UINT16 *)&FlashBuf)[Index] = ((UINT16 *)&NewFlashBuf)[Index];
    }
  }

  Print (L"DdrSettingsApp: erasing...\n");
  Err = SmcFlashErase (BAIKAL_STORAGE_ADDR + (BAIKAL_STR_PORT_OFFS * Port), 512);
  if (Err) {
    Print (L"DdrSettingsApp: error %d\n", Err);
    goto ExitLock;
  }

  Print (L"DdrSettingsApp: writing...\n");
  Err = SmcFlashWrite (BAIKAL_STORAGE_ADDR + (BAIKAL_STR_PORT_OFFS * Port),
                              &FlashBuf, sizeof (DDR_SPD_CONFIG));
  if (Err) {
    Print (L"DdrSettingsApp: error %d\n", Err);
    goto ExitLock;
  }

ExitLock:
  Print (L"DdrSettingsApp: locking...\n");
  SmcFlashLock (1);

Exit:
  return Status;

PrintHelp:
  Print (L"Change ddr settings on a next start.\n");
  Print (L"\n");
  Print (L"Usage: DdrSettings.efi [option] ...\n");
  Print (L"  -p, --port\n");
  Print (L"    choose a channel for which to apply new settings.\n");
  Print (L"    Possible values are: 0 and 1\n");
  Print (L"\n");
  Print (L"  -sb, --speedbin\n");
  Print (L"    set value of a speedbin on a next start.\n");
  Print (L"    Possible values are: 1600, 1866, 2133, 2400\n");
  Print (L"\n");
  Print (L"  flash, spd\n");
  Print (L"    Use settings stored in FLASH or SPD, accordingly.\n");
  Print (L"\n");
  Print (L"  --help-rtts\n");
  Print (L"    Provide help message for changing rtts values.\n");
  Print (L"\n");
  Print (L"Examples:\n");
  Print (L"  DdrSettings.efi -p 1 spd\n");
  Print (L"  DdrSettings.efi --port 0 flash -sb 2400\n");
  Print (L"  DdrSettings.efi -p 1 --speedbin 1866\n");

  return EFI_SUCCESS;
}
