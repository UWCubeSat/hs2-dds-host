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

#include <stdint.h>   // for fixed-width integer types
#include <stdbool.h>

// HIDAPI
#include <hidapi.h>

// device ID for our application
#define VID                         0x04D8
#define PID                         0x00DE

#define TIMEOUT                     0 // ms

// MCP2210 Access Control modes
#define UNPROTECTED                 0x00
#define PROTECTED                   0x40
#define LOCKED                      0x80

// MCP2210 GPIO Pin Modes
#define GPIO                        0x00
#define CS                          0x01
#define DF                          0x02

// MCP2210 GPIO Pin Directions
#define GPIO_OUTPUT                 0x00
#define GPIO_INPUT                  0x01

// MCP2210 GPIO Pin Count
#define GPIO_COUNT                  9

// MCP2210 Pin Flags
#define GPIO0                       (1 << 0)
#define GPIO1                       (1 << 1)
#define GPIO2                       (1 << 2)
#define GPIO3                       (1 << 3)
#define GPIO4                       (1 << 4)
#define GPIO5                       (1 << 5)
#define GPIO6                       (1 << 6)
#define GPIO7                       (1 << 7)
#define GPIO8                       (1 << 8)

// MCP2210 Power Options
#define HOST_POWERED                (0x1 << 7)
#define SELF_POWERED                (0x1 << 6)
#define REMOTE_WAKE_UP              (0x1 << 5)

// Manufacturer name stuff
#define MAX_MAN_STR_LEN             29

// EEPROM stuff
#define EEPROM_MIN_ADDR             0
#define EEPROM_MAX_ADDR             255

// SPI Data transfer stuff
#define MAX_TRANSACTION_BYTES       65536
#define MAX_PACKET_BYTES            60
#define MAX_ATTEMPTS                 2000

// bytes in an MCP2210 report
#define MCP2210_REPORT_LEN          64

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
} MCP2210SubCommand;

// Convenient members for configuring USB Key Settings
typedef struct mcp2210_key_settings_st {
  uint8_t powerOption;
  uint8_t requestedCurrent;
  uint16_t vid;
  uint16_t pid;
} MCP2210USBKeySettings;

// Convenient members for configuring Chip Settings
typedef struct mcp2210_chip_settings_st {
  uint8_t gp0Designation;
  uint8_t gp1Designation;
  uint8_t gp2Designation;
  uint8_t gp3Designation;
  uint8_t gp4Designation;
  uint8_t gp5Designation;
  uint8_t gp6Designation;
  uint8_t gp7Designation;
  uint8_t gp8Designation;
  uint16_t defaultGPIOValue;
  uint16_t defaultGPIODirection;
  uint8_t chipSettings;
  uint8_t chipAccessControl;

  // 8 bytes long plus an extra for null terminator
  char pass[9];
} MCP2210ChipSettings;

// Convenient members for configuring SPI Transfer Settings
typedef struct mcp2210_spi_transfer_settings_st {
  uint32_t bitRate;
  uint16_t idleCSValue;
  uint16_t activeCSValue;
  uint16_t csToDataDelay;
  uint16_t lastDataToCSDelay;
  uint16_t dataToDataDelay;
  uint16_t bytesPerTransaction;
  uint8_t SPIMode;
} MCP2210SPITransferSettings;

typedef struct mcp2210_settings_t {
  MCP2210SPITransferSettings spi;
  MCP2210ChipSettings chip;
  MCP2210USBKeySettings usb;
} MCP2210Settings;

// Initializes the MCP2210. Places a hid_device handle in 'out'.
// Returns NULL on failure, otherwise a new hid_device.
hid_device * MCP2210_Init();

// Releases the MCP2210 and associated memory.
void MCP2210_Close(hid_device *handle);

// configures the MCP2210 with the provided settings.
int MCP2210_Configure(hid_device *handle, const MCP2210Settings *settings);

// updates spi transfer settings. if 'vm' is false, updates NVRAM settings.
// otherwise, updates ram settings. Returns false if write fails, true otherwise.
int MCP2210_WriteSpiSettings(hid_device *handle, const MCP2210SPITransferSettings *newSettings, bool vm);

