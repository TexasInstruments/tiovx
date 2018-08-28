# Copyright (C) 2013 Texas Instruments
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifeq ($(TARGET_CPU),A72)
	CROSS_COMPILE:=aarch64-elf-
endif

$(if $(GCC_SYSBIOS_ARM_ROOT),,$(error GCC_SYSBIOS_ARM_ROOT must be defined!))

# check for the support OS types for this compiler
ifeq ($(filter $(TARGET_OS),SYSBIOS),)
$(error TARGET_OS $(TARGET_OS) is not supported by this compiler)
endif

ifneq ($(GCC_SYSBIOS_ARM_ROOT),)
CC = $(GCC_SYSBIOS_ARM_ROOT)/bin/$(CROSS_COMPILE)gcc
CP = $(GCC_SYSBIOS_ARM_ROOT)/bin/$(CROSS_COMPILE)g++
AS = $(GCC_SYSBIOS_ARM_ROOT)/bin/$(CROSS_COMPILE)as
AR = $(GCC_SYSBIOS_ARM_ROOT)/bin/$(CROSS_COMPILE)ar
LD = $(GCC_SYSBIOS_ARM_ROOT)/bin/$(CROSS_COMPILE)gcc
else
$(error GCC_SYSBIOS_ARM_ROOT is not defined)
endif

ifdef LOGFILE
LOGGING:=&>$(LOGFILE)
else
LOGGING:=
endif

ifeq ($(strip $($(_MODULE)_TYPE)),library)
	BIN_PRE=lib
	BIN_EXT=.a
else
	BIN_PRE=
	BIN_EXT=.out
endif

ifneq ($(CPU_ID),)
$(_MODULE)_OUT  := $(BIN_PRE)$($(_MODULE)_TARGET)_$(CPU_ID)$(BIN_EXT)
else
$(_MODULE)_OUT  := $(BIN_PRE)$($(_MODULE)_TARGET)$(BIN_EXT)
endif
$(_MODULE)_BIN  := $($(_MODULE)_TDIR)/$($(_MODULE)_OUT)
$(_MODULE)_OBJS := $(ASSEMBLY:%.S=$($(_MODULE)_ODIR)/%.o) $(CPPSOURCES:%.cpp=$($(_MODULE)_ODIR)/%.o) $(CSOURCES:%.c=$($(_MODULE)_ODIR)/%.o)
$(_MODULE)_SHARED_LIBS := $(foreach lib,$(SHARED_LIBS),$($(_MODULE)_TDIR)/lib$(lib)$(DSO_EXT))
ifeq ($(BUILD_MULTI_PROJECT),1)
$(_MODULE)_STATIC_LIBS += $(foreach lib,$(SYS_STATIC_LIBS),$($(_MODULE)_TDIR)/lib$(lib).a)
endif

$(_MODULE)_DEP_HEADERS := $(foreach inc,$($(_MODULE)_HEADERS),$($(_MODULE)_SDIR)/$(inc).h)

$(_MODULE)_COPT += -Wall -fms-extensions -Wno-write-strings -Wno-format-security
$(_MODULE)_COPT += -Dxdc_target_types__=gnu/targets/arm/std.h -Dxdc_target_name__=A53F -DCGT_GCC -mcpu=cortex-a53 -g -ffunction-sections -fdata-sections
$(_MODULE)_COPT += -Wno-unknown-pragmas -Wno-missing-braces -Wno-format -Wno-unused-variable

ifeq ($(TARGET_BUILD),debug)
$(_MODULE)_COPT += -ggdb -ggdb3 -gdwarf-2 -DDEBUG
else ifneq ($(filter $(TARGET_BUILD),release production),)
$(_MODULE)_COPT += -O3 -DNDEBUG
endif

ifeq ($(TARGET_BUILD),production)
# Remove all symbols.
$(_MODULE)_LOPT += -s
endif

$(_MODULE)_COPT += -mlittle-endian


ifneq ($(filter $(TARGET_CPU),A53 A72 A53F A72F),)
$(_MODULE)_COPT += -mcpu=cortex-a53
else ifneq ($(filter $(TARGET_CPU),A15 A15F),)
$(_MODULE)_COPT += -mcpu=cortex-a15
endif

ifeq ($(BUILD_IGNORE_LIB_ORDER),yes)
LINK_START_GROUP=-Wl,--start-group
LINK_END_GROUP=-Wl,--end-group
else
LINK_START_GROUP=
LINK_END_GROUP=
endif

