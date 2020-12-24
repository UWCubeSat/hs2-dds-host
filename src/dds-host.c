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
#include <stdint.h>   // for uint8_t
#include <string.h>   // for memset()
#include <stdio.h>    // for printf()
#include <stdlib.h>   // for exit()
#include <unistd.h>   // for getopt()

// HIDAPI
#include "hidapi.h"

// project libraries
#include "dds-host/dds-host.h"
#include "dds-host/dds-util.h"

int main(int argc, char **argv) {
  int res;
  int fDebug;
  const char * data_file_name = NULL;
  hid_device *handle;

  // extract our command line options
  

  res = dds_init(&handle);

  if (res == -1) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static int dds_init(hid_device **out) {
  int res;

  res = hid_init();
  
  if (res == -1) {
    fprintf(stderr, "Couldn't initialize HIDAPI\n");
    return res;
  }

  // get the device handle
  *out = hid_open(VID, PID, NULL);

  if (!*out) {
    fprintf(stderr, "Couldn't open device\n");
    return -1;
  }
  return 1;
}

static int dds_mcp_write_read(hid_device *dev,
                       uint8_t cmd,
                       uint8_t sub_cmd,
                       MCPUSBPacket *data,
                       MCPUSBPacket *response) {
  int res;
  data->__data__[0] = cmd;
  data->__data__[1] = sub_cmd;
  res = hid_write(dev, data->__data__, 64);

  if (res >= 0) {
    memset(data->__data__, 0, 64);
    res = hid_read(dev, response->__data__, 64);
  } else {
    fprintf(stderr, "Failed to write command.\n");
  }
  return res;
}

static int dds_dac_write(hid_device *dev,
                         uint8_t *bytes,
                         uint8_t n_bytes,
                         uint8_t addr,
                         MCPUSBPacket *response) {
  uint8_t header = 0;

  if (n_bytes > 4 || n_bytes <= 0) {
    fprintf(stderr, "Invalid number of bytes: %d.\n", n_bytes);
    return -1;
  }

  if (addr > 0x1F) {
    fprintf(stderr, "Address is beyond the range of DAC registers.\n");
    return -1;
  }
  // decrement before adding to header
  header |= ((n_bytes - 1) << 5) | addr;

  //add one for header byte
  n_bytes++;

  return dds_dac_transfer(dev, bytes, n_bytes, header, response);
}

static int dds_dac_read(hid_device *dev,
                         uint8_t n_bytes,
                         uint8_t addr,
                         MCPUSBPacket *response) {
  uint8_t header = 0;

  if (n_bytes > 4 || n_bytes <= 0) {
    fprintf(stderr, "Invalid number of bytes: %d.\n", n_bytes);
    return -1;
  }

  if (addr > 0x1F) {
    fprintf(stderr, "Address is beyond the range of DAC registers.\n");
    return -1;
  }
  // decrement before adding to header
  header |= (0x1 << 7) | ((n_bytes - 1) << 5) | addr;

  // add one for header byte
  n_bytes++;

  return dds_dac_transfer(dev, NULL, n_bytes, header, response);
}

static int dds_dac_transfer(hid_device *dev,
                            uint8_t *bytes,
                            uint8_t n_bytes,
                            uint8_t header,
                            MCPUSBPacket *response) {
  int res, i;

  // write necessary SPI transfer settings
  MCPSPITransferSettings spi_transfer_settings = {0};
  MCPResponse spi_res = {0};

  spi_transfer_settings.br2 = 0x1B;
  spi_transfer_settings.br1 = 0xB7;

  // we only want CS_DAC active for this transfer cycle
  spi_transfer_settings.idle_cs_val_low = 0x01;
  spi_transfer_settings.active_cs_val_low = 0xFE;
  spi_transfer_settings.cs_to_data_delay_low = 0x01;
  spi_transfer_settings.last_data_to_cs_delay_low = 0x01;
  spi_transfer_settings.inter_data_delay_low = 0x01;
  spi_transfer_settings.bytes_per_transaction_low = n_bytes;
  res = dds_mcp_write_read(dev, CMD_SET_RAM_SPI_SETTINGS, 0x00,
                           (MCPUSBPacket *) &spi_transfer_settings,
                           (MCPUSBPacket *) &spi_res);
  if (res == -1) {
    return res;
  }

  MCPSPIDataTransfer spi_data_transfer = {0};
  spi_data_transfer.bytes = n_bytes;
  spi_data_transfer.spi_data[0] = header;

  // only add bytes if we're doing a write operation
  if (bytes) {
    for (i = 0; i < n_bytes - 1; i++) {
      spi_data_transfer.spi_data[i + 1] = bytes[i];
    }
  }
  res = dds_mcp_write_read(dev, CMD_SPI_DATA_TRANSFER, 0x00,
                           (MCPUSBPacket *) &spi_data_transfer,
                           (MCPUSBPacket *) response);
  return res;
}