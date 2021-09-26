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
#include <string.h> // for memset()
#include <unistd.h> // for access()
#include <fcntl.h>

// HIDAPI
#include <hidapi.h>

// project libraries
#include "cfg.h"
#include "dds-host.h"
#include "mcp2210.h"
#include "dac5687.h"
#include "sram.h"
#include "csv.h"

static void PrintUsage() {
  fprintf(stderr, "Usage: ./bin/dds-host <--i | --c [-d <dac_config_file_path> -m <mcp_config_file_path> -D <data_folder>]>\n");
  fprintf(stderr, "\t--i: Enter interactive and manually configure each device. Specify either this or --c\n");
  fprintf(stderr, "\t--c: Enter configuration mode and pass configuration files. Specify either this or --i\n");
  fprintf(stderr, "\t\t-d: Specifies the path to the DAC5687 configuration file in config mode.\n");
  fprintf(stderr, "\t\t-m: Specifies the path to the MCP2210 configuration file in config mode.\n");
  fprintf(stderr, "\t-D: Specifies the path to a folder containing data files.\n");
}

static bool HandleDACConfiguration(DAC5687Settings *out) {
  return true;
}

static bool HandleMCPConfiguration(MCP2210Settings *out) {
  return true;
}

static bool HandleInteractiveMode(Settings *out) {
  // TODO: implement interactive mode
  //       - Step for each DAC register, user chooses if they'd like to configure that register
  //          - default values for registers which aren't configured
  //       - Step for each MCP2210 config block: usb, chip, spi
  //          - user can choose to, defaults will be chosen otherwise
  return true;
}

static bool HandleMCPConfigOptions(ConfigBlock *blocks, MCP2210Settings *settings) {
  return true;
}

static bool HandleDACVersionBlock(ConfigBlock *block, DAC5687Settings *settings) {
  for (int i = 0; i < block->num_elements; i++) {
    ConfigElement *element = block->elements + i;
    char * end;
    uint8_t val_int = (uint8_t) strtol(element->value, &end, CFG_BASE);
    if (strcmp(element->key, DAC_VERSION_SLEEP_A) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->version.sleep_dac_a = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_VERSION_SLEEP_B) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->version.sleep_dac_b = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_VERSION_HPLA) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->version.hpla = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_VERSION_HPLB) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->version.hplb = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", element->key);
      return false;
    }
  }
  return true;
}

static bool HandleDACConfig0Block(ConfigBlock *block, DAC5687Settings *settings) {
  for (int i = 0; i < block->num_elements; i++) {
    ConfigElement *element = block->elements + i;
    char * end;
    uint64_t val_int = strtol(element->value, &end, CFG_BASE);
    if (strcmp(element->key, DAC_CONFIG0_PLL_DIV) == 0) {
      if (val_int > 0b11) {
        return false;
      }
      settings->config_0.pll_vco_div = (PLLVCODiv) val_int;
    } else if (strcmp(element->key, DAC_CONFIG0_PLL_FREQ) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_0.pll_freq = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG0_PLL_KV) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_0.pll_kv = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG0_INTERP) == 0) {
      if (val_int > 0b11) {
        return false;
      }
      settings->config_0.fir_interp = (FIRInterp) val_int;
    } else if (strcmp(element->key, DAC_CONFIG0_INV_PLL_LOCK) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_0.inv_pll_lock = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG0_FIFO_BYPASS) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_0.fifo_bypass = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", element->key);
      return false;
    }
  }
  return true;
}

static bool HandleDACConfig1Block(ConfigBlock *block, DAC5687Settings *settings) {
  for (int i = 0; i < block->num_elements; i++) {
    ConfigElement *element = block->elements + i;
    char * end;
    uint64_t val_int = strtol(element->value, &end, CFG_BASE);
    if (strcmp(element->key, DAC_CONFIG1_QFLAG) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_1.qflag = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG1_INTERL) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_1.interl = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG1_DUAL_CLK) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_1.dual_clk = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG1_TWOS) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_1.twos = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG1_REV_ABUS) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_1.rev_abus = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG1_REV_BBUS) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_1.rev_bbus = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG1_FIR_BYPASS) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_1.fir_bypass = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG1_FULL_BYPASS) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_1.full_bypass = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", element->key);
      return false;
    }
  }
  return true;
}

