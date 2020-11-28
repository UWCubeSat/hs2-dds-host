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
#include <stdint.h> // for uint8_t
#include <string.h> // for memset()
#include <stdio.h>  // for printf()
#include <stdlib.h> // for exit()

// HIDAPI
#include "hidapi.h"

#include "dds-host/dds-host.h"

int main(int argc, char **argv) {
  int res, i;
  uint8_t buf[65];
  hid_device *handle;

  res = hid_init();
  
  if (res == -1) {
    printf("Couldn't initialize HIDAPI\n");
    exit(1);
  }

  // get the device handle
  handle = hid_open(0x4D8, 0xDE, NULL);

  if (!handle) {
    printf("Couldn't open device\n");
    exit(1);
  }

  // memset the buffer so we start with a clean slate
  memset(buf, 0, 64);

  // attempt to get the USB Product Name
  buf[0] = 0x61;  // Set NVRAM Settings 
  buf[1] = 0x40;  // Set USB Product Name
  // buf[2] = 0x00;  // Reserved
  // buf[3] = 0x00;  // reserved
  // buf[4] = 0x10;  // 16 bytes in "HSL DDS", (7 * 2) + 2 = 16 (dec) = 0x10 (hex)
  // buf[5] = 0x03;  // always fill USB String Descriptor ID with 0x03;
  // buf[6] = 0x48;  // H
  // buf[7] = 0x00;
  // buf[8] = 0x53;  // S
  // buf[9] = 0x00;
  // buf[10] = 0x4c; // L
  // buf[11] = 0x00; 
  // buf[12] = 0x20; // space
  // buf[13] = 0x00;
  // buf[14] = 0x44; // D
  // buf[15] = 0x00;
  // buf[16] = 0x44; // D
  // buf[17] = 0x00;
  // buf[18] = 0x53; // S
  // buf[19] = 0x00;

  //fill the rest with 0xFF
  for (i = 20; i < 64; i++)
    buf[i] = 0xFF;

  res = hid_write(handle, buf, 64);

  memset(buf, 0, 64);

  // read the response
  res = hid_read(handle, buf, 64);

  // if (res >= 0) {
  //   for (i = 0; i < 3; i++) {
  //     printf("buf[%d]: %X\n", i, buf[i]);
  //   }
  // } 

  // print the product name to stdout in hexadecimal
  printf("0x");
  for (i = 6; i <= 19; i++) {
    if (buf[i])
      printf("%X", buf[i]);
  }
  printf("\n");

  return 0;
}