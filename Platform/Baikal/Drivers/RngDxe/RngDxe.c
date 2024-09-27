/** @file
  Copyright (c) 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/RngLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/Rng.h>

STATIC
EFI_STATUS
EFIAPI
RngGetInfo (
  IN      EFI_RNG_PROTOCOL   *This,
  IN OUT  UINTN              *RNGAlgorithmListSize,
  OUT     EFI_RNG_ALGORITHM  *RNGAlgorithmList
  )
{
  if (This == NULL || RNGAlgorithmListSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*RNGAlgorithmListSize < sizeof (EFI_RNG_ALGORITHM)) {
    *RNGAlgorithmListSize = sizeof (EFI_RNG_ALGORITHM);
    return EFI_BUFFER_TOO_SMALL;
  }

  if (RNGAlgorithmList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *RNGAlgorithmListSize = sizeof (EFI_RNG_ALGORITHM);
  CopyGuid (RNGAlgorithmList, &gEfiRngAlgorithmRaw);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
RngGetRNG (
  IN  EFI_RNG_PROTOCOL   *This,
  IN  EFI_RNG_ALGORITHM  *RNGAlgorithm, OPTIONAL
  IN  UINTN               RNGValueLength,
  OUT UINT8              *RNGValue
  )
{
  UINT16  Rand;

  if (This == NULL || RNGValueLength == 0 || RNGValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (RNGAlgorithm != NULL && !CompareGuid (RNGAlgorithm, &gEfiRngAlgorithmRaw)) {
    return EFI_UNSUPPORTED;
  }

  do {
    if (!GetRandomNumber16(&Rand)) {
      return EFI_DEVICE_ERROR;
    }

    if (RNGValueLength >= sizeof (UINT16)) {
      *RNGValue = (UINT8)(Rand >> 8);
      ++RNGValue;
      --RNGValueLength;
    }

    *RNGValue = (UINT8)Rand;
    ++RNGValue;
    --RNGValueLength;
  } while (RNGValueLength != 0);

  return EFI_SUCCESS;
}

STATIC EFI_RNG_PROTOCOL mRngProtocol = {
  RngGetInfo,
  RngGetRNG
};

EFI_STATUS
EFIAPI
RngDxeEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiRngProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mRngProtocol
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
