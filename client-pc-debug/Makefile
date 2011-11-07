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
# protocol definition shared with client
LIBDIRS :=

# initialize common_cflags
COMMON_CFLAGS := -Wall

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

all: gzmcp-client-debug.x86 

gzmcp-client-debug.x86: client.c client.h gzmcp_protocol.h
	@echo BUILD: compiling Guitar-ZyX Master Control Program client code...
	$(CC) $(CFLAGS) $(CPPFLAGS) -lasound -o gzmcp-client-debug.x86 client.c

tidy:
	@echo removing transient files
	rm -f *~ *.o 

.PHONY: clean
clean: tidy
	rm -f gzmcp-client-debug.x86 

