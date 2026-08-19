#---------------------------------------------------------------------------------
# magmanolia — Wii game engine library by MagmaCrunch
#---------------------------------------------------------------------------------
# Include this engine in your game by adding its path to SOURCES in your Makefile:
#   SOURCES := source ../magmanolia/source
#   INCLUDES := ../magmanolia
#---------------------------------------------------------------------------------

.PHONY: clean

clean:
	@echo clean ...
	@rm -fr build
