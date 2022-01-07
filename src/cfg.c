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

static int HandleMCPUSBBlock(MCP2210Settings *settings, const char *name,
                             const char *value) {
    char * end;
    uint64_t val_int = (uint64_t) strtol(value, &end, CFG_BASE);
    if (key_match(name, MCP_USB_POWER_OPTION)) {
      if (val_int > 0xA0) {
        return 0;
      }
      settings->usb.powerOption = (uint8_t) val_int;
    } else if (key_match(name, MCP_USB_REQUEST_CURRENT)) {
      if (val_int > ((0b1 << 8) - 1)) {   // 2^8 - 1
        return 0;
      }
      settings->usb.requestedCurrent = (uint8_t) val_int;
    } else if (key_match(name, MCP_USB_VID)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return 0;
      }
      settings->usb.vid = (uint16_t) val_int;
    } else if (key_match(name, MCP_USB_PID)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return 0;
      }
      settings->usb.pid = (uint16_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return 0;
    }
    return 1;
}

static int HandleMCPChipBlock(MCP2210Settings *settings, const char *name,
                               const char *value) {
    char * end;
    uint64_t val_int = (uint64_t) strtol(value, &end, CFG_BASE);

    if (key_contains(name, MCP_CHIP_GP_DES)) {
      uint8_t id = (uint8_t) strtol(name, &end, CFG_BASE);

      if (id > 8 || val_int > 2) {
        return 0;
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
        return 0;
      }
    } else if (key_contains(name, MCP_CHIP_GP_DIR)) {
      uint8_t id = (uint8_t) strtol(name, &end, CFG_BASE);
      if (id > 8) {
        return 0;
      }
      settings->chip.defaultGPIODirection |= (((uint8_t) val_int) << id);
    } else if (key_contains(name, MCP_CHIP_GP_OUT)) {
      uint8_t id = (uint8_t) strtol(name, &end, CFG_BASE);
      if (id > 8) {
        return 0;
      }
      settings->chip.defaultGPIOValue |= (((uint8_t) val_int) << id);
    } else if (key_match(name, MCP_CHIP_REMOTE_WAKEUP)) {
      if (val_int > 1) {
        return 0;
      }
      settings->chip.chipSettings |= (((uint8_t) val_int) << 4);
    } else if (key_match(name, MCP_CHIP_INT_MODE)) {
      if (val_int > 0b100) {
        return 0;
      }
      settings->chip.chipSettings |= (((uint8_t) val_int) << 1);
    } else if (key_match(name, MCP_CHIP_SPI_RELEASE)) {
      if (val_int > 1) {
        return 0;
      }
      settings->chip.chipSettings |= ((uint8_t) val_int);
    } else if (key_match(name, MCP_CHIP_ACC_CONTROL)) {
      if (!(val_int == 0x00 || val_int == 0x40 || val_int == 0x80)) {
        return 0;
      }
      settings->chip.chipAccessControl |= ((uint8_t) val_int);
    } else if (key_match(name, MCP_CHIP_PASS)) {
      memcpy(&settings->chip.pass, value, strlen(value));
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return 0;
    }
    return 1;
}

static bool HandleMCPSPIBlock(MCP2210Settings *settings, const char *name,
                               const char *value) {
    char * end;
    uint64_t val_int = (uint64_t) strtol(value, &end, CFG_BASE);
    if (key_match(name, MCP_SPI_BIT_RATE)) {
      if (val_int > ((0b1ULL << 32) - 1)) {  // 2^32 - 1
        return 0;
      }
      settings->spi.bitRate = (uint32_t) val_int;
    } else if (key_contains(name, MCP_SPI_CS_IDLE)) {
      uint8_t id = (uint8_t) strtol(name, &end, CFG_BASE);
      if (id > 8) {
        return 0;
      }
      settings->spi.idleCSValue |= (((uint8_t) val_int) << id);
    } else if (key_contains(name, MCP_SPI_CS_ACTIVE)) {
      uint8_t id = (uint8_t) strtol(name, &end, CFG_BASE);
      if (id > 8) {
        return 0;
      }
      settings->spi.activeCSValue |= (((uint8_t) val_int) << id);
    } else if (key_match(name, MCP_SPI_CS_DATA_DLY)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return 0;
      }
      settings->spi.csToDataDelay = (uint16_t) val_int;
    } else if (key_match(name, MCP_SPI_DATA_CS_DLY)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return 0;
      }
      settings->spi.lastDataToCSDelay = (uint16_t) val_int;
    } else if (key_match(name, MCP_SPI_DATA_DATA_DLY)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return 0;
      }
      settings->spi.dataToDataDelay = (uint16_t) val_int;
    } else if (key_match(name, MCP_SPI_BYTES)) {
      if (val_int > ((0b1 << 16) - 1)) {   // 2^16 - 1
        return 0;
      }
      settings->spi.bytesPerTransaction = (uint16_t) val_int;
    } else if (key_match(name, MCP_SPI_MODE)) {
      if (val_int > 3) {
        return 0;
      }
      settings->spi.lastDataToCSDelay = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return 0;
    }
    return 1;
}

