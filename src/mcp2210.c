/*
 * MIT License
 * 
 * Copyright (c) 2020 Eli Reed
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Defines the interface for communicating with the MCP2210
 * USB-to-SPI Controller
 */

// C System Libraries
#include <string.h>   // for strlen(), memset()
#include <stdio.h>    // for fprintf(), stderr
#include <stdbool.h>  // for bool type and true/false macros
#include <unistd.h>   // for usleep()

// HIDAPI
#include "hidapi/hidapi.h"

// MCP2210
#include "dds-host/mcp2210.h"

static int MCP2210_GenericWriteRead(hid_device *handle, uint8_t *txBuf, uint8_t *rxBuf) {
  if (handle == NULL) {
    fprintf(stderr, "GenericWriteRead()-> handle can't be null\n");
    return -1;
  }

  if (txBuf == NULL) {
    fprintf(stderr, "GenericWriteRead()-> txBuf can't be null\n");
    return -1;
  }

  if (rxBuf == NULL) {
    fprintf(stderr, "GenericWriteRead()-> rxBuf can't be null\n");
    return -1;
  }

  int res = hid_write(handle, txBuf, MCP2210_REPORT_LEN);

  if (res < 0) {
    fprintf(stderr, "GenericWriteRead()->hid_write() failed\n");
    return -1;
  }

  res = hid_read(handle, rxBuf, MCP2210_REPORT_LEN);

  if (res < 0) {
    fprintf(stderr, "GenericWriteRead()->hid_read() failed\n");
    return -1;
  }

  // return the error code
  return rxBuf[1];
}

hid_device * MCP2210_Init() {
  // initialize the underlying HID interface
  int res = hid_init();

  if (res < 0) {
    fprintf(stderr, "Failed to initialize HIDAPI\n");
    return false;
  }

  // attempt to open the attached MCP2210
  hid_device *handle = hid_open(VID, PID, NULL);

  if (handle == NULL) {
    fprintf(stderr, "Failed to open specified device %#x:%#x\n", VID, PID);
    return NULL;
  }
  return handle;
}

int MCP2210_WriteSpiSettings(hid_device *handle, const MCP2210SPITransferSettings *newSettings, bool vm) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  // determine if we're configuring power-up settings or current settings
  if (vm) {
    txBuf[0] = SetCurrentSpiSettings;
    txBuf[1] = 0x00;
  } else {
    txBuf[0] = SetNVRAMSettings;
    txBuf[1] = SpiSettings;
  }

  txBuf[2] = txBuf[3] = 0x00;

  txBuf[4] = (uint8_t) (newSettings->bitRate & 0xFF);
  txBuf[5] = (uint8_t) ((newSettings->bitRate & 0xFF00) >> 8);
  txBuf[6] = (uint8_t) ((newSettings->bitRate & 0xFF0000) >> 16);
  txBuf[7] = (uint8_t) ((newSettings->bitRate & 0xFF000000) >> 24);

  txBuf[8] = (uint8_t) (newSettings->idleCSValue & 0xFF);
  txBuf[9] = (uint8_t) ((newSettings->idleCSValue & 0xFF00) >> 8);

  txBuf[10] = (uint8_t) (newSettings->activeCSValue & 0xFF);
  txBuf[11] = (uint8_t) ((newSettings->activeCSValue & 0xFF00) >> 8);

  txBuf[12] = (uint8_t) (newSettings->csToDataDelay & 0xFF);
  txBuf[13] = (uint8_t) ((newSettings->csToDataDelay & 0xFF00) >> 8);

  txBuf[14] = (uint8_t) (newSettings->lastDataToCSDelay & 0xFF);
  txBuf[15] = (uint8_t) ((newSettings->lastDataToCSDelay & 0xFF00) >> 8);

  txBuf[16] = (uint8_t) (newSettings->dataToDataDelay & 0xFF);
  txBuf[17] = (uint8_t) ((newSettings->dataToDataDelay & 0xFF00) >> 8);

  txBuf[18] = (uint8_t) (newSettings->bytesPerTransaction & 0xFF);
  txBuf[19] = (uint8_t) ((newSettings->bytesPerTransaction & 0xFF00) >> 8);

  txBuf[20] = newSettings->SPIMode;

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

