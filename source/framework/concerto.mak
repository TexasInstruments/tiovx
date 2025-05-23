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

ifeq ($(LDRA_COVERAGE_ENABLED), yes)
include $(TIOVX_PATH)/tiovx_dev/internal_docs/coverage_files/concerto_inc.mak
else
DEFS += HOST_ONLY
DEFS += LDRA_UNTESTABLE_CODE
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 C66))
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

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64))
CFLAGS += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
endif

endif

endif

include $(FINALE)
