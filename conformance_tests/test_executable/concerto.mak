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

ifeq ($(TARGET_PLATFORM),PC)


include $(PRELUDE)
TARGET      := vx_conformance_tests_exe
TARGETTYPE  := exe
CSOURCES    := $(call all-c-files)

LDIRS       := $(TIOVX_PATH)/lib/PC/X86/$(TARGET_OS)/$(TARGET_BUILD)

STATIC_LIBS := vx_conformance_tests 
STATIC_LIBS += vx_tiovx_tests 

STATIC_LIBS += vx_conformance_engine vx_conformance_tests_testmodule
STATIC_LIBS += vx_vxu vx_framework
STATIC_LIBS += vx_platform_pc vx_framework 

STATIC_LIBS += vx_kernels_openvx_core vx_target_kernels_openvx_core

include $(HOST_ROOT)/kernels/concerto_inc.mak

ifeq ($(BUILD_TUTORIAL),yes)
STATIC_LIBS += vx_target_kernels_tutorial
endif


ifeq ($(BUILD_BAM),yes)
STATIC_LIBS += vx_target_kernels_openvx_core_bam vx_target_kernels_openvx_core
endif

STATIC_LIBS += vx_framework

ifeq ($(BUILD_BAM),yes)
STATIC_LIBS += vxlib_bamplugin_X86 
endif

STATIC_LIBS += vxlib_X86 c6xsim_X86_C66

ifeq ($(BUILD_BAM),yes)
STATIC_LIBS += algframework_X86 dmautils_X86 
endif


include $(FINALE)

endif
