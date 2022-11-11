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

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72 R5F))
ifeq ($(BUILD_CONFORMANCE_TEST),yes)

include $(PRELUDE)
TARGET      := vx_tiovx_tests
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/conformance_tests
IDIRS       += $(HOST_ROOT)/utils/include
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/include
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/test_kernels/include
IDIRS       += $(HOST_ROOT)/conformance_tests/test_tiovx/utils

CSOURCES    += utils/test_utils_file_bmp_rd_wr.c

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
CFLAGS += --diag_suppress=179
CFLAGS += --diag_suppress=112
CFLAGS += --diag_suppress=552
endif

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_WINDOWS GCC_LINUX GCC_SYSBIOS_ARM GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security
CFLAGS += -Wno-unused-but-set-variable
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
DEFS += PLATFORM_PC
endif

ifeq ($(BUILD_BAM),yes)
DEFS += BUILD_BAM
endif

include $(FINALE)

endif
endif
