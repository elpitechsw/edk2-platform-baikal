/** @file
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef PCICONFIG_H_
#define PCICONFIG_H_

#define PCI_CONFIG_GUID { \
  0xec6f0d43, 0x29c7, 0x482b, { 0xc4, 0xbf, 0x38, 0xa4, 0xbb, 0x3e, 0xac, 0xfa } \
  }

extern EFI_GUID gPciConfigGuid;

typedef struct {
  UINT8 MaxSpeed[3];
  UINT8 _NotUsed;
} PCI_CONFIG_VARSTORE_DATA;

typedef struct {
  UINT8 Val0:1;
  UINT8 Val1:1;
  UINT8 Val2:1;
} PCI_SEGMENT_MASK_VARSTORE_DATA;

#endif
