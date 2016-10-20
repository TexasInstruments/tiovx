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


include $(PRELUDE)
TARGET      := vx_platform_vision_sdk_bios
TARGETTYPE  := library
CSOURCES    := tivx_event.c tivx_init.c tivx_ipc.c tivx_mem.c \
               tivx_mutex.c tivx_platform.c tivx_platform_common.c \
               tivx_queue.c tivx_task.c
IDIRS       += $(HOST_ROOT)/source/include
IDIRS       += $(HOST_ROOT)/source/platform/vision_sdk/common
IDIRS       += $(STW_PATH)/include
IDIRS       += $(XDC_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages
IDIRS       += $(VSDK_PATH)

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
endif

ifeq ($(HOST_COMPILER),GCC)

endif

ifeq ($(TARGET_CPU),C66)
CSOURCES += tivx_target_config_dsp_eve.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),EVE)
CSOURCES += tivx_target_config_dsp_eve.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),A15)
CSOURCES += tivx_target_config_a15.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),M4)
CSOURCES += tivx_target_config_ipu1_0.c
SKIPBUILD=0
endif

include $(FINALE)