static int HandleDACVersionBlock(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint8_t val_int = (uint8_t) strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_VERSION_SLEEP_A)) {
        if (val_int > 1) {
            return 0;
        }
        settings->version.sleep_dac_a = (uint8_t) val_int;
    } else if (key_match(name, DAC_VERSION_SLEEP_B)) {
        if (val_int > 1) {
            return 0;
        }
        settings->version.sleep_dac_b = (uint8_t) val_int;
    } else if (key_match(name, DAC_VERSION_HPLA)) {
        if (val_int > 1) {
            return 0;
        }
        settings->version.hpla = (uint8_t) val_int;
    } else if (key_match(name, DAC_VERSION_HPLB)) {
        if (val_int > 1) {
            return 0;
        }
        settings->version.hplb = (uint8_t) val_int;
    } else {
        fprintf(stderr, "Unknown setting: %s\n", name);
        return 0;
    }
  return 1;
}

static int HandleDACConfig0Block(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_CONFIG0_PLL_DIV)) {
        if (val_int > 0b11) {
        return 0;
        }
        settings->config_0.pll_vco_div = (PLLVCODiv) val_int;
    } else if (key_match(name, DAC_CONFIG0_PLL_FREQ)) {
        if (val_int > 1) {
        return 0;
        }
        settings->config_0.pll_freq = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG0_PLL_KV)) {
        if (val_int > 1) {
        return 0;
        }
        settings->config_0.pll_kv = (uint8_t) val_int;
    } else if (key_match(name,  DAC_CONFIG0_INTERP)) {
        if (val_int > 0b11) {
        return 0;
        }
        settings->config_0.fir_interp = (FIRInterp) val_int;
    } else if (key_match(name,  DAC_CONFIG0_INV_PLL_LOCK)) {
        if (val_int > 1) {
        return 0;
        }
        settings->config_0.inv_pll_lock = (uint8_t) val_int;
    } else if (key_match(name,  DAC_CONFIG0_FIFO_BYPASS)) {
        if (val_int > 1) {
        return 0;
        }
        settings->config_0.fifo_bypass = (uint8_t) val_int;
    } else {
        fprintf(stderr, "Unknown setting: %s\n", name);
        return 0;
    }
    return 1;
}

static int HandleDACConfig1Block(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_CONFIG1_QFLAG)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_1.qflag = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_INTERL)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_1.interl = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_DUAL_CLK)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_1.dual_clk = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_TWOS)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_1.twos = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_REV_ABUS)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_1.rev_abus = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_REV_BBUS)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_1.rev_bbus = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_FIR_BYPASS)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_1.fir_bypass = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG1_FULL_BYPASS)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_1.full_bypass = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return 0;
    }
    return 1;
}

static int HandleDACConfig2Block(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_CONFIG2_NCO)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_2.nco = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG2_NCO_GAIN)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_2.nco_gain = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG2_QMC)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_2.qmc = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG2_CM_MODE)) {
      if (val_int > 0b1111) {
        return 0;
      }
      settings->config_2.cm_mode = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG2_INV_SINC)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_2.invsinc = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return 0;
    }
    return 1;
}

static int HandleDACConfig3Block(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_CONFIG3_SIF_4PIN)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_3.sif_4_pin = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG3_DAC_SER_DATA)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_3.dac_ser_data = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG3_HALF_RATE)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_3.half_rate = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG3_USB)) {
      if (val_int > 1) {
        return 0;
      }
      settings->config_3.usb = (uint8_t) val_int;
    } else if (key_match(name, DAC_CONFIG3_COUNTER_MODE)) {
      if (val_int > 7) {
        return 0;
      }
      settings->config_3.counter_mode = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return 0;
    }
    return 1;
}

