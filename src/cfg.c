// C
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // for bool type
#include <unistd.h>     // for fstat
#include <fcntl.h>      // for POSIX
#include <sys/stat.h>   // for fstat
#include <errno.h>      // for errno
#include <string.h>     // for string things

// inih
#include <ini.h>

// project libraries
#include "cfg.h"
#include "mcp2210.h"
#include "dac5687.h"

static inline bool section_match(const char *section, const char *s) {
    return strcmp(section, s) == 0;
}

static inline bool key_match(const char *key, const char *k) {
    return strcmp(key, k) == 0;
}

static inline bool key_contains(const char *key, const char *k) {
    return strstr(key, k) != NULL;
}

static int HandleMCPUSBBlock(MCP2210Settings *settings, const char *value,
                             const char *name) {
    char * end;
    uint64_t val_int = (uint64_t) strtol(value, &end, CFG_BASE);
    if (key_match(name, MCP_USB_POWER_OPTION)) {
      if (val_int > 0xA0) {
        return -1;
      }
      settings->usb_settings.powerOption = (uint8_t) val_int;
    } else if (key_match(name, MCP_USB_REQUEST_CURRENT)) {
      if (val_int > ((0b1 << 8) - 1)) {   // 2^8 - 1
        return -1;
      }
      settings->usb_settings.requestedCurrent = (uint8_t) val_int;
    } else if (key_match(name, MCP_USB_VID)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return -1;
      }
      settings->usb_settings.vid = (uint16_t) val_int;
    } else if (key_match(name, MCP_USB_PID)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return -1;
      }
      settings->usb_settings.pid = (uint16_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return -1;
    }
    return 0;
}

static bool HandleMCPChipBlock(MCP2210Settings *settings, const char *value,
                               const char *name) {
    char * end;
    uint64_t val_int = (uint64_t) strtol(value, &end, CFG_BASE);

    if (key_contains(name, MCP_CHIP_GP)) {
      uint8_t id = (uint8_t) strtol(name, &end, CFG_BASE);

      if (id > 8 || val_int > 2) {
        return false;
      }

      switch (id)
      {
      case 0:
        settings->chip.gp0Designation = val_int;
        break;
      case 1:
        settings->chip.gp1Designation = val_int;
        break;
      case 2:
        settings->chip.gp2Designation = val_int;
        break;
      case 3:
        settings->chip.gp3Designation = val_int;
        break;
      case 4:
        settings->chip.gp4Designation = val_int;
        break;
      case 5:
        settings->chip.gp5Designation = val_int;
        break;
      case 6:
        settings->chip.gp6Designation = val_int;
        break;
      case 7:
        settings->chip.gp7Designation = val_int;
        break;
      case 8:
        settings->chip.gp8Designation = val_int;
        break;
      
      default:
        return -1;
      }
    } else if (key_contains(name, MCP_CHIP_GP_DIR)) {
      uint8_t id = (uint8_t) strtol(name, &end, CFG_BASE);
      if (id > 8) {
        return -1;
      }
      settings->chip.defaultGPIODirection |= (((uint8_t) val_int) << id);
    } else if (key_contains(name, MCP_CHIP_GP_OUT)) {
      uint8_t id = (uint8_t) strtol(name, &end, CFG_BASE);
      if (id > 8) {
        return -1;
      }
      settings->chip.defaultGPIOValue |= (((uint8_t) val_int) << id);
    } else if (key_match(name, MCP_CHIP_REMOTE_WAKEUP)) {
      if (val_int > 1) {
        return -1;
      }
      settings->chip.chipSettings |= (((uint8_t) val_int) << 4);
    } else if (key_match(name, MCP_CHIP_INT_MODE)) {
      if (val_int > 0b100) {
        return -1;
      }
      settings->chip.chipSettings |= (((uint8_t) val_int) << 1);
    } else if (key_match(name, MCP_CHIP_SPI_RELEASE)) {
      if (val_int > 1) {
        return -1;
      }
      settings->chip.chipSettings |= ((uint8_t) val_int);
    } else if (key_match(name, MCP_CHIP_ACC_CONTROL)) {
      if (!(val_int == 0x00 || val_int == 0x40 || val_int == 0x80)) {
        return -1;
      }
      settings->chip.chipAccessControl |= ((uint8_t) val_int);
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return -1;
    }
    return 0;
}

