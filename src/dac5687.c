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

#include <stdio.h>    // for printf(), fprintf()
#include <stdbool.h>  // for bool type
#include <string.h>   // for memcpy()
#include <unistd.h>   // for usleep()

// HIDAPI
#include "hidapi/hidapi.h"

// project libraries
#include "dds-host/mcp2210.h"
#include "dds-host/dac5687.h"

bool DAC5687_WriteRegister(hid_device *handle, DAC5687Address addr, unsigned char txByte) {
  if (handle == NULL) {
    fprintf(stderr, "dev must not be null\n");
    return false;
  }

  if (addr == 0x08 || addr == 0x1A || addr >= 0x1D) {
    fprintf(stderr, "can't write to address %x# as it's for factory use only\n", addr);
    return false;
  }

  // get current SPI settings
  MCP2210SPITransferSettings spiSettings = {0};
  if (MCP2210_ReadSpiSettings(handle, &spiSettings, true) < 0) {
    fprintf(stderr, "WriteRegister()->ReadSpiSettings() failed\n");
    return false;
  }

  // number of bytes in the transfer + 1 for the instruction cycle
  spiSettings.bitRate = 3000000;

  spiSettings.bytesPerTransaction = 2;

  spiSettings.csToDataDelay = 0x01;

  spiSettings.dataToDataDelay = 0x00;

  spiSettings.lastDataToCSDelay = 0x01;
  // CS_DAC is high when idle
  spiSettings.idleCSValue = 0x0003;

  // CS_DAC is low when active
  spiSettings.activeCSValue = 0x0002;

  MCP2210ChipSettings chipSettings = {0};

  if (MCP2210_ReadChipSettings(handle, &chipSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->ReadChipSettings() failed\n");
    return false;
  }

  chipSettings.gp0Designation = CS;
  chipSettings.gp1Designation = GPIO;
  chipSettings.gp5Designation = DF;

  chipSettings.defaultGPIODirection = 0x0000;
  chipSettings.defaultGPIOValue = 0xFFFF;

  if (MCP2210_WriteChipSettings(handle, &chipSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->WriteChipSettings() failed\n");
    return false;
  }
  
  // construct the instruction cycle byte
  unsigned char instrByte = (addr & 0x1F);

  unsigned char spiTxBytes[2] = {instrByte, txByte};
  unsigned char rxBuf[2];

  if (MCP2210_SpiDataTransfer(handle, 2, spiTxBytes, rxBuf, &spiSettings) < 0) {
    fprintf(stderr, "WriteRegister() failed\n");
    return false;
  } 
  return true;
}

bool DAC5687_WriteRegisters(hid_device *handle, DAC5687Address startAddr, unsigned char *txBytes, unsigned int bytes) {
  if (handle == NULL) {
    fprintf(stderr, "handle can't be null\n");
    return false;
  }

  if (txBytes == NULL) {
    fprintf(stderr, "txBytes must not be null\n");
    return false;
  }

  if (bytes > 4) {
    fprintf(stderr, "can't write more than 4 more registers\n");
    return false;
  }

  if (startAddr == 0x08 || startAddr == 0x1A || startAddr >= 0x1D) {
    fprintf(stderr, "can't write to address %x# as it's for factory use only\n", startAddr);
    return false;
  }

  if (bytes > (0x08 - startAddr) || bytes > (0x1A - startAddr) || bytes > (0x1D - startAddr)) {
    fprintf(stderr, "write intersects with factory use only register\n");
    return false;
  }

  // get current SPI settings
  MCP2210SPITransferSettings spiSettings = {0};
  if (MCP2210_ReadSpiSettings(handle, &spiSettings, true) < 0) {
    fprintf(stderr, "WriteRegisters()->ReadSpiSettings() failed\n");
    return false;
  }

  // number of bytes in the transfer + 1 for the instruction cycle
  spiSettings.bitRate = 3000000;

  spiSettings.bytesPerTransaction = bytes + 1;

  spiSettings.csToDataDelay = 0x01;

  spiSettings.dataToDataDelay = 0x00;

  spiSettings.lastDataToCSDelay = 0x01;
  // CS_DAC is high when idle
  spiSettings.idleCSValue = 0x0001;

  // CS_DAC is low when active
  spiSettings.activeCSValue = 0x0000;

  // construct the instruction cycle byte
  unsigned int writeBytes = bytes - 1;
  unsigned char instrByte = (startAddr & 0x1F) | ((writeBytes & 0x03) << 5);

  unsigned char spiTxBytes[bytes + 1];
  unsigned char rxBuf[bytes + 1];

  spiTxBytes[0] = instrByte;
  memcpy(&spiTxBytes[1], txBytes, bytes);
  memset(rxBuf, 0, sizeof(rxBuf));

  if (MCP2210_SpiDataTransfer(handle, sizeof(spiTxBytes), spiTxBytes, rxBuf, &spiSettings) < 0) {
    fprintf(stderr, "WriteRegisters() failed\n");
    return false;
  } 
  return true;
}

bool DAC5687_ReadRegister(hid_device *handle, DAC5687Address addr, unsigned char *rxByte) {
  if (handle == NULL) {
    fprintf(stderr, "dev must not be null\n");
    return false;
  }

  if (rxByte == NULL) {
    fprintf(stderr, "output byte can't be null\n");
    return false;
  }

  // get current SPI settings
  MCP2210SPITransferSettings spiSettings = {0};
  if (MCP2210_ReadSpiSettings(handle, &spiSettings, true) < 0) {
    fprintf(stderr, "WriteRegister()->ReadSpiSettings() failed\n");
    return false;
  }

  // number of bytes in the transfer + 1 for the instruction cycle
  spiSettings.bitRate = 3000000;

  spiSettings.bytesPerTransaction = 2;

  spiSettings.csToDataDelay = 0x01;

  spiSettings.dataToDataDelay = 0x00;

  spiSettings.lastDataToCSDelay = 0x01;
  // CS_DAC is high when idle
  spiSettings.idleCSValue = 0x0003;

  // CS_DAC is low when active
  spiSettings.activeCSValue = 0x0002;

  MCP2210ChipSettings chipSettings = {0};

  if (MCP2210_ReadChipSettings(handle, &chipSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->ReadChipSettings() failed\n");
    return false;
  }

  chipSettings.gp0Designation = CS;
  chipSettings.gp1Designation = GPIO;
  chipSettings.gp5Designation = DF;

  chipSettings.defaultGPIODirection = 0x0000;
  chipSettings.defaultGPIOValue = 0xFFFF;

  if (MCP2210_WriteChipSettings(handle, &chipSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->WriteChipSettings() failed\n");
    return false;
  }
  
  // construct the instruction cycle byte
  unsigned char instrByte = (0x1 << 7) | (addr & 0x1F);

  unsigned char spiTxBytes[2] = {instrByte, 0x00};
  unsigned char rxBuf[2];

  if (MCP2210_SpiDataTransfer(handle, sizeof(spiTxBytes), spiTxBytes, rxBuf, &spiSettings) < 0) {
    fprintf(stderr, "RegisterRead() failed\n");
    return false;
  }
  *rxByte = rxBuf[1];
  return true;
}

bool DAC5687_ReadRegisters(hid_device *handle, DAC5687Address startAddr, unsigned char *rxBytes, unsigned int bytes) {
  if (handle == NULL) {
    fprintf(stderr, "handle can't be null\n");
    return false;
  }

  if (rxBytes == NULL) {
    fprintf(stderr, "rxBytes must not be null\n");
    return false;
  }

  if (bytes > 4) {
    fprintf(stderr, "can't read more than 4 more registers\n");
    return false;
  }

  if (startAddr == 0x08 || startAddr == 0x1A || startAddr >= 0x1D) {
    fprintf(stderr, "can't read from address %x# as it's for factory use only\n", startAddr);
    return false;
  }

  if (bytes > (0x08 - startAddr) || bytes > (0x1A - startAddr) || bytes > (0x1D - startAddr)) {
    fprintf(stderr, "read intersects with factory use only register\n");
    return false;
  }

  // get current SPI settings
  MCP2210SPITransferSettings spiSettings = {0};
  if (MCP2210_ReadSpiSettings(handle, &spiSettings, true) < 0) {
    fprintf(stderr, "ReadRegisters()->ReadSpiSettings() failed\n");
    return false;
  }

  // number of bytes in the transfer + 1 for the instruction cycle
  spiSettings.bitRate = 3000000;

  spiSettings.bytesPerTransaction = bytes + 1;

  spiSettings.csToDataDelay = 0x01;

  spiSettings.dataToDataDelay = 0x00;

  spiSettings.lastDataToCSDelay = 0x01;

  // CS_DAC is high when idle
  spiSettings.idleCSValue = 0x0001;

  // CS_DAC is low when active
  spiSettings.activeCSValue = 0x0000;
  
  // construct the instruction cycle byte
  unsigned char readBytes = bytes - 1;
  unsigned char instrByte = (0x1 << 7) | (startAddr & 0x1F) | ((readBytes & 0x03) << 5);

  unsigned char spiTxBytes[bytes + 1];
  unsigned char rxBuf[bytes + 1];

  spiTxBytes[0] = instrByte;
  memset(&spiTxBytes[1], 0, bytes);
  memset(rxBytes, 0, sizeof(rxBuf));

  if (MCP2210_SpiDataTransfer(handle, sizeof(spiTxBytes), spiTxBytes, rxBuf, &spiSettings) < 0) {
    fprintf(stderr, "ReadRegisters() failed\n");
    return false;
  }
  memcpy(rxBytes, &rxBuf[1], bytes);
  return true;
}