static bool HandleDACConfig2Block(ConfigBlock *block, DAC5687Settings *settings) {
  for (int i = 0; i < block->num_elements; i++) {
    ConfigElement *element = block->elements + i;
    char * end;
    uint64_t val_int = strtol(element->value, &end, CFG_BASE);
    if (strcmp(element->key, DAC_CONFIG2_NCO) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_2.nco = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG2_NCO_GAIN) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_2.nco_gain = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG2_QMC) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_2.qmc = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG2_CM_MODE) == 0) {
      if (val_int > 0b1111) {
        return false;
      }
      settings->config_2.cm_mode = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG2_INV_SINC) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_2.invsinc = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", element->key);
      return false;
    }
  }
  return true;
}

static bool HandleDACConfig3Block(ConfigBlock *block, DAC5687Settings *settings) {
  for (int i = 0; i < block->num_elements; i++) {
    ConfigElement *element = block->elements + i;
    char * end;
    uint64_t val_int = strtol(element->value, &end, CFG_BASE);
    if (strcmp(element->key, DAC_CONFIG3_SIF_4PIN) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_3.sif_4_pin = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG3_DAC_SER_DATA) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_3.dac_ser_data = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG3_HALF_RATE) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_3.half_rate = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG3_USB) == 0) {
      if (val_int > 1) {
        return false;
      }
      settings->config_3.usb = (uint8_t) val_int;
    } else if (strcmp(element->key, DAC_CONFIG3_COUNTER_MODE) == 0) {
      if (val_int > 7) {
        return false;
      }
      settings->config_3.counter_mode = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", element->key);
      return false;
    }
  }
  return true;
}

static bool HandleDACSyncCntlBlock(ConfigBlock *block, DAC5687Settings *settings) {
  for (int i = 0; i < block->num_elements; i++) {
    ConfigElement *element = block->elements + i;
    if (strcmp(element->key, DAC_SYNCCNTL_SYNC_PHSTR) == 0) {
      
    } else if (strcmp(element->key, DAC_SYNCCNTL_SYNC_NCO) == 0) {
      
    } else if (strcmp(element->key, DAC_SYNCCNTL_SYNC_CM) == 0) {
      
    } else if (strcmp(element->key, DAC_SYNCCNTL_SYNC_FIFO) == 0) {
      
    } else {
      fprintf(stderr, "Unknown setting: %s\n", element->key);
      return false;
    }
  }
  return true;
}

static bool HandleDACNCOBlock(ConfigBlock *block, DAC5687Settings *settings) {
  for (int i = 0; i < block->num_elements; i++) {
    ConfigElement *element = block->elements + i;
    if (strcmp(element->key, DAC_NCO_FREQ) == 0) {
      
    } else if (strcmp(element->key, DAC_NCO_PHASE) == 0) {
      
    } else {
      fprintf(stderr, "Unknown setting: %s\n", element->key);
      return false;
    }
  }
  return true;
}

static bool HandleDACDACABlock(ConfigBlock *block, DAC5687Settings *settings) {
  for (int i = 0; i < block->num_elements; i++) {
    ConfigElement *element = block->elements + i;
    if (strcmp(element->key, DAC_DAC_OFFSET) == 0) {
      
    } else if (strcmp(element->key, DAC_DAC_GAIN) == 0) {
      
    } else if (strcmp(element->key, DAC_VERSION_HPLA) == 0) {
      
    } else if (strcmp(element->key, DAC_VERSION_HPLB) == 0) {
      
    } else {
      fprintf(stderr, "Unknown setting: %s\n", element->key);
      return false;
    }
  }
  return true;
}