static bool HandleMCPSPIBlock(MCP2210Settings *settings, const char *value,
                               const char *name) {
    char * end;
    uint64_t val_int = (uint64_t) strtol(value, &end, CFG_BASE);
    printf("%s, %ld\n", name, val_int);
    if (key_match(name, MCP_SPI_BIT_RATE)) {
      if (val_int > ((1UL << 32) - 1)) {  // 2^32 - 1
        return -1;
      }
      settings->spi_settings.bitRate = (uint32_t) val_int;
    } else if (key_contains(name, MCP_SPI_CS_IDLE)) {
      uint8_t id = (uint8_t) strtol(name, &end, CFG_BASE);
      if (id > 8) {
        return -1;
      }
      settings->spi_settings.idleCSValue |= (((uint8_t) val_int) << id);
    } else if (key_contains(name, MCP_SPI_CS_ACTIVE)) {
      uint8_t id = (uint8_t) strtol(name, &end, CFG_BASE);
      if (id > 8) {
        return -1;
      }
      settings->spi_settings.activeCSValue |= (((uint8_t) val_int) << id);
    } else if (key_match(name, MCP_SPI_CS_DATA_DLY)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return -1;
      }
      settings->spi_settings.csToDataDelay = (uint16_t) val_int;
    } else if (key_match(name, MCP_SPI_DATA_CS_DLY)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return -1;
      }
      settings->spi_settings.lastDataToCSDelay = (uint16_t) val_int;
    } else if (key_match(name, MCP_SPI_DATA_DATA_DLY)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return -1;
      }
      settings->spi_settings.dataToDataDelay = (uint16_t) val_int;
    } else if (key_match(name, MCP_SPI_BYTES)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return -1;
      }
      settings->spi_settings.bytesPerTransaction = (uint16_t) val_int;
    } else if (key_match(name, MCP_SPI_MODE)) {
      if (val_int > 3) {
        return -1;
      }
      settings->spi_settings.lastDataToCSDelay = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return -1;
    }
    return 0;
}

static int HandleDACVersionBlock(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint8_t val_int = (uint8_t) strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_VERSION_SLEEP_A)) {
        if (val_int > 1) {
            return -1;
        }
        settings->version.sleep_dac_a = (uint8_t) val_int;
    } else if (key_match(name, DAC_VERSION_SLEEP_B)) {
        if (val_int > 1) {
            return -1;
        }
        settings->version.sleep_dac_b = (uint8_t) val_int;
    } else if (key_match(name, DAC_VERSION_HPLA)) {
        if (val_int > 1) {
            return -1;
        }
        settings->version.hpla = (uint8_t) val_int;
    } else if (key_match(name, DAC_VERSION_HPLB)) {
        if (val_int > 1) {
            return -1;
        }
        settings->version.hplb = (uint8_t) val_int;
    } else {
        fprintf(stderr, "Unknown setting: %s\n", name);
        return -1;
    }
  return 0;
}

