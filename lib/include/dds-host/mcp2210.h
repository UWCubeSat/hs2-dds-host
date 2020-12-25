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

 /*
  * This file describes an interface to the MCP2210 USB-to-SPI converter IC.
  */

#ifndef MCP2210_H_
#define MCP2210_H_

// HIDAPI
#include "hidapi.h"

// device ID for our application
#define VID                         0x04D8
#define PID                         0x00DE

// MCP2210 Access Control modes
#define UNPROTECTED                 0x00
#define PROTECTED                   0x40
#define LOCKED                      0x80

// MCP2210 GPIO Pin Modes
#define GPIO                        0x00
#define CS                          0x01
#define DF                          0x02

// MCP2210 Power Options
#define HOST_POWERED                (0x1 << 7)
#define SELF_POWERED                (0x1 << 6)
#define REMOTE_WAKE_UP              (0x1 << 5)

// Manufacturer name stuff
#define MAX_MAN_STR_LEN             29

// Convenient members for GPIO Pin Directions
typedef struct mcp2210_gpio_pin_dir_st {
  unsigned char gp0:1;
  unsigned char gp1:1;
  unsigned char gp2:1;
  unsigned char gp3:1;
  unsigned char gp4:1;
  unsigned char gp5:1;
  unsigned char gp6:1;
  unsigned char gp7:1;
  unsigned char gp8:1;
} MCP2210GPIOPinDir;

// Convenient members for GPIO Pin Values
typedef struct mcp2210_gpio_pin_val_st {
  unsigned char gp0:1;
  unsigned char gp1:1;
  unsigned char gp2:1;
  unsigned char gp3:1;
  unsigned char gp4:1;
  unsigned char gp5:1;
  unsigned char gp6:1;
  unsigned char gp7:1;
  unsigned char gp8:1;
} MCP2210GPIOPinVal;

// All command codes listed in the MCP2210 datasheet
typedef enum mcp2210_command_t {
  GetNVRAMSettings = 0x61,
  SetNVRAMSettings = 0x60,
  GetCurrentSpiSettings = 0x41,
  SetCurrentSpiSettings = 0x40,
  GetCurrentChipSettings = 0x20,
  SetCurrentChipSettings = 0x21,
  GetCurrentGPIOPinDir = 0x33,
  SetCurrentGPIOPinDir = 0x32,
  GetCurrentGPIOPinVal = 0x31,
  SetCurrentGPIOPinVal = 0x30,
  ReadEEPROM = 0x50,
  WriteEEPROM = 0x51,
  GetCurrentInterruptCount = 0x12,
  SpiDataTransfer = 0x42,
  CancelSpiDataTransfer = 0x11,
  ReleaseSpiBus = 0x80,
  GetChipStatus = 0x10,
  SendPassword = 0x70,
} MCP2210Command;

// All sub-commands listed in the MCP2210 datasheet
// for configuring power-up settings
typedef enum mcp2210_sub_command_t {
  ChipSettings = 0x20,
  SpiSettings = 0x10,
  USBSettings = 0x30,
  ManufacturerName = 0x50,
  ProductName = 0x40,
  None = 0x00,
} MCP2210SubCommand;

// Generic MCP2210 packet
typedef struct mcp2210_generic_packet_st {
  unsigned char __data__[64];
} MCP2210GenericPacket;

// Convenient members for configuring USB Key Settings
typedef struct mcp2210_key_settings_st {
  unsigned char __pad1__[4];
  unsigned char vid_low;
  unsigned char vid_high;
  unsigned char pid_low;
  unsigned char pid_high;
  unsigned char power_option;
  unsigned char requested_current;
  unsigned char __pad2__[54];
} MCP2210USBKeySettings;

// Convenient members for setting access password
typedef struct mcp2210_access_pass_st {
  unsigned char pass_char_0;
  unsigned char pass_char_1;
  unsigned char pass_char_2;
  unsigned char pass_char_3;
  unsigned char pass_char_4;
  unsigned char pass_char_5;
  unsigned char pass_char_6;
  unsigned char pass_char_7;
} MCP2210AccessPassword;

// Convenient members for configuring Chip Settings
typedef struct mcp2210_chip_settings_st {
  unsigned char __pad1__[4];
  unsigned char gp0_des;
  unsigned char gp1_des;
  unsigned char gp2_des;
  unsigned char gp3_des;
  unsigned char gp4_des;
  unsigned char gp5_des;
  unsigned char gp6_des;
  unsigned char gp7_des;
  unsigned char gp8_des;
  MCP2210GPIOPinVal default_val;
  MCP2210GPIOPinDir default_dir;
  unsigned char chip_settings;
  unsigned char chip_access_control;
  MCP2210AccessPassword pass;
  unsigned char __pad2__[37];
} MCP2210ChipSettings;

typedef struct mcp2210_gpio_val_settings_st {
  unsigned char __pad1__[4];
  MCP2210GPIOPinVal pin_vals;
  unsigned char __pad2__[58];
} MCP2210GPIOPinValSettings;

typedef struct mcp2210_gpio_dir_settings_st {
  unsigned char __pad1__[4];
  MCP2210GPIOPinDir pin_dirs;
  unsigned char __pad2__[58];
} MCP2210GPIOPinDirSettings;

// Convenient members for a generic response packet
typedef struct mcp2210_generic_response_st {
  unsigned char cmd_echo;
  unsigned char status;
  unsigned char __pad1__[62];
} MCP2210GenericResponse;