static bool HandleDACDACBBlock(ConfigBlock *block, DAC5687Settings *settings) {
  for (int i = 0; i < block->num_elements; i++) {
    ConfigElement *element = block->elements + i;
    if (strcmp(element->key, DAC_VERSION_SLEEP_A) == 0) {
      
    } else if (strcmp(element->key, DAC_VERSION_SLEEP_B) == 0) {
      
    } else if (strcmp(element->key, DAC_VERSION_HPLA) == 0) {
      
    } else if (strcmp(element->key, DAC_VERSION_HPLB) == 0) {
      
    } else {
      fprintf(stderr, "Unknown setting: %s\n", element->key);
      return false;
    }
  }
  return true;
}

static bool HandleDACQMCBlock(ConfigBlock *block, DAC5687Settings *settings) {
  for (int i = 0; i < block->num_elements; i++) {
    ConfigElement *element = block->elements + i;
    if (strcmp(element->key, DAC_VERSION_SLEEP_A) == 0) {
      
    } else if (strcmp(element->key, DAC_VERSION_SLEEP_B) == 0) {
      
    } else if (strcmp(element->key, DAC_VERSION_HPLA) == 0) {
      
    } else if (strcmp(element->key, DAC_VERSION_HPLB) == 0) {
      
    } else {
      fprintf(stderr, "Unknown setting: %s\n", element->key);
      return false;
    }
  }
  return true;
}

static bool HandleDACConfigOptions(ConfigBlock *blocks, DAC5687Settings *settings) {
  // spicy if-else if-else block
  for (int i = 0; i < blocks->num_elements; i++) {
    ConfigBlock *block = blocks + i;
    if (strcmp(block->key, DAC_VERSION_CFG_KEY) == 0) {
      HandleDACVersionBlock(block, settings);
    } else if (strcmp(block->key, DAC_CONFIG_0_CFG_KEY) == 0) {
      HandleDACConfig0Block(block, settings);
    } else if (strcmp(block->key, DAC_CONFIG_1_CFG_KEY) == 0) {
      HandleDACConfig1Block(block, settings);
    } else if (strcmp(block->key, DAC_CONFIG_2_CFG_KEY) == 0) {
      HandleDACConfig2Block(block, settings);
    } else if (strcmp(block->key, DAC_CONFIG_3_CFG_KEY) == 0) {
      HandleDACConfig3Block(block, settings);
    } else if (strcmp(block->key, DAC_SYNC_CNTL_CFG_KEY) == 0) {
      HandleDACSyncCntlBlock(block, settings);
    } else if (strcmp(block->key, DAC_NCO_CFG_KEY) == 0) {
      HandleDACNCOBlock(block, settings);
    } else if (strcmp(block->key, DAC_DACA_CFG_KEY) == 0) {
      HandleDACDACABlock(block, settings);
    } else if (strcmp(block->key, DAC_DACB_CFG_KEY) == 0) {
      HandleDACDACBBlock(block, settings);
    } else if (strcmp(block->key, DAC_QMC_CFG_KEY) == 0) {
      HandleDACQMCBlock(block, settings);
    } else {
      fprintf(stderr, "Encountered unknown block: %s.\n", block->key);
      return false;
    }
  }
  return true;
}

static bool HandleConfigMode(Settings *out, char *dac_cfg, char *mcp_cfg) {
  int mcp_fd;
  int dac_fd;
  DAC5687Settings dac_settings = {0};
  MCP2210Settings mcp_settings = {0};
  Config dac_config = {0};
  Config mcp_config = {0};

  if (out == NULL) {
    return false;
  }

  mcp_fd = open(mcp_cfg, O_RDONLY);
  dac_fd = open(dac_cfg, O_RDONLY);

  if (mcp_fd < 0) {
    perror("Failed to open the MCP2210 config file.");
    return false;
  }

  if (dac_fd < 0) {
    perror("Failed to open the DAC5687 config file.");
    return false;
  }

  if (!CFG_ParseConfigFile(mcp_fd, &mcp_cfg)) {
    fprintf(stderr, "Failed to parse MCP config file.\n");
    return false;
  }
  
  if (!CFG_ParseConfigFile(dac_fd, &dac_cfg)) {
    fprintf(stderr, "Failed to parse DAC config file.\n");
    return false;
  }

  if (!HandleDACConfigOptions(&dac_config, &dac_settings)) {
    fprintf(stderr, "Failed to read out DAC settings.\n");
    return false;
  }  

  if (!HandleMCPConfigOptions(&mcp_config, &mcp_settings)) {
    fprintf(stderr, "Failed to read out MCP settings.\n");
    return false;
  }
  return true;
}