static int HandleDACSyncCntlBlock(DAC5687Settings *settings, const char *name,
                                  const char *value) {
    char * end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_SYNCCNTL_SYNC_PHSTR)) {
      if (val_int > 1) {
        return 0;
      }
      settings->sync_cntl.sync_phstr = (uint8_t) val_int;
    } else if (key_match(name, DAC_SYNCCNTL_SYNC_NCO)) {
      if (val_int > 1) {
        return 0;
      }
      settings->sync_cntl.sync_nco = (uint8_t) val_int;
    } else if (key_match(name, DAC_SYNCCNTL_SYNC_CM)) {
      if (val_int > 1) {
        return 0;
      }
      settings->sync_cntl.sync_cm = (uint8_t) val_int;
    } else if (key_match(name, DAC_SYNCCNTL_SYNC_FIFO)) {
      if (val_int > 7) {
        return 0;
      }
      settings->sync_cntl.sync_fifo = (uint8_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return 0;
    }
    return 1;
}

static int HandleDACNCOBlock(DAC5687Settings *settings, const char *name,
                              const char *value) {
    char *end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_NCO_FREQ)) {
      if (val_int > ((0b1ULL << 32) - 1)) { // 2^32 - 1
        return 0;
      }
      settings->nco_freq = (uint32_t) val_int;
    } else if (key_match(name, DAC_NCO_PHASE)) {
      if (val_int > ((0b1 << 16) - 1)) {  // 2^16 - 1
        return 0;
      }
      settings->nco_phase = (uint16_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return 0;
    }
    return 1;
}

static int HandleDACDACABlock(DAC5687Settings *settings, const char *name,
                               const char *value) {
    char *end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_DAC_OFFSET)) {
      if (val_int > ((0b1 << 12) - 1)) {  // 2^12 - 1
        return 0;
      }
      settings->dac_a_off = (uint16_t) val_int;
    } else if (key_match(name, DAC_DAC_GAIN)) {
      if (val_int > ((0b1 << 11) - 1)) {  // 2^11 - 1
        return 0;
      }
      settings->dac_a_gain = (uint16_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return 0;
    }
    return 1;
}

static bool HandleDACDACBBlock(DAC5687Settings *settings, const char *name,
                               const char *value) {
    char *end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_DAC_OFFSET)) {
      if (val_int > ((0b1 << 12) - 1)) {  // 2^12 - 1
        return 0;
      }
      settings->dac_b_off = (uint16_t) val_int;
    } else if (key_match(name, DAC_DAC_GAIN)) {
      if (val_int > ((0b1 << 11) - 1)) {  // 2^11 - 1
        return 0;
      }
      settings->dac_b_gain = (uint16_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", value);
      return 0;
    }
    return 1;
}

static bool HandleDACQMCBlock(DAC5687Settings *settings, const char *name,
                              const char *value) {
    char *end;
    uint64_t val_int = strtol(value, &end, CFG_BASE);
    if (key_match(name, DAC_QMC_GAIN_A)) {
      if (val_int > ((0b1 << 10) - 1)) {  // 2^10 - 1
        return 0;
      }
      settings->qmc_a_gain = (uint16_t) val_int;
    } else if (key_match(name, DAC_QMC_GAIN_B)) {
      if (val_int > ((0b1 << 10) - 1)) {  // 2^10 - 1
        return 0;
      }
      settings->qmc_b_gain = (uint16_t) val_int;
    } else if (key_match(name, DAC_QMC_PHASE)) {
      if (val_int > ((0b1 << 9) - 1)) { // 2^9 - 1
        return 0;
      }
      settings->qmc_phase = (uint16_t) val_int;
    } else {
      fprintf(stderr, "Unknown setting: %s\n", name);
      return 0;
    }
    return 1;
}

int CFG_HandleMCP2210Config(void *user, const char *section, const char *name,
                            const char *value) {
    MCP2210Settings *config = (MCP2210Settings *) user;
    if (section_match(section, MCP_USB_SECTION)) {
        return HandleMCPUSBBlock(config, name, value);
    } else if (section_match(section, MCP_SPI_SECTION)) {
        return HandleMCPSPIBlock(config, name, value);
    } else if (section_match(section, MCP_CHIP_SECTION)) {
        return HandleMCPChipBlock(config, name, value);
    } else {
        fprintf(stderr, "Encountered unknown section: %s.\n", section);
        return -1;
    }
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
        return 0;
    }
    return 1;
}

