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

// C System Libraries
#include <string.h>   // for strlen(), memset()
#include <stdio.h>    // for fprintf(), stderr
#include <stdbool.h>  // for bool type and true/false macros

// HIDAPI
#include "hidapi.h"

// MCP2210
#include "dds-host/mcp2210.h"

// performs a generic write/read operation. Returns false on failure.
// Stores the responses from the MCP2210 in 'response'.
static bool MCP2210_GenericWriteRead(hid_device *handle, 
                                      MCP2210Command cmd,
                                      MCP2210SubCommand subCmd,
                                      MCP2210GenericPacket data,
                                      MCP2210GenericPacket *response) {
  data.__data__[0] = cmd;
  if (subCmd != None && (cmd == SetNVRAMSettings || cmd == GetNVRAMSettings)) {
    data.__data__[1] = subCmd;
  }

  int res = hid_write(handle, data.__data__, sizeof(MCP2210GenericPacket));

  if (res == -1) {
    fprintf(stderr, "hid_write failed\n");
    return false;
  }

  res = hid_read(handle, response->__data__, sizeof(MCP2210GenericPacket));

  if (res == -1) {
    fprintf(stderr, "hid_read failed\n");
    return false;
  }
  return true;
}

bool MCP2210_Init(hid_device **handle) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  int res = hid_init();

  if (res == -1) {
    fprintf(stderr, "Failed to initialize HIDAPI\n");
    return false;
  }

  *handle = hid_open(VID, PID, NULL);

  if (*handle == NULL) {
    fprintf(stderr, "Failed to open specified device %#x:%#x\n", VID, PID);
    return false;
  }

  return true;
}

bool MCP2210_WriteSpiSettings(hid_device *handle, const MCP2210SPITransferSettings newSettings, bool vm) {
  if (handle == NULL) {
    fprintf(stderr, "handle must not be null\n");
    return false;
  }

  bool result = false;
  MCP2210GenericPacket data = {0};
  MCP2210GenericPacket response = {0};

  // move SPI transfer settings into a generic packet
  memcpy(&data, &newSettings, sizeof(MCP2210GenericPacket));

  // determine if we're configuring power-up settings or current settings
  if (vm) {
    result = MCP2210_GenericWriteRead(handle, SetCurrentSpiSettings, None, data, &response);
  } else {
    result = MCP2210_GenericWriteRead(handle, SetNVRAMSettings, SpiSettings, data, &response);
  }

  if (!result) {
    fprintf(stderr, "SPI settings write failed\n");
    return false;
  }

  // TODO: process our response and decide what to do.
  return true;
}