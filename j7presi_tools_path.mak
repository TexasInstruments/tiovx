
TIOVX_PATH = $(abspath .)
PSDK_PATH = $(abspath ..)

CUSTOM_KERNEL_PATH = $(TIOVX_PATH)/kernels_tda4x

CGT7X_PATH ?= $(PSDK_PATH)/ti-cgt-c7000_1.0.0A18023
VXLIB_PATH ?= $(PSDK_PATH)/vxlib_c66x_1_1_1_0
TDA4X_C_MODELS_PATH ?= $(PSDK_PATH)/tda4x_c_models
OPENCV_LIB_PATH ?= /usr/lib/x86_64-linux-gnu
PTK_PATH ?= $(PSDK_PATH)/perception

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







