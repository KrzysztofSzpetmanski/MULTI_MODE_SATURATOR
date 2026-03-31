RACK_DIR ?= ../Rack-SDK
ifeq ("$(wildcard $(RACK_DIR)/plugin.mk)","")
RACK_DIR := /Users/lazuli/Documents/PROGRAMMING/TEENSY/KSZ_TEENSY_PLATFORMIO/Teensy_Chord_Gen/Rack-SDK
endif

PLUGIN_SLUG := MultiModeFilterLab
LOCAL_RACK_PLUGIN_DIR ?= $(HOME)/Library/Application Support/Rack2/plugins-mac-arm64
BIG_MAC_MOUNT_DIR ?= /Volumes/music
BIG_MAC_RACK_SUBDIR ?= Library/Application Support/Rack2/plugins-mac-arm64

LOCAL_DEPLOY_DIR := $(LOCAL_RACK_PLUGIN_DIR)/$(PLUGIN_SLUG)
BIG_MAC_DEPLOY_DIR := $(BIG_MAC_MOUNT_DIR)/$(BIG_MAC_RACK_SUBDIR)/$(PLUGIN_SLUG)

BUILD_BUMP_SCRIPT := scripts/bump_build.sh
NO_BUMP_GOALS := clean cleandist
SHOULD_BUMP := 0
ifeq ($(strip $(MAKECMDGOALS)),)
SHOULD_BUMP := 1
else ifneq ($(strip $(filter-out $(NO_BUMP_GOALS),$(MAKECMDGOALS))),)
SHOULD_BUMP := 1
endif

ifeq ($(SHOULD_BUMP),1)
BUMP_RESULT := $(shell $(BUILD_BUMP_SCRIPT))
ifneq ($(strip $(BUMP_RESULT)),)
$(info $(BUMP_RESULT))
endif
endif

FLAGS += -std=c++17

SOURCES += src/plugin.cpp
SOURCES += src/platform/vcv/DualFilterModule.cpp
SOURCES += src/platform/vcv/DualFilterWidget.cpp
SOURCES += src/dsp/engine/FilterSlot.cpp
SOURCES += src/dsp/engine/FilterRouter.cpp
SOURCES += src/dsp/engine/DualFilterEngine.cpp
SOURCES += src/dsp/filters/SVF.cpp
SOURCES += src/dsp/filters/TransistorLadder.cpp
SOURCES += src/dsp/filters/CombFilter.cpp
SOURCES += src/dsp/filters/BiquadFilter.cpp
SOURCES += src/dsp/filters/WaspFilter.cpp
SOURCES += src/dsp/filters/PhaserFilter.cpp
SOURCES += src/dsp/filters/DiodeLadder.cpp
SOURCES += src/dsp/filters/LPG.cpp

DISTRIBUTABLES += $(wildcard LICENSE*) res README.md SETUP.md BUILD_AND_RUN.md ARCHITECTURE.md KNOWN_ISSUES.md TEST_CHECKLIST.md NEW_CHAT_PROMPT.md docs

include $(RACK_DIR)/plugin.mk

.PHONY: deploy-local deploy-big deploy-both

deploy-local: dist
	rsync -av --delete dist/$(PLUGIN_SLUG)/ "$(LOCAL_DEPLOY_DIR)/"

deploy-big: dist
	rsync -av --delete dist/$(PLUGIN_SLUG)/ "$(BIG_MAC_DEPLOY_DIR)/"

deploy-both: dist
	rsync -av --delete dist/$(PLUGIN_SLUG)/ "$(LOCAL_DEPLOY_DIR)/"
	rsync -av --delete dist/$(PLUGIN_SLUG)/ "$(BIG_MAC_DEPLOY_DIR)/"
