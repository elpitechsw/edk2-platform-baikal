/** @file
  Copyright (c) 2023 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _RAM_MENU_H_
#define _RAM_MENU_H_

#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>

#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/HiiString.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiConfigRouting.h>

#include <Library/UefiRuntimeServicesTableLib.h>

#include "RamMenuDataStruct.h"

#define RAM_MENU_DRIVER_VERSION  0x000000019

#define RAM_MENU_SIGNATURE SIGNATURE_32 ('b', 'k', 'l', 'd')
#define BAIKAL_DEV_FROM_THIS(a)  CR (a, RAM_MENU_DEV, ConfigAccess, RAM_MENU_SIGNATURE)

#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;
#pragma pack()

typedef struct {
  UINT32                           Signature;

  EFI_HANDLE                       Handle;
  RAM_MENU_STRUC                   Configuration;

  EFI_HANDLE                       DriverHandle[2];
  EFI_HII_HANDLE                   HiiHandle[2];
  //
  // Consumed protocol
  //
  EFI_HII_DATABASE_PROTOCOL        *HiiDatabase;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;
  //
  // Produced protocol
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL   ConfigAccess;
} RAM_MENU_DEV;

#endif // _RAM_MENU_H_