int MCP2210_ReadSpiSettings(hid_device *handle, MCP2210SPITransferSettings *currentSettings, bool vm) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  if (vm) {
    txBuf[0] = GetCurrentSpiSettings;
    txBuf[1] = 0x00;
  } else {
    txBuf[0] = GetNVRAMSettings;
    txBuf[1] = SpiSettings;
  }

  int res = MCP2210_GenericWriteRead(handle, txBuf, rxBuf);

  if (res == 0x00) {
    currentSettings->bitRate = rxBuf[4] | (rxBuf[5] << 8) | (rxBuf[6] << 16) | (rxBuf[7] << 24);
    currentSettings->idleCSValue = rxBuf[8] | (rxBuf[9] << 8);
    currentSettings->activeCSValue = rxBuf[10] | (rxBuf[11] << 8);
    currentSettings->csToDataDelay = rxBuf[12] | (rxBuf[13] << 8);
    currentSettings->lastDataToCSDelay = rxBuf[14] | (rxBuf[15] << 8);
    currentSettings->dataToDataDelay = rxBuf[16] | (rxBuf[17] << 8);
    currentSettings->bytesPerTransaction = rxBuf[18] | (rxBuf[19] << 8);
    currentSettings->SPIMode = rxBuf[20];
  }
  return res;
}

