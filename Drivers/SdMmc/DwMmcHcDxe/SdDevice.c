/** @file
  This file provides some helper functions which are specific for SD card device.

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "DwMmcHcDxe.h"

/**
  Send command GO_IDLE_STATE to the device to make it go to Idle State.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in] PassThru       A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in] Slot           The slot number of the SD card to send the command to.

  @retval EFI_SUCCESS       The SD device is reset correctly.
  @retval Others            The device reset fails.

**/
EFI_STATUS
SdCardReset (
  IN EFI_SD_MMC_PASS_THRU_PROTOCOL      *PassThru,
  IN UINT8                              Slot
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_GO_IDLE_STATE;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeBc;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);

  return Status;
}

/**
  Send command SEND_IF_COND to the device to inquiry the SD Memory Card interface
  condition.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in] PassThru       A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in] Slot           The slot number of the SD card to send the command to.
  @param[in] SupplyVoltage  The supplied voltage by the host.
  @param[in] CheckPattern   The check pattern to be sent to the device.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardVoltageCheck (
  IN EFI_SD_MMC_PASS_THRU_PROTOCOL      *PassThru,
  IN UINT8                              Slot,
  IN UINT8                              SupplyVoltage,
  IN UINT8                              CheckPattern
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_SEND_IF_COND;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeBcr;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR7;
  SdMmcCmdBlk.CommandArgument = (SupplyVoltage << 8) | CheckPattern;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);

  if (!EFI_ERROR (Status)) {
    if (SdMmcStatusBlk.Resp0 != SdMmcCmdBlk.CommandArgument) {
      return EFI_DEVICE_ERROR;
    }
  }

  return Status;
}

/**
  Send command SDIO_SEND_OP_COND to the device to see whether it is SDIO device.

  Refer to SDIO Simplified Spec 3 Section 3.2 for details.

  @param[in] PassThru       A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in] Slot           The slot number of the SD card to send the command to.
  @param[in] VoltageWindow  The supply voltage window.
  @param[in] S18R           The boolean to show if it should switch to 1.8v.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdioSendOpCond (
  IN EFI_SD_MMC_PASS_THRU_PROTOCOL      *PassThru,
  IN UINT8                              Slot,
  IN UINT32                             VoltageWindow,
  IN BOOLEAN                            S18R
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;
  UINT32                                Switch;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SDIO_SEND_OP_COND;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeBcr;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR4;

  Switch = S18R ? BIT24 : 0;

  SdMmcCmdBlk.CommandArgument = (VoltageWindow & 0xFFFFFF) | Switch;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);

  return Status;
}

/**
  Send command SD_SEND_OP_COND to the device to see whether it is SDIO device.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in]  PassThru       A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in]  Slot           The slot number of the SD card to send the command to.
  @param[in]  Rca            The relative device address of addressed device.
  @param[in]  VoltageWindow  The supply voltage window.
  @param[in]  S18R           The boolean to show if it should switch to 1.8v.
  @param[in]  Xpc            The boolean to show if it should provide 0.36w power control.
  @param[in]  Hcs            The boolean to show if it support host capacity info.
  @param[out] Ocr            The buffer to store returned OCR register value.

  @retval EFI_SUCCESS        The operation is done correctly.
  @retval Others             The operation fails.

**/
EFI_STATUS
SdCardSendOpCond (
  IN     EFI_SD_MMC_PASS_THRU_PROTOCOL  *PassThru,
  IN     UINT8                          Slot,
  IN     UINT16                         Rca,
  IN     UINT32                         VoltageWindow,
  IN     BOOLEAN                        S18R,
  IN     BOOLEAN                        Xpc,
  IN     BOOLEAN                        Hcs,
     OUT UINT32                         *Ocr
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;
  UINT32                                Switch;
  UINT32                                MaxPower;
  UINT32                                HostCapacity;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_APP_CMD;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAc;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR1;
  SdMmcCmdBlk.CommandArgument = (UINT32)Rca << 16;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SdMmcCmdBlk.CommandIndex = SD_SEND_OP_COND;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeBcr;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR3;

  Switch       = S18R ? BIT24 : 0;
  MaxPower     = Xpc ? BIT28 : 0;
  HostCapacity = Hcs ? BIT30 : 0;

  SdMmcCmdBlk.CommandArgument = (VoltageWindow & 0xFFFFFF) | Switch | MaxPower | HostCapacity;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);
  if (!EFI_ERROR (Status)) {
    //
    // For details, refer to SD Host Controller Simplified Spec 3.0 Table 2-12.
    //
    *Ocr = SdMmcStatusBlk.Resp0;
  }

  return Status;
}

