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
#include <hidapi/hidapi.h>

// MCP2210
#include "dds-host/mcp2210.h"

// CPLD
#include "dds-host/cpld.h"

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

  txBytes[0] = (uint8_t)((addr & 0x02) << 6);
  txBytes[1] = (uint8_t)((addr & 0x1FC) >> 2);
  txBytes[2] = (uint8_t)((addr & 0xFE00) >> 10);
  memcpy(&txData[3], &txData, sizeof(txData));



  if (MCP2210_SpiDataTransfer(handle, SRAM_PACKET_SIZE, txBytes, rxBytes, NULL));
  return true;
}