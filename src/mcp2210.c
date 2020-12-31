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

// HIDAPI
#include "hidapi/hidapi.h"

// MCP2210
#include "dds-host/mcp2210.h"

// performs a generic write/read operation. Returns false on failure.
// Stores the responses from the MCP2210 in 'response'.
static bool MCP2210_GenericWriteRead(hid_device *handle, 
                                      MCP2210Command cmd,
                                      MCP2210SubCommand subCmd,
                                      MCP2210GenericPacket *data,
                                      MCP2210GenericPacket *response) {
  data->__data__[0] = cmd;
  if (subCmd != None && (cmd == SetNVRAMSettings || cmd == GetNVRAMSettings)) {
    data->__data__[1] = subCmd;
  }

  // write the packet
  int res = hid_write(handle, data->__data__, sizeof(data->__data__));

  if (res == -1) {
    fprintf(stderr, "hid_write failed\n");
    return false;
  }

  // read the response 
  res = hid_read(handle, response->__data__, sizeof(response->__data__));

  if (res == -1) {
    fprintf(stderr, "hid_read failed\n");
    return false;
  }
  return true;
}

bool MCP2210_Init(hid_device **handle) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  // initialize the underlying HID interface
  int res = hid_init();

  if (res == -1) {
    fprintf(stderr, "Failed to initialize HIDAPI\n");
    return false;
  }

  // attempt to open the attached MCP2210
  *handle = hid_open(VID, PID, NULL);

  if (*handle == NULL) {
    fprintf(stderr, "Failed to open specified device %#x:%#x\n", VID, PID);
    return false;
  }

  return true;
}

bool MCP2210_WriteSpiSettings(hid_device *handle, const MCP2210SPITransferSettings *newSettings, bool vm) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  MCP2210GenericPacket response = {0};

  bool result;
  // determine if we're configuring power-up settings or current settings
  if (vm) {
    result = MCP2210_GenericWriteRead(handle, SetCurrentSpiSettings, None, (MCP2210GenericPacket *)newSettings, &response);
  } else {
    result = MCP2210_GenericWriteRead(handle, SetNVRAMSettings, SpiSettings, (MCP2210GenericPacket *)newSettings, &response);
  }

  if (!result) {
    fprintf(stderr, "SPI settings write failed\n");
    return false;
  }

  // TODO: process our response and decide what to do.
  return true;
}

bool MCP2210_ReadSpiSettings(hid_device *handle, MCP2210SPITransferSettings *currentSettings, bool vm) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  MCP2210GenericPacket data = {0};

  bool result;
  if (vm) {
    result = MCP2210_GenericWriteRead(handle, GetCurrentSpiSettings, None, &data, (MCP2210GenericPacket *)currentSettings);
  } else {
    result = MCP2210_GenericWriteRead(handle, GetNVRAMSettings, SpiSettings, &data, (MCP2210GenericPacket *)currentSettings);
  }

  if (!result) {
    fprintf(stderr, "SPI settings read failed\n");
    return false;
  }

  // TODO: Process our response to make sure nothing goofy happened
  return true;
}

bool MCP2210_WriteChipSettings(hid_device *handle, const MCP2210ChipSettings *newSettings, bool vm) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (newSettings == NULL) {
    fprintf(stderr, "newSettings must not be null\n");
    return false;
  }

  MCP2210GenericPacket response = {0};

  bool result;
  if (vm) {
    result = MCP2210_GenericWriteRead(handle, SetCurrentChipSettings, None, (MCP2210GenericPacket *)newSettings, &response);
  } else {
    result = MCP2210_GenericWriteRead(handle, SetNVRAMSettings, ChipSettings, (MCP2210GenericPacket *)newSettings, &response);
  }

  if (!result) {
    fprintf(stderr, "Chip settings write failed\n");
    return false;
  }

  // TODO: process our response and do something interesting if necessary
  return true;
}

