/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "RamMenuComponent.h"

EFI_STATUS
EFIAPI
RamMenuDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RamMenuDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RamMenuDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer OPTIONAL
  )
{
  return EFI_SUCCESS;
}