// get current spi transfer settings. if 'vm' is false, reads NVRAM settings.
// otherwise, reads ram settings. returns false if read fails, true otherwise.
int MCP2210_ReadSpiSettings(hid_device *handle, MCP2210SPITransferSettings *currentSettings, bool vm);

// updates usb settings. if 'vm' is false, updates NVRAM settings.
// otherwise, updates ram settings. Retruns false if write fails, true otherwise.
int MCP2210_WriteUSBSettings(hid_device *handle, const MCP2210USBKeySettings *newSettings);

// get current usb settings. if 'vm' is false, updates NVRAM settings.
// otherwise, reads ram settings. Retruns false if read fails, true otherwise.
int MCP2210_ReadUSBSettings(hid_device *handle, MCP2210USBKeySettings *currentSettings);

// updates chip settings. if 'vm' is false, updates NVRAM settings.
// otherwise, updates ram settings. Returns false if write fails, true otherwise.
int MCP2210_WriteChipSettings(hid_device *handle, const MCP2210ChipSettings *newSettings, bool vm);

// reads chip settings. if 'vm' is false, reads NVRAM settings.
// otherwise, reads ram settings. Returns false if read fails, true otherwise.
int MCP2210_ReadChipSettings(hid_device *handle, MCP2210ChipSettings *currentSettings, bool vm);

// sends the access password. only needs to be called if chip has conditional access enabled.
int MCP2210_SendAccessPassword(hid_device *handle, const char *pass);

// updates the manufacturer string that the MCP2210 displays when enumerated. returns
// false if write fails, true otherwise.
int MCP2210_WriteManufacturerName(hid_device *handle, const char *newName, size_t nameLen);

// reads the configured manufacturer string. returns false if read fails, true otherwise.
int MCP2210_ReadManufacturerName(hid_device *handle, char *currentName);

// updates the product string that the MCP2210 displays when enumerated. returns
// false if write fails, true otherwise.
int MCP2210_WriteProductName(hid_device *handle, const char *newName, size_t nameLen);

// reads the configured manufacturer string. returns false if read fails, true otherwise.
int MCP2210_ReadProductName(hid_device *handle, char *currentName);

// updates the current GPIO pin values. returns false if write fails, true otherwise.
int MCP2210_WriteGPIOValues(hid_device *handle, uint16_t newGPIOValues);

// reads the current GPIO pin values. returns false if read fails, true otherwise.
int MCP2210_ReadGPIOValues(hid_device *handle, uint16_t *currentGPIOValues);

// updates the current GPIO pin directions. returns false if write fails, true otherwise.
int MCP2210_WriteGPIODirections(hid_device *handle, uint16_t newGPIODirections);

// reads the current GPIO pin directions. returns false if read fails, true otherwise.
int MCP2210_ReadGPIODirections(hid_device *handle, uint16_t *currentGPIODirections);

// writes to an MCP2210 EEPROM location. returns false if the write fails, true otherwise.
int MCP2210_WriteEERPOM(hid_device *handle, unsigned char addr, unsigned char byte);

// reads from an MCP2210 EEPROM location. returns false if the read fails, true otherwise. 
int MCP2210_ReadEEPROM(hid_device *handle, unsigned char addr, unsigned char *byte);

// reads the current number of interrupt events. returns false if the read fails, true otherwise.
int MCP2210_ReadInterruptCount(hid_device *handle, unsigned int *interrupts, bool reset);

// initiates a SPI data transfer that is 'bytes' long (0 <= bytes < 65536).
// Returns -1 if transfer fails, otherwise returns the number of received bytes.
int MCP2210_SpiDataTransfer(hid_device *handle,
                              unsigned int txBytes,
                              unsigned char *txData,
                              unsigned char *rxData,
                              MCP2210SPITransferSettings *settings);

// cancels an ongoing SPI transfers. Returns false on failure, true on success
int MCP2210_CancelSpiDataTransfer(hid_device *handle);

// requests that external host releases SPI bus. returns false on failure, true on success.
int MCP2210_RequestSpiBusRelease(hid_device *handle);

// reads the current chip status. returns false on read failure, true otherwise.
int MCP2210_ReadChipStatus(hid_device *handle);

#endif  // MCP2210_H_