static int HandleDACConfig0Block(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_CONFIG0_PLL_DIV)) {
        if (val_int > 0b11) {
        return -1;
        }
        settings->config_0.pll_vco_div = (PLLVCODiv) val_int;
    } else if (key_match(name, DAC_CONFIG0_PLL_FREQ)) {
        if (val_int > 1) {
        return -1;
        }
        settings->config_0.pll_freq = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG0_PLL_KV)) {
        if (val_int > 1) {
        return -1;
        }
        settings->config_0.pll_kv = (uint8_t) val_int;
    } else if (key_match(name,  DAC_CONFIG0_INTERP)) {
        if (val_int > 0b11) {
        return -1;
        }
        settings->config_0.fir_interp = (FIRInterp) val_int;
    } else if (key_match(name,  DAC_CONFIG0_INV_PLL_LOCK)) {
        if (val_int > 1) {
        return -1;
        }
        settings->config_0.inv_pll_lock = (uint8_t) val_int;
    } else if (key_match(name,  DAC_CONFIG0_FIFO_BYPASS)) {
        if (val_int > 1) {
        return -1;
        }
        settings->config_0.fifo_bypass = (uint8_t) val_int;
    } else {
        fprintf(stderr, "Unknown setting: %s\n", name);
        return -1;
    }
    return 0;
}

static int HandleDACConfig1Block(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_CONFIG1_QFLAG)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_1.qflag = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_INTERL)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_1.interl = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_DUAL_CLK)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_1.dual_clk = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_TWOS)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_1.twos = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_REV_ABUS)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_1.rev_abus = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_REV_BBUS)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_1.rev_bbus = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_FIR_BYPASS)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_1.fir_bypass = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_FULL_BYPASS)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_1.full_bypass = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return -1;
    }
    return 0;
}

static int HandleDACConfig2Block(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_CONFIG2_NCO)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_2.nco = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG2_NCO_GAIN)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_2.nco_gain = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG2_QMC)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_2.qmc = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG2_CM_MODE)) {
      if (val_int > 0b1111) {
        return -1;
      }
      settings->config_2.cm_mode = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG2_INV_SINC)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_2.invsinc = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return -1;
    }
    return 0;
}

static int HandleDACConfig3Block(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_CONFIG3_SIF_4PIN)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_3.sif_4_pin = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG3_DAC_SER_DATA)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_3.dac_ser_data = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG3_HALF_RATE)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_3.half_rate = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG3_USB)) {
      if (val_int > 1) {
        return -1;
      }
      settings->config_3.usb = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG3_COUNTER_MODE)) {
      if (val_int > 7) {
        return -1;
      }
      settings->config_3.counter_mode = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return -1;
    }
    return 0;
}

static int HandleDACSyncCntlBlock(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_SYNCCNTL_SYNC_PHSTR)) {
      if (val_int > 1) {
        return -1;
      }
      settings->sync_cntl.sync_phstr = (uint8_t) val_int;
    } else if (key_match(name, DAC_SYNCCNTL_SYNC_NCO)) {
      if (val_int > 1) {
        return -1;
      }
      settings->sync_cntl.sync_nco = (uint8_t) val_int;
    } else if (key_match(name, DAC_SYNCCNTL_SYNC_CM)) {
      if (val_int > 1) {
        return -1;
      }
      settings->sync_cntl.sync_cm = (uint8_t) val_int;
    } else if (key_match(name, DAC_SYNCCNTL_SYNC_FIFO)) {
      if (val_int > 7) {
        return -1;
      }
      settings->sync_cntl.sync_fifo = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return -1;
    }
    return 0;
}

static int HandleDACNCOBlock(DAC5687Settings *settings, const char *name,
                              const char *value) {
    char *end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_NCO_FREQ)) {
      if (val_int > ((0b1 << 32) - 1)) { // 2^32 - 1
        return -1;
      }
      settings->nco_freq = (uint32_t) val_int;
    } else if (key_match(name, DAC_NCO_PHASE)) {
      if (val_int > ((0b1 << 16) - 1)) {  // 2^16 - 1
        return -1;
      }
      settings->nco_phase = (uint16_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return -1;
    }
    return 0;
}

