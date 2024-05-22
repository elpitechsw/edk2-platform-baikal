/** @file
  Copyright (c) 2023 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "RamMenu.h"
#include "RamMenuHii.h"
#include "RamMenuComponent.h"
#include "RamStruc.h"

EFI_GUID  mRamMenuDriverFormSetGuid  =  BAIKALFORMSET_GUID;

// HII support for Device Path
HII_VENDOR_DEVICE_PATH  mHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    BAIKALFORMSET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

CHAR16 mIfrVariableName[]    = L"BaikalRamStruc";
EFI_HANDLE mDriverHandle[2]  = {NULL, NULL};
RAM_MENU_DEV *PrivateData = NULL;

EFI_DRIVER_BINDING_PROTOCOL gRamMenuDriverBinding = {
  RamMenuDriverBindingSupported,
  RamMenuDriverBindingStart,
  RamMenuDriverBindingStop,
  RAM_MENU_DRIVER_VERSION,
  NULL,
  NULL
};

EFI_STATUS
EFIAPI
RamMenuUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RamMenuEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  // HII Locals
  EFI_HII_PACKAGE_LIST_HEADER     *PackageListHeader;
  EFI_HII_DATABASE_PROTOCOL       *HiiDatabase;
  EFI_HII_HANDLE                   HiiHandle[2] = {0};
  EFI_HII_CONFIG_ROUTING_PROTOCOL *HiiConfigRouting;
  EFI_STRING                       ConfigRequestHdr;
  UINTN                            BufferSize;
  RAM_MENU_STRUC                  *Configuration;
  BOOLEAN                          ActionFlag;

  //
  // Install UEFI Driver Model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gRamMenuDriverBinding,
             ImageHandle,
             NULL,
             NULL
             );
  ASSERT_EFI_ERROR (Status);

  // Initialize the local variables.
  ConfigRequestHdr = NULL;
  //
  // Initialize driver private data
  //
  PrivateData = AllocateZeroPool (sizeof (RAM_MENU_DEV));
  if (PrivateData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  PrivateData->Signature = RAM_MENU_SIGNATURE;

  PrivateData->ConfigAccess.ExtractConfig = RamMenuHiiConfigAccessExtractConfig;
  PrivateData->ConfigAccess.RouteConfig = RamMenuHiiConfigAccessRouteConfig;
  PrivateData->ConfigAccess.Callback = RamMenuHiiConfigAccessCallback;

  //
  // Locate Hii Database protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  PrivateData->HiiDatabase = HiiDatabase;

  //
  // Locate ConfigRouting protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &HiiConfigRouting);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  PrivateData->HiiConfigRouting = HiiConfigRouting;

  //
  // Publish sample Fromset and config access
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mDriverHandle[0],
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &PrivateData->ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  PrivateData->DriverHandle[0] = mDriverHandle[0];
  //
  // Retrieve HII Package List Header on ImageHandle
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageListHeader,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Register list of HII packages in the HII Database
    //
    Status = HiiDatabase->NewPackageList (
                              HiiDatabase,
                              PackageListHeader,
                              mDriverHandle[0],
                              &HiiHandle[0]
                              );
    ASSERT_EFI_ERROR (Status);
  }
  Status = EFI_SUCCESS;

  PrivateData->HiiHandle[0] = HiiHandle[0];

  BufferSize = sizeof (RAM_MENU_STRUC);
  //
  // Initialize configuration data
  //
  Configuration = &PrivateData->Configuration;

  //
  // Try to read NV config EFI variable first
  //
  ConfigRequestHdr = HiiConstructConfigHdr (&mRamMenuDriverFormSetGuid, mIfrVariableName, mDriverHandle[0]);
  ASSERT (ConfigRequestHdr != NULL);

  // IF driver is not part of the Platform then need to get/set defaults for the NVRAM configuration that the driver will use.
  Status = gRT->GetVariable (
            mIfrVariableName,
            &mRamMenuDriverFormSetGuid,
            NULL,
            &BufferSize,
            Configuration
            );
  if (EFI_ERROR (Status)) {  // Not definded yet so add it to the NV Variables.
    Status = RamStrucCurrentSettings (Configuration);
    if (EFI_ERROR (Status)) { // Bad data in flash
      ZeroMem (Configuration, sizeof (RAM_MENU_STRUC));
    }

    Status = gRT->SetVariable(
                  mIfrVariableName,
                  &mRamMenuDriverFormSetGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof (RAM_MENU_STRUC),
                  Configuration
                  );

    if (Configuration->Speedbin0 != 0x0) {
      //
      // Build current menu settings in accordance with what was found in flash
      //
      ActionFlag = HiiSetBrowserData (
                      &mRamMenuDriverFormSetGuid,
                      mIfrVariableName,
                      sizeof (RAM_MENU_STRUC),
                      (UINT8 *)Configuration,
                      NULL
                      );
    } else {
      //
      // EFI variable for NV config doesn't exist, we should build this variable
      // based on default values stored in IFR
      //
      ActionFlag = HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
      ASSERT (ActionFlag);
    }
  } else {
    //
    // EFI variable does exist and Validate Current Setting
    //
    ActionFlag = HiiValidateSettings (ConfigRequestHdr);
    ASSERT (ActionFlag);
  }
  FreePool (ConfigRequestHdr);

  return Status;
}
