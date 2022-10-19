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



OVX_INC=$(HOST_ROOT)/include $(HOST_ROOT)/kernels/include $(CUSTOM_KERNEL_PATH)/include $(CUSTOM_APPLICATION_PATH)/kernels


ifeq ($(BUILD_DEBUG),1)
$(info TI_TOOLS_ROOT=$(TI_TOOLS_ROOT))
$(info TIARMCGT_LLVM_ROOT=$(TIARMCGT_LLVM_ROOT))
endif

# DEP_PROJECTS does not need to be set as the dependencies are contained in the build.

SYSIDIRS := $(OVX_INC)
SYSLDIRS :=

SYSDEFS  :=

ifeq ($(TARGET_PLATFORM),PC)
    SYSDEFS +=
    SYSIDIRS += $(GCC_WINDOWS_ROOT)/include
    SYSLDIRS += $(GCC_WINDOWS_ROOT)/lib
else
    SYSDEFS +=
    ifeq ($(TARGET_FAMILY),ARM)
        ifeq ($(TARGET_CPU),A15)
              ifeq ($(TARGET_OS),QNX)
                 SYSIDIRS += $(GCC_QNX_ARM_ROOT)/../usr/include
                 SYSLDIRS += $(GCC_QNX_ARM_ROOT)/../usr/lib
              else
                 SYSIDIRS += $(GCC_ROOT)/include
                 SYSLDIRS += $(GCC_ROOT)/lib
              endif
        else ifeq ($(TARGET_CPU),A72)
              ifeq ($(TARGET_OS),SYSBIOS)
                 SYSIDIRS += $(GCC_SYSBIOS_ARM_ROOT)/include
                 SYSLDIRS += $(GCC_SYSBIOS_ARM_ROOT)/lib
              else ifeq ($(TARGET_OS),QNX)
                 SYSIDIRS += $(GCC_QNX_ARM_ROOT)/../usr/include
                 SYSLDIRS += $(GCC_QNX_ARM_ROOT)/../usr/lib
              endif
        else
            SYSIDIRS += $(TIARMCGT_LLVM_ROOT)/include
            SYSLDIRS += $(TIARMCGT_LLVM_ROOT)/lib
        endif
    else ifeq ($(TARGET_FAMILY),DSP)
        ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7120 C7504))
            SYSIDIRS += $(CGT7X_ROOT)/include
            SYSLDIRS += $(CGT7X_ROOT)/lib
        else
            SYSIDIRS += $(CGT6X_ROOT)/include
            SYSLDIRS += $(CGT6X_ROOT)/lib
        endif
    else ifeq ($(TARGET_FAMILY),EVE)
        SYSIDIRS += $(ARP32CGT_ROOT)/include
        SYSLDIRS += $(ARP32CGT_ROOT)/lib
    endif
endif
