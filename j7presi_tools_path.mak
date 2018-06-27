
TIOVX_PATH = $(abspath .)
PSDK_PATH = $(abspath ..)

CUSTOM_KERNEL_PATH = $(TIOVX_PATH)/tiovx_dev/kernels_tda4x

VXLIB_PATH ?= $(PSDK_PATH)/vxlib_c66x_1_1_2_0
TDA4X_C_MODELS_PATH ?= $(PSDK_PATH)/tda4x_c_models
TIDL_PATH ?= $(PSDK_PATH)/REL.TIDL.J7.00.02.00.00/ti_dl
IVISION_PATH ?= $(PSDK_PATH)/ivision
OPENCV_LIB_PATH ?= /usr/lib/x86_64-linux-gnu

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







