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

#ifndef DDS_HOST_H_
#define DDS_HOST_H_

#include "mcp2210.h"
#include "dac5687.h"

// MCP2210 Config file keys
const char * MCP_SPI_CFG_KEY = "SPI";
const char * MCP_USB_CFG_KEY = "USB";
const char * MCP_CHIP_CFG_KEY = "CHIP";

const char * MCP_USB_POWER_OPTION = "power_option";
const char * MCP_USB_REQUEST_CURRENT = "request_current";
const char * MCP_USB_VID = "vid";
const char * MCP_USB_PID = "pid";

const char * MCP_CHIP_GP0 = "gp0";
const char * MCP_CHIP_GP1 = "gp1";
const char * MCP_CHIP_GP2 = "gp2";
const char * MCP_CHIP_GP3 = "gp3";
const char * MCP_CHIP_GP4 = "gp4";
const char * MCP_CHIP_GP5 = "gp5";
const char * MCP_CHIP_GP6 = "gp6";
const char * MCP_CHIP_GP7 = "gp7";
const char * MCP_CHIP_GP8 = "gp8";

const char * MCP_CHIP_GP0_DIR = "gp0_dir";
const char * MCP_CHIP_GP1_DIR = "gp1_dir";
const char * MCP_CHIP_GP2_DIR = "gp2_dir";
const char * MCP_CHIP_GP3_DIR = "gp3_dir";
const char * MCP_CHIP_GP4_DIR = "gp4_dir";
const char * MCP_CHIP_GP5_DIR = "gp5_dir";
const char * MCP_CHIP_GP6_DIR = "gp6_dir";
const char * MCP_CHIP_GP7_DIR = "gp7_dir";
const char * MCP_CHIP_GP8_DIR = "gp8_dir";

const char * MCP_CHIP_GP0_OUT = "gp0_out";
const char * MCP_CHIP_GP1_OUT = "gp1_out";
const char * MCP_CHIP_GP2_OUT = "gp2_out";
const char * MCP_CHIP_GP3_OUT = "gp3_out";
const char * MCP_CHIP_GP4_OUT = "gp4_out";
const char * MCP_CHIP_GP5_OUT = "gp5_out";
const char * MCP_CHIP_GP6_OUT = "gp6_out";
const char * MCP_CHIP_GP7_OUT = "gp7_out";
const char * MCP_CHIP_GP8_OUT = "gp8_out";

const char * MCP_CHIP_REMOTE_WAKEUP = "remote_wakeup";
const char * MCP_CHIP_INT_MODE = "interrupt_mode";
const char * MCP_CHIP_SPI_RELEASE = "spi_bus_release";
const char * MCP_CHIP_ACC_CONTROL = "access_control";
const char * MCP_CHIP_PASS = "password";

const char * MCP_SPI_BIT_RATE = "bit_rate";
const char * MCP_SPI_CS_IDLE_0 = "cs_idle_0";
const char * MCP_SPI_CS_IDLE_1 = "cs_idle_1";
const char * MCP_SPI_CS_IDLE_2 = "cs_idle_2";
const char * MCP_SPI_CS_IDLE_3 = "cs_idle_3";
const char * MCP_SPI_CS_IDLE_4 = "cs_idle_4";
const char * MCP_SPI_CS_IDLE_5 = "cs_idle_5";
const char * MCP_SPI_CS_IDLE_6 = "cs_idle_6";
const char * MCP_SPI_CS_IDLE_7 = "cs_idle_7";
const char * MCP_SPI_CS_IDLE_8 = "cs_idle_8";

const char * MCP_SPI_CS_ACTIVE_0 = "cs_active_0";
const char * MCP_SPI_CS_ACTIVE_1 = "cs_active_1";
const char * MCP_SPI_CS_ACTIVE_2 = "cs_active_2";
const char * MCP_SPI_CS_ACTIVE_3 = "cs_active_3";
const char * MCP_SPI_CS_ACTIVE_4 = "cs_active_4";
const char * MCP_SPI_CS_ACTIVE_5 = "cs_active_5";
const char * MCP_SPI_CS_ACTIVE_6 = "cs_active_6";
const char * MCP_SPI_CS_ACTIVE_7 = "cs_active_7";
const char * MCP_SPI_CS_ACTIVE_8 = "cs_active_8";

