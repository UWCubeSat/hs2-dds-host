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

#ifndef DAC5687_H_
#define DAC5687_H_

#include "dds-host/mcp2210.h"
#include "dds-host/util/csv.h"

// DAC register addresses
typedef enum dac5687_reg_t {
  Version = 0x00,
  Config0 = 0x01,
  Config1 = 0x02,
  Config2 = 0x03,
  Config3 = 0x04,
  SyncCntl = 0x05,
  SerData0 = 0x06,
  SerData1 = 0x07,
  NCOFreq0 = 0x08,
  NCOFreq1 = 0x09,
  NCOFreq2 = 0x0A,
  NCOFreq3 = 0x0B,
  NCOFreq4 = 0x0C,
  NCOPhase0 = 0x0D,
  NCOPhase1 = 0x0E,
  DACAOffset0 = 0x0F,
  DACBOffset0 = 0x10,
  DACAOffset1 = 0x11,
  DACBOffset1 = 0x12,
  QMCAGain0 = 0x13,
  QMCBGain0 = 0x14,
  QMCPhaseGain0 = 0x15,
  QMCPhaseGain1 = 0x16,
  DACAGain0 = 0x17,
  DACBGain0 = 0x18,
  DACA_DACBGain1 = 0x19,
  ATEST = 0x1B,
  DACTest = 0x1C,
} DAC5687Address;

bool DAC5687_Configure(CSVFile *file, hid_device *handle);

bool DAC5687_WriteRegister(hid_device *handle, DAC5687Address addr, unsigned char txByte);

bool DAC5687_WriteRegisters(hid_device *handle, DAC5687Address startAddr, unsigned char *txBytes, unsigned int bytes);

bool DAC5687_ReadRegister(hid_device *handle, DAC5687Address addr, unsigned char *rxByte);

bool DAC5687_ReadRegisters(hid_device *handle, DAC5687Address startAddr, unsigned char *rxBytes, unsigned int bytes);

bool DAC5687_Init(hid_device **handle);

#endif  // DAC5687_H_