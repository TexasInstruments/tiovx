# 
# Copyright (c) 2012-2017 The Khronos Group Inc.
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


include $(PRELUDE)
TARGET      := vx_target_kernels_openvx_core_bam
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/openvx-core/include \
               $(HOST_ROOT)/kernels/openvx-core/c66x \
               $(VXLIB_PATH)/packages \
               $(VXLIB2_PATH) \
               $(BAM_PATH)/algframework/inc \
               $(BAM_PATH)/algframework/src/bam_dma_nodes \
               $(EDMA3_LLD_ROOT)/inc $(EDMA3_LLD_ROOT)/inc/baseaddress/vayu/dsp \
               $(EDMA3_LLD_ROOT)/inc/edma_csl $(EDMA3_LLD_ROOT)/inc/edma_utils \
               $(DMAUTILS_PATH)/inc \
               $(DMAUTILS_PATH) \
               $(DMAUTILS_PATH)/inc \
               $(DMAUTILS_PATH)/inc/edma_utils \
               $(DMAUTILS_PATH)/inc/edma_csl \
               $(DMAUTILS_PATH)/inc/baseaddress/vayu/dsp \
               $(EDMA3_LLD_PATH) \
               $(EDMA3_LLD_PATH)/packages/ti/sdo/edma3/rm \
               $(EDMA3_LLD_PATH)/packages

DEFS += CORE_C6X

ifeq ($(USE_BAM),yes)
SKIPBUILD=0
else
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),X86)
CFLAGS		+= -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION 
endif

ifeq ($(TARGET_CPU),C66)
endif

ifeq ($(TARGET_CPU),EVE)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),A15)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),M4)
SKIPBUILD=1
endif


include $(FINALE)
