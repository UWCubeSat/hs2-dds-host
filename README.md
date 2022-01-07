# DDS-Host

## Table of Contents
* [Description](#description)
* [Compilation/Installation](#compilationinstallation)
  * [Ubuntu/Debian](#ubuntudebian)
  * [Windows](#windows)
    * [HIDAPI](#hidapi)
    * [inih](#inih)
* [Usage](#usage)
  * [Device Configuration](#device-configuration)
  * [Data Files](#data-files)


## Description
dds-host is a console-based (for now) application that allows us to configure and write data to the Direct-Digital Synthesizer Arbitrary Waveform Generator (DDS-AWG) from some host (in this case a PC). The DDS uses an MCP2210 USB-to-SPI IC which enumerates as a HID. This lets us control the DDS using the HID interface; more specifically, this application leverages hidapi to communicate with the MCP2210.

## Compilation/Installation
The Makefile supports compilation on both Windows (using MinGW) and linux (using GNU GCC).

### Ubuntu/Debian
Install HIDAPI and inih using apt:
```
$ sudo apt update && sudo apt install libhidapi-libusb0 libinih-dev libinih1
```

Configure a udev rule for the device:
```
$ cp /path/to/dds-host/udev/70-dds.rules
$ sudo udevadm control --reload-rules && sudo udevadm trigger
```
Note: the `idVendor` and `idProduct` attributes in the udev rule must match the `VID` and `PID` you configure the device to have. These values must be **lowercase**, hexadecimal numbers.

### Windows
Download the hidapi and inih sources and compile them manually using MinGW64. HIDAPI can be downloaded/cloned from [here](https://github.com/libusb/hidapi), and inih can be downloaded/cloned from[here](https://github.com/benhoyt/inih).

#### HIDAPI
```
$ cd C:/path/to/hidapi/windows
$ gcc -I../hidapi -g -c hid.c -o hid.o
$ ar ru libhidapi.a hid.o
$ cp ../hidapi/hidapi.h C:/path/to/MinGW64/installation/include
$ cp libhidapi.a C:/path/to/MinGW64/installation/lib
```

#### inih
```
$ cd C:/path/to/inih
$ gcc -g -c inih.c -o inih.o
$ ar ru libinih.a inih.o
$ cp inih.h C:/path/to/MinGW64/installation/include
$ cp libinih.a C:/path/to/MinGW64/installation/lib
```
Note: additional flags can be passed to the compiler to configure inih's behavior. See [inih's README](https://github.com/benhoyt/inih/blob/master/README.md) for details.

## Usage
```
Usage: dds-host <--i | --c [-d <dac_config_file_path> -m <mcp_config_file_path> -D <data_folder>]>
        --i: Enter interactive and manually configure each device. Specify either this or --c
        --c: Enter configuration mode and pass configuration files. Specify either this or --i
                -d: Specifies the path to the DAC5687 configuration file in config mode.
                -m: Specifies the path to the MCP2210 configuration file in config mode.
        -D: Specifies the path to a folder containing data files.
```

### Device Configuration
In `config` mode, both the MCP2210 and DAC5687 can be configured via a config file in the [INI Format](https://en.wikipedia.org/wiki/INI_file). DDS-Host will read from these configuration files at runtime and write the configuration to the corresponding device. Currently this is the only mode supported. Eventually, I will include an `interactive` mode which will allow the user to choose settings interactively on the command line.

Check out [these example configs](../tree/develop/cfg) for an idea of what settings can be configured. Also check out the datasheets ([MCP2210](https://ww1.microchip.com/downloads/en/DeviceDoc/22288A.pdf) and [DAC5687](https://www.ti.com/lit/gpn/dac5687)) for these devices to learn about the numerical range for each configuration item.

### Data Files
See the data folder for an example of some canned data for the DDS. In actual operation, these files would be generated from our simulation of the instrument and would represent a single received reflection from the target. The canned data is a simpler waveform to demonstrate functionality. These data are formatted as a series of comma separated bytes: the first 3 bytes in a row are the SRAM address to which the data should be written, the last 4 bytes are the data word to write.

Data files must be in the CSV format and formatted in a particular way. Each row needs to end in a newline ('\n' on \*nix-like machines), NOT a comma. There needs to be a newline at the end of the file as well (1 empty line).
