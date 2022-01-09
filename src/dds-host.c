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
#include <dirent.h>
#include <sys/types.h>

// HIDAPI
#include <hidapi.h>

// inih
#include <ini.h>

// project libraries
#include "cfg.h"
#include "dds-host.h"
#include "mcp2210.h"
#include "dac5687.h"
#include "sram.h"
#include "csv.h"

static void PrintUsage() {
  fprintf(stderr, "Usage: dds-host <--i | --c [-d <dac_config_file_path> -m <mcp_config_file_path> -D <data_folder>]>\n");
  fprintf(stderr, "\t--i: Enter interactive and manually configure each device. Specify either this or --c\n");
  fprintf(stderr, "\t--c: Enter configuration mode and pass configuration files. Specify either this or --i\n");
  fprintf(stderr, "\t\t-d: Specifies the path to the DAC5687 configuration file in config mode.\n");
  fprintf(stderr, "\t\t-m: Specifies the path to the MCP2210 configuration file in config mode.\n");
  fprintf(stderr, "\t-D: Specifies the path to a folder containing data files.\n");
}

static int HandleInteractiveMode(Settings *out) {
  // TODO: implement interactive mode
  //       - Step for each DAC register, user chooses if they'd like to configure that register
  //          - default values for registers which aren't configured
  //       - Step for each MCP2210 config block: usb, chip, spi
  //          - user can choose to, defaults will be chosen otherwise
  return 0;
}

static int HandleConfigMode(Settings *out, char *dac_cfg_path, char *mcp_cfg_path) {
  int status;

  if (out == NULL) {
    fprintf(stderr, "out can't be null\n");
    return -1;
  }

  status = ini_parse(mcp_cfg_path, CFG_HandleMCP2210Config, &out->mcp);
  if (status != 0) {
    fprintf(stderr, "Failed to parse MCP2210 config file. Error code: %d\n", status);
    return -1;
  }
  
  status = ini_parse(dac_cfg_path, CFG_HandleDAC5687Config, &out->dac);
  if (status != 0) {
    fprintf(stderr, "Failed to parse DAC config file. Error code: %d\n", status);
    return -1;
  }
  return 0;
}

// verify that arguments are correct for config mode
static int CheckConfigArgs(int argc, char *dac_cfg, char *mcp_cfg, char *data_path) {
  // first check that the number of args was right
  if (argc != 8) {
    fprintf(stderr, "Error - wrong number of arguments in config mode.\n");
    return -1;
  }

  // check that files exist
  if (access(dac_cfg, F_OK) < 0) {
    fprintf(stderr, "Error - supplied DAC config file doesn't appear to exist.\n");
    return -1;
  }

  if (access(mcp_cfg, F_OK) < 0) {
    fprintf(stderr, "Error - supplied MCP config file doesn't appear to exist.\n");
    return -1;
  }

  if (access(data_path, F_OK) < 0) {
    fprintf(stderr, "Error - supplied data directory file doesn't appear to exist.\n");
    return -1;
  }
  return 0;
}

// verify that arguments are correct for interactive mode
static int CheckInteractiveArgs(int argc, char *data_path) {
  if (argc != 4) {
    return -1;
  }

  if (!access(data_path, F_OK)) {
    return -1;
  }
  return 0;
}

// configures the attached peripherals
static int ConfigureDevices(hid_device *handle, Settings *settings) {
  if (!MCP2210_Configure(handle, &settings->mcp) || 
      !DAC5687_Configure(handle, &settings->dac)) {
    return -1;
  }
  return 0;
}

// returns the number of data items, and places data items in data buffer.
static void WriteData(hid_device *handle, CSVFile *data_file) {
  // Go file-by-file in data directory, and read out CSV data
  // first three bytes are address, left-shifted 1 bit
  // next 4 bytes are data to write to SRAM as 32-bit int (technically a word is 36 bits but we'll ignore that for now)

  printf("starting read...\n");
  for (unsigned long long i = 0; i < data_file->numRows; i++) {
    uint32_t address = 0;
    uint32_t data = 0;

    for (unsigned long long j = 0; j < data_file->numCols; j++) {
      char *cell_str = CSV_ReadElement(data_file, i + 1, j + 1);
      char *end;
      uint32_t cell = strtol(cell_str, &end, 16);
      if (j < 3) {
        // shift into address
        address |= (cell & 0xFF) << ((2 - j) * 8);
      } else {
        // shift into our data
        data |= (cell & 0xFF) << ((6 - j) * 8);
      }
    }
    address >>= 1;
    printf("Address: 0x%X, Data: 0x%X\n", address, data);
    // SRAM_WriteAddress(handle, address, data);
  }
}

