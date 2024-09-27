/** @file
  Copyright (c) 2023 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "RamMenu.h"
#include "RamMenuHii.h"
#include "RamStruc.h"

extern CHAR16    mIfrVariableName[];
extern EFI_GUID  mRamMenuDriverFormSetGuid;

EFI_STATUS
EFIAPI
RamMenuHiiConfigAccessExtractConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Request,
  OUT       EFI_STRING                      *Progress,
  OUT       EFI_STRING                      *Results
  )
{
  EFI_STATUS                       Status;
  UINTN                            BufferSize;
  RAM_MENU_DEV                     *PrivateData;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;
  EFI_STRING                       ConfigRequest;
  EFI_STRING                       ConfigRequestHdr;
  UINTN                            Size;
  BOOLEAN                          AllocatedRequest;

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Initialize the local variables.
  //
  ConfigRequestHdr  = NULL;
  ConfigRequest     = NULL;
  Size              = 0;
  *Progress         = Request;
  AllocatedRequest  = FALSE;
  PrivateData = BAIKAL_DEV_FROM_THIS (This);
  HiiConfigRouting = PrivateData->HiiConfigRouting;
  //
  // Get Buffer Storage data from EFI variable.
  // Try to get the current setting from variable.
  //
  BufferSize = sizeof (RAM_MENU_STRUC);
  Status = gRT->GetVariable (
                  mIfrVariableName,
                  &mRamMenuDriverFormSetGuid,
                  NULL,
                  &BufferSize,
                  &PrivateData->Configuration
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  if (Request == NULL) {
    DEBUG ((DEBUG_INFO, "\n:: Inside of Extract Config and Request == Null "));
  } else {
    ConfigRequest = Request;
  }
  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  Status = HiiConfigRouting->BlockToConfig (
                               HiiConfigRouting,
                               ConfigRequest,
                               (UINT8 *) &PrivateData->Configuration,
                               BufferSize,
                               Results,
                               Progress
                               );
  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool (ConfigRequest);
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return Status;
}

EFI_STATUS
EFIAPI
RamMenuHiiConfigAccessRouteConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Configuration,
  OUT       EFI_STRING                      *Progress
  )
{
  EFI_STATUS                       Status;
  UINTN                            BufferSize;
  RAM_MENU_DEV                     *PrivateData;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;

  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PrivateData = BAIKAL_DEV_FROM_THIS (This);
  HiiConfigRouting = PrivateData->HiiConfigRouting;
  *Progress = Configuration;

  //
  // Get Buffer Storage data from EFI variable
  //
  BufferSize = sizeof (RAM_MENU_STRUC);
  Status = gRT->GetVariable (
            mIfrVariableName,
            &mRamMenuDriverFormSetGuid,
            NULL,
            &BufferSize,
            &PrivateData->Configuration
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Convert <ConfigResp> to buffer data by helper function ConfigToBlock()
  //
  BufferSize = sizeof (RAM_MENU_STRUC);
  Status = HiiConfigRouting->ConfigToBlock (
                               HiiConfigRouting,
                               Configuration,
                               (UINT8 *) &PrivateData->Configuration,
                               &BufferSize,
                               Progress
                               );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Store Buffer Storage back to EFI variable
  //
  Status = gRT->SetVariable(
                  mIfrVariableName,
                  &mRamMenuDriverFormSetGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof (RAM_MENU_STRUC),
                  &PrivateData->Configuration
                  );

  Status = RamStrucSaveCurrentSettings (&PrivateData->Configuration); // FIXME

  return Status;
}

EFI_STATUS
EFIAPI
RamMenuHiiConfigAccessCallback (
  IN     CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN     EFI_BROWSER_ACTION                     Action,
  IN     EFI_QUESTION_ID                        QuestionId,
  IN     UINT8                                  Type,
  IN OUT EFI_IFR_TYPE_VALUE                     *Value,
  OUT    EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  return EFI_UNSUPPORTED;
}
