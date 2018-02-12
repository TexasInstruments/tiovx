
TIOVX_PATH = $(abspath .)
PSDK_PATH = $(abspath ..)

CUSTOM_KERNEL_PATH = $(TIOVX_PATH)/tiovx_dev/kernels_tda4x
CUSTOM_APPLICATION_PATH = $(TIOVX_PATH)/tiovx_dev/demos

BUILD_OS ?= Linux

ifneq ($(CUSTOM_KERNEL_PATH),)
include $(CUSTOM_KERNEL_PATH)/custom_tools_path.mak
endif
ifneq ($(CUSTOM_APPLICATION_PATH),)
include $(CUSTOM_APPLICATION_PATH)/custom_tools_path.mak
endif

ifeq ($(BUILD_OS),Linux)
GCC_LINUX_ROOT ?= /usr/
endif

CGT7X_PATH ?= $(PSDK_PATH)/ti-cgt-c7000_1.0.0A18023
VXLIB_PATH ?= $(TIOVX_PATH)/../vxlib_c66x_1_1_1_0





