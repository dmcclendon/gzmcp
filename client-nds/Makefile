#############################################################################
#############################################################################
#
# Makefile for gzmcpc4nds
#
# Copyright 2008-2009 Douglas McClendon -- dmc AT viros DOT org
#
#############################################################################
#############################################################################

# export all make variables by default 
# (until I discover a con at least)
export

# these are visible during download play (and possibly other bootloaders)
# note: convert -colors 8 -size 104x16 mybanner.bmp gzmcpc_banner.bmp
#BANNER_IMAGE = gzmcpc_banner_dmcsig.bmp 
BANNER_IMAGE = resources/bitmaps/gzmcpc_banner.bmp 
# note: convert -colors 8 -size 32x32 mylogo.jpg gzmcpc_logo.bmp
LOGO_IMAGE = resources/bitmaps/gzmcpc_logo.bmp 
LOGO_STRING = "Guitar-ZyX MCP v.2k9.dev" 


# define nds client sources
# (could be a glob, but nah)
NDS_ARM9_SOURCES = \
	cloader/arm9_loader.c \
	configfiles.c \
	debug.c \
	libdmc.c \
	graphics.c \
	input.c \
	libmcp.c \
	metrognome.c \
	modes.c \
	mode__intro__main.c \
	mode__intro__credits.c \
	mode__get_update.c \
	mode__main_menu.c \
	mode__ssid__input.c \
	mode__tpw__jam.c \
	network.c \
	rak.c \
	sound.c \
	time.c \
	main.c 

NDS_ARM7_SOURCES = \
	cloader/arm7_loader.c \
	arm7_main.c 

# where sound effects are found, all files here get linked
# into the program.
# TODO: load these at runtime from flash, with simplest/smallest
#       of defaults, to support download play.
SOUND_DIR := resources/sounds

# where splash images are found, all *.splash.*jpg files get
# linked into the program.
# TODO: load these at runtime from flash, with simplest/smallest
#       of defaults, to support download play.
BITMAPS_DIR := resources/bitmaps


#
# set a sane default for ZYXDEVSUITE, and handle errors well
#
ifeq ($(strip $(ZYXDEVSUITE)),)
ZYXDEVSUITE := /usr/lib/ZyXdevSuite/ZyXdev_arm
ifneq ($(wildcard $(ZYXDEVSUITE)),$(ZYXDEVSUITE))
$(error "The default ZYXDEVSUITE directory $(ZYXDEVSUITE) does not exist, probably you want to override this by setting the ZYXDEVSUITE variable in your environment to the location where you installed it.")
endif
endif

ifneq ($(wildcard $(ZYXDEVSUITE)),$(ZYXDEVSUITE))
$(error "ZYXDEVSUITE directory $(ZYXDEVSUITE) does not exist")
endif

ZYXDEVSUITEARM := $(ZYXDEVSUITE)

# add zyxdevsuitearm to path
PATH := $(ZYXDEVSUITEARM)/bin:$(PATH)

include $(ZYXDEVSUITEARM)/ds_rules

