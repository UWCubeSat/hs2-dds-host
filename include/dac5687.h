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

// #include "mcp2210.h"
#include "csv.h"

#include <stdlib.h>
#include <stdbool.h>
#include <hidapi.h>

// DAC register addresses
typedef enum dac5687_reg_t {
  VERSION = 0x00,
  CONFIG_0 = 0x01,
  CONFIG_1 = 0x02,
  CONFIG_2 = 0x03,
  CONFIG_3 = 0x04,
  SYNC_CNTL = 0x05,
  SER_DATA_0 = 0x06,
  SER_DATA_1 = 0x07,
  NCO_FREQ_0 = 0x09,
  NCO_FREQ_1 = 0x0A,
  NCO_FREQ_2 = 0x0B,
  NCO_FREQ_3 = 0x0C,
  NCO_PHASE_0 = 0x0D,
  NCO_PHASE_1 = 0x0E,
  DAC_A_OFFSET_0 = 0x0F,
  DAC_B_OFFSET_0 = 0x10,
  DAC_A_OFFSET_1 = 0x11,
  DAC_B_OFFSET_1 = 0x12,
  QMC_A_GAIN_0 = 0x13,
  QMCB__GAIN_0 = 0x14,
  QMC_PHASE_0 = 0x15,
  QMC_PHASE_GAIN_1 = 0x16,
  DAC_A_GAIN_0 = 0x17,
  DAC_B_GAIN_0 = 0x18,
  DAC_A_DAC_B_GAIN_1 = 0x19,
  ATEST = 0x1B,
  DAC_TEST = 0x1C,
} DAC5687Address;

typedef enum dac5687_pll_vco_div_t {
  DIV_1 = 0b00,
  DIV_2 = 0b01,
  DIV_4 = 0b10,
  DIV_8 = 0b11
} PLLVCODiv;

typedef enum dac5687_fir_interp_t {
  X2 = 0b00,
  X4 = 0b01,
  X4L = 0b10,
  X8 = 0b11
} FIRInterp;

typedef enum dac5687_counter_mode_t {
  NO_COUNT = 0b000,
  ALL = 0b100,
  LSB = 0b101,
  MID = 0b110,
  MSB = 0b111
} CounterMode;

typedef enum dac5687_sync_fifo_t {
  TXENABLE = 0b000,
  PHSTR = 0b001,
  QFLAG = 0b010,
  DB = 0b011,
  DA = 0b100,
  SIF = 0b101,
  OFF = 0b110,
  ON = 0b111
} SyncFIFO;

typedef struct dac5687_version_t {
  uint8_t sleep_dac_a;
  uint8_t sleep_dac_b;
  uint8_t hpla;
  uint8_t hplb;
} Version;

typedef struct dac5687_config_0_t {
  PLLVCODiv pll_vco_div;
  uint8_t pll_freq;
  uint8_t pll_kv;
  FIRInterp fir_interp;
  uint8_t inv_pll_lock;
  uint8_t fifo_bypass;
} Config0;

typedef struct dac5687_config_1_t {
  uint8_t qflag;
  uint8_t interl;
  uint8_t dual_clk;
  uint8_t twos;
  uint8_t rev_abus;
  uint8_t rev_bbus;
  uint8_t fir_bypass;
  uint8_t full_bypass;
} Config1;

typedef struct dac5687_config_2_t {
  uint8_t nco;
  uint8_t nco_gain;
  uint8_t qmc;
  uint8_t cm_mode;
  uint8_t invsinc;
} Config2;

typedef struct dac5687_config_3_t {
  uint8_t sif_4_pin;
  uint8_t dac_ser_data;
  uint8_t half_rate;
  uint8_t usb;
  CounterMode counter_mode;
} Config3;

typedef struct dac5687_sync_cntl_t {
  uint8_t sync_phstr;
  uint8_t sync_nco;
  uint8_t sync_cm;
  SyncFIFO sync_fifo;
} SyncCntl;

typedef struct dac5687_settings_t {
  Version version;
  Config0 config_0;
  Config1 config_1;
  Config2 config_2;
  Config3 config_3;
  SyncCntl sync_cntl;
  uint16_t dac_data;
  uint32_t nco_freq;
  uint16_t nco_phase;
  uint16_t dac_a_off;
  uint16_t dac_b_off;
  uint16_t qmc_a_gain;
  uint16_t qmc_b_gain;
  uint16_t qmc_phase;
  uint16_t dac_a_gain;
  uint16_t dac_b_gain;
  uint8_t atest;
  uint8_t phstr_del;
  uint8_t phstr_clk_div_ctl;
} DAC5687Settings;

bool DAC5687_Configure(hid_device *handle, const DAC5687Settings *settings);

bool DAC5687_WriteRegister(hid_device *handle, DAC5687Address addr, unsigned char txByte);

bool DAC5687_WriteRegisters(hid_device *handle, DAC5687Address startAddr, unsigned char *txBytes, unsigned int bytes);

bool DAC5687_ReadRegister(hid_device *handle, DAC5687Address addr, unsigned char *rxByte);

bool DAC5687_ReadRegisters(hid_device *handle, DAC5687Address startAddr, unsigned char *rxBytes, unsigned int bytes);

#endif  // DAC5687_H_