void CFG_PrintMCP2210Config(MCP2210Settings settings) {
    printf("--------MCP2210 Configuration--------\n");

    printf("\t----USB----\n");
    printf("\t\tPower Option: %d\n", settings.usb.powerOption);
    printf("\t\tRequested Current: %d\n", settings.usb.requestedCurrent);
    printf("\t\tVID: 0x%04x\n", settings.usb.vid);
    printf("\t\tPID: 0x%04x\n\n", settings.usb.pid);

    printf("\t----CHIP----\n");
    printf("\t\tGPIO 0 Designation: 0x%02x\n", settings.chip.gp0Designation);
    printf("\t\tGPIO 1 Designation: 0x%02x\n", settings.chip.gp1Designation);
    printf("\t\tGPIO 2 Designation: 0x%02x\n", settings.chip.gp2Designation);
    printf("\t\tGPIO 3 Designation: 0x%02x\n", settings.chip.gp3Designation);
    printf("\t\tGPIO 4 Designation: 0x%02x\n", settings.chip.gp4Designation);
    printf("\t\tGPIO 5 Designation: 0x%02x\n", settings.chip.gp5Designation);
    printf("\t\tGPIO 6 Designation: 0x%02x\n", settings.chip.gp6Designation);
    printf("\t\tGPIO 7 Designation: 0x%02x\n", settings.chip.gp7Designation);
    printf("\t\tGPIO 8 Designation: 0x%02x\n", settings.chip.gp8Designation);
    printf("\t\tGPIO 0 Direction: 0x%01x\n", settings.chip.defaultGPIODirection & GPIO0);
    printf("\t\tGPIO 1 Direction: 0x%01x\n", settings.chip.defaultGPIODirection & GPIO1);
    printf("\t\tGPIO 2 Direction: 0x%01x\n", settings.chip.defaultGPIODirection & GPIO2);
    printf("\t\tGPIO 3 Direction: 0x%01x\n", settings.chip.defaultGPIODirection & GPIO3);
    printf("\t\tGPIO 4 Direction: 0x%01x\n", settings.chip.defaultGPIODirection & GPIO4);
    printf("\t\tGPIO 5 Direction: 0x%01x\n", settings.chip.defaultGPIODirection & GPIO5);
    printf("\t\tGPIO 6 Direction: 0x%01x\n", settings.chip.defaultGPIODirection & GPIO6);
    printf("\t\tGPIO 7 Direction: 0x%01x\n", settings.chip.defaultGPIODirection & GPIO7);
    printf("\t\tGPIO 8 Direction: 0x%01x\n", settings.chip.defaultGPIODirection & GPIO8);
    printf("\t\tGPIO 0 Value: 0x%01x\n", settings.chip.defaultGPIOValue & GPIO0);
    printf("\t\tGPIO 1 Value: 0x%01x\n", settings.chip.defaultGPIOValue & GPIO1);
    printf("\t\tGPIO 2 Value: 0x%01x\n", settings.chip.defaultGPIOValue & GPIO2);
    printf("\t\tGPIO 3 Value: 0x%01x\n", settings.chip.defaultGPIOValue & GPIO3);
    printf("\t\tGPIO 4 Value: 0x%01x\n", settings.chip.defaultGPIOValue & GPIO4);
    printf("\t\tGPIO 5 Value: 0x%01x\n", settings.chip.defaultGPIOValue & GPIO5);
    printf("\t\tGPIO 6 Value: 0x%01x\n", settings.chip.defaultGPIOValue & GPIO6);
    printf("\t\tGPIO 7 Value: 0x%01x\n", settings.chip.defaultGPIOValue & GPIO7);
    printf("\t\tGPIO 8 Value: 0x%01x\n", settings.chip.defaultGPIOValue & GPIO8);
    printf("\t\tChip Settings: 0x%02x\n", settings.chip.chipSettings);
    printf("\t\tAccess Control Enabled: %d\n", settings.chip.chipAccessControl);
    printf("\t\tChip Password: %c%c%c%c%c%c%c%c\n\n", settings.chip.pass.char0,
                                                settings.chip.pass.char1,
                                                settings.chip.pass.char2,
                                                settings.chip.pass.char3,
                                                settings.chip.pass.char4,
                                                settings.chip.pass.char5,
                                                settings.chip.pass.char6,
                                                settings.chip.pass.char7);

    printf("\t----SPI----\n");
    printf("\t\tBit Rate: %d\n bps", settings.spi.bitRate);
    printf("\t\tIdle CS Values: 0x%02x\n", settings.spi.idleCSValue);
    printf("\t\tActive CS Values: 0x%02x\n", settings.spi.activeCSValue);
    printf("\t\tCS-to-Data Delay: %d us\n", settings.spi.csToDataDelay);
    printf("\t\tData-to-CS Delay: %d us\n", settings.spi.lastDataToCSDelay);
    printf("\t\tData-to-Data Rate: %d us\n", settings.spi.dataToDataDelay);
    printf("\t\tBytes per Transaction: %d bytes\n\n", settings.spi.bytesPerTransaction);
}