bool MCP2210_ReadChipSettings(hid_device *handle, MCP2210ChipSettings *currentSettings, bool vm) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (currentSettings == NULL) {
    fprintf(stderr, "newSettings must not be null\n");
    return false;
  }

  MCP2210GenericPacket data = {0};

  bool result;
  if (vm) {
    result = MCP2210_GenericWriteRead(handle, GetCurrentChipSettings, None, &data, (MCP2210GenericPacket *)currentSettings);
  } else {
    result = MCP2210_GenericWriteRead(handle, GetNVRAMSettings, ChipSettings, &data, (MCP2210GenericPacket *)currentSettings);
  }

  if (!result) {
    fprintf(stderr, "Chip settings read failed\n");
    return false;
  }

  // TODO: process our response and do something interesting if necessary
  return true;
}

bool MCP2210_SendAccessPassword(hid_device *handle, MCP2210AccessPassword pass) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  MCP2210GenericPacket data = {0};
  MCP2210GenericPacket response = {0};

  memcpy(&data.__data__[4], &pass, sizeof(pass));

  bool result = MCP2210_GenericWriteRead(handle, SendPassword, None, &data, &response);

  if (!result) {
    fprintf(stderr, "Send access password failed\n");
    return false;
  }

  // TODO: Process our response to make sure the password was accepted
  return true;
}

bool MCP2210_WriteManufacturerName(hid_device *handle, const char *newName, size_t nameLen) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (newName == NULL) {
    fprintf(stderr, "newName must not be bull\n");
    return false;
  }

  if (nameLen > MAX_MAN_STR_LEN) {
    fprintf(stderr, "manufacturer name must be less than or equal to 29 characters\n");
    return false;
  }

  // TODO: define a struct we can use here to make this process easier
  MCP2210GenericPacket data = {0};
  MCP2210GenericPacket response = {0};

  // truncation here is fine, we know nameLen will always be the right length
  data.__data__[4] = nameLen * 2 + 2;

  // this is the string descriptor ID, must always be 0x03
  data.__data__[5] = 0x03;

  // process the new name
  int i;
  for (i = 0; i < nameLen * 2; i += 2) {
    data.__data__[i] = newName[i / 2];
  }
  
  if (!MCP2210_GenericWriteRead(handle, SetNVRAMSettings, ManufacturerName, &data, &response)) {
    fprintf(stderr, "Failed to set manufacturer name\n");
    return false;
  }

  // TODO: Process response to check if we need to do anything
  return true;
}

bool MCP2210_ReadManufacturerName(hid_device *handle, char currentName[30]) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (currentName == NULL) {
    fprintf(stderr, "currentName must not be null\n");
    return false;
  }

  // TODO: define a struct we can use here to make this process easier
  MCP2210GenericPacket data = {0};
  MCP2210GenericPacket response = {0};
  
  if (!MCP2210_GenericWriteRead(handle, SetNVRAMSettings, ManufacturerName, &data, &response)) {
    fprintf(stderr, "Failed to read manufacturer name\n");
    return false;
  }

  // TODO: Process response to check if we need to do anything
  // get the current name out
  int i;
  for (i = 0; i < response.__data__[4]; i += 2) {
    currentName[i / 2] = response.__data__[i];
  }
  return true;
}

bool MCP2210_WriteProductName(hid_device *handle, const char *newName, size_t nameLen) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (newName == NULL) {
    fprintf(stderr, "newName must not be bull\n");
    return false;
  }

  if (nameLen > MAX_MAN_STR_LEN) {
    fprintf(stderr, "product name must be less than or equal to 29 characters\n");
    return false;
  }

  // TODO: define a struct we can use here to make this process easier
  MCP2210GenericPacket data = {0};
  MCP2210GenericPacket response = {0};

  // truncation here is fine, we know nameLen will always be the right length
  data.__data__[4] = nameLen * 2 + 2;

  // this is the string descriptor ID, must always be 0x03
  data.__data__[5] = 0x03;

  // process the new name
  int i;
  for (i = 0; i < nameLen * 2; i += 2) {
    data.__data__[i] = newName[i / 2];
  }
  
  if (!MCP2210_GenericWriteRead(handle, SetNVRAMSettings, ProductName, &data, &response)) {
    fprintf(stderr, "Failed to set manufacturer name\n");
    return false;
  }

  // TODO: Process response to check if we need to do anything
  return true;
}

