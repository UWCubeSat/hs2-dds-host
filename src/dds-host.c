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

// C
#include <stdio.h>  // for printf()
#include <stdbool.h>  // for bool type
#include <stdlib.h> // for exit()
#include <string.h> // for memset()
#include <unistd.h> // for access()
#include <fcntl.h>

// HIDAPI
#include <hidapi.h>

// project libraries
#include "cfg.h"
#include "dds-host.h"
#include "mcp2210.h"
#include "dac5687.h"
#include "cpld.h"
#include "csv.h"

static void PrintUsage() {
  fprintf(stderr, "Usage: ./bin/dds-host <--i | --c [-d <dac_config_file_path> -m <mcp_config_file_path> -D <data_folder>]>\n");
  fprintf(stderr, "\t--i: Enter interactive and manually configure each device. Specify either this or --c\n");
  fprintf(stderr, "\t--c: Enter configuration mode and pass configuration files. Specify either this or --i\n");
  fprintf(stderr, "\t\t-d: Specifies the path to the DAC5687 configuration file in config mode.\n");
  fprintf(stderr, "\t\t-m: Specifies the path to the MCP2210 configuration file in config mode.\n");
  fprintf(stderr, "\t-D: Specifies the path to a folder containing data files.\n");
}

static bool HandleDACConfiguration(DAC5687Settings *out) {
  return true;
}

static bool HandleMCPConfiguration(MCP2210Settings *out) {
  return true;
}

static bool HandleInteractiveMode(Settings *out) {
  // TODO: implement interactive mode
  //       - Step for each DAC register, user chooses if they'd like to configure that register
  //          - default values for registers which aren't configured
  //       - Step for each MCP2210 config block: usb, chip, spi
  //          - user can choose to, defaults will be chosen otherwise
  return true;
}

static bool HandleMCPConfigOptions() {
  return true;
}

static bool HandleConfigMode(Settings *out, char * dac_cfg, char * mcp_cfg) {
  ConfigFile *mcp_config_file;
  ConfigFile *dac_config_file;

  mcp_config_file = CFG_OpenConfigFile(mcp_cfg);
  dac_config_file = CFG_OpenConfigFile(dac_cfg);

  if (mcp_config_file == NULL || dac_config_file == NULL) {
    return false;
  }

  if (!CFG_ParseConfigFile(mcp_config_file)) {
    fprintf(stderr, "Failed to parse MCP config file.\n");
    return false;
  }
  
  if (!CFG_ParseConfigFile(dac_config_file)) {
    fprintf(stderr, "Failed to parse DAC config file.\n");
    return false;
  }

  Settings *settings = (Settings *) malloc(sizeof(Settings));

  return true;
}

static bool CheckConfigArgs(int argc, char * dac_cfg, char * mcp_cfg, char * data_path) {
  // first check that the number of args was right
  if (argc != 8) {
    PrintUsage();
    return false;
  }

  // check that files exist
  if (!access(dac_cfg, F_OK)) {
    PrintUsage();
    return false;
  }

  if (!access(mcp_cfg, F_OK)) {
    PrintUsage();
    return false;
  }

  if (!access(data_path, F_OK)) {
    PrintUsage();
    return false;
  }
  return true;
}

static bool CheckInteractiveArgs(int argc) {
  if (argc != 2) {
    PrintUsage();
    return false;
  }
  return true;
}

static bool ConfigureDevices(hid_device *handle, Settings settings) {
  // configure the DAC first
  if (!DAC5687_Configure(handle, settings.dac_settings)) {
    return false;
  }

  // TODO: use a config file
  // configure MCP2210 Chip settings
  MCP2210ChipSettings chipSettings = {0};
  chipSettings.gp0Designation = CS;
  chipSettings.gp1Designation = CS;
  chipSettings.gp3Designation = DF;
  // leave the rest of pins as GPIOs
  chipSettings.chipSettings = 0x00;
  chipSettings.defaultGPIODirection = 0x0000;
  chipSettings.defaultGPIOValue = 0xFFFF;

  bool status = MCP2210_WriteChipSettings(handle, &chipSettings, true) < 0;
  return status;
}

int main(int argc, char *argv[]) {
  // flags for command line switches
  bool interactive = false;
  bool config = false;
  bool dac_config = false;
  bool mcp_config = false;
  bool data = false;
  char dac_path[MAX_PATH_LEN];
  char mcp_path[MAX_PATH_LEN];
  char data_path[MAX_PATH_LEN];
  
  // parse command string
  int i;
  for (i = 1; i < argc; i++) {
    // extract the command code
    char cmd = argv[i][1] == '-' ? argv[i][2] : argv[i][1];
    switch(cmd) {
      case 'c':
        config = true;
        break;
      case 'i':
        interactive = true;
        break;
      case 'd':
        dac_config = true;
        if (i < argc - 1) {
          strcpy(dac_path, argv[++i]);
        }
        break;
      case 'm':
        mcp_config = true;
        if (i < argc - 1) {
          strcpy(mcp_path, argv[++i]);
        }
        break;
      case 'D':
        data = true;
        if (i < argc - 1) {
          strcpy(data_path, argv[++i]);
        }
        break;
      default:
        fprintf(stderr, "Unknown command code: %c", cmd);
        return EXIT_FAILURE;
    }
  }

  Settings settings = {0};
  bool success = false;
  if (config) {
    if (!CheckConfigArgs(argc, dac_path, mcp_path, data_path)) {
      PrintUsage();
      return EXIT_FAILURE;
    } else {
      success = HandleConfigMode(&settings, dac_path, mcp_path);
    }
  } else if (interactive) {
    if (!CheckInteractiveArgs(argc)) {
      PrintUsage();
      return EXIT_FAILURE;
    } else {
      success = HandleInteractiveMode(&settings);
    } 
    printf("Entering interactive mode.\n");
  } else {
    PrintUsage();
    return EXIT_FAILURE;
  }

   // attempt to open an attached HID
  hid_device *handle = MCP2210_Init();

  if (handle == NULL) {
    hid_exit();
    return EXIT_FAILURE;
  }

  // if (!ConfigureDevices(handle, argv[2], argv[4])) {
  //   MCP2210_Close(handle);
  //   return EXIT_FAILURE;
  // }
  
  // CSVFile *dataFile = CSV_Open(argv[6]);

  // if (dataFile == NULL) {
  //   MCP2210_Close(handle);
  //   return EXIT_FAILURE;
  // }

  // CSV_Close(dataFile);
  // MCP2210_Close(handle);
  return EXIT_SUCCESS;
}