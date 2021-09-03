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

#include <stdio.h>      // for printf(), fprintf()
#include <stdbool.h>    // for bool type and true/false macros
#include <stdint.h>     // for fixed-width integer types
#include <string.h>     // for memset(), memcpy()

// HIDAPI
#include <hidapi.h>

// MCP2210
#include "mcp2210.h"

// CPLD
#include "cpld.h"

bool CPLD_WriteSRAMAddress(hid_device *handle, unsigned int addr, unsigned int txData) {
  if (handle == NULL) {
    fprintf(stderr, "handle can't be null\n");
    return false;
  }

  if (addr > SRAM_MAX_ADDRESS) {
    fprintf(stderr, "addr is out of range\n");
    return false;
  }

  uint8_t txBytes[SRAM_PACKET_SIZE];
  uint8_t rxBytes[SRAM_PACKET_SIZE];

  memset(txBytes, 0, sizeof(txBytes));
  memset(rxBytes, 0, sizeof(rxBytes));

  txBytes[0] = (uint8_t)((addr & 0x03) << 6);
  txBytes[1] = (uint8_t)((addr & 0x1FC) >> 2);
  txBytes[2] = (uint8_t)((addr & 0xFE00) >> 10);
  memcpy(&txBytes[3], &txData, sizeof(txData));

  MCP2210SPITransferSettings spiSettings = {0};

  if (MCP2210_ReadSpiSettings(handle, &spiSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->ReadSpiSettings() failed\n");
    return false;
  }

  // number of bytes in the transfer + 1 for the instruction cycle
  spiSettings.bitRate = 3000000;

  spiSettings.bytesPerTransaction = SRAM_PACKET_SIZE;

  spiSettings.csToDataDelay = 0x01;

  spiSettings.dataToDataDelay = 0x00;

  spiSettings.lastDataToCSDelay = 0x01;

  // CS_MEM is high when idle
  spiSettings.idleCSValue = 0x0002;

  // CS_MEM is low when active
  spiSettings.activeCSValue = 0x0000;

  if (MCP2210_WriteSpiSettings(handle, &spiSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->WriteSpiSettings() failed\n");
    return false;
  }

  MCP2210ChipSettings chipSettings = {0};

  if (MCP2210_ReadChipSettings(handle, &chipSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->ReadChipSettings() failed\n");
    return false;
  }

  chipSettings.gp0Designation = GPIO;
  chipSettings.gp1Designation = CS;
  chipSettings.gp5Designation = DF;

  chipSettings.defaultGPIODirection = 0x0000;
  chipSettings.defaultGPIOValue = 0xFFFF;

  if (MCP2210_WriteChipSettings(handle, &chipSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->WriteChipSettings() failed\n");
    return false;
  }

  // attempt the data transfer
  if (MCP2210_SpiDataTransfer(handle, SRAM_PACKET_SIZE, txBytes, rxBytes, &spiSettings) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->SpiDataTransfer() failed\n");
    return false;
  }
  return true;
}

bool CPLD_ReadSRAMAddress(hid_device *handle, unsigned int addr, unsigned int *rxData) {
  if (handle == NULL) {
    fprintf(stderr, "handle can't be null\n");
    return false;
  }

  if (addr > SRAM_MAX_ADDRESS) {
    fprintf(stderr, "addr is out of range\n");
    return false;
  }

  uint8_t txBytes[SRAM_PACKET_SIZE];
  uint8_t rxBytes[SRAM_PACKET_SIZE];

  memset(txBytes, 0, sizeof(txBytes));
  memset(rxBytes, 0, sizeof(rxBytes));

  // construct the instruction cycle
  txBytes[0] = (uint8_t)(((addr & 0x03) << 6) | 0x01);
  txBytes[1] = (uint8_t)((addr & 0x1FC) >> 2);
  txBytes[2] = (uint8_t)((addr & 0xFE00) >> 10);

  MCP2210SPITransferSettings spiSettings = {0};

  if (MCP2210_ReadSpiSettings(handle, &spiSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->ReadSpiSettings() failed\n");
    return false;
  }

  // number of bytes in the transfer + 1 for the instruction cycle
  spiSettings.bitRate = 3000000;

  spiSettings.bytesPerTransaction = SRAM_PACKET_SIZE;

  spiSettings.csToDataDelay = 0x01;

  spiSettings.dataToDataDelay = 0x00;

  spiSettings.lastDataToCSDelay = 0x01;

  // CS_MEM is high when idle
  spiSettings.idleCSValue = 0x0002;

  // CS_MEM is low when active
  spiSettings.activeCSValue = 0x0000;

  if (MCP2210_WriteSpiSettings(handle, &spiSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->WriteSpiSettings() failed\n");
    return false;
  }

  MCP2210ChipSettings chipSettings = {0};

  if (MCP2210_ReadChipSettings(handle, &chipSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->ReadChipSettings() failed\n");
    return false;
  }

  chipSettings.gp0Designation = GPIO;
  chipSettings.gp1Designation = CS;
  chipSettings.gp5Designation = DF;

  chipSettings.defaultGPIODirection = 0x0000;
  chipSettings.defaultGPIOValue = 0xFFFF;

  if (MCP2210_WriteChipSettings(handle, &chipSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->WriteChipSettings() failed\n");
    return false;
  }

  // attempt the data transfer
  if (MCP2210_SpiDataTransfer(handle, SRAM_PACKET_SIZE, txBytes, rxBytes, &spiSettings) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->SpiDataTransfer() failed\n");
    return false;
  }

  // extract the data we read
  memcpy(rxData, &rxBytes[3], SRAM_DATA_SIZE);
  return true;
}