/**
  Broadcast command ALL_SEND_CID to the bus to ask all the SD devices to send the
  data of their CID registers.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in] PassThru       A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in] Slot           The slot number of the SD card to send the command to.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardAllSendCid (
  IN EFI_SD_MMC_PASS_THRU_PROTOCOL      *PassThru,
  IN UINT8                              Slot
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_ALL_SEND_CID;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeBcr;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR2;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);

  return Status;
}

/**
  Send command SET_RELATIVE_ADDR to the SD device to assign a Relative device
  Address (RCA).

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in]  PassThru      A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in]  Slot          The slot number of the SD card to send the command to.
  @param[out] Rca           The relative device address to assign.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardSetRca (
  IN     EFI_SD_MMC_PASS_THRU_PROTOCOL  *PassThru,
  IN     UINT8                          Slot,
     OUT UINT16                         *Rca
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_SET_RELATIVE_ADDR;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeBcr;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR6;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);
  if (!EFI_ERROR (Status)) {
    *Rca = (UINT16)(SdMmcStatusBlk.Resp0 >> 16);
  }

  return Status;
}

/**
  Send command SEND_CSD to the SD device to get the data of the CSD register.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in]  PassThru      A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in]  Slot          The slot number of the SD card to send the command to.
  @param[in]  Rca           The relative device address of selected device.
  @param[out] Csd           The buffer to store the content of the CSD register.
                            Note the caller should ignore the lowest byte of this
                            buffer as the content of this byte is meaningless even
                            if the operation succeeds.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardGetCsd (
  IN     EFI_SD_MMC_PASS_THRU_PROTOCOL  *PassThru,
  IN     UINT8                          Slot,
  IN     UINT16                         Rca,
     OUT SD_CSD                         *Csd
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_SEND_CSD;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAc;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR2;
  SdMmcCmdBlk.CommandArgument = (UINT32)Rca << 16;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);
  if (!EFI_ERROR (Status)) {
    //
    // For details, refer to SD Host Controller Simplified Spec 3.0 Table 2-12.
    //
    CopyMem (((UINT8*)Csd) + 1, &SdMmcStatusBlk.Resp0, sizeof (SD_CSD) - 1);
  }

  return Status;
}

/**
  Send command SEND_CSD to the SD device to get the data of the CSD register.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in]  PassThru      A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in]  Slot          The slot number of the SD card to send the command to.
  @param[in]  Rca           The relative device address of selected device.
  @param[out] Scr           The buffer to store the content of the SCR register.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardGetScr (
  IN     EFI_SD_MMC_PASS_THRU_PROTOCOL  *PassThru,
  IN     UINT8                          Slot,
  IN     UINT16                         Rca,
     OUT SD_SCR                         *Scr
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_APP_CMD;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAc;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR1;
  SdMmcCmdBlk.CommandArgument = (UINT32)Rca << 16;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SdMmcCmdBlk.CommandIndex = SD_SEND_SCR;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAdtc;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR1;

  Packet.InDataBuffer     = Scr;
  Packet.InTransferLength = sizeof (SD_SCR);

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);

  return Status;
}

/**
  Send command SELECT_DESELECT_CARD to the SD device to select/deselect it.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in]  PassThru      A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in]  Slot          The slot number of the SD card to send the command to.
  @param[in]  Rca           The relative device address of selected device.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardSelect (
  IN EFI_SD_MMC_PASS_THRU_PROTOCOL      *PassThru,
  IN UINT8                              Slot,
  IN UINT16                             Rca
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_SELECT_DESELECT_CARD;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAc;
  if (Rca != 0) {
    SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR1b;
  }
  SdMmcCmdBlk.CommandArgument = (UINT32)Rca << 16;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);

  return Status;
}

/**
  Send command VOLTAGE_SWITCH to the SD device to switch the voltage of the device.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in]  PassThru      A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in]  Slot          The slot number of the SD card to send the command to.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardVoltageSwitch (
  IN EFI_SD_MMC_PASS_THRU_PROTOCOL      *PassThru,
  IN UINT8                              Slot
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_VOLTAGE_SWITCH;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAc;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR1;
  SdMmcCmdBlk.CommandArgument = 0;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);

  return Status;
}

/**
  Send command SET_BUS_WIDTH to the SD device to set the bus width.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in] PassThru       A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in] Slot           The slot number of the SD card to send the command to.
  @param[in] Rca            The relative device address of addressed device.
  @param[in] BusWidth       The bus width to be set, it could be 1 or 4.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardSetBusWidth (
  IN EFI_SD_MMC_PASS_THRU_PROTOCOL      *PassThru,
  IN UINT8                              Slot,
  IN UINT16                             Rca,
  IN UINT8                              BusWidth
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;
  UINT8                                 Value;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_APP_CMD;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAc;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR1;
  SdMmcCmdBlk.CommandArgument = (UINT32)Rca << 16;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SdMmcCmdBlk.CommandIndex = SD_SET_BUS_WIDTH;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAc;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR1;

  if (BusWidth == 1) {
    Value = 0;
  } else if (BusWidth == 4) {
    Value = 2;
  } else {
    return EFI_INVALID_PARAMETER;
  }

  SdMmcCmdBlk.CommandArgument = Value & 0x3;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);
  return Status;
}

/**
  Send command SWITCH_FUNC to the SD device to check switchable function or switch card function.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in]  PassThru      A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in]  Slot          The slot number of the SD card to send the command to.
  @param[in]  AccessMode    The value for access mode group.
  @param[in]  CommandSystem The value for command set group.
  @param[in]  DriveStrength The value for drive length group.
  @param[in]  PowerLimit    The value for power limit group.
  @param[in]  Mode          Switch or check function.
  @param[out] SwitchResp    The return switch function status.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardSwitch (
  IN     EFI_SD_MMC_PASS_THRU_PROTOCOL  *PassThru,
  IN     UINT8                          Slot,
  IN     UINT8                          AccessMode,
  IN     UINT8                          CommandSystem,
  IN     UINT8                          DriveStrength,
  IN     UINT8                          PowerLimit,
  IN     BOOLEAN                        Mode,
     OUT UINT8                          *SwitchResp
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;
  UINT32                                ModeValue;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_SWITCH_FUNC;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAdtc;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR1;

  ModeValue = Mode ? BIT31 : 0;
  SdMmcCmdBlk.CommandArgument = (AccessMode & 0xF) | ((PowerLimit & 0xF) << 4) | \
                                ((DriveStrength & 0xF) << 8) | ((DriveStrength & 0xF) << 12) | \
                                ModeValue;

  Packet.InDataBuffer     = SwitchResp;
  Packet.InTransferLength = 64;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);

  return Status;
}

/**
  Send command SEND_STATUS to the addressed SD device to get its status register.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in]  PassThru      A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in]  Slot          The slot number of the SD card to send the command to.
  @param[in]  Rca           The relative device address of addressed device.
  @param[out] DevStatus     The returned device status.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardSendStatus (
  IN     EFI_SD_MMC_PASS_THRU_PROTOCOL  *PassThru,
  IN     UINT8                          Slot,
  IN     UINT16                         Rca,
     OUT UINT32                         *DevStatus
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_SEND_STATUS;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAc;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR1;
  SdMmcCmdBlk.CommandArgument = (UINT32)Rca << 16;

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);
  if (!EFI_ERROR (Status)) {
    *DevStatus = SdMmcStatusBlk.Resp0;
  }

  return Status;
}

/**
  Send command SEND_TUNING_BLOCK to the SD device for HS200 optimal sampling point
  detection.

  It may be sent up to 40 times until the host finishes the tuning procedure.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 for details.

  @param[in] PassThru       A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in] Slot           The slot number of the SD card to send the command to.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardSendTuningBlk (
  IN EFI_SD_MMC_PASS_THRU_PROTOCOL      *PassThru,
  IN UINT8                              Slot
  )
{
  EFI_SD_MMC_COMMAND_BLOCK              SdMmcCmdBlk;
  EFI_SD_MMC_STATUS_BLOCK               SdMmcStatusBlk;
  EFI_SD_MMC_PASS_THRU_COMMAND_PACKET   Packet;
  EFI_STATUS                            Status;
  UINT8                                 TuningBlock[64];

  ZeroMem (&SdMmcCmdBlk, sizeof (SdMmcCmdBlk));
  ZeroMem (&SdMmcStatusBlk, sizeof (SdMmcStatusBlk));
  ZeroMem (&Packet, sizeof (Packet));

  Packet.SdMmcCmdBlk    = &SdMmcCmdBlk;
  Packet.SdMmcStatusBlk = &SdMmcStatusBlk;
  Packet.Timeout        = DW_MMC_HC_GENERIC_TIMEOUT;

  SdMmcCmdBlk.CommandIndex = SD_SEND_TUNING_BLOCK;
  SdMmcCmdBlk.CommandType  = SdMmcCommandTypeAdtc;
  SdMmcCmdBlk.ResponseType = SdMmcResponseTypeR1;
  SdMmcCmdBlk.CommandArgument = 0;

  Packet.InDataBuffer     = TuningBlock;
  Packet.InTransferLength = sizeof (TuningBlock);

  Status = DwMmcPassThruPassThru (PassThru, Slot, &Packet, NULL);

  return Status;
}

/**
  Switch the bus width to specified width.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 and
  SD Host Controller Simplified Spec 3.0 section Figure 3-7 for details.

  @param[in] PciIo          A pointer to the EFI_PCI_IO_PROTOCOL instance.
  @param[in] PassThru       A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in] Slot           The slot number of the SD card to send the command to.
  @param[in] Rca            The relative device address to be assigned.
  @param[in] BusWidth       The bus width to be set, it could be 4 or 8.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardSwitchBusWidth (
  IN EFI_PCI_IO_PROTOCOL                *PciIo,
  IN EFI_SD_MMC_PASS_THRU_PROTOCOL      *PassThru,
  IN UINT8                              Slot,
  IN UINT16                             Rca,
  IN BOOLEAN                            IsDdr,
  IN UINT8                              BusWidth
  )
{
  EFI_STATUS          Status;
  UINT32              DevStatus;

  Status = SdCardSetBusWidth (PassThru, Slot, Rca, BusWidth);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SdCardSwitchBusWidth: Switch to bus width %d fails with %r\n", BusWidth, Status));
    return Status;
  }

  Status = SdCardSendStatus (PassThru, Slot, Rca, &DevStatus);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SdCardSwitchBusWidth: Send status fails with %r\n", Status));
    return Status;
  }
  //
  // Check the switch operation is really successful or not.
  //
  if ((DevStatus >> 16) != 0) {
    DEBUG ((DEBUG_ERROR, "SdCardSwitchBusWidth: The switch operation fails as DevStatus is 0x%08x\n", DevStatus));
    return EFI_DEVICE_ERROR;
  }

  Status = DwMmcHcSetBusWidth (PciIo, Slot, IsDdr, BusWidth);

  return Status;
}

/**
  Switch the high speed timing according to request.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 4.7 and
  SD Host Controller Simplified Spec 3.0 section Figure 2-29 for details.

  @param[in] PciIo          A pointer to the EFI_PCI_IO_PROTOCOL instance.
  @param[in] PassThru       A pointer to the EFI_SD_MMC_PASS_THRU_PROTOCOL instance.
  @param[in] Slot           The slot number of the SD card to send the command to.
  @param[in] Rca            The relative device address to be assigned.
  @param[in] S18A           The boolean to show if it's a UHS-I SD card.
  @param[in] BusWidths      The bus width of the SD card.

  @retval EFI_SUCCESS       The operation is done correctly.
  @retval Others            The operation fails.

**/
EFI_STATUS
SdCardSetBusMode (
  IN EFI_PCI_IO_PROTOCOL                *PciIo,
  IN EFI_SD_MMC_PASS_THRU_PROTOCOL      *PassThru,
  IN UINT8                              Slot,
  IN UINT16                             Rca,
  IN BOOLEAN                            S18A,
  IN UINT32                             BusWidths
  )
{
  EFI_STATUS                   Status;
  DW_MMC_HC_SLOT_CAP           *Capability;
  UINT32                       ClockFreq;
  UINT8                        AccessMode;
  UINT8                        SwitchResp[64];
  DW_MMC_HC_PRIVATE_DATA       *Private;
  BOOLEAN                      IsDdr;

  Private = DW_MMC_HC_PRIVATE_FROM_THIS (PassThru);

  Capability = &Private->Capability[Slot];

  if ((Capability->BusWidth == 1) || (Capability->BusWidth == 4)) {
    BusWidths &= Capability[Slot].BusWidth;
  } else {
    DEBUG ((DEBUG_ERROR, "SdCardSetBusMode: BusWidths (%d) in capability are wrong\n", Capability->BusWidth));
    return EFI_INVALID_PARAMETER;
  }

  if (BusWidths == 0) {
    DEBUG ((DEBUG_ERROR, "SdCardSetBusMode: Get wrong BusWidths:%d\n", BusWidths));
    return EFI_INVALID_PARAMETER;
  }

  if (Private->Capability[Slot].Ddr50) {
    IsDdr = TRUE;
  } else {
    IsDdr = FALSE;
  }

  Status = SdCardSwitchBusWidth (PciIo, PassThru, Slot, Rca, IsDdr, BusWidths);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SdCardSetBusMode: Executing SdCardSwitchBusWidth fails with %r\n", Status));
    return Status;
  }

  //
  // Get the supported bus speed from SWITCH cmd return data group #1.
  //
  Status = SdCardSwitch (PassThru, Slot, 0xF, 0xF, 0xF, 0xF, FALSE, SwitchResp);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Calculate supported bus speed/bus width/clock frequency by host and device capability.
  //
  ClockFreq = 0;
  if (S18A && (Capability->Sdr104 != 0) && ((SwitchResp[13] & BIT3) != 0)) {
    ClockFreq = 208;
    AccessMode = 3;
  } else if (S18A && (Capability->Sdr50 != 0) && ((SwitchResp[13] & BIT2) != 0)) {
    ClockFreq = 100;
    AccessMode = 2;
  } else if (S18A && (Capability->Ddr50 != 0) && ((SwitchResp[13] & BIT4) != 0)) {
    ClockFreq = 50;
    AccessMode = 4;
  } else if ((SwitchResp[13] & BIT1) != 0) {
    ClockFreq = 50;
    AccessMode = 1;
  } else {
    ClockFreq = 25;
    AccessMode = 0;
  }

  Status = SdCardSwitch (PassThru, Slot, AccessMode, 0xF, 0xF, 0xF, TRUE, SwitchResp);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((SwitchResp[16] & 0xF) != AccessMode) {
    DEBUG ((DEBUG_ERROR, "SdCardSetBusMode: Switch to AccessMode %d ClockFreq %d fails! The Switch response is 0x%1x\n", AccessMode, ClockFreq, SwitchResp[16] & 0xF));
    return EFI_DEVICE_ERROR;
  }

  DEBUG ((DEBUG_INFO, "SdCardSetBusMode: Switch to AccessMode %d ClockFreq %d \n", AccessMode, ClockFreq));

  Status = DwMmcHcClockSupply (PciIo, Slot, ClockFreq * 1000, *Capability);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Execute SD device identification procedure.

  Refer to SD Physical Layer Simplified Spec 4.1 Section 3.6 for details.

  @param[in] Private        A pointer to the DW_MMC_HC_PRIVATE_DATA instance.
  @param[in] Slot           The slot number of the SD card to send the command to.

  @retval EFI_SUCCESS       There is a SD card.
  @retval Others            There is not a SD card.

