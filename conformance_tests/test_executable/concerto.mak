# 
# Copyright (c) 2012-2016 The Khronos Group Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and/or associated documentation files (the
# "Materials"), to deal in the Materials without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Materials, and to
# permit persons to whom the Materials are furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Materials.
#
# MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
# KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
# SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
#    https://www.khronos.org/registry/
#
# THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
#

ifeq ($(TARGET_PLATFORM),PC)


include $(PRELUDE)
TARGET      := vx_conformance_tests_exe
TARGETTYPE  := exe
CSOURCES    := $(call all-c-files)

LDIRS       := $(TIOVX_PATH)/lib/PC/X86/$(TARGET_OS)/$(TARGET_BUILD)

STATIC_LIBS := vx_conformance_tests 
STATIC_LIBS += vx_tiovx_tests 

ifeq ($(BUILD_IVISION_KERNELS),yes)
STATIC_LIBS += vx_tiovx_ivision_tests 
endif

STATIC_LIBS += vx_conformance_engine vx_conformance_tests_testmodule
STATIC_LIBS += vx_vxu vx_framework
STATIC_LIBS += vx_platform_pc vx_framework 

ifeq ($(BUILD_IVISION_KERNELS),yes)
STATIC_LIBS += vx_kernels_ivision vx_target_kernels_ivision vx_target_kernels_ivision_common
endif

STATIC_LIBS += vx_kernels_openvx_core vx_target_kernels_openvx_core

ifeq ($(USE_BAM),yes)
STATIC_LIBS += vx_target_kernels_openvx_core_bam
endif

STATIC_LIBS += vx_framework

ifeq ($(USE_BAM),yes)
STATIC_LIBS += algframework_X86 dmautils_X86 vxlib_bamplugin_X86 
endif

STATIC_LIBS += vxlib_X86 c6xsim_X86_C66

ifeq ($(BUILD_IVISION_KERNELS),yes)
STATIC_LIBS += eveHarrisCornerDetection32.eve 
STATIC_LIBS += evekernels.eve eveprivkernels.eve 
STATIC_LIBS += evenatckernels.eve eveprivnatckernels.eve
STATIC_LIBS += evealgframework.eve eveextmem.eve evestarterware_eve
endif



include $(FINALE)

endif