# all files in SOUND_DIR are audio files
AUDIO_FILES := \
	$(foreach file, \
		$(notdir $(wildcard $(SOUND_DIR)/*.*)), \
		$(CURDIR)/$(SOUND_DIR)/$(file))

# all *.splash.*jpg files in BITMAPS_DIR are splash image files
SPLASH_FILES := \
	$(foreach file, \
		$(notdir $(wildcard $(BITMAPS_DIR)/*.splash.*jpg)), \
		$(CURDIR)/$(BITMAPS_DIR)/$(file))
PNG_IMAGE_FILES := \
	$(foreach file, \
		$(notdir $(wildcard $(BITMAPS_DIR)/*.png)), \
		$(CURDIR)/$(BITMAPS_DIR)/$(file))
BMP_IMAGE_FILES := \
	$(foreach file, \
		$(notdir $(wildcard $(BITMAPS_DIR)/*font*.bmp)), \
		$(CURDIR)/$(BITMAPS_DIR)/$(file))

# TODO: obsolete with wfbl
SERVE_NIFI := zyx-serve-nifi

# define c compiler, etc...
NDS_ARM9_CC = arm-eabi-gcc 
NDS_ARM9_CXX = arm-eabi-gcc 
NDS_ARM9_OBJCOPY = arm-eabi-objcopy
NDS_ARM7_CC = arm-eabi-gcc 
NDS_ARM7_CXX = arm-eabi-gcc 
NDS_ARM7_OBJCOPY = arm-eabi-objcopy

# common includes and libdirs
INCLUDES :=
# todo: figure out why this instead of "../load_bin.h" in 
#       /cloader/arm9_loader.c causes problems
#INCLUDES += $(CURDIR)
LIBDIRS :=

# initialize common_cflags
COMMON_CFLAGS := 
COMMON_CFLAGS += -Wall

ifneq ($(strip $(FMODE)),)
	COMMON_CFLAGS += -DFMODE=$(FMODE)
endif

# use this to enable compilation of the 'pro' version
#COMMON_CFLAGS += -DIM_POOR_OR_LICENSED_OR_NOT
COMMON_CFLAGS += -DIM_POOR_OR_LICENSED_OR_NOT

# add common cflags
COMMON_CFLAGS += -O2

# perhaps some way exists to use gdb, maybe with desmume
#COMMON_CFLAGS += -g

# TODO: educational comments for all these
# nds arm9 flags
NDS_ARM9_CFLAGS := $(COMMON_CFLAGS)
NDS_ARM9_CFLAGS += -mcpu=arm9tdmi
NDS_ARM9_CFLAGS += -mtune=arm9tdmi
NDS_ARM9_CFLAGS += -DARM9
NDS_ARM9_CFLAGS += -fomit-frame-pointer
NDS_ARM9_CFLAGS += -ffast-math
NDS_ARM9_CFLAGS += -mthumb
NDS_ARM9_CFLAGS += -mthumb-interwork

# nds arm7 flags
NDS_ARM7_CFLAGS := $(COMMON_CFLAGS)
NDS_ARM7_CFLAGS += -mcpu=arm7tdmi
NDS_ARM7_CFLAGS += -mtune=arm7tdmi
NDS_ARM7_CFLAGS += -DARM7
NDS_ARM7_CFLAGS += -fomit-frame-pointer
NDS_ARM7_CFLAGS += -ffast-math
NDS_ARM7_CFLAGS += -mthumb
NDS_ARM7_CFLAGS += -mthumb-interwork


NDS_ARM9_INCLUDES := $(INCLUDES)
NDS_ARM7_INCLUDES := $(INCLUDES)
NDS_ARM9_LIBDIRS := $(LIBDIRS)
NDS_ARM7_LIBDIRS := $(LIBDIRS)

NDS_ARM9_INCLUDES += $(ZYXDEVSUITE)/include
NDS_ARM7_INCLUDES += $(ZYXDEVSUITE)/include

NDS_ARM9_LIBDIRS += $(ZYXDEVSUITE)/lib
NDS_ARM7_LIBDIRS += $(ZYXDEVSUITE)/lib

NDS_ARM9_INCLUDES += $(ZYXDEVSUITE)/libnds/include
NDS_ARM7_INCLUDES += $(ZYXDEVSUITE)/libnds/include

NDS_ARM9_LIBDIRS += $(ZYXDEVSUITE)/libnds/lib
NDS_ARM7_LIBDIRS += $(ZYXDEVSUITE)/libnds/lib

NDS_ARM9_INCLUDES += $(ZYXDEVSUITE)/arm-eabi/include
NDS_ARM7_INCLUDES += $(ZYXDEVSUITE)/arm-eabi/include

NDS_ARM9_INCLUDES += $(BITMAPS_DIR)

#NDS_ARM9_INCLUDES += /usr/include
#NDS_ARM7_INCLUDES += /usr/include
#NDS_ARM9_LIBDIRS += /usr/lib
#NDS_ARM7_LIBDIRS += /usr/lib

NDS_ARM9_LIBS :=
NDS_ARM7_LIBS :=
# maxmod sound library
NDS_ARM9_LIBS += mm9
NDS_ARM7_LIBS += mm7
# flash fat filesystem access library
NDS_ARM9_LIBS += fat
# wifi library
NDS_ARM9_LIBS += dswifi9
NDS_ARM7_LIBS += dswifi7
# main nds library
NDS_ARM9_LIBS += nds9
NDS_ARM7_LIBS += nds7

NDS_ARM9_DEPS = $(patsubst %.c,%.d,$(NDS_ARM9_SOURCES)) 
NDS_ARM7_DEPS = $(patsubst %.c,%.d,$(NDS_ARM7_SOURCES)) 

NDS_ARM9_OBJECTS = $(patsubst %.c,%.o,$(NDS_ARM9_SOURCES)) 
# XXX first pass is with .s, then try generating .bin's and loading from fat
NDS_ARM9_OBJECTS += $(patsubst %.jpg,%.o,$(SPLASH_FILES)) 
NDS_ARM9_OBJECTS += $(patsubst %.bmp,%.o,$(BMP_IMAGE_FILES)) 
NDS_ARM9_OBJECTS += $(patsubst %.png,%.o,$(PNG_IMAGE_FILES)) 
NDS_ARM9_OBJECTS += sounds.bin.o
NDS_ARM9_OBJECTS += load.bin.o

NDS_ARM7_OBJECTS = $(patsubst %.c,%.o,$(NDS_ARM7_SOURCES)) 

NDS_ARM9_INCFLAGS := $(foreach dir,$(NDS_ARM9_INCLUDES),-I$(dir)) 
NDS_ARM7_INCFLAGS := $(foreach dir,$(NDS_ARM7_INCLUDES),-I$(dir)) 

NDS_ARM9_LDFLAGS := $(foreach dir,$(NDS_ARM9_LIBDIRS),-L$(dir)) 
NDS_ARM7_LDFLAGS := $(foreach dir,$(NDS_ARM7_LIBDIRS),-L$(dir)) 

NDS_ARM9_LDFLAGS += $(foreach lib,$(NDS_ARM9_LIBS),-l$(lib)) 
NDS_ARM7_LDFLAGS += $(foreach lib,$(NDS_ARM7_LIBS),-l$(lib)) 

NDS_ARM9_CFLAGS += $(NDS_ARM9_INCFLAGS)
NDS_ARM7_CFLAGS += $(NDS_ARM7_INCFLAGS)

# arm9 gets to use the implicit rules, while arm7 uses static rules
CC = $(NDS_ARM9_CC)
OBJCOPY = $(NDS_ARM9_OBJCOPY)
CFLAGS = $(NDS_ARM9_CFLAGS)

#############################################################################
#############
# Variables #
#############

ALL_TARGETS = \
	gzmcp-client.nds \
	nroot


#############################################################################
#############################################################################
# Build Rules 

#############################################################################
# Build Rules :: pattern rules

# the generic rule is 
%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

# note: based on nds examples, this must(?) be near default as well, though
#       not mentioned as such in the gnu make manual where it was copied
#       from.  
# note: using -MM instead of -M to ignore system headers
# note: using -MG because we generate some header files (e.g. sounds.h)
%.d: %.c
	set -e; rm -f $@; \
	 $(CC) -MM -MG $(CPPFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

# image resources via grit
%.s %.h: %.bmp %.grit
	grit $< -fts -o$*.s
%.s %.h: %.jpg %.grit
	grit $< -fts -o$*.s
%.s %.h: %.png %.grit
	grit $< -fts -o$*.s
%.o: %.s
	$(CC) -x assembler-with-cpp -c -o $*.o $<

#############################################################################
# Build Rules :: special first is default

default:
	make all
	make nroot
	make tidy

all: $(ALL_TARGETS)

#############################################################################
# Build Rules :: ARM7 
$(NDS_ARM7_OBJECTS): %.o: %.c
	$(NDS_ARM7_CC) -c $(NDS_ARM7_CFLAGS) $(CPPFLAGS) $< -o $@

#############################################################################
# Build Rules :: ARM9/rest

cloader/arm9_loader.o: load_bin.h

# desmume testing
dmtest: gzmcp-client.nds
	desmume gzmcp-client.nds

# for development serving up via wmb
dev: 
	make all
	make tidy
	make serve

serve: gzmcp-client.nds
	sudo $(SERVE_NIFI) gzmcp-client_r4.nds

nroot: gzmcp-client.nds
	mkdir -p nroot/gzmcp
	mkdir -p nroot/boot
	cp gzmcp-client.nds nroot/gzmcp/
	cp resources/text/example.defaults nroot/gzmcp/defaults
	cp resources/text/example.midispec nroot/gzmcp/midispec
	cp resources/text/example.presets nroot/gzmcp/presets
	cp resources/text/example.zds-boot.cfg nroot/boot/zds-boot.cfg
	cp resources/text/example.zds-boot.cfg nroot/boot/g_zds-boot.cfg
	

# generate maxmod soundbank sources that can be referenced by our sources
# -d is to make it suitable for an nds project
sounds.bin sounds.h: $(AUDIO_FILES)
	mmutil $^ -d -osounds.bin -hsounds.h

sounds.bin.o sounds_bin.h: sounds.bin
	$(bin2o)

load.bin.o load_bin.h: load.bin
	# bin2o needs much improvement
	$(bin2o)

# todo: specify deps as a patsubst on all files under cl/bl
load.bin:
	make -C cloader/bootloader

.PHONY: tidy
tidy:
	@echo removing transient files
	rm -f *~
	rm -f *.o
	rm -f *.d
	rm -f *.elf
	rm -f nds-client.arm[79]
	rm -f .builderr 
	rm -f .map
	rm -f load_bin.h
	rm -f load.bin
	rm -f sounds.h
	rm -f sounds_bin.h
	rm -f sounds.bin
	rm -f cloader/*~ 
	rm -rf cloader/bootloader/build
	rm -f cloader/bootloader/*~ 
	rm -f cloader/bootloader/source/*~ 
	rm -f cloader/*.o
	rm -f cloader/*.d

.PHONY: clean
clean: tidy
	make -C cloader/bootloader clean 
	rm -rf nroot 
	rm -rf nroot 
	rm -f $(ALL_TARGETS) 
	rm -f $(NDS_ARM9_DEPS) 
	rm -f $(NDS_ARM7_DEPS) 
	rm -f $(NDS_ARM9_OBJECTS) 
	rm -f $(NDS_ARM7_OBJECTS) 
	rm -f $(BITMAPS_DIR)/*.[sh]


nds-client.arm7: $(NDS_ARM7_OBJECTS)

	$(NDS_ARM7_CC) \
		-specs=ds_arm7.specs \
		$(NDS_ARM7_CFLAGS) \
		$(CPPFLAGS) \
		-mno-fpu \
		-Wl,-Map,.map \
		$^ \
		$(NDS_ARM7_LDFLAGS) \
		-o $@.elf

	$(NDS_ARM7_OBJCOPY) \
		-O binary \
		$@.elf \
		$@

# TODO: split this into at least two targets
nds-client.arm9: $(NDS_ARM9_OBJECTS)

	$(CC) \
		-specs=ds_arm9.specs \
		$(CFLAGS) \
		$(CPPFLAGS) \
		-mno-fpu \
		-Wl,-Map,.map \
		$^ \
		$(NDS_ARM9_LDFLAGS) \
		-o $@.elf

	$(OBJCOPY) \
		-O binary \
		$@.elf \
		$@


gzmcp-client.nds: nds-client.arm9 nds-client.arm7
	
	@echo BUILD: running ndstool to combine arm7 and arm9 executables...
	ndstool	\
		-c $@ \
		-9 nds-client.arm9 \
		-7 nds-client.arm7 \
		-o $(BANNER_IMAGE) \
		-b $(LOGO_IMAGE) \
		$(LOGO_STRING) \
		>> ./.builderr 2>&1
	
	@echo
	@echo
	@echo BUILD: shiny new nds homebrew ready to rock-n-roll!!!
	@echo
	@echo


# include the automatically generated dependencies
# note: the - means it is ok if some of the dependency files do not exist
-include $(NDS_ARM9_DEPS)
-include $(NDS_ARM7_DEPS)

#
# END MAKEFILE
#############################################################################