// Convenient members for configuring SPI Transfer Settings
typedef struct mcp2210_spi_transfer_settings_st {
  unsigned char __pad1__[4];
  unsigned char br3;
  unsigned char br2;
  unsigned char br1;
  unsigned char br0;
  unsigned char idle_cs_val_low;
  unsigned char idle_cs_val_high;
  unsigned char active_cs_val_low;
  unsigned char active_cs_val_high;
  unsigned char cs_to_data_delay_low;
  unsigned char cs_to_data_delay_high;
  unsigned char last_data_to_cs_delay_low;
  unsigned char last_data_to_cs_delay_high;
  unsigned char inter_data_delay_low;
  unsigned char inter_data_delay_high;
  unsigned char bytes_per_transaction_low;
  unsigned char bytes_per_transaction_high;
  unsigned char spi_mode;
  unsigned char __pad2__[43];
} MCP2210SPITransferSettings;

// Convenient members for a SPI transfer
typedef struct mcp2210_spi_data_transfer_st {
  unsigned char __pad1__;
  unsigned char bytes;
  unsigned char __pad2__[2];
  unsigned char spi_data[60];
} MCP2210SPIDataTransfer;

// Convenient members for a SPI Transfer response
typedef struct mcp2210_spi_data_transfer_res_st {
  unsigned char cmd_echo;
  unsigned char status;
  unsigned char bytes;
  unsigned char engine_status;
  unsigned char spi_data[60];
} MCP2210SPIDataTransferResponse;

// maps to GPIO pin configured as CS. Expand this enum for
// other devices using the following pattern:
// (0x01 << CS_PIN_NUM)
typedef enum device {
  DAC = 0x01,
  MEM = (0x01 << 1),
} Device;

// Initializes the MCP2210. Places a hid_device handle in 'out'.
// Returns false on failure, true otherwise.
bool MCP2210_Init(hid_device **out);

// updates spi transfer settings. if 'vm' is false, updates NVRAM settings.
// otherwise, updates ram settings. Returns false if write fails, true otherwise.
bool MCP2210_WriteSpiSettings(hid_device *handle, const MCP2210SPITransferSettings newSettings, bool vm);

// get current spi transfer settings. if 'vm' is false, reads NVRAM settings.
// otherwise, reads ram settings. returns false if read fails, true otherwise.
bool MCP2210_ReadSpiSettings(hid_device *handle, MCP2210SPITransferSettings *currentSettings, bool vm);

// updates chip settings. if 'vm' is false, updates NVRAM settings.
// otherwise, updates ram settings. Returns false if write fails, true otherwise.
bool MCP2210_WriteChipSettings(hid_device *handle, const MCP2210ChipSettings newSettings, bool vm);

// reads chip settings. if 'vm' is false, reads NVRAM settings.
// otherwise, reads ram settings. Returns false if read fails, true otherwise.
bool MCP2210_ReadChipSettings(hid_device *handle, MCP2210ChipSettings *currentSettings, bool vm);

// sends the access password. only needs to be called if chip has conditional access enabled.
bool MCP2210_SendAccessPassword(hid_device *handle, MCP2210AccessPassword pass);

// updates the manufacturer string that the MCP2210 displays when enumerated. returns
// false if write fails, true otherwise.
bool MCP2210_WriteManufacturerName(hid_device *handle, const char * newName, size_t nameLen);

// reads the configured manufacturer string. returns false if read fails, true otherwise.
bool MCP2210_ReadManufacturerName(hid_device *handle, char currentName[30]);

// updates the product string that the MCP2210 displays when enumerated. returns
// false if write fails, true otherwise.
bool MCP2210_WriteProductName(hid_device *handle, const char * newName, size_t nameLen);

// reads the configured manufacturer string. returns false if read fails, true otherwise.
bool MCP2210_ReadProductName(hid_device *handle, char currentName[30]);

// updates the current GPIO pin values. returns false if write fails, true otherwise.
bool MCP2210_WriteGPIOValues(hid_device *handle, MCP2210GPIOPinValSettings newSettings);

// reads the current GPIO pin values. returns false if read fails, true otherwise.
bool MCP2210_ReadGPIOValues(hid_device *handle, MCP2210GPIOPinValSettings *currentSettings);

// updates the current GPIO pin directions. returns false if write fails, true otherwise.
bool MCP2210_WriteGPIODirections(hid_device *handle, MCP2210GPIOPinDirSettings newSettings);

// reads the current GPIO pin directions. returns false if read fails, true otherwise.
bool MCP2210_ReadGPIODirections(hid_device *handle, MCP2210GPIOPinDirSettings *currentSettings);

// writes to an MCP2210 EEPROM location. returns false if the write fails, true otherwise.
bool MCP2210_WriteEERPOM(unsigned char addr, unsigned char byte);

// reads from an MCP2210 EEPROM location. returns false if the read fails, true otherwise. 
bool MCP2210_ReadEEPROM(unsigned char addr, unsigned char *byte);

// reads the current number of interrupt events. returns false if the read fails, true otherwise.
bool MCP2210_ReadInterruptCount(unsigned int *interrupts);

// initiates a SPI data transfer that is 'bytes' long (0 <= bytes < 65536).
// Returns -1 if transfer fails, otherwise returns the number of received bytes.
int MCP2210_SpiDataTransfer(hid_device *handle,
                              unsigned int txBytes,
                              unsigned char * txData,
                              unsigned char *rxData,
                              Device dev);

// cancels an ongoing SPI transfers. Returns false on failure, true on success
bool MCP2210_CancelSpiDataTransfer(hid_device *handle);

// requests that external host releases SPI bus. returns false on failure, true on success.
bool MCP2210_RequestSpiBusRelease(hid_device *handle);

// reads the current chip status. returns false on read failure, true otherwise.
bool MCP2210_ReadChipStatus(hid_device *handle);

#endif  // MCP2210_H_