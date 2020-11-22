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

#define VID 0x04D8
#define PID 0x00DE

// commenting these out before I make these needlessly complex
// // generic structure for USB commands, which are all 64 bytes.
// typedef struct mcp_usb_cmd_generic_st {
//   uint8_t cmd_code;
//   uint8_t sub_cmd_code;
//   uint8_t __data__[62];
// } MCPGenericUSBCommand;

// // generic structure for USB response, which are all 64 bytes.
// typedef struct mcp_usb_res_generic_st {
//   uint8_t cmd_code_echo;
//   uint8_t cmd_success;
//   uint8_t sub_cmd_echo;
//   uint8_t __data__[61];
// } MCPGenericUSBResponse;



#endif  // DDS_HOST_H_
