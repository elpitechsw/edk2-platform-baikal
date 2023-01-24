/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef FRU_INTERNALS_H_
#define FRU_INTERNALS_H_

#pragma pack(1)
typedef struct {
  UINT8  TypeId;
  UINT8  Format;
  UINT8  Length;
  UINT8  RecordChecksum;
  UINT8  HeaderChecksum;
} MULTIRECORD_HEADER;
#pragma pack()

EFI_STATUS
FruInternalsBoardAreaLocate (
  IN   CONST UINT8 * CONST    Buf,
  IN   CONST UINTN            BufSize,
  OUT  CONST UINT8          **BoardArea,
  OUT        UINTN * CONST    BoardAreaSize
  );

EFI_STATUS
FruInternalsMultirecordAreaLocate (
  IN   CONST UINT8 * CONST    Buf,
  IN   CONST UINTN            BufSize,
  OUT  CONST UINT8          **MultirecordArea
  );

EFI_STATUS
FruInternalsProductAreaLocate (
  IN   CONST UINT8 * CONST    Buf,
  IN   CONST UINTN            BufSize,
  OUT  CONST UINT8          **ProductArea,
  OUT        UINTN * CONST    ProductAreaSize
  );

EFI_STATUS
FruInternalsMultirecordCheckData (
  IN  CONST UINT8 * CONST               MrecBuf,
  IN  CONST UINTN                       MrecBufSize,
  IN  CONST MULTIRECORD_HEADER *        MrecHdr
  );

EFI_STATUS
FruInternalsMultirecordParseHeader (
  IN   CONST UINT8 * CONST         MrecBuf,
  IN   CONST UINTN                 MrecBufSize,
  OUT  MULTIRECORD_HEADER **       MrecHdr
  );

CONST VOID *
FruInternalsTypLenEncReadData (
  IN      CONST UINT8 * CONST  EncBuf,
  OUT           CHAR8 * CONST  DecBuf,
  IN OUT        UINTN * CONST  DecLen
  );

EFI_STATUS
FruInternalsGetMultirecord (
  IN   CONST UINT8 * CONST    Buf,
  IN   CONST UINTN            BufSize,
  IN         UINT8            MrecType,
  OUT  CONST MULTIRECORD_HEADER **MrecHeader
  );

EFI_STATUS
FruInternalsSetMultirecord (
  IN   CONST UINT8 * CONST    Buf,
  IN   CONST UINTN            BufSize,
  IN         UINT8            MrecType,
  IN         UINT8            MrecFmt,
  IN         UINT8            MrecLen,
  IN         UINTN            OemId,
  IN   CONST UINT8 *          MrecData
  );

#endif // FRU_INTERNALS_H_
