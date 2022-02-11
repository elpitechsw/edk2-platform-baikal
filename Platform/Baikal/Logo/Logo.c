/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

typedef struct {
  EFI_IMAGE_ID                           ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  Attribute;
  INTN                                   OffsetX;
  INTN                                   OffsetY;
} LOGO_ENTRY;

STATIC EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
STATIC EFI_HII_HANDLE              mHiiHandle;
STATIC LOGO_ENTRY                  mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

EFI_STATUS
EFIAPI
GetImage (
  IN      EDKII_PLATFORM_LOGO_PROTOCOL           *This,
  IN OUT  UINT32                                 *Instance,
     OUT  EFI_IMAGE_INPUT                        *Image,
     OUT  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
     OUT  INTN                                   *OffsetX,
     OUT  INTN                                   *OffsetY
  )
{
  UINT32  Current;

  if (Instance == NULL || Image == NULL || Attribute == NULL ||
      OffsetX == NULL || OffsetY == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  ++(*Instance);
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;

  return mHiiImageEx->GetImageEx (
                        mHiiImageEx,
                        mHiiHandle,
                        mLogos[Current].ImageId,
                        Image
                        );
}

STATIC EDKII_PLATFORM_LOGO_PROTOCOL mPlatformLogo = {
  GetImage
};

EFI_STATUS
EFIAPI
InitializeLogo (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                    Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                    Handle;

  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **) &HiiDatabase
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **) &mHiiImageEx
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **) &PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "HII Image Package with logo not found in PE/COFF resource section\n"
      ));

    return Status;
  }

  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (!EFI_ERROR (Status)) {
    Handle = NULL;
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Handle,
                    &gEdkiiPlatformLogoProtocolGuid,
                    &mPlatformLogo,
                    NULL
                    );
  }
  return Status;
}
