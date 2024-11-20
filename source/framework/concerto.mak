#
# Copyright (c) 2012-2017 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#


include $(PRELUDE)
TARGET      := vx_framework
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/source/include
IDIRS       += $(HOST_ROOT)/utils/include
IDIRS       += $(IPPC_PATH)
IDIRS       += $(APP_UTILS_PATH)

DEFS        += LDRA_UNTESTABLE_CODE
# This is used to signify which sections of code is only applicable
# for the host for code coverage purposes. It has been left defined
# for all cores, but can be wrapped in the appropriate CPU when generating
# code coverage reports.
DEFS        += HOST_ONLY

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
endif

ifeq ($(HOST_COMPILER), $(filter $(HOST_COMPILER), GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-sizeof-pointer-memaccess
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F C66 C71 C7120 C7504 C7524))
CSOURCES_EXCLUDE := vx_producer.c vx_consumer.c
CSOURCES := $(filter-out $(CSOURCES_EXCLUDE), $(CSOURCES))
endif

ifeq ($(CONNECTOR_TP), IPPC_SHEM)
DEFS += IPPC_SHEM_ENABLED
else
DEFS += SOCKET_ENABLED
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C66))
ifeq ($(BUILD_BAM),yes)

DEFS += BUILD_BAM

IDIRS       += $(VXLIB_PATH)/packages \
               $(ALGFRAMEWORK_PATH)/inc \
               $(DMAUTILS_PATH)/inc \
               $(DMAUTILS_PATH) \
               $(DMAUTILS_PATH)/inc \
               $(DMAUTILS_PATH)/inc/edma_utils \
               $(DMAUTILS_PATH)/inc/edma_csl \
               $(DMAUTILS_PATH)/inc/baseaddress/vayu/dsp \
               $(EDMA3_LLD_PATH) \
               $(EDMA3_LLD_PATH)/packages/ti/sdo/edma3/rm \
               $(EDMA3_LLD_PATH)/packages

DEFS += CORE_DSP CORE_C6XX

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
endif

endif

endif

include $(FINALE)
