# DDS-Host

## Table of Contents
* [Description](#description)
* [Folder Structure](#folder-structure)

## Description
dds-host is a console-based (for now) application that allows us to configure and write data to the Direct-Digital Synthesizer Arbitrary Waveform Generator (DDS-AWG) from some host (in this case a PC). The DDS uses an MCP2210 USB-to-SPI IC which enumerates as a HID. This lets us control the DDS using the HID interface; more specifically, this application leverages hidapi to communicate with the MCP2210.


## src
### cpld.c
This file enumerates a few functions to read and write to the SRAM on the DDS-AWG.

### csv.c
This file provides a really simple interface to read from CSV Files (but not write).

### cfg.c
This files provides an interface for parsing and generating config files.

### dac5687.c
This file provides an interface to read, write and configure a DAC5687 via the MCP2210.

### mcp2210.c
This provides a full-featured interface for the MCP2210 implemented on top of the HIDAPI library.

### dds-host.c
This is the 'main' file. It ties the other modules together, and lets us write a rangeline to the
DDS-AWG.

## Compilation
The Makefile supports compilation on both Windows (using MinGW) and linux (using GNU GCC).

On Ubuntu/Debian, you can install hidapi and inih via apt:
```
$ sudo apt update && sudo apt install libhidapi-libusb0 libinih-dev libinih1
```

On Windows, you can download the hidapi and inih sources and compile them manually using MinGW64. HIDAPI can be downloaded/cloned [here](https://github.com/libusb/hidapi), and inih can be downloaded/cloned [here](https://github.com/benhoyt/inih).

For HIDAPI:
```
$ cd C:/path/to/hidapi/windows
$ gcc -I../hidapi -g -c hid.c -o hid.o
$ ar ru libhidapi.a hid.o
$ cp ../hidapi/hidapi.h C:/path/to/MinGW64/installation/include
$ cp libhidapi.a C:/path/to/MinGW64/installation/lib
```

For inih:
```
$ cd C:/path/to/inih
$ gcc -g -c inih.c -o inih.o
$ ar ru libinih.a inih.o
$ cp inih.h C:/path/to/MinGW64/installation/include
$ cp libinih.a C:/path/to/MinGW64/installation/lib
```
Note: additional flags can be passed to the compiler to configure inih's behavior. See [inih's README](https://github.com/benhoyt/inih/blob/master/README.md) for details.

## Usage
On Linux, this tool requires sudo since it accesses USB devices, which are a privileged resource. Eventually, I'll set up a udev rule that will bypass this requirement. On Windows, simply invoke the provided executable from your choice of terminal.

## CSV Files
CSV files in general need to be formatted in a particular way. Each row needs to end in a newline ('\n' on \*nix-like machines), NOT a comma.
There needs to be a newline at the end of the file as well (1 empty line).

## Device Configuration
In `config` mode, both the MCP2210 and DAC5687 can be configured via a config file in the [INI Format](https://en.wikipedia.org/wiki/INI_file). DDS-Host will read from these configuration files at runtime and write the configuration to the corresponding device. Currently this is the only mode supported, but I will include an `interactive` mode eventually which will allow the user to choose settings interactively on the command line.

## Data File
See the data folder for an example of some canned data for the DDS. In actual operation, these files would be generated from our simulation of the instrument and would represent a single received reflection from the target. The canned data is a simpler waveform to demonstrate functionality. These data are formatted as a series of comma separated bytes - the first 3 bytes in a row are the SRAM address to which the data should be written, is the data word to write.
