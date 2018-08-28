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

# Any OS can build itself and maybe some secondary OS's

ifeq ($(SHOW_MAKEDEBUG),1)
$(info Starting COMBOS:)
$(foreach combo,$(TARGET_COMBOS),$(info TARGET_COMBOS+=$(combo)))
endif

ifeq ($(HOST_PLATFORM),PC)
ifeq ($(HOST_OS),LINUX)
TARGET_COMBOS := $(call FILTER_COMBO,LINUX SYSBIOS)
else ifeq ($(HOST_OS),Windows_NT)
TARGET_COMBOS := $(call FILTER_COMBO,WINDOWS SYSBIOS)
endif
endif

# If the platform is set, remove others which are not on that platform.
ifneq ($(filter $(origin TARGET_PLATFORM),environment command),)
$(info Keep only $(TARGET_PLATFORM) platform in TARGET_COMBOS)
TARGET_COMBOS := $(call FILTER_COMBO,$(TARGET_PLATFORM))
endif

# If the OS is set, remove others which are not on that OS.
ifneq ($(filter $(origin TARGET_OS),environment command),)
$(info Keep only $(TARGET_OS) OS in TARGET_COMBOS)
TARGET_COMBOS := $(call FILTER_COMBO,$(TARGET_OS))
endif

# If the CPU is set, remove others which are not on that CPU.
ifneq ($(filter $(origin TARGET_CPU),environment command),)
$(info Keep only $(TARGET_CPU) CPU in TARGET_COMBOS)
TARGET_COMBOS := $(call FILTER_COMBO,$(TARGET_CPU))
endif

# If the BUILD is set, remove others which are not on that BUILD
ifneq ($(filter $(origin TARGET_BUILD),environment command),)
$(info Keep only $(TARGET_BUILD) BUILDS in TARGET_COMBOS)
TARGET_COMBOS := $(call FILTER_COMBO,$(TARGET_BUILD))
endif

# The compilers which must have roots set.
COMPILER_ROOTS := TIARMCGT_ROOT GCC_SYSBIOS_ARM_ROOT CGT6X_ROOT GCC_WINDOWS_ROOT GCC_LINUX_ROOT

$(foreach root,$(COMPILER_ROOTS),$(info $(origin $(root)) $(root)=$(value $(root))))

# The compiler which do not have roots set.
REMOVE_ROOTS := $(foreach root,$(COMPILER_ROOTS),$(if $(filter $(origin $(root)),undefined),$(root)))

# Remove the list of combos which can not be built
TARGET_COMBOS := $(call FILTER_OUT_COMBO,$(foreach root,$(REMOVE_ROOTS),$(subst _ROOT,,$(root))))

TARGET_COMBOS := $(strip $(TARGET_COMBOS))

ifeq ($(SHOW_MAKEDEBUG),1)
$(info Remaining COMBOS:)
$(foreach combo,$(TARGET_COMBOS),$(info TARGET_COMBOS+=$(combo)))
endif

$(if $(strip $(TARGET_COMBOS)),,$(error No TARGET_COMBOS remain! Nothing to make))
