/** BaikalI2cDxe.h
  Header defining the constant, base address and functions for I2C controller

  Copyright 2020 Baikal Electronics

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _BAIKAL_I2C_DXE_H_
#define _BAIKAL_I2C_DXE_H_

#include <Uefi.h>
#include <Library/UefiLib.h>

extern
EFI_COMPONENT_NAME2_PROTOCOL gBaikalI2cDriverComponentName2;

EFI_STATUS
BaikalI2cInit (
  IN EFI_HANDLE  DriverBindingHandle,
  IN EFI_HANDLE  ControllerHandle
  );

EFI_STATUS
BaikalI2cRelease (
  IN EFI_HANDLE  DriverBindingHandle,
  IN EFI_HANDLE  ControllerHandle
  );

#endif //_BAIKAL_I2C_DXE_H_
