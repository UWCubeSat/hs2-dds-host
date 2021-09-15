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
#include <stdlib.h>   // for atoi()
#include <stdbool.h>  // for bool type
#include <string.h>   // for memcpy()

// HIDAPI
#include <hidapi.h>

// project libraries
#include "mcp2210.h"
#include "dac5687.h"
#include "csv.h"

// verify that the address is writable
#define CHECK_ADDR(addr) (addr != 0x08 && addr != 0x1A && addr < 0x1D)

// verify that a multi-byte write doesn't intersect with a factory only address
#define CHECK_BYTES(addr, bytes) (bytes <= (0x08 - startAddr) && bytes <= (0x1A - startAddr) && bytes <= (0x1D - startAddr))

bool DAC5687_Configure(hid_device *handle, const DAC5687Settings *settings) {
  if (handle == NULL) {
    fprintf(stderr, "handle can't be null\n");
    return false;
  }

  if (settings == NULL) {
    fprintf(stderr, "settings can't be null\n");
    return false;
  }

  const int kNumRegisters = 28;
  char txBuf[kNumRegisters];
  memset(txBuf, 0, kNumRegisters * sizeof(char));

  // populate registers
  txBuf[0] = (settings->version.sleep_dac_a << 7) | (settings->version.sleep_dac_b << 6) |
              (settings->version.hpla << 5) | (settings->version.hplb << 4);
  txBuf[1] = (settings->config_0.pll_vco_div << 6) | (settings->config_0.pll_freq << 5) |
              (settings->config_0.pll_kv << 5) | (settings->config_0.fir_interp << 3) |
              (settings->config_0.inv_pll_lock << 2) | (settings->config_0.fifo_bypass);
  txBuf[2] = (settings->config_1.qflag << 7) | (settings->config_1.interl << 6) |
              (settings->config_1.dual_clk << 5) | (settings->config_1.twos << 4) |
              (settings->config_1.rev_abus << 3) | (settings->config_1.rev_bbus << 2) |
              (settings->config_1.fir_bypass << 1) | (settings->config_1.full_bypass);
  txBuf[3] = (settings->config_1.qflag << 7) | (settings->config_1.interl << 6) |
              (settings->config_1.dual_clk << 5) | (settings->config_1.twos << 4) |
              (settings->config_1.rev_abus << 3) | (settings->config_1.rev_bbus << 2) |
              (settings->config_1.fir_bypass << 1) | (settings->config_1.full_bypass);
  txBuf[4] = (settings->config_3.sif_4_pin << 7) | (settings->config_3.dac_ser_data << 6) |
              (settings->config_3.half_rate << 5) | (settings->config_3.usb << 3) |
              (settings->config_3.counter_mode);
  txBuf[5] = (settings->sync_cntl.sync_phstr << 7) | (settings->sync_cntl.sync_nco << 6) |
              (settings->sync_cntl.sync_cm << 5) | (settings->sync_cntl.sync_fifo << 2);
  txBuf[9] = (settings->nco_freq & 0xFF);
  txBuf[10] = (settings->nco_freq & 0xFF00) >> 8;
  txBuf[11] = (settings->nco_freq & 0xFF0000) >> 16;
  txBuf[12] = (settings->nco_freq & 0xFF000000) >> 24;
  txBuf[13] = (settings->nco_phase & 0xFF);
  txBuf[14] = (settings->nco_phase & 0xFF00) >> 8;
  txBuf[15] = (settings->dac_a_off & 0xFF);
  txBuf[16] = (settings->dac_b_off & 0xFF);
  txBuf[17] = (settings->dac_a_off & 0xF00) >> 8;
  txBuf[18] = (settings->dac_b_off & 0xF00) >> 8;
  txBuf[19] = (settings->qmc_a_gain & 0xFF);
  txBuf[20] = (settings->qmc_b_gain & 0xFF);
  txBuf[21] = (settings->qmc_phase & 0xFF);
  txBuf[22] = ((settings->qmc_phase & 0x300) >> 2) | ((settings->qmc_a_gain & 0x500) >> 5) |
              ((settings->qmc_b_gain & 0x500) >> 8);
  txBuf[23] = (settings->dac_a_gain & 0xFF);
  txBuf[24] = (settings->dac_b_gain & 0xFF);
  txBuf[25] = ((settings->dac_a_gain & 0x800) >> 4) | ((settings->dac_b_gain & 0x800) >> 8); 
  txBuf[27] = ((settings->atest & 0x8) << 4) | (settings->phstr_del & 0x2);
  txBuf[28] = (settings->phstr_clk_div_ctl & 0x1);

  for (int i = 0; i < kNumRegisters; i++) {
    if (i == 0x08 || i == 0x1A) {
      // factory use only so we skip these
      continue;
    }
    if (!DAC5687_WriteRegister(handle, (DAC5687Address) i, txBuf[i])) {
      return false;
    }
  }
  return true;
}

bool DAC5687_WriteRegister(hid_device *handle, DAC5687Address addr, unsigned char txByte) {
  if (handle == NULL) {
    fprintf(stderr, "dev must not be null\n");
    return false;
  }

  if (!CHECK_ADDR(addr)) {
    fprintf(stderr, "can't write to address %x# as it's for factory use only\n", addr);
    return false;
  }

  // save a copy of the current SPI settings
  MCP2210SPITransferSettings currentSpiSettings = {0};
  if (MCP2210_ReadSpiSettings(handle, &currentSpiSettings, true) < 0) {
    fprintf(stderr, "WriteRegister()->ReadSpiSettings() failed\n");
    return false;
  }

  MCP2210SPITransferSettings tempSpiSettings;
  memcpy(&tempSpiSettings, &currentSpiSettings, sizeof(MCP2210SPITransferSettings));

  // number of bytes in the transfer + 1 for the instruction cycle
  tempSpiSettings.bitRate = 3000000;

  tempSpiSettings.bytesPerTransaction = 2;

  tempSpiSettings.csToDataDelay = 0x01;

  tempSpiSettings.dataToDataDelay = 0x00;

  tempSpiSettings.lastDataToCSDelay = 0x01;
  
  // CS_DAC is high when idle
  tempSpiSettings.idleCSValue = 0x0003;

  // CS_DAC is low when active
  tempSpiSettings.activeCSValue = 0x0002;
  

  MCP2210ChipSettings chipSettings = {0};

  if (MCP2210_ReadChipSettings(handle, &chipSettings, true) < 0) {
    fprintf(stderr, "WriteSRAMAddress()->ReadChipSettings() failed\n");
    return false;
  }

  chipSettings.gp0Designation = CS;
  chipSettings.gp1Designation = CS;
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

  if (MCP2210_SpiDataTransfer(handle, 2, spiTxBytes, rxBuf, &tempSpiSettings) < 0) {
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
  spiSettings.idleCSValue = 0x0003;

  // CS_DAC is low when active
  spiSettings.activeCSValue = 0x0002;

  // construct the instruction cycle byte
  unsigned int writeBytes = bytes - 1;
  unsigned char instrByte = (startAddr & 0x1F) | ((writeBytes & 0x03) << 5);

  unsigned char *spiTxBytes = malloc(sizeof(unsigned char) * (bytes +1));
  unsigned char *rxBuf = malloc(sizeof(unsigned char) * (bytes +1));

  spiTxBytes[0] = instrByte;
  memcpy((spiTxBytes + 1), txBytes, bytes);
  memset(rxBuf, 0, bytes + 1);

  if (MCP2210_SpiDataTransfer(handle, sizeof(spiTxBytes), spiTxBytes, rxBuf, &spiSettings) < 0) {
    fprintf(stderr, "WriteRegisters() failed\n");
    free(spiTxBytes);
    free(rxBuf);
    return false;
  }
  free(spiTxBytes);
  free(rxBuf); 
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
  spiSettings.idleCSValue = 0x0003;

  // CS_DAC is low when active
  spiSettings.activeCSValue = 0x0002;
  
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