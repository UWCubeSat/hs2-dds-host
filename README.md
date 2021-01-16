## Purpose
dds-host is a console-based (for now) application that allows us to configure and write data to the Direct-Digital Synthesizer Arbitrary Waveform Generator (DDS-AWG) from a host controller (in this case a PC). The DDS uses an MCP2210 USB-to-SPI IC which enumerates as a HID. This lets us control the DDS using the HID interface; more specifically, this application leverages hidapi to communicate with the MCP2210. This project is currently very small while I figure out how to use HID devices in general and get comfortable with the MCP2210's interface in particular.

## src
# cpld.c
This file enumerates a few functions to read and write to the SRAM on the DDS-AWG.

# csv.c
This file provides a really simple interface to read from CSV Files (but not read).

# dac5687.c
This file provides an interface to read, write and configure a DAC5687 via the MCP2210.

# dds-host.c
This is the 'main' file. It ties the other modules together, and lets us write a rangeline to the
DDS-AWG.

# mcp2210.c
This provides a full-featured interface for the MCP2210 implemented on top of the HIDAPI library.

## Compilation
Until I make a better makefile, simply type "make" in the root of the repository directory. 

## Usage
Because I don't want to learn how to write udev rules right now, this program requires that the user invoke it as 'root' using sudo.
Invoking the program then looks like:
$sudo bin/dds-host --dac-config <filename> --mcp-config <filename> --data <filename>