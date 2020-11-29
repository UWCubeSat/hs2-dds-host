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

#include <stdint.h>

// To open attached DDS
#define VID 0x04D8
#define PID 0x00DE

// all MCP commands
#define CMD_GET_NVRAM_SETTINGS      0x61
#define CMD_SET_NVRAM_SETTINGS      0x60
#define CMD_GET_RAM_SPI_SETTINGS    0x41
#define CMD_SET_RAM_SPI_SETTINGS    0x40
#define CMD_GET_RAM_GPIO_SETTINGS   0x20
#define CMD_SET_RAM_GPIO_SETTINGS   0x21
#define CMD_GET_RAM_GPIO_PIN_DIR    0x33
#define CMD_SET_RAM_GPIO_PIN_DIR    0x32
#define CMD_GET_RAM_GPIO_PIN_VAL    0x31
#define CMD_SET_RAM_GPIO_PIN_VAL    0x30
#define CMD_READ_EEPROM             0x50
#define CMD_WRITE_EEPROM            0x51
#define CMD_GET_RAM_INTRPT_CNT      0x12
#define CMD_SPI_DATA_TRANSFER       0x42
#define CMD_CANCEL_SPI_TRANSFER     0x11
#define CMD_SPI_BUS_RELEASE         0x80
#define CMD_GET_MCP_STATUS          0x10
#define CMD_SEND_PASS               0x70

// all MCP sub-commands
#define SUB_CMD_SET_CHIP_PU         0x20
#define SUB_CMD_SET_SPI_PU          0x10
#define SUB_CMD_SET_USB_PU          0x30
#define SUB_CMD_SET_MAN_NAME        0x50
#define SUB_CMD_SET_PROD_NAME       0x40

// chip settings protection modes
#define AC_MODE_UNPROTECTED         0x00
#define AC_MODE_PROTECTED           0x40
#define AC_MODE_LOCKED              0x80

// GPIO modes
#define GPIO_MODE_GPIO              0x00
#define GPIO_MODE_CS                0x01
#define GPIO_MODE_DF                0x02

// DAC Register addresses
#define DAC_REG_VERSION             0x00
#define DAC_REG_CONFIG0             0x01
#define DAC_REG_CONFIG1             0x02
#define DAC_REG_CONFIG2             0x03
#define DAC_REG_CONFIG3             0x04
#define DAC_REG_SYNC_CNTL           0x05
#define DAC_REG_SER_DATA_0          0x06
#define DAC_REG_SER_DATA_1          0x07
#define DAC_REG_NCO_FREQ_0          0x09
#define DAC_REG_NCO_FREQ_1          0x0A
#define DAC_REG_NCO_FREQ_3          0x0B
#define DAC_REG_NC0_FREQ_4          0x0

typedef struct mcp_generic_packet_st {
  uint8_t __data__[64];
} MCPUSBPacket;

typedef struct mcp_access_pass_st {
  uint8_t pass_char_0;
  uint8_t pass_char_1;
  uint8_t pass_char_2;
  uint8_t pass_char_3;
  uint8_t pass_char_4;
  uint8_t pass_char_5;
  uint8_t pass_char_6;
  uint8_t pass_char_7;
} MCPAccessPassword;

typedef struct mcp_chip_settings_st {
  uint8_t __pad1__[4];
  uint8_t gp0_des;
  uint8_t gp1_des;
  uint8_t gp2_des;
  uint8_t gp3_des;
  uint8_t gp4_des;
  uint8_t gp5_des;
  uint8_t gp6_des;
  uint8_t gp7_des;
  uint8_t gp8_des;
  uint8_t gpio_def_out_low;
  uint8_t gpio_def_out_high;
  uint8_t gpio_def_dir_low;
  uint8_t gpio_def_dir_high;
  uint8_t chip_settings;
  uint8_t chip_access_control;
  MCPAccessPassword pass;
  uint8_t __pad2__[37];
} MCPChipSettings;

typedef struct mcp_chip_settings_response_st {
  uint8_t cmd_echo;
  uint8_t status;
  uint8_t __pad1__[62];
} MCPResponse;

typedef struct mcp_spi_transfer_settings_st {
  uint8_t __pad1__[4];
  uint8_t br3;
  uint8_t br2;
  uint8_t br1;
  uint8_t br0;
  uint8_t idle_cs_val_low;
  uint8_t idle_cs_val_high;
  uint8_t active_cs_val_low;
  uint8_t active_cs_val_high;
  uint8_t cs_to_data_delay_low;
  uint8_t cs_to_data_delay_high;
  uint8_t last_data_to_cs_delay_low;
  uint8_t last_data_to_cs_delay_high;
  uint8_t inter_data_delay_low;
  uint8_t inter_data_delay_high;
  uint8_t bytes_per_transaction_low;
  uint8_t bytes_per_transaction_high;
  uint8_t spi_mode;
  uint8_t __pad2__[43];
} MCPSPITransferSettings;

typedef struct mcp_spi_data_transfer_st {
  uint8_t __pad1__;
  uint8_t bytes;
  uint8_t __pad2__[2];
  uint8_t spi_data[60];
} MCPSPIDataTransfer;

typedef struct mcp_spi_data_transfer_res_st {
  uint8_t cmd_echo;
  uint8_t status;
  uint8_t bytes;
  uint8_t engine_status;
  uint8_t spi_data[60];
} MCPSPIDataTransferResponse;

// returns >= 0 on success, -1 otherwise. Puts opened device handle in 'out'.
static int dds_init(hid_device **out);

static int dds_mcp_write_read(hid_device *dev,
                       uint8_t cmd,
                       uint8_t sub_cmd,
                       MCPUSBPacket *data,
                       MCPUSBPacket *response);

static int dds_dac_write(hid_device *dev,
                        uint8_t *bytes,
                        uint8_t n_bytes,
                        uint8_t addr,
                        MCPUSBPacket *response);

static int dds_dac_read(hid_device *dev,
                        uint8_t n_bytes,
                        uint8_t addr,
                        MCPUSBPacket *response);

static int dds_dac_transfer(hid_device *dev,
                            uint8_t *bytes,
                            uint8_t n_bytes,
                            uint8_t header,
                            MCPUSBPacket *response);

#endif  // DDS_HOST_H_
