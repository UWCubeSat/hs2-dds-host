/*
 * MIT License
 * 
 * Copyright (c) 2021 Eli Reed
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

#ifndef CFG_H_
#define CFG_H_

#include <stdint.h>
#include <stdbool.h>
#include <ini.h>

#include "mcp2210.h"
#include "dac5687.h"

// MCP2210 Config file keys
#define MCP_SPI_SECTION  "SPI"
#define MCP_USB_SECTION  "USB"
#define MCP_CHIP_SECTION  "CHIP"

#define MCP_USB_POWER_OPTION  "poweroption"
#define MCP_USB_REQUEST_CURRENT  "request_current"
#define MCP_USB_VID  "vid"
#define MCP_USB_PID  "pid"

#define MCP_CHIP_GP_DES   "gp_des"
// #define MCP_CHIP_GP0  "gp0"
// #define MCP_CHIP_GP1  "gp1"
// #define MCP_CHIP_GP2  "gp2"
// #define MCP_CHIP_GP3  "gp3"
// #define MCP_CHIP_GP4  "gp4"
// #define MCP_CHIP_GP5  "gp5"
// #define MCP_CHIP_GP6  "gp6"
// #define MCP_CHIP_GP7  "gp7"
// #define MCP_CHIP_GP8  "gp8"

#define MCP_CHIP_GP_DIR  "gp_dir"
// #define MCP_CHIP_GP0_DIR  "gp1_dir"
// #define MCP_CHIP_GP1_DIR  "gp1_dir"
// #define MCP_CHIP_GP2_DIR  "gp2_dir"
// #define MCP_CHIP_GP3_DIR  "gp3_dir"
// #define MCP_CHIP_GP4_DIR  "gp4_dir"
// #define MCP_CHIP_GP5_DIR  "gp5_dir"
// #define MCP_CHIP_GP6_DIR  "gp6_dir"
// #define MCP_CHIP_GP7_DIR  "gp7_dir"
// #define MCP_CHIP_GP8_DIR  "gp8_dir"

#define MCP_CHIP_GP_OUT  "gp_out"
// #define MCP_CHIP_GP0_OUT  "gp0_out"
// #define MCP_CHIP_GP1_OUT  "gp1_out"
// #define MCP_CHIP_GP2_OUT  "gp2_out"
// #define MCP_CHIP_GP3_OUT  "gp3_out"
// #define MCP_CHIP_GP4_OUT  "gp4_out"
// #define MCP_CHIP_GP5_OUT  "gp5_out"
// #define MCP_CHIP_GP6_OUT  "gp6_out"
// #define MCP_CHIP_GP7_OUT  "gp7_out"
// #define MCP_CHIP_GP8_OUT  "gp8_out"

#define MCP_CHIP_REMOTE_WAKEUP  "remote_wakeup"
#define MCP_CHIP_INT_MODE  "interrupt_mode"
#define MCP_CHIP_SPI_RELEASE  "spi_bus_release"
#define MCP_CHIP_ACC_CONTROL  "access_control"
#define MCP_CHIP_PASS  "password"

#define MCP_SPI_BIT_RATE  "bit_rate"
#define MCP_SPI_CS_IDLE  "cs_idle"
// #define MCP_SPI_CS_IDLE_0  "cs_idle_0"
// #define MCP_SPI_CS_IDLE_1  "cs_idle_1"
// #define MCP_SPI_CS_IDLE_2  "cs_idle_2"
// #define MCP_SPI_CS_IDLE_3  "cs_idle_3"
// #define MCP_SPI_CS_IDLE_4  "cs_idle_4"
// #define MCP_SPI_CS_IDLE_5  "cs_idle_5"
// #define MCP_SPI_CS_IDLE_6  "cs_idle_6"
// #define MCP_SPI_CS_IDLE_7  "cs_idle_7"
// #define MCP_SPI_CS_IDLE_8  "cs_idle_8"

#define MCP_SPI_CS_ACTIVE  "cs_active"
// #define MCP_SPI_CS_ACTIVE_0  "cs_active_0"
// #define MCP_SPI_CS_ACTIVE_1  "cs_active_1"
// #define MCP_SPI_CS_ACTIVE_2  "cs_active_2"
// #define MCP_SPI_CS_ACTIVE_3  "cs_active_3"
// #define MCP_SPI_CS_ACTIVE_4  "cs_active_4"
// #define MCP_SPI_CS_ACTIVE_5  "cs_active_5"
// #define MCP_SPI_CS_ACTIVE_6  "cs_active_6"
// #define MCP_SPI_CS_ACTIVE_7  "cs_active_7"
// #define MCP_SPI_CS_ACTIVE_8  "cs_active_8"

#define MCP_SPI_CS_DATA_DLY  "cs_data_delay"
#define MCP_SPI_DATA_CS_DLY  "data_cs_delay"
#define MCP_SPI_DATA_DATA_DLY  "data_data_delay"
#define MCP_SPI_BYTES  "bytes_per_transaction"
#define MCP_SPI_MODE  "spi_mode"


// DAC5687 Config file keys
#define DAC_VERSION_SECTION  "VERSION"
#define DAC_CONFIG_0_SECTION  "CONFIG0"
#define DAC_CONFIG_1_SECTION  "CONFIG1"
#define DAC_CONFIG_2_SECTION  "CONFIG2"
#define DAC_CONFIG_3_SECTION  "CONFIG3"
#define DAC_SYNC_CNTL_SECTION  "SYNCCNTL"
#define DAC_NCO_SECTION  "NCO"
#define DAC_DACA_SECTION  "DACA"
#define DAC_DACB_SECTION  "DACB"
#define DAC_QMC_SECTION  "QMC"

#define DAC_VERSION_SLEEP_A  "sleep_daca"
#define DAC_VERSION_SLEEP_B  "sleep_dacb"
#define DAC_VERSION_HPLA  "hpla"
#define DAC_VERSION_HPLB  "hplb"

#define DAC_CONFIG0_PLL_DIV  "pll_div"
#define DAC_CONFIG0_PLL_FREQ  "pll_freq"
#define DAC_CONFIG0_PLL_KV  "pll_kv"
#define DAC_CONFIG0_INTERP  "interp"
#define DAC_CONFIG0_INV_PLL_LOCK  "inv_plllock"
#define DAC_CONFIG0_FIFO_BYPASS  "fifo_bypass"

#define DAC_CONFIG1_QFLAG  "qflag"
#define DAC_CONFIG1_INTERL  "interl"
#define DAC_CONFIG1_DUAL_CLK  "dual_clk"
#define DAC_CONFIG1_TWOS  "twos"
#define DAC_CONFIG1_REV_ABUS  "rev_abus"
#define DAC_CONFIG1_REV_BBUS  "rev_bbus"
#define DAC_CONFIG1_FIR_BYPASS  "fir_bypass"
#define DAC_CONFIG1_FULL_BYPASS  "full_bypass"

#define DAC_CONFIG2_NCO  "nco"
#define DAC_CONFIG2_NCO_GAIN  "nco_gain"
#define DAC_CONFIG2_QMC  "qmc"
#define DAC_CONFIG2_CM_MODE  "cm_mode"
#define DAC_CONFIG2_INV_SINC  "invsinc"

#define DAC_CONFIG3_SIF_4PIN  "sif_4pin"
#define DAC_CONFIG3_DAC_SER_DATA  "dac_ser_data"
#define DAC_CONFIG3_HALF_RATE  "half_rate"
#define DAC_CONFIG3_USB  "usb"
#define DAC_CONFIG3_COUNTER_MODE  "counter_mode"

#define DAC_SYNCCNTL_SYNC_PHSTR  "sync_phstr"
#define DAC_SYNCCNTL_SYNC_NCO  "sync_nco"
#define DAC_SYNCCNTL_SYNC_CM  "sync_cm"
#define DAC_SYNCCNTL_SYNC_FIFO  "sync_fifo"

#define DAC_NCO_FREQ  "frequency"
#define DAC_NCO_PHASE  "phase"

#define DAC_DAC_OFFSET  "offset"
#define DAC_DAC_GAIN  "gain"

#define DAC_QMC_GAIN_A  "gain_a"
#define DAC_QMC_GAIN_B  "gain_b"
#define DAC_QMC_PHASE  "phase"

#define CFG_BASE 16

typedef struct settings_t {
    MCP2210Settings mcp;
    DAC5687Settings dac;
} Settings;

int CFG_HandleMCP2210Config(void *user, const char *section, const char *name,
                            const char *value);

int CFG_HandleDAC5687Config(void *user, const char *section, const char *name,
                            const char *value);

int CFG_GenerateMCP2210Config(MCP2210Settings settings, const char *path);

int CFG_GenerateDAC5687Config(DAC5687Settings settings, const char *path);

void CFG_PrintMCP2210Config(MCP2210Settings settings);

void CFG_PrintDAC5687Config(DAC5687Settings settings);

void CFG_PrintConfig(Settings settings);

// creates a config file based on the blocks in the structure
// bool CFG_GenerateConfigFromBlocks(ConfigBlock *blocks, int num_blocks, char * path);

#endif  // CFG_H_