const char * MCP_SPI_CS_DATA_DLY = "cs_data_delay";
const char * MCP_SPI_DATA_CS_DLY = "data_cs_delay";
const char * MCP_SPI_DATA_DATA_DLY = "data_data_delay";
const char * MCP_SPI_BYTES = "bytes_per_transaction";
const char * MCP_SPI_MODE = "spi_mode";


// DAC5687 Config file keys
const char * DAC_VERSION_CFG_KEY = "VERSION";
const char * DAC_CONFIG_0_CFG_KEY = "CONFIG0";
const char * DAC_CONFIG_1_CFG_KEY = "CONFIG1";
const char * DAC_CONFIG_2_CFG_KEY = "CONFIG2";
const char * DAC_CONFIG_3_CFG_KEY = "CONFIG3";
const char * DAC_SYNC_CNTL_CFG_KEY = "SYNCCNTL";
const char * DAC_NCO_CFG_KEY = "NCO";
const char * DAC_DACA_CFG_KEY = "DACA";
const char * DAC_DACB_CFG_KEY = "DACB";
const char * DAC_QMC_CFG_KEY = "QMC";

const char * DAC_VERSION_SLEEP_A = "sleep_daca";
const char * DAC_VERSION_SLEEP_B = "sleep_dacb";
const char * DAC_VERSION_HPLA = "hpla";
const char * DAC_VERSION_HPLB = "hbla";

const char * DAC_CONFIG0_PLL_DIV = "pll_div";
const char * DAC_CONFIG0_PLL_FREQ = "pll_freq";
const char * DAC_CONFIG0_PLL_KV = "pll_kv";
const char * DAC_CONFIG0_INTERP = "interp";
const char * DAC_CONFIG0_INV_PLL_LOCK = "inv_plllock";
const char * DAC_CONFIG0_FIFO_BYPASS = "fifo_bypass";

const char * DAC_CONFIG1_QFLAG = "qflag";
const char * DAC_CONFIG1_INTERL = "interl";
const char * DAC_CONFIG1_DUAL_CLK = "dual_clk";
const char * DAC_CONFIG1_TWOS = "twos";
const char * DAC_CONFIG1_REV_ABUS = "rev_abus";
const char * DAC_CONFIG1_REV_BBUS = "rev_bbus";
const char * DAC_CONFIG1_FIR_BYPASS = "fir_bypass";
const char * DAC_CONFIG1_FULL_BYPASS = "full_bypass";

const char * DAC_CONFIG2_NCO = "nco";
const char * DAC_CONFIG2_NCO_GAIN = "nco_gain";
const char * DAC_CONFIG2_QMC = "qmc";
const char * DAC_CONFIG2_CM_MODE = "cm_mode";
const char * DAC_CONFIG2_INV_SINC = "invsinc";

const char * DAC_CONFIG3_SIF_4PIN = "sif_4pin";
const char * DAC_CONFIG3_DAC_SER_DATA = "dac_ser_data";
const char * DAC_CONFIG3_HALF_RATE = "half_rate";
const char * DAC_CONFIG3_USB = "usb";
const char * DAC_CONFIG3_COUNTER_MODE = "counter_mode";

const char * DAC_SYNCCNTL_SYNC_PHSTR = "sync_phstr";
const char * DAC_SYNCCNTL_SYNC_NCO = "sync_nco";
const char * DAC_SYNCCNTL_SYNC_CM = "sync_cm";
const char * DAC_SYNCCNTL_SYNC_FIFO = "sync_fifo";

const char * DAC_NCO_FREQ = "frequency";
const char * DAC_NCO_PHASE = "phase";

const char * DAC_DAC_OFFSET = "offset";
const char * DAC_DAC_GAIN = "gain";

const char * DAC_QMC_GAIN_A = "gain_a";
const char * DAC_QMC_GAIN_B = "gain_b";
const char * DAC_QMC_PHASE = "phase";

typedef struct settings_t {
    MCP2210Settings mcp_settings;
    DAC5687Settings dac_settings;
} Settings;

typedef struct dds_data_t {
    uint64_t address;
    uint32_t data;
} Data;

#define MAX_PATH_LEN 64

#define CFG_BASE 10

#endif  // DDS_HOST_H_
