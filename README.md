# Purpose
dds-host is a console-based (for now) application that allows us to configure and write data to the Direct-Digital Synthesizer Arbitrary Waveform Generator (DDS-AWG) from a host controller (in this case a PC). The DDS uses an MCP2210 USB-to-SPI IC which enumerates as a HID. This lets us control the DDS using the HID interface; more specifically, this application leverages hidapi to communicate with the MCP2210. This project is currently very small while I figure out how to use HID devices in general and get comfortable with the MCP2210's interface in particular.

# src
## cpld.c
This file enumerates a few functions to read and write to the SRAM on the DDS-AWG.

## csv.c
This file provides a really simple interface to read from CSV Files (but not write).

## dac5687.c
This file provides an interface to read, write and configure a DAC5687 via the MCP2210.

## dds-host.c
This is the 'main' file. It ties the other modules together, and lets us write a rangeline to the
DDS-AWG.

## mcp2210.c
This provides a full-featured interface for the MCP2210 implemented on top of the HIDAPI library.

# Compilation
Until I make a better makefile, simply type "make" in the root of the repository directory. 
On Ubuntu/Debian, you can install hidapi via apt:
$ sudo apt update && sudo apt install libhidapi-libusb0

# Usage
Because I don't want to learn how to write udev rules right now, this program requires that the user invoke it as 'root' using sudo.
Invoking the program then looks like:
$sudo bin/dds-host --dac-config <filename> --mcp-config <filename> --data <filename>

## CSV Files
CSV files in general need to be formatted in a particular way. Each row needs to end in a newline ('\n' on *nix-like machines), NOT a comma.
There needs to be a newline at the end of the file as well (1 empty line).

### DAC Config File
This file is formatted so that the address (in base-16) is in the first column, and the byte to be written to that address is in the second column (also base-16). 

### MCP Config File
Currently this file isn't used.

### Data File
Data can be formatted 1 of 3 ways:
1) 4 bytes per column (the CSV is N X 1). 
2) 2 bytes per column (the CSV is N X 2).
3) 1 byte per column (the CSV is N X 4).