int main(int argc, char *argv[]) {
  // flags for command line switches
  bool interactive = false;
  bool config = false;
  char dac_path[MAX_PATH_LEN];
  char mcp_path[MAX_PATH_LEN];
  char data_path[MAX_PATH_LEN];

  Settings settings = {0};
  bool success = false;

  hid_device *mcp;
  
  // parse command string
  for (int i = 1; i < argc; i++) {
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
      case 'm':
      case 'D':
        i++;
        break;
      default:
        fprintf(stderr, "Unknown command code: %c\n", cmd);
        PrintUsage();
        return EXIT_FAILURE;
    }
  }

  if (config && !interactive) {
    strcpy(dac_path, argv[3]);
    strcpy(mcp_path, argv[5]);
    strcpy(data_path, argv[7]);
    if (CheckConfigArgs(argc, dac_path, mcp_path, data_path) < 0) {
      PrintUsage();
      return EXIT_FAILURE;
    } else {
      printf("Entering config mode...\n");
      success = HandleConfigMode(&settings, dac_path, mcp_path) == 0;
    }
  } else if (interactive && !config) {
    if (CheckInteractiveArgs(argc, data_path) < 0) {
      PrintUsage();
      return EXIT_FAILURE;
    } else {
      printf("Entering interactive mode...\n");
      success = HandleInteractiveMode(&settings) == 0;
    } 
  } else {
    PrintUsage();
    return EXIT_FAILURE;
  }

  if (!success) {
    if (config) fprintf(stderr, "Failed to read from config files. Check formatting.\n");
    else if (interactive) fprintf(stderr, "Failed to get all the way through interactive mode. Please start again.\n");
    return EXIT_FAILURE;
  }

  // attempt to open an attached HID
  mcp = MCP2210_Init();

  if (mcp == NULL) {
    hid_exit();
    return EXIT_FAILURE;
  }

  if (!ConfigureDevices(mcp, &settings)) {
    return EXIT_FAILURE;
  }

  DIR *dir_fd = opendir(data_path);
  if (dir_fd == NULL) {
    fprintf(stderr, "Failed to open data directory.\n");
  }
  struct dirent *dir_entry;
  CSVFile *next_data_file;
  chdir(data_path);
  while ((dir_entry = readdir(dir_fd))) {
    if (!strcmp (dir_entry->d_name, "."))
        continue;
    if (!strcmp (dir_entry->d_name, ".."))    
        continue;
    
    printf("File name: %s\n", dir_entry->d_name);

    next_data_file = CSV_Open(dir_entry->d_name);

    if (next_data_file == NULL) {
      fprintf(stderr, "Failed to open next data file\n");
      return EXIT_FAILURE;
    }

    WriteData(mcp, next_data_file);
    printf("Wrote contents of %s to SRAM\n", dir_entry->d_name);
    CSV_Close(next_data_file);
  }
  printf("No more files...\n");

  // trigger the burst-write from the SRAM to the DAC
  if (MCP2210_ReadGPIOValues(mcp, &settings.mcp.chip.defaultGPIOValue) < 0) {
    return EXIT_FAILURE;
  }

  if (MCP2210_ReadGPIODirections(mcp, &settings.mcp.chip.defaultGPIODirection) < 0) {
    return EXIT_FAILURE;
  }

  // ensure it's an output
  settings.mcp.chip.defaultGPIODirection &= ~(GPIO7);
  if (MCP2210_WriteGPIODirections(mcp, settings.mcp.chip.defaultGPIODirection) < 0) {
    return EXIT_FAILURE;
  }

  // pull the pin HIGH
  settings.mcp.chip.defaultGPIOValue |= GPIO7;
  if (MCP2210_WriteGPIOValues(mcp, settings.mcp.chip.defaultGPIOValue) < 0) {
    return EXIT_FAILURE;
  }

  MCP2210_Close(mcp);
  return EXIT_SUCCESS;
}