static int HandleDACDACABlock(DAC5687Settings *settings, const char *name,
                               const char *value) {
    char *end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_DAC_OFFSET)) {
      if (val_int > ((0b1 << 12) - 1)) {  // 2^12 - 1
        return -1;
      }
      settings->dac_a_off = (uint16_t) val_int;
    } else if (key_match(name, DAC_DAC_GAIN)) {
      if (val_int > ((0b1 << 11) - 1)) {  // 2^11 - 1
        return -1;
      }
      settings->dac_a_gain = (uint16_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return -1;
    }
    return 0;
}

static bool HandleDACDACBBlock(DAC5687Settings *settings, const char *name,
                               const char *value) {
    char *end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_DAC_OFFSET)) {
      if (val_int > ((0b1 << 12) - 1)) {  // 2^12 - 1
        return -1;
      }
      settings->dac_b_off = (uint16_t) val_int;
    } else if (key_match(name, DAC_DAC_GAIN)) {
      if (val_int > ((0b1 << 11) - 1)) {  // 2^11 - 1
        return -1;
      }
      settings->dac_b_gain = (uint16_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", value);
      return -1;
    }
    return 0;
}

static bool HandleDACQMCBlock(DAC5687Settings *settings, const char *name,
                              const char *value) {
    char *end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_QMC_GAIN_A)) {
      if (val_int > ((0b1 << 10) - 1)) {  // 2^10 - 1
        return -1;
      }
      settings->qmc_a_gain = (uint16_t) val_int;
    } else if (key_match(name, DAC_QMC_GAIN_B)) {
      if (val_int > ((0b1 << 10) - 1)) {  // 2^10 - 1
        return -1;
      }
      settings->qmc_b_gain = (uint16_t) val_int;
    } else if (key_match(name, DAC_QMC_PHASE)) {
      if (val_int > ((0b1 << 9) - 1)) { // 2^9 - 1
        return -1;
      }
      settings->qmc_phase = (uint16_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return -1;
    }
    return 0;
}

int CFG_HandleMCP2210Config(void *user, const char *section, const char *name,
                            const char *value) {
    MCP2210Settings *config = (MCP2210Settings *) user;

    if (section_match(section, MCP_USB_SECTION)) {
        HandleMCPUSBBlock(config, name, value);
    } else if (section_match(section, MCP_SPI_SECTION)) {
        HandleMCPSPIBlock(config, name, value);
    } else if (section_match(section, MCP_CHIP_SECTION)) {
        HandleMCPChipBlock(config, name, value);
    } else {
        fprintf(stderr, "Encountered unknown section: %s.\n", section);
        return -1;
    }
    return 0;
}

int CFG_HandleDAC5687Config(void *user, const char *section, const char *name,
                            const char *value) {
    DAC5687Settings *config = (DAC5687Settings *) user;

    if (section_match(section, DAC_VERSION_SECTION)) {
        HandleDACVersionBlock(config, name, value);
    } else if (section_match(section, DAC_CONFIG_0_SECTION)) {
        HandleDACConfig0Block(config, name, value);
    } else if (section_match(section, DAC_CONFIG_1_SECTION)) {
        HandleDACConfig1Block(config, name, value);
    } else if (section_match(section, DAC_CONFIG_2_SECTION)) {
        HandleDACConfig2Block(config, name, value);
    } else if (section_match(section, DAC_CONFIG_3_SECTION)) {
        HandleDACConfig3Block(config, name, value);
    } else if (section_match(section, DAC_SYNC_CNTL_SECTION)) {
        HandleDACSyncCntlBlock(config, name, value);
    } else if (section_match(section, DAC_NCO_SECTION)) {
        HandleDACNCOBlock(config, name, value);
    } else if (section_match(section, DAC_DACA_SECTION)) {
        HandleDACDACABlock(config, name, value);
    } else if (section_match(section, DAC_DACB_SECTION)) {
        HandleDACDACBBlock(config, name, value);
    } else if (section_match(section, DAC_QMC_SECTION)) {
        HandleDACQMCBlock(config, name, value);
    } else {
        fprintf(stderr, "Encountered unknown block: %s.\n", section);
        return -1;
    }
    return 0;
}