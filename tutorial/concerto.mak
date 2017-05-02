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
TARGET      := vx_tutorial
TARGETTYPE  := library

IDIRS       += $(TIOVX_PATH)/tutorial
IDIRS       += $(TIOVX_PATH)/conformance_tests/
IDIRS       += $(TIOVX_PATH)/tutorial/ch01_common

CSOURCES    := \
	ch01_common/bmp_rd_wr.c \
    ch01_common/vx_tutorial.c \
	ch02_image/vx_tutorial_image.c \
	ch02_image/vx_tutorial_image_load_save.c \
	ch02_image/vx_tutorial_image_query.c \
	ch02_image/vx_tutorial_image_crop_roi.c \
	ch02_image/vx_tutorial_image_extract_channel.c \
	ch02_image/vx_tutorial_image_color_convert.c \
	ch02_image/vx_tutorial_image_histogram.c \
	ch03_graph/vx_tutorial_graph.c \
	ch03_graph/vx_tutorial_graph_image_gradients.c \
	ch03_graph/vx_tutorial_graph_image_gradients_pytiovx.c \
	ch03_graph/vx_tutorial_graph_image_gradients_pytiovx_uc.c \
	ch04_target_kernel/vx_tutorial_target_kernel.c \

ifeq ($(TARGET_CPU),C66)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),EVE)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),A15)
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),M4)
SKIPBUILD=0
endif

include $(FINALE)