$(_MODULE)_MAP      := $($(_MODULE)_BIN).map
$(_MODULE)_INCLUDES := $(foreach inc,$($(_MODULE)_IDIRS),-I$(inc))
$(_MODULE)_DEFINES  := $(foreach def,$($(_MODULE)_DEFS),-D$(def))
$(_MODULE)_LIBRARIES:= $(foreach lib,$(SYS_STATIC_LIBS),-l$(lib)) \
                       $(foreach ldir,$($(_MODULE)_LDIRS),-L$(ldir)) \
					   -Wl,-Bstatic \
					   $(LINK_START_GROUP) \
					   $(foreach lib,$(STATIC_LIBS),-l$(lib)) \
					   $(LINK_END_GROUP)
$(_MODULE)_AFLAGS   := $($(_MODULE)_INCLUDES)
$(_MODULE)_LDFLAGS  += $($(_MODULE)_LOPT)
$(_MODULE)_CPLDFLAGS  = $(foreach ldf,$($(_MODULE)_LDFLAGS),-Wl,$(ldf))
$(_MODULE)_CPLDFLAGS += -Werror -Wl,-static -Wl,--gc-sections -nostartfiles -Wl,--build-id=none
$(_MODULE)_CFLAGS   := -c $($(_MODULE)_INCLUDES) $($(_MODULE)_DEFINES) $($(_MODULE)_COPT) $(CFLAGS)
$(_MODULE)_XDC_TARGET := gnu.targets.arm.A53F
$(_MODULE)_CGT_ROOT = $(GCC_SYSBIOS_ARM_ROOT)

$(_MODULE)_LINKER_CMD_FILES_OPTS:= $(foreach lcmd,$($(_MODULE)_LINKER_CMD_FILES),-Wl,-T $(lcmd))


ifdef DEBUG
$(_MODULE)_AFLAGS += --gdwarf-2
endif

###################################################
# COMMANDS
###################################################

$(_MODULE)_LINK_LIB   := $(AR) -rscu $($(_MODULE)_BIN) $($(_MODULE)_OBJS)
$(_MODULE)_LINK_EXE   := $(LD) $($(_MODULE)_CPLDFLAGS) $($(_MODULE)_OBJS) $($(_MODULE)_LINKER_CMD_FILES_OPTS) $($(_MODULE)_LIBRARIES) -o $($(_MODULE)_BIN) -Wl,-Map=$($(_MODULE)_MAP)

###################################################
# MACROS FOR COMPILING
###################################################

define $(_MODULE)_BUILD
build:: $($(_MODULE)_BIN)
endef

ifeq ($(HOST_OS),Windows_NT)

ifeq ($(MAKE_VERSION),3.80)
$(_MODULE)_GCC_DEPS = -MMD -MF $(ODIR)/$(1).dep -MT '$(ODIR)/$(1).o'
$(_MODULE)_ASM_DEPS = -MD $(ODIR)/$(1).dep
endif

define $(_MODULE)_COMPILE_TOOLS
$(ODIR)/%.o: $(SDIR)/%.c $($(_MODULE)_DEP_HEADERS)
	@echo [GCC] Compiling C99 $$(notdir $$<)
	$(Q)$(CC) -std=c99 $($(_MODULE)_CFLAGS) $(call $(_MODULE)_GCC_DEPS,$$*) $$< -o $$@ $(LOGGING)

$(ODIR)/%.o: $(SDIR)/%.cpp $($(_MODULE)_DEP_HEADERS)
	@echo [GCC] Compiling C++ $$(notdir $$<)
	$(Q)$(CP) $($(_MODULE)_CFLAGS) $(call $(_MODULE)_GCC_DEPS,$$*) $$< -o $$@ $(LOGGING)

$(ODIR)/%.o: $(SDIR)/%.S
	@echo [GCC] Assembling $$(notdir $$<)
	$(Q)$(AS) $($(_MODULE)_AFLAGS) $(call $(_MODULE)_ASM_DEPS,$$*) $$< -o $$@ $(LOGGING)
endef

else

define $(_MODULE)_COMPILE_TOOLS
$(ODIR)/%.o: $(SDIR)/%.c $($(_MODULE)_DEP_HEADERS)
	@echo [GCC] Compiling C99 $$(notdir $$<)
	$(Q)$(CC) -std=gnu99 $($(_MODULE)_CFLAGS) -MMD -MF $(ODIR)/$$*.dep -MT '$(ODIR)/$$*.o' $$< -o $$@ $(LOGGING)

$(ODIR)/%.o: $(SDIR)/%.cpp $($(_MODULE)_DEP_HEADERS)
	@echo [GCC] Compiling C++ $$(notdir $$<)
	$(Q)$(CP) $($(_MODULE)_CFLAGS) -MMD -MF $(ODIR)/$$*.dep -MT '$(ODIR)/$$*.o' $$< -o $$@ $(LOGGING)

$(ODIR)/%.o: $(SDIR)/%.S
	@echo [GCC] Assembling $$(notdir $$<)
	$(Q)$(AS) $($(_MODULE)_AFLAGS) -MD $(ODIR)/$$*.dep $$< -o $$@ $(LOGGING)
endef

endif
