#---------------------------------------------------------------------------------
# magnolia - Wii game engine library by MagmaCrunch
#---------------------------------------------------------------------------------
# Games consume the sources directly; add this engine to your own Makefile with:
#   SOURCES  := source ../magnolia/source ../magnolia/font
#   INCLUDES := ../magnolia ../magnolia/source ../magnolia/font source
#
# This Makefile builds the engine STANDALONE into libmagnolia.a. Its only purpose
# is to prove the engine compiles with no game on the include path -- if engine
# code ever reaches for a game header again, this build fails immediately rather
# than the coupling being discovered by the next game months later.
#
#   make          build libmagnolia.a
#   make test     run the host-side tests (no devkitPPC, no console)
#   make clean
#---------------------------------------------------------------------------------

# The host tests deliberately need no cross-compiler: requiring devkitPPC to run
# them would put them out of reach on the machine where they are most useful.
ifeq ($(filter test,$(MAKECMDGOALS)),)
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC. export DEVKITPPC=<path to>devkitPPC")
endif
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO. export DEVKITPRO=<path to>devkitPro")
endif
endif

PREFIX  := $(DEVKITPPC)/bin/powerpc-eabi-
CC      := $(PREFIX)gcc
AR      := $(PREFIX)ar

TARGET  := libmagnolia.a
BUILD   := build

MACHDEP := -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float

# Deliberately engine-only: source/, font/ and the toolchain. No game directory.
INCLUDE := -I$(CURDIR) -I$(CURDIR)/source -I$(CURDIR)/font \
           -I$(DEVKITPRO)/libogc/include \
           -I$(DEVKITPRO)/portlibs/wii/include \
           -I$(DEVKITPRO)/portlibs/ppc/include \
           -I$(DEVKITPRO)/portlibs/ppc/include/freetype2

CFLAGS  := -g -O2 -Wall -Wextra $(MACHDEP) $(INCLUDE)

SOURCES := $(wildcard source/*.c) $(wildcard font/*.c)
OBJS    := $(patsubst %.c,$(BUILD)/%.o,$(SOURCES))

.PHONY: all clean test

all: $(TARGET)

# The storage modules are plain C with no libogc in them, so they can be tested
# on the machine you are sitting at, in a second, with no emulator involved. That
# matters more than it sounds: an emulated SD card can refuse every write while
# reporting itself mounted, and a test that runs on the host is the one that can
# tell a broken save from a broken card.
HOSTCC ?= cc
test:
	@$(HOSTCC) -Wall -Wextra -I source -o $(BUILD)/test_storage \
	    tests/test_storage.c source/prefs.c source/scoring.c
	@$(BUILD)/test_storage

$(TARGET): $(OBJS)
	@echo "archiving ... $@"
	@$(AR) rcs $@ $(OBJS)
	@echo "built $@ ($(words $(OBJS)) objects)"

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "$<"
	@$(CC) $(CFLAGS) -c $< -o $@

test: | $(BUILD)

$(BUILD):
	@mkdir -p $(BUILD)

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET)
