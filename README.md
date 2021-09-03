# Purpose
dds-host is a console-based (for now) application that allows us to configure and write data to the Direct-Digital Synthesizer Arbitrary Waveform Generator (DDS-AWG) from some host (in this case a PC). The DDS uses an MCP2210 USB-to-SPI IC which enumerates as a HID. This lets us control the DDS using the HID interface; more specifically, this application leverages hidapi to communicate with the MCP2210.

# src
## cpld.c
This file enumerates a few functions to read and write to the SRAM on the DDS-AWG.

## csv.c
This file provides a really simple interface to read from CSV Files (but not write).

## cfg.c
This files provides an interface for parsing and generating config files.

## dac5687.c
This file provides an interface to read, write and configure a DAC5687 via the MCP2210.

## mcp2210.c
This provides a full-featured interface for the MCP2210 implemented on top of the HIDAPI library.

## dds-host.c
This is the 'main' file. It ties the other modules together, and lets us write a rangeline to the
DDS-AWG.

# Compilation
The Makefile supports compilation on both Windows (using MinGW) and linux (using GNU GCC).

On Ubuntu/Debian, you can install hidapi via apt:
`$sudo apt update && sudo apt install libhidapi-libusb0`

On Windows, it is necessary to download the hidapi sources and compile them manually using MinGW64. This can be done using MinGW64's gcc, then invoking the `ar ru` command on the generated hid.o file. Then, the provided header file and shared library (libhidapi.a) should be added to your MinGW64 include and lib folders, respectively.

# Usage
On Linux, this tool requires sudo since it accesses USB devices, which are a privileged resource. Eventually, I'll set up a udev rule that will bypass this requirement. On Windows, simply invoke the provided executable from your choice of terminal.

## CSV Files
CSV files in general need to be formatted in a particular way. Each row needs to end in a newline ('\n' on *nix-like machines), NOT a comma.
There needs to be a newline at the end of the file as well (1 empty line).

### DAC Config File
See the cfg folder for an example DAC5687 config file. Config files are broken up into "blocks", which contain config elements. The tool parses these files and generates settings which are then written to the DAC5687 at runtime.  

### MCP Config File
See the cfg folder for an example MCP2210 config file. Config files are broken up into "blocks", which contain config elements. The tool parses these files and generates settings which are then written to the MCP2210 at runtime. 

### Data File
See the data folder for an example of some canned data for the DDS. In actual operation, these files would be generated from our simulation of the instrument and would represent a single received reflection from the target. The canned data is a simpler waveform to demonstrate functionality. These data are formatted as a series of comma separated values - the first 3 bytes in the file are the address of the SRAM to which the data should be written, and the bottom 4 bytes are the actual data to write.