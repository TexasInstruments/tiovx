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
ifeq ($(TARGET_OS),WINDOWS)

include $(PRELUDE)
TARGET      := vx_platform_pc
TARGETTYPE  := library

OS_FILES_REL_PATH = ../../os/win32
COMMON_FILES_REL_PATH = ../common

COMMON_FILES_BASE_PATH = $(TIOVX_PATH)/source/platform/pc/common


CSOURCES    := \
	$(OS_FILES_REL_PATH)/tivx_event.c \
	$(OS_FILES_REL_PATH)/tivx_mutex.c \
	$(OS_FILES_REL_PATH)/tivx_task.c  \
	$(OS_FILES_REL_PATH)/tivx_queue.c \
	$(COMMON_FILES_REL_PATH)/tivx_mem.c \
	$(COMMON_FILES_REL_PATH)/tivx_ipc.c \
	$(COMMON_FILES_REL_PATH)/tivx_init.c \
	$(COMMON_FILES_REL_PATH)/tivx_target_config_pc.c \
	$(COMMON_FILES_REL_PATH)/tivx_platform_common.c \
    tivx_platform.c

IDIRS += $(TIOVX_PATH)/source/include $(COMMON_FILES_BASE_PATH)


include $(FINALE)

endif
endif