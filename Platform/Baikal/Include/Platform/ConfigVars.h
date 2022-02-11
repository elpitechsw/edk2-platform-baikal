/** @file
 *
 *  Copyright (c) 2020 - 2021, Andrei Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) 2020, ARM Limited. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef CONFIG_VARS_H_
#define CONFIG_VARS_H_

typedef struct {
#define ACPI_PCIE_OFF     0
#define ACPI_PCIE_CUSTOM  1
#define ACPI_PCIE_ECAM    2
  UINT32  Mode;
} ACPI_PCIE_VARSTORE_DATA;

typedef struct {
#define ACPI_MSI_NONE     0
#define ACPI_MSI_ITS      1
#define ACPI_MSI_V2M      2
  UINT32  Mode;
} ACPI_MSI_VARSTORE_DATA;

#endif // CONFIG_VARS_H_
