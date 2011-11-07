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

####################
# Global Variables #
####################

# define c compiler, etc...
CC = gcc
CXX = g++
OBJCOPY = objcopy


# common includes and libdirs
INCLUDES := 
LIBDIRS :=

# initialize common_cflags
COMMON_CFLAGS := $(USER_CFLAGS)
COMMON_CFLAGS += -Wall

# add common cflags
COMMON_CFLAGS += -g
COMMON_CFLAGS += -O2

INCFLAGS := $(foreach dir,$(INCLUDES),-I$(dir)) 

CFLAGS := $(COMMON_CFLAGS)
CFLAGS += $(INCFLAGS)

###############
# Build Rules #
###############

# Generic Rules
# (note: unchanged from default - for override reference)
%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@


default: all

all: gzmcp-server.x86 

gzmcp-server.x86: server.c server.h gzmcp_protocol.h
	@echo BUILD: compiling Guitar-ZyX Master Control Program server code...
	$(CC) $(CFLAGS) $(CPPFLAGS) -lasound -o gzmcp-server.x86 server.c

tidy:
	@echo removing transient files
	rm -f *~ *.o 

.PHONY: clean
clean: tidy
	rm -f gzmcp-server.x86 

