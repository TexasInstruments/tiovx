# Copyright (C) 2011 Texas Insruments, Inc.
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


OVX_INC=$(HOST_ROOT)/include

ifeq ($(BUILD_DEBUG),1)
$(info TI_TOOLS_ROOT=$(TI_TOOLS_ROOT))
$(info TIARMCGT_ROOT=$(TIARMCGT_ROOT))
endif

# DEP_PROJECTS does not need to be set as the dependencies are contained in the build.

SYSIDIRS := $(OVX_INC)
SYSLDIRS := $(HOST_ROOT)/out/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
SYSDEFS  :=

BSP_PATH := D:/vision_sdk_02_10_00_00/ti_components/drivers/bsp_01_06_00_11
STW_PATH := D:/vision_sdk_02_10_00_00/ti_components/drivers/starterware_01_06_00_15
XDC_PATH := D:/vision_sdk_02_10_00_00/ti_components/os_tools/windows/xdctools_3_32_00_06_core
BIOS_PATH := D:/vision_sdk_02_10_00_00/ti_components/os_tools/bios_6_46_00_23

ifeq ($(TARGET_PLATFORM),TDAX)
    SYSDEFS +=
    ifeq ($(TARGET_FAMILY),ARM)
        ifeq ($(TARGET_CPU),A15)
        SYSIDIRS += $(GCC_ROOT)/include
        SYSLDIRS += $(GCC_ROOT)/lib
        else
        SYSIDIRS += $(TIARMCGT_ROOT)/include
        SYSLDIRS += $(TIARMCGT_ROOT)/lib
        endif
    else ifeq ($(TARGET_FAMILY),DSP)
        SYSIDIRS += $(CGT6X_ROOT)/include
        SYSLDIRS += $(CGT6X_ROOT)/lib
    else ifeq ($(TARGET_FAMILY),EVE)
        SYSIDIRS += $(ARP32CGT_ROOT)/include
        SYSLDIRS += $(ARP32CGT_ROOT)/lib
    endif
endif