**/
EFI_STATUS
SdCardIdentification (
  IN DW_MMC_HC_PRIVATE_DATA             *Private,
  IN UINT8                              Slot
  )
{
  EFI_STATUS                     Status;
  EFI_PCI_IO_PROTOCOL            *PciIo;
  EFI_SD_MMC_PASS_THRU_PROTOCOL  *PassThru;
  UINT32                         Ocr;
  UINT16                         Rca;
  BOOLEAN                        Xpc;
  BOOLEAN                        S18r;
  UINT64                         MaxCurrent;
  SD_SCR                         Scr;
  SD_CSD                         Csd;

  PciIo    = Private->PciIo;
  PassThru = &Private->PassThru;
  //
  // 1. Send Cmd0 to the device
  //
  Status = SdCardReset (PassThru, Slot);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "SdCardIdentification: Executing Cmd0 fails with %r\n", Status));
    return Status;
  }
  MicroSecondDelay (10000);
  //
  // 2. Send Cmd8 to the device
  //
  Status = SdCardVoltageCheck (PassThru, Slot, 0x1, 0xFF);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "SdCardIdentification: Executing Cmd8 fails with %r\n", Status));
    return Status;
  }
  //
  // 3. Send SDIO Cmd5 to the device to the SDIO device OCR register.
  //
  Status = SdioSendOpCond (PassThru, Slot, 0, FALSE);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "SdCardIdentification: Found SDIO device, ignore it as we don't support\n"));
    return EFI_DEVICE_ERROR;
  }
  MicroSecondDelay (10000);
  //
  // 4. Send Acmd41 with voltage window 0 to the device
  //
  Status = SdCardSendOpCond (PassThru, Slot, 0, 0, FALSE, FALSE, FALSE, &Ocr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "SdCardIdentification: Executing SdCardSendOpCond fails with %r\n", Status));
    return EFI_DEVICE_ERROR;
  }

  if (Private->Capability[Slot].Voltage33 != 0) {
    //
    // Support 3.3V
    //
    MaxCurrent = ((UINT32)Private->MaxCurrent[Slot] & 0xFF) * 4;
    S18r = FALSE;
  } else if (Private->Capability[Slot].Voltage30 != 0) {
    //
    // Support 3.0V
    //
    MaxCurrent = (((UINT32)Private->MaxCurrent[Slot] >> 8) & 0xFF) * 4;
    S18r = FALSE;
  } else if (Private->Capability[Slot].Voltage18 != 0) {
    //
    // Support 1.8V
    //
    MaxCurrent = (((UINT32)Private->MaxCurrent[Slot] >> 16) & 0xFF) * 4;
    S18r = TRUE;
  } else {
    ASSERT (FALSE);
    return EFI_DEVICE_ERROR;
  }

  if (MaxCurrent >= 150) {
    Xpc = TRUE;
  } else {
    Xpc = FALSE;
  }

  //
  // 5. Repeatly send Acmd41 with supply voltage window to the device.
  //    Note here we only support the cards complied with SD physical
  //    layer simplified spec version 2.0 and version 3.0 and above.
  //
  do {
    Status = SdCardSendOpCond (PassThru, Slot, 0, Ocr, S18r, Xpc, TRUE, &Ocr);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "SdCardIdentification: SdCardSendOpCond fails with %r Ocr %x, S18r %x, Xpc %x\n", Status, Ocr, S18r, Xpc));
      return EFI_DEVICE_ERROR;
    }
  } while ((Ocr & BIT31) == 0);

  Status = SdCardAllSendCid (PassThru, Slot);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SdCardIdentification: Executing SdCardAllSendCid fails with %r\n", Status));
    return Status;
  }

  Status = SdCardSetRca (PassThru, Slot, &Rca);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SdCardIdentification: Executing SdCardSetRca fails with %r\n", Status));
    return Status;
  }

  Status = SdCardSelect (PassThru, Slot, Rca);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SdCardIdentification: Selecting card fails with %r\n", Status));
    return Status;
  }

  Status = SdCardSwitchBusWidth (PciIo, PassThru, Slot, Rca, FALSE, 1);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SdCardIdentification: Executing SdCardSwitchBusWidth fails with %r\n", Status));
    return Status;
  }

  Status = SdCardGetScr (PassThru, Slot, Rca, &Scr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SdCardIdentification: Executing SdCardGetScr fails with %r\n", Status));
    return Status;
  }

  //
  // Enter Data Tranfer Mode.
  //
  DEBUG ((DEBUG_INFO, "SdCardIdentification: Found a SD device at slot [%d]\n", Slot));
  Private->Slot[Slot].CardType = SdCardType;

  Status = SdCardSetBusMode (PciIo, PassThru, Slot, Rca, S18r, Scr.SdBusWidths);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SdCardGetCsd (PassThru, Slot, Rca, &Csd);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SdCardIdentification: Executing SdCardGetCsd fails with %r\n", Status));
    return Status;
  }
  Private->Slot[Slot].Initialized = TRUE;

  return Status;
}
