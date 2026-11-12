# =============================================================================
# Param-lock demo firmwares : Hermetic Modular Alchemy Lab (V2 only)
#
# Five builds of one visual-only firmware, each showing a different ParamLock
# threshold / LED-style configuration.  No DSP.  The clock comes in on J2.
#
#   make                            : build every variant → build/lock_demo_<v>.bin
#   make DEMO=sweep                 : build one variant
#   make DEMO=sweep program-live    : reboot the running module over USB and flash it
#   make DEMO=sweep program-dfu     : flash a module already in DFU mode
#   make clean                      : remove the build tree
#
# Variants (DEMO=): original | new_default | breathe | loop_pulse | sweep
# (the bin name works too: DEMO=lock_demo_breathe)
#
# DEMO= is REQUIRED to build or flash a single variant; there is no default,
# so a bare `make program-live` refuses rather than flashing the wrong demo.
# From the template root the same commands work as
#   make -C demos/param_lock DEMO=sweep program-live
#
# At boot each demo lights rings P1..PN white for one second (N = demo
# number, 1 = original through 5 = sweep) so you can see which one is loaded.
#
# Build lib/libDaisy first (`make libdaisy` from the template root).
# =============================================================================

VARIANTS := original new_default breathe loop_pulse sweep

ifeq ($(origin DEMO),undefined)
  # No variant chosen: only `make` (all five) and `make clean` make sense.
  ifneq ($(filter all program-live program-dfu,$(MAKECMDGOALS)),)
    $(error choose a variant:  make DEMO=<$(subst $(subst ,, ),|,$(VARIANTS))> $(MAKECMDGOALS))
  endif
  DEMO := new_default        # placeholder so the core Makefile parses
  DEFAULT_GOAL := demos
else
  # Accept `breathe`, `lock_demo_breathe`, or `lock_demo_breathe.bin`.
  override DEMO := $(patsubst lock_demo_%,%,$(patsubst %.bin,%,$(strip $(DEMO))))
  ifneq ($(words $(DEMO)),1)
    $(error choose exactly one DEMO variant)
  endif
  ifeq ($(filter $(DEMO),$(VARIANTS)),)
    $(error DEMO must be one of: $(VARIANTS) (got '$(DEMO)'))
  endif
  DEFAULT_GOAL := all
endif

TARGET = lock_demo_$(DEMO)

ROOT         = ../..
ALCHEMY_DIR  = $(ROOT)/lib/alchemy-sdk
LIBDAISY_DIR = $(ROOT)/lib/libDaisy

# ── Sources: the shared firmware plus this variant's one-file config ────────
CPP_SOURCES = lock_demo.cpp demo_$(DEMO).cpp

# ── Alchemy SDK, compiled straight from the submodule (V2 board) ───────────
CPP_SOURCES += $(sort $(shell find $(ALCHEMY_DIR)/framework/src -name '*.cpp'))
CPP_SOURCES += $(sort $(wildcard $(ALCHEMY_DIR)/hardware/alchemy-lab/v2/src/*.cpp))

C_INCLUDES += \
    -I. \
    -I$(ALCHEMY_DIR)/framework/include \
    -I$(ALCHEMY_DIR)/hardware/include \
    -I$(ALCHEMY_DIR)/hardware/alchemy-lab/v2/include

C_DEFS += -DALCHEMY_BOARD_V2

# ── Daisy bootloader build (BOOT_SRAM) ──────────────────────────────────────
APP_TYPE = BOOT_SRAM
LDSCRIPT = $(ALCHEMY_DIR)/cmake/linkers/alchemy_stm32h750ib_sram.lds

# The Alchemy SDK requires C++17 (libDaisy's default is gnu++14).
CPP_STANDARD = -std=gnu++17

# ── libDaisy core Makefile does the rest ────────────────────────────────────
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

.DEFAULT_GOAL := $(DEFAULT_GOAL)

.PHONY: demos
demos:
	@for v in $(VARIANTS); do \
	    $(MAKE) --no-print-directory DEMO=$$v all || exit 1; \
	done
	@echo && ls -l $(BUILD_DIR)/lock_demo_*.bin

# ── Flash without touching the module (same recipe as the template) ────────
USBPID ?= df11

.PHONY: program-live
program-live: all
	@echo ">>> flashing $(TARGET_BIN)  (demo $(DEMO))"
	node $(ALCHEMY_DIR)/tools/hostlink-cli/hostlink.mjs reboot bootloader
	dfu-util -w -a 0 -s $(FLASH_ADDRESS):leave -D $(BUILD_DIR)/$(TARGET_BIN) -d ,0483:$(USBPID)
