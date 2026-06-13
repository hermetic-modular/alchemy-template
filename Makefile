# =============================================================================
# alchemy-template — Daisy-bootloader firmware for Hermetic Modular Alchemy Lab
#
# Standard Daisy workflow (libDaisy core Makefile underneath):
#   make libdaisy       — build lib/libDaisy once after cloning
#   make                — build firmware (BOARD=v2 by default)
#   make program-dfu    — flash over USB (module in DFU mode first; see README)
#   make clean          — remove the build tree
# =============================================================================

TARGET = stereo_eq

# Alchemy Lab board revision: v1 | v2
BOARD ?= v2
ifeq ($(filter $(BOARD),v1 v2),)
$(error BOARD must be 'v1' or 'v2' (got '$(BOARD)'))
endif

ALCHEMY_DIR  = lib/alchemy-sdk
LIBDAISY_DIR = lib/libDaisy

# ── App sources — yours to edit ─────────────────────────────────────────────
CPP_SOURCES = \
    src/stereo_eq.cpp \
    src/stereo_eq_dsp.cpp

# ── Alchemy SDK, compiled straight from the submodule ───────────────────────
CPP_SOURCES += $(sort $(shell find $(ALCHEMY_DIR)/framework/src -name '*.cpp'))
CPP_SOURCES += $(sort $(wildcard $(ALCHEMY_DIR)/hardware/alchemy-lab/$(BOARD)/src/*.cpp))

C_INCLUDES += \
    -Isrc \
    -I$(ALCHEMY_DIR)/framework/include \
    -I$(ALCHEMY_DIR)/hardware/include \
    -I$(ALCHEMY_DIR)/hardware/alchemy-lab/$(BOARD)/include

ifeq ($(BOARD),v2)
C_DEFS += -DALCHEMY_BOARD_V2
endif

# ── Daisy bootloader build (BOOT_SRAM) ──────────────────────────────────────
APP_TYPE = BOOT_SRAM
LDSCRIPT = $(ALCHEMY_DIR)/cmake/linkers/alchemy_stm32h750ib_sram.lds

# The Alchemy SDK requires C++17 (libDaisy's default is gnu++14).
CPP_STANDARD = -std=gnu++17

# ── libDaisy core Makefile does the rest ────────────────────────────────────
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

BOARD_STAMP := $(BUILD_DIR)/.board-$(BOARD)
ifeq ($(wildcard $(BOARD_STAMP)),)
_BOARD_GUARD := $(shell rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d $(BUILD_DIR)/*.lst $(BUILD_DIR)/.board-* 2>/dev/null; mkdir -p $(BUILD_DIR); touch $(BOARD_STAMP))
endif

.PHONY: libdaisy
libdaisy:
	$(MAKE) -C $(LIBDAISY_DIR)
