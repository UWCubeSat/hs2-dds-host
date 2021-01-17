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
#include <unistd.h> // for getopt()
#include <string.h> // for memset()

// HIDAPI
#include "hidapi/hidapi.h"

// project libraries
#include "dds-host/dds-host.h"
#include "dds-host/mcp2210.h"
#include "dds-host/dac5687.h"
#include "dds-host/cpld.h"
#include "dds-host/util/csv.h"

static void PrintUsage() {
  fprintf(stderr, "Usage: ./bin/dds-host --dac-config <filename> --mcp-config <filename> --data <filename>\n");
}

static bool CheckArgs(int argc, char *argv[]) {
  // we expect the args: --dac-config <filename> --mcp2210-config <filename> --data <filename>
  // TODO: make CLI more flexible
  if (argc != 7) {
    fprintf(stderr, "Not enough args: %d\n", argc);
    return false;
  }

  if (strcmp(argv[1], "--dac-config") != 0) {
    fprintf(stderr, "missing dac config file option\n");
    return false;
  }

  if (strcmp(argv[3], "--mcp-config") != 0) {
    fprintf(stderr, "missing mcp config file option\n");
    return false;
  }

  if (strcmp(argv[5], "--data") != 0) {
    fprintf(stderr, "missing data file option\n");
    return false;
  }
  return true;
}

static bool ConfigureDevices(hid_device *handle, char *dacFileName, char *mcpFileName) {
  CSVFile *dacConfigFile = CSV_Open(dacFileName);
  CSVFile *mcpConfigFile = CSV_Open(mcpFileName);

  if (dacConfigFile == NULL) {
    return false;
  }

  if (mcpConfigFile == NULL) {
    return false;
  }

  // configure the DAC first
  if (!DAC5687_Configure(dacConfigFile, handle)) {
    CSV_Close(mcpConfigFile);
    CSV_Close(dacConfigFile);
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

  if (MCP2210_WriteChipSettings(handle, &chipSettings, true) < 0) {
    CSV_Close(mcpConfigFile);
    CSV_Close(dacConfigFile);
    return false;
  }

  // clean up
  CSV_Close(mcpConfigFile);
  CSV_Close(dacConfigFile);
  return true;
}

int main(int argc, char *argv[]) {
  if (!CheckArgs(argc, argv)) {
    PrintUsage();
    return EXIT_FAILURE;
  }

   // attempt to open an attached HID
  hid_device *handle = MCP2210_Init();

  if (handle == NULL) {
    hid_exit();
    return EXIT_FAILURE;
  }

  if (!ConfigureDevices(handle, argv[2], argv[4])) {
    MCP2210_Close(handle);
    return EXIT_FAILURE;
  }
  
  CSVFile *dataFile = CSV_Open(argv[6]);

  if (dataFile == NULL) {
    MCP2210_Close(handle);
    return EXIT_FAILURE;
  }

  // write SRAM data in whatever format we've been given
  unsigned long long row, col;
  unsigned int addr = 0;

  switch (dataFile->numCols) {
    case (4):
    {
      // 1 byte per column
      for (row = 1; row <= dataFile->numRows; row++) {
        unsigned int txData = 0;
        for (col = 1; col <= dataFile->numCols; col++) {
          // read the first byte and shift it in
          const char * elem = CSV_ReadElement(dataFile, row, col);
          unsigned int byteAsInt = (unsigned int) strtol(elem, NULL, 16);
          free(elem);
          txData |= ((byteAsInt && 0xFF) << (col - 1) * 8);
        }
        if (!CPLD_WriteSRAMAddress(handle, addr, txData)) {
          fprintf(stderr, "WriteSRAMAddress() failed for addr: %d\n", addr);
        }
        addr++;
      }
      break;
    }
    case (2):
    {
      // 2 bytes per column
      for (row = 1; row <= dataFile->numRows; row++) {
        unsigned int txData = 0;
        for (col = 1; col <= dataFile->numCols; col++) {
          // read the first byte and shift it in
          const char * elem = CSV_ReadElement(dataFile, row, col);
          unsigned int byteAsInt = (unsigned int) strtol(elem, NULL, 16);
          free(elem);
          txData |= ((byteAsInt && 0xFF) << (col - 1) * 16);
        }
        if (!CPLD_WriteSRAMAddress(handle, addr, txData)) {
          fprintf(stderr, "WriteSRAMAddress() failed for addr: %d\n", addr);
        }
        addr++;
      }
      break;
    }
    case (1):
    {
      // 4 bytes per column
      for (row = 1; row <= dataFile->numRows; row++) {
        unsigned int txData = 0;
        const char * elem = CSV_ReadElement(dataFile, row, 1);
        unsigned int byteAsInt = (unsigned int) strtol(elem, NULL, 16);
        free(elem);
        txData |= ((byteAsInt && 0xFF) << (col - 1) * 16);
        if (!CPLD_WriteSRAMAddress(handle, addr, txData)) {
          fprintf(stderr, "WriteSRAMAddress() failed for addr: %d\n", addr);
        }
        addr++;
      }
      break;
    }
    default:
    {
      fprintf(stderr, "Data in the csv isn't formatted correctly. Check README for formatting notes.\n");
      CSV_Close(dataFile);
      MCP2210_Close(handle);
      return EXIT_FAILURE;
    }
  }

  CSV_Close(dataFile);
  MCP2210_Close(handle);
  return EXIT_SUCCESS;
}