void CFG_PrintDAC5687Config(DAC5687Settings settings) {
    printf("--------DAC5687 Configuration--------\n");

    printf("\t----VERSION----\n");
    printf("\t\tDACA Sleep: 0x%01x\n", settings.version.sleep_dac_a);
    printf("\t\tDACB Sleep: 0x%01x\n", settings.version.sleep_dac_b);
    printf("\t\tHPLA Sleep: 0x%01x\n", settings.version.hpla);
    printf("\t\tHPLB Sleep: 0x%01x\n", settings.version.hplb);

    printf("\t----CONFIG0----\n");
    printf("\t\tPLL Divider: %d\n", settings.config_0.pll_vco_div);
    printf("\t\tPLL Frequency: 0x%01x\n", settings.config_0.pll_freq);
    printf("\t\tPLL KV: %d\n", settings.config_0.pll_kv);
    printf("\t\tFIR Interpolation: %d\n", settings.config_0.fir_interp);
    printf("\t\tInv PLL Lock: %d\n", settings.config_0.inv_pll_lock);
    printf("\t\tFIFO Bypass: %d\n\n", settings.config_0.fifo_bypass);

    printf("\t----CONFIG1----\n");
    printf("\t\tQ Flag: %d\n", settings.config_1.qflag);
    printf("\t\tInterl: %d\n", settings.config_1.interl);
    printf("\t\tDual Clock: %d\n", settings.config_1.dual_clk);
    printf("\t\tTwos: %d\n", settings.config_1.twos);
    printf("\t\tRev A Bus: %d\n", settings.config_1.rev_abus);
    printf("\t\tRev B Bus: %d\n", settings.config_1.rev_bbus);
    printf("\t\tFIR Bypass: %d\n", settings.config_1.fir_bypass);
    printf("\t\tFull Bypass: %d\n\n", settings.config_1.full_bypass);
    
    printf("\t----CONFIG2----\n");
    printf("\t\tNCO: %d\n", settings.config_2.nco);
    printf("\t\tNCO Gain: %d\n", settings.config_2.nco_gain);
    printf("\t\tQMC: %d\n", settings.config_2.qmc);
    printf("\t\tCM Mode: %d\n", settings.config_2.cm_mode);
    printf("\t\tInvsinc: %d\n\n", settings.config_2.invsinc);

    printf("\t----CONFIG3----\n");
    printf("\t\tSerial Interface 4-pin: %d\n", settings.config_3.sif_4_pin);
    printf("\t\tDAC Serial Data: %d\n", settings.config_3.dac_ser_data);
    printf("\t\tHalf Rate: %d\n", settings.config_3.half_rate);
    printf("\t\tUSB: %d\n", settings.config_3.usb);
    printf("\t\tCounter Mode: %d\n\n", settings.config_3.counter_mode);

    printf("\t----SYNCCNTL----\n");
    printf("\t\tSync Phstr: 0x%01x\n", settings.sync_cntl.sync_phstr);
    printf("\t\tSync NCO: 0x%01x\n", settings.sync_cntl.sync_nco);
    printf("\t\tSync CM: 0x%01x\n", settings.sync_cntl.sync_cm);
    printf("\t\tSync FIFO: 0x%01x\n", settings.sync_cntl.sync_fifo);

    printf("\t----NCO----\n");
    printf("\t\tFrequency: %d\n", settings.nco_freq);
    printf("\t\tPhase: %d\n", settings.nco_phase);

    printf("\t----DACA----\n");
    printf("\t\tOffset: %d\n", settings.dac_a_off);
    printf("\t\tGain: %d\n", settings.dac_a_gain);

    printf("\t----DACB----\n");
    printf("\t\tOffset: %d\n", settings.dac_b_off);
    printf("\t\tGain: %d\n", settings.dac_b_gain);

    printf("\t----QMC----\n");
    printf("\t\tA Gain: %d\n", settings.qmc_a_gain);
    printf("\t\tB Gain: %d\n", settings.qmc_b_gain);
    printf("\t\tPhase: %d\n", settings.qmc_phase);
}

void CFG_PrintConfig(Settings settings) {
    CFG_PrintMCP2210Config(settings.mcp);
    CFG_PrintDAC5687Config(settings.dac);
}