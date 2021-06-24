##
##  Copyright (c) 2021, Texas Instruments Incorporated
##
##  Redistribution and use in source and binary forms, with or without
##  modification, are permitted provided that the following conditions
##  are met:
##
##  *  Redistributions of source code must retain the above copyright
##     notice, this list of conditions and the following disclaimer.
##
##  *  Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in the
##     documentation and/or other materials provided with the distribution.
##
##  *  Neither the name of Texas Instruments Incorporated nor the names of
##     its contributors may be used to endorse or promote products derived
##     from this software without specific prior written permission.
##
##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
##  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
##  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
##  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
##  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
##  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
##  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
##  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
##  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
##  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
##  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##

PLATFORM  = j721e
PROJECT   = dynload_$(PLATFORM)
DEVICE    = DEVICE_J721E
SOC       = SOC_J721E
LIB_NAME  = dynload_c7x.lib

## !Need -O* to inline CACHE_* functions!
CC= cl7x -O3 -mv7100 --abi=eabi --gcc
AR= ar7x
STRIP= strip7x
MKDIR_P = mkdir -p
RMDIR = rm -fr

INCLUDES  = -I. -I DLOAD -I DLOAD_API -I C70_DLOAD_DYN -I C70_DLOAD_REL
INCLUDES += -D$(DEVICE) -D$(SOC) -D_SYS_BIOS -DPSDK_RTOS_APP
INCLUDES += -DC70_TARGET -DELF64=1

SOURCES = dsp_load.c \
          DLOAD/ArrayList.c \
          DLOAD/dload.c \
          DLOAD/dload_endian.c \
          DLOAD/elf32.c \
          DLOAD_SYM/symtab.c \
          C70_DLOAD_DYN/c70_dynamic.c \
          C70_DLOAD_REL/c70_reloc.c \

HEADERS := $(shell find . -name "*.h")

OBJ_DIR = build
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.obj,$(SOURCES))

all: $(LIB_NAME)

$(LIB_NAME): $(OBJS)
	$(AR) -r $@ $^

$(OBJ_DIR)/%.obj: %.c $(HEADERS)
	$(MKDIR_P) $(dir $@)
	$(CC) $(INCLUDES) -c $< -fr=$(OBJ_DIR)/$(dir $<)

clean:
	$(RMDIR) $(OBJ_DIR)