bool MCP2210_ReadProductName(hid_device *handle, char currentName[30]) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (currentName == NULL) {
    fprintf(stderr, "newName must not be bull\n");
    return false;
  }

  // TODO: define a struct we can use here to make this process easier
  MCP2210GenericPacket data = {0};
  MCP2210GenericPacket response = {0};

  
  if (!MCP2210_GenericWriteRead(handle, GetNVRAMSettings, ProductName, &data, &response)) {
    fprintf(stderr, "Failed to set manufacturer name\n");
    return false;
  }

  // TODO: Process response to check if we need to do anything
  // get the current name out
  int i;
  for (i = 0; i < response.__data__[4]; i += 2) {
    currentName[i / 2] = response.__data__[i];
  }
  return true;
}

bool MCP2210_WriteGPIOValues(hid_device *handle, const MCP2210GPIOPinValSettings *newSettings) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (newSettings == NULL) {
    fprintf(stderr, "newSettings must not be null\n");
    return false;
  }

  MCP2210GenericPacket response = {0};

  if (!MCP2210_GenericWriteRead(handle, SetCurrentGPIOPinVal, None, (MCP2210GenericPacket *)newSettings, &response)) {
    fprintf(stderr, "Write GPIO pin values failed\n");
    return false;
  }

  // TODO: process the response to make sure it all worked
  return true;
}

bool MCP2210_ReadGPIOValues(hid_device *handle, MCP2210GPIOPinValSettings *currentSettings) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (currentSettings == NULL) {
    fprintf(stderr, "currentSettings must not be null\n");
    return false;
  }

  MCP2210GenericPacket data = {0};

  if (!MCP2210_GenericWriteRead(handle, GetCurrentGPIOPinVal, None, &data, (MCP2210GenericPacket *)currentSettings)) {
    fprintf(stderr, "Read gpio pin values failed\n");
    return false;
  }

  // TODO: process the response to make sure everything worked
  return true;
}

bool MCP2210_WriteGPIODirections(hid_device *handle, const MCP2210GPIOPinDirSettings *newSettings) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (newSettings == NULL) {
    fprintf(stderr, "newSettings must not be null\n");
    return false;
  }

  MCP2210GenericPacket response = {0};

  if (!MCP2210_GenericWriteRead(handle, SetCurrentGPIOPinDir, None, (MCP2210GenericPacket *)newSettings, &response)) {
    fprintf(stderr, "Write GPIO pin directions failed\n");
    return false;
  }

  // TODO: process the response to make sure it all worked
  return true;
}

bool MCP2210_ReadGPIODirections(hid_device *handle, MCP2210GPIOPinDirSettings *currentSettings) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (currentSettings == NULL) {
    fprintf(stderr, "currentSettings must not be null\n");
    return false;
  }

  MCP2210GenericPacket data = {0};

  if (!MCP2210_GenericWriteRead(handle, GetCurrentGPIOPinDir, None, &data, (MCP2210GenericPacket *)currentSettings)) {
    fprintf(stderr, "Read gpio pin directions failed\n");
    return false;
  }

  // TODO: process the response to make sure everything worked
  return true;
}

bool MCP2210_WriteEEPROM(hid_device *handle, unsigned char addr, unsigned char byte) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (addr > EEPROM_MAX_ADDR) {
    fprintf(stderr, "addr is out of range\n");
    return false;
  }

  MCP2210GenericPacket data = {0};
  MCP2210GenericPacket response = {0};

  data.__data__[1] = addr;
  data.__data__[2] = byte;

  if (!MCP2210_GenericWriteRead(handle, WriteEEPROM, None, &data, &response)) {
    fprintf(stderr, "Write to EEPROM failed\n");
    return false;
  }

  // TODO: process response to make sure nothing went wrong
  return true;
}

bool MCP2210_ReadEEPROM(hid_device *handle, unsigned char addr, unsigned char *byte) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (byte == NULL) {
    fprintf(stderr, "output byte must not be null\n");
    return false;
  }

  MCP2210GenericPacket data = {0};
  MCP2210GenericPacket response = {0};

  data.__data__[1] = addr;

  if (!MCP2210_GenericWriteRead(handle, ReadEEPROM, None, &data, &response)) {
    fprintf(stderr, "read from EEPROM failed\n");
    return false;
  }

  // TODO: process the response to make sure nothing went wrong
  return true;
}