int MCP2210_WriteChipSettings(hid_device *handle, const MCP2210ChipSettings *newSettings, bool vm) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  if (newSettings == NULL) {
    fprintf(stderr, "newSettings must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  if (vm) {
    txBuf[0] = SetCurrentChipSettings;
    txBuf[1] = 0x00;
  } else {
    txBuf[0] = SetNVRAMSettings;
    txBuf[1] = ChipSettings;
  }

  txBuf[2] = txBuf[3] = 0;

  txBuf[4] = newSettings->gp0Designation;
  txBuf[5] = newSettings->gp1Designation;
  txBuf[6] = newSettings->gp2Designation;
  txBuf[7] = newSettings->gp3Designation;
  txBuf[8] = newSettings->gp4Designation;
  txBuf[9] = newSettings->gp5Designation;
  txBuf[10] = newSettings->gp6Designation;
  txBuf[11] = newSettings->gp7Designation;
  txBuf[12] = newSettings->gp8Designation;

  txBuf[13] = newSettings->defaultGPIOValue & 0xFF;
  txBuf[14] = (uint8_t)((newSettings->defaultGPIOValue & 0xFF00) >> 8);

  txBuf[15] = newSettings->defaultGPIODirection & 0xFF;
  txBuf[16] = (uint8_t)((newSettings->defaultGPIODirection & 0xFF00) >> 8);

  txBuf[17] = newSettings->chipSettings;

  txBuf[18] = newSettings->chipAccessControl;

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

int MCP2210_ReadChipSettings(hid_device *handle, MCP2210ChipSettings *currentSettings, bool vm) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  if (currentSettings == NULL) {
    fprintf(stderr, "currentSettings must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  if (vm) {
    txBuf[0] = GetCurrentChipSettings;
  } else {
    txBuf[0] = GetNVRAMSettings;
    txBuf[1] = ChipSettings;
  }

  int res = MCP2210_GenericWriteRead(handle, txBuf, rxBuf);

  if (res == 0x00) {
    currentSettings->gp0Designation = rxBuf[4];
    currentSettings->gp1Designation = rxBuf[5];
    currentSettings->gp2Designation = rxBuf[6];
    currentSettings->gp3Designation = rxBuf[7];
    currentSettings->gp4Designation = rxBuf[8];
    currentSettings->gp5Designation = rxBuf[9];
    currentSettings->gp6Designation = rxBuf[10];
    currentSettings->gp7Designation = rxBuf[11];
    currentSettings->gp8Designation = rxBuf[12];

    currentSettings->defaultGPIOValue = rxBuf[13] | (rxBuf[14] << 8);
    currentSettings->defaultGPIODirection = rxBuf[15] | (rxBuf[16] << 8);

    currentSettings->chipSettings = rxBuf[17];

    currentSettings->chipAccessControl = rxBuf[18];
  }
  return res;
}

int MCP2210_SendAccessPassword(hid_device *handle, MCP2210AccessPassword pass) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = SendPassword;

  memcpy(&txBuf[4], &pass, sizeof(pass));

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

int MCP2210_WriteManufacturerName(hid_device *handle, const char *newName, size_t nameLen) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (newName == NULL) {
    fprintf(stderr, "newName must not be bull\n");
    return -1;
  }

  if (nameLen > MAX_MAN_STR_LEN) {
    fprintf(stderr, "manufacturer name must be less than or equal to 29 characters\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = SetNVRAMSettings;
  txBuf[1] = ManufacturerName;

  // truncation here is fine, we know nameLen will always be the right length
  txBuf[4] = nameLen * 2 + 2;

  // this is the string descriptor ID, must always be 0x03
  txBuf[5] = 0x03;

  // process the new name
  int i;
  for (i = 0; i < nameLen * 2; i += 2) {
    txBuf[i + 6] = newName[i / 2];
  }

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

int MCP2210_ReadManufacturerName(hid_device *handle, char currentName[30]) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  if (currentName == NULL) {
    fprintf(stderr, "currentName must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = GetNVRAMSettings;
  txBuf[1] = ManufacturerName;
  int res = MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
  if (res == 0x00) {
    int i;
    for (i = 0; i < rxBuf[4]; i += 2) {
      currentName[i / 2] = rxBuf[i];
    }
    currentName[rxBuf[4] / 2] = '\0';
  }
  return res;
}

int MCP2210_WriteProductName(hid_device *handle, const char *newName, size_t nameLen) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (newName == NULL) {
    fprintf(stderr, "newName must not be bull\n");
    return -1;
  }

  if (nameLen > MAX_MAN_STR_LEN) {
    fprintf(stderr, "product name must be less than or equal to 29 characters\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = SetNVRAMSettings;
  txBuf[1] = ProductName;

  // truncation here is fine, we know nameLen will always be the right length
  txBuf[4] = nameLen * 2 + 2;

  // this is the string descriptor ID, must always be 0x03
  txBuf[5] = 0x03;

  // process the new name
  int i;
  for (i = 0; i < nameLen * 2; i += 2) {
    txBuf[i + 6] = newName[i / 2];
  }

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

int MCP2210_ReadProductName(hid_device *handle, char currentName[30]) {
    if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  if (currentName == NULL) {
    fprintf(stderr, "currentName must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = GetNVRAMSettings;
  txBuf[1] = ProductName;
  int res = MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
  if (res == 0x00) {
    int i;
    for (i = 0; i < rxBuf[4]; i += 2) {
      currentName[i / 2] = rxBuf[i];
    }
    currentName[rxBuf[4] / 2] = '\0';
  }
  return res;
}

int MCP2210_WriteGPIOValues(hid_device *handle, uint16_t newGPIOValues) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = GetCurrentGPIOPinVal;

  txBuf[4] = (uint8_t) (newGPIOValues & 0xFF);
  txBuf[5] = (uint8_t) ((newGPIOValues & 0xFF00) >> 8);

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

int MCP2210_ReadGPIOValues(hid_device *handle, uint16_t *currentGPIOValues) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  if (currentGPIOValues == NULL) {
    fprintf(stderr, "currentGPIOValues must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = GetCurrentGPIOPinVal;

  int res = MCP2210_GenericWriteRead(handle, txBuf, rxBuf);

  if (res == 0x00) {
    *currentGPIOValues = rxBuf[4] | (rxBuf[5] << 8);
  }
  return res;
}

int MCP2210_WriteGPIODirections(hid_device *handle, uint16_t newGPIODirections) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = SetCurrentGPIOPinDir;

  txBuf[4] = (uint8_t) (newGPIODirections & 0xFF);
  txBuf[5] = (uint8_t) ((newGPIODirections & 0xFF00) >> 8);

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

int MCP2210_ReadGPIODirections(hid_device *handle, uint16_t *currentGPIODirections) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  if (currentGPIODirections == NULL) {
    fprintf(stderr, "currentGPIODirections must not be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = GetCurrentGPIOPinVal;

  int res = MCP2210_GenericWriteRead(handle, txBuf, rxBuf);

  if (res == 0x00) {
    *currentGPIODirections = rxBuf[4] | (rxBuf[5] << 8);
  }
  return res;
}

int MCP2210_WriteEEPROM(hid_device *handle, unsigned char addr, unsigned char byte) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  if (addr > EEPROM_MAX_ADDR) {
    fprintf(stderr, "addr is out of range\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = WriteEEPROM;

  txBuf[1] = addr;
  txBuf[2] = byte;

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

int MCP2210_ReadEEPROM(hid_device *handle, unsigned char addr, unsigned char *byte) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  if (addr > EEPROM_MAX_ADDR) {
    fprintf(stderr, "addr is out of range\n");
    return -1;
  }

  if (byte == NULL) {
    fprintf(stderr, "byte can't be null\n");
    return -1;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = ReadEEPROM;

  txBuf[1] = addr;

  int res = MCP2210_GenericWriteRead(handle, txBuf, rxBuf);

  if (res == 0x00) {
    *byte = rxBuf[3];
  }
  return res;
}

int MCP2210_ReadInterruptCount(hid_device *handle, unsigned int *interrupts, bool reset) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (interrupts == NULL) {
    fprintf(stderr, "interrupts must not be null\n");
    return false;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = GetCurrentInterruptCount;
  if (reset) {
    txBuf[1] = 0x01;
  }

  int res = MCP2210_GenericWriteRead(handle, txBuf, rxBuf);

  if (res == 0x00) {
    *interrupts = rxBuf[4] | (rxBuf[5] << 8);
  }
  return res;
}

int MCP2210_SpiDataTransfer(hid_device *handle,
                              unsigned int txBytes,
                              unsigned char *txData,
                              unsigned char *rxData,
                              MCP2210SPITransferSettings *settings) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return -1;
  }

  if (txData == NULL) {
    fprintf(stderr, "input buffer must not be null\n");
    return -1;
  }

  if (rxData == NULL) {
    fprintf(stderr, "output buffer must not be empty\n");
    return -1;
  }

  if (txBytes > MAX_TRANSACTION_BYTES) {
    fprintf(stderr, "can't transfer more than 65536 bytes");
    return -1;
  }

  if (settings == NULL) {
    fprintf(stderr, "settings can't be null\n");
    return -1;
  }

  // make sure the transaction is the right length
  settings->bytesPerTransaction = txBytes;
  printf("SPI bytes for this transfer: %d\n", txBytes);

  // write the settings we were given
  if (MCP2210_WriteSpiSettings(handle, settings, true) != 0x00) {
    fprintf(stderr, "Failed to write settings before SPI transfer\n");
    return -1;
  }

  // start processing the input buffer
  // we can transfer at most 60 bytes per spi transfer command,
  // so we need a running total of how many bytes we've sent

  unsigned int bytesLeft = txBytes;
  unsigned int rxBytes = 0;
  
  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  // start writing
  // each loop is a new attempt to transfer a packet
  txBuf[0] = SpiDataTransfer;
  int count = 0;
  do { 
    if (bytesLeft >= 60) {
      txBuf[1] = 60;
      memcpy(&txBuf[4], (txData + txBytes - bytesLeft), 60);
    } else {
      txBuf[1] = bytesLeft;
      memcpy(&txBuf[4], (txData + txBytes - bytesLeft), bytesLeft);
    }

    int res = MCP2210_GenericWriteRead(handle, txBuf, rxBuf);

    if (res == 0x00 && rxBuf[3] == 0x10) {
      fprintf(stderr, "SPI transfer successful\n");
      bytesLeft -= txBuf[1];
    } else if (res == 0xF7) {
      fprintf(stderr, "SPI Bus not available\n");
      return -1;
    } else if (res == 0xF8) {
      fprintf(stderr, "Cannot accept SPI data\n");
      return -1;
    }
    
    if (rxBuf[3] == 0x30 || rxBuf[3] == 0x10) {
      fprintf(stderr, "received data available\n");
      memcpy(&rxData[rxBytes], &rxBuf[4], rxBuf[2]);
      rxBytes += rxBuf[2];
    }
    count++;
  } while (count < 1000 && (rxBuf[3] == 0x30 || rxBuf[3] == 0x20));

  if (count == 1000) {
    fprintf(stderr, "SPI transfer timed out\n");
    return -1;
  }
  return rxBytes;
}

int MCP2210_RequestSpiBusRelease(hid_device *handle) {
  if (handle == NULL) {
    fprintf(stderr, "handle can't be null\n");
    return false;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = ReleaseSpiBus;

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

int MCP2210_CancelSpiDataTransfer(hid_device *handle) {
  if (handle == NULL) {
    fprintf(stderr, "handle can't be null\n");
    return false;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = CancelSpiDataTransfer;

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

int MCP2210_ReadChipStatus(hid_device *handle) {
    if (handle == NULL) {
    fprintf(stderr, "handle can't be null\n");
    return false;
  }

  uint8_t txBuf[MCP2210_REPORT_LEN];
  uint8_t rxBuf[MCP2210_REPORT_LEN];

  memset(txBuf, 0, MCP2210_REPORT_LEN);
  memset(rxBuf, 0, MCP2210_REPORT_LEN);

  txBuf[0] = GetChipStatus;

  return MCP2210_GenericWriteRead(handle, txBuf, rxBuf);
}

void MCP2210_Close(hid_device *handle) {
  if (handle == NULL) {
    fprintf(stderr, "handle can't be null\n");
    return;
  }

  hid_close(handle);
  hid_exit();
}