// verify that arguments are correct for config mode
static bool CheckConfigArgs(int argc, char *dac_cfg, char *mcp_cfg, char *data_path) {
  // first check that the number of args was right
  if (argc != 8) {
    return false;
  }

  // check that files exist
  if (!access(dac_cfg, F_OK)) {
    return false;
  }

  if (!access(mcp_cfg, F_OK)) {
    return false;
  }

  if (!access(data_path, F_OK)) {
    return false;
  }
  return true;
}

// verify that arguments are correct for interactive mode
static bool CheckInteractiveArgs(int argc, char *data_path) {
  if (argc != 4) {
    return false;
  }

  if (!access(data_path, F_OK)) {
    return false;
  }
  return true;
}

// configures the attached peripherals
static bool ConfigureDevices(hid_device *handle, Settings *settings) {
  if (!MCP2210_Configure(handle, &settings->mcp_settings) || 
      !DAC5687_Configure(handle, &settings->dac_settings)) {
    return false;
  }
  return true;
}

// returns the number of data items, and places data items in data buffer.
static Data ParseData(CSVFile *data_file) {
  
}

static bool LoadData(char *data_path) {
  CSVFile *data = CSV_Open(data_path);

  if (data == NULL) {
    return false;
  }

  return false;
}

int main(int argc, char *argv[]) {
  // flags for command line switches
  bool interactive = false;
  bool config = false;
  bool dac_config = false;
  bool mcp_config = false;
  bool data = false;
  char dac_path[MAX_PATH_LEN];
  char mcp_path[MAX_PATH_LEN];
  char data_path[MAX_PATH_LEN];

  Settings settings = {0};
  bool success = false;

  hid_device *mcp;
  
  // parse command string
  for (int i = 1; i < argc; i++) {
    // extract the command code
    char cmd = argv[i][1] == '-' ? argv[i][2] : argv[i][1];
    switch(cmd) {
      case 'c':
        config = true;
        break;
      case 'i':
        interactive = true;
        break;
      case 'd':
        dac_config = true;
        i++;
        break;
      case 'm':
        mcp_config = true;
        i++;
        break;
      case 'D':
        data = true;
        i++;
        break;
      default:
        fprintf(stderr, "Unknown command code: %c\n", cmd);
        PrintUsage();
        return EXIT_FAILURE;
    }
  }

  if (config) {
    if (!CheckConfigArgs(argc, dac_path, mcp_path, data_path)) {
      PrintUsage();
      return EXIT_FAILURE;
    } else {
      printf("Entering config mode...\n");
      success = HandleConfigMode(&settings, dac_path, mcp_path);
    }
  } else if (interactive) {
    if (!CheckInteractiveArgs(argc, data_path)) {
      PrintUsage();
      return EXIT_FAILURE;
    } else {
      printf("Entering interactive mode...\n");
      success = HandleInteractiveMode(&settings);
    } 
  } else {
    PrintUsage();
    return EXIT_FAILURE;
  }

  if (!success) {
    if (config) fprintf(stderr, "Failed to read from config files. Check formatting.\n");
    else if (interactive) fprintf(stderr, "Failed to get all the way through interactive mode. Please start again.\n");
  }

   // attempt to open an attached HID
  mcp = MCP2210_Init();

  if (mcp == NULL) {
    hid_exit();
    return EXIT_FAILURE;
  }

  if (!ConfigureDevices(mcp, &settings)) {
    return false;
  }

  ParseData(data_path);

  // just close it
  MCP2210_Close(mcp);
  return EXIT_SUCCESS;
}