bool MCP2210_ReadInterruptCount(hid_device *handle, unsigned int *interrupts, bool reset) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (interrupts == NULL) {
    fprintf(stderr, "interrupts must not be null\n");
    return false;
  }

  MCP2210GenericPacket data = {0};
  MCP2210GenericPacket response = {0};

  if (reset) {
    data.__data__[1] = 0x01;
  }

  if (!MCP2210_GenericWriteRead(handle, GetCurrentInterruptCount, None, &data, &response)) {
    fprintf(stderr, "read interrupt count failed\n");
    return false;
  }

  // TODO: process the response to make sure nothing went wrong
  *interrupts = response.__data__[4] | (((unsigned int) response.__data__[5]) << 8);
  return true;
}

int MCP2210_SpiDataTransfer(hid_device *handle,
                              unsigned int txBytes,
                              unsigned char *txData,
                              unsigned char *rxData,
                              MCP2210SPITransferSettings *settings) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  if (txData == NULL) {
    fprintf(stderr, "input buffer must not be null\n");
    return false;
  }

  if (rxData == NULL) {
    fprintf(stderr, "output buffer must not be empty\n");
    return false;
  }

  if (txBytes > MAX_TRANSACTION_BYTES) {
    fprintf(stderr, "can't transfer more than 65536 bytes");
    return false;
  }

  if (settings == NULL) {
    fprintf(stderr, "settings can't be null\n");
    return false;
  }

  // make sure the transaction is the right length
  settings->bytes_per_transaction_low = txBytes;
  settings->bytes_per_transaction_high = (txBytes >> 8);

  // write the settings we were given
  if (!MCP2210_WriteSpiSettings(handle, settings, true)) {
    return false;
  }

  // start processing the input buffer
  // we can transfer at most 60 bytes per spi transfer command,
  // so we need a running total of how many bytes we've sent

  unsigned int bytesLeft = txBytes;
  unsigned int rxBytes = 0;
  MCP2210SPIDataTransfer dataTransfer = {0};
  MCP2210SPIDataTransferResponse dataTransferResponse = {0};

  // represents the number of times we want to try a write until
  // we assume it's failed
  unsigned int const kTimeout = 1500;

  // running total of how many times we've tried a write
  unsigned int tries = 0;

  // start writing
  do {
    if (bytesLeft >= 60) {
      dataTransfer.bytes = 60;
      memcpy(&dataTransfer.spi_data, (txData + txBytes - bytesLeft), 60);
    } else {
      dataTransfer.bytes = bytesLeft;
      memcpy(&dataTransfer.spi_data, (txData + txBytes - bytesLeft), bytesLeft);
    }

    if (!MCP2210_GenericWriteRead(handle, SpiDataTransfer, None, (MCP2210GenericPacket *)&dataTransfer, (MCP2210GenericPacket *)&dataTransferResponse)) {
      return -1;
    }

    switch(dataTransferResponse.status) {
      case 0x00:
        // success
        switch (dataTransferResponse.engine_status) {
          case 0x20:
            // no data to receive
            break;
          case 0x30:
            // received data available
            // process the received bytes
          case 0x10:
            // finished
            rxBytes += dataTransferResponse.bytes;
            memcpy((&rxData + rxBytes), dataTransferResponse.spi_data, dataTransferResponse.bytes);
            break;
        }

        // update the bytes left only on success
        bytesLeft -= dataTransfer.bytes;
        break;
      case 0xF7:
        // SPI data not accepted
        printf("SPI data not accepted. External master has control of bus.\n");
        break;
      case 0xF8:
        // transfer in progress
        printf("Transfer still in progress\n");
        break;
      default:
        fprintf(stderr, "unknown response\n");
        break;
    }
    tries++;
  } while(bytesLeft && tries < kTimeout);

  if (tries == kTimeout) {
    fprintf(stderr, "spi transfer timed out\n");
    return -1;
  }
  return rxBytes;
}