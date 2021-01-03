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

  // get current SPI settings
  MCP2210SPITransferSettings spiSettings = {0};
  if (!MCP2210_ReadSpiSettings(handle, &spiSettings, true)) {
    fprintf(stderr, "failed to read spi settings in dac register write\n");
    return false;
  }

  // number of bytes in the transfer + 1 for the instruction cycle
  spiSettings.br3 = 0xC0;
  spiSettings.br2 = 0xC6;
  spiSettings.br1 = 0x2D;
  spiSettings.br0 = 0x00;

  spiSettings.bytes_per_transaction_low = 2;
  spiSettings.bytes_per_transaction_high = 0;

  spiSettings.cs_to_data_delay_low = 0x0A;
  spiSettings.cs_to_data_delay_high = 0x00;

  spiSettings.inter_data_delay_low = 0x00;
  spiSettings.inter_data_delay_high = 0x00;

  spiSettings.last_data_to_cs_delay_low = 0x0A;
  spiSettings.last_data_to_cs_delay_high = 0x00;

  // CS_DAC is high when idle
  spiSettings.idle_cs_val_low = 0x01;
  spiSettings.idle_cs_val_high = 0x00;

  // CS_DAC is low when active
  spiSettings.active_cs_val_low = 0x00;
  spiSettings.active_cs_val_high = 0x00;
  
  // construct the instruction cycle byte
  unsigned char instrByte = (addr & 0b00011111);

  unsigned char spiTxBytes[2] = {instrByte, txByte};
  unsigned char rxBytes[2];

  if (MCP2210_SpiDataTransfer(handle, 2, spiTxBytes, rxBytes, &spiSettings) == -1) {
    fprintf(stderr, "DAC write failed\n");
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
  if (!MCP2210_ReadSpiSettings(handle, &spiSettings, true)) {
    fprintf(stderr, "failed to read spi settings in dac register read\n");
    return false;
  }

// number of bytes in the transfer + 1 for the instruction cycle
  spiSettings.br3 = 0xC0;
  spiSettings.br2 = 0xC6;
  spiSettings.br1 = 0x2D;
  spiSettings.br0 = 0x00;

  spiSettings.bytes_per_transaction_low = 2;
  spiSettings.bytes_per_transaction_high = 0;

  spiSettings.cs_to_data_delay_low = 0x0A;
  spiSettings.cs_to_data_delay_high = 0x00;

  spiSettings.inter_data_delay_low = 0x00;
  spiSettings.inter_data_delay_high = 0x00;

  spiSettings.last_data_to_cs_delay_low = 0x0A;
  spiSettings.last_data_to_cs_delay_high = 0x00;

  // CS_DAC is high when idle
  spiSettings.idle_cs_val_low = 0x01;
  spiSettings.idle_cs_val_high = 0x00;

  // CS_DAC is low when active
  spiSettings.active_cs_val_low = 0x00;
  spiSettings.active_cs_val_high = 0x00;
  
  // construct the instruction cycle byte
  unsigned char instrByte = (0x1 << 7) | (addr & 0b00011111);

  unsigned char spiTxBytes[2] = {instrByte, 0x00};
  unsigned char rxBytes[2];

  if (MCP2210_SpiDataTransfer(handle, sizeof(spiTxBytes), spiTxBytes, rxBytes, &spiSettings) == -1) {
    fprintf(stderr, "DAC write failed\n");
    return false;
  }
  *rxByte = rxBytes[1];
  printf("Rx[0] = %d, Rx[1] = %d\n", rxBytes[0], rxBytes[1]);
  return true;
}