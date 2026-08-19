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
#   make clean
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC. export DEVKITPPC=<path to>devkitPPC")
endif
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO. export DEVKITPRO=<path to>devkitPro")
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

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "archiving ... $@"
	@$(AR) rcs $@ $(OBJS)
	@echo "built $@ ($(words $(OBJS)) objects)"

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "$<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET)
