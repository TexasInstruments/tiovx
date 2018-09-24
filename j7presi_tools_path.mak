
TIOVX_PATH = $(abspath .)
PSDK_PATH = $(abspath ..)

CUSTOM_KERNEL_PATH = $(TIOVX_PATH)/tiovx_dev/kernels_tda4x
CUSTOM_PLATFORM_PATH = $(TIOVX_PATH)/tiovx_dev/platform

VXLIB_PATH ?= $(PSDK_PATH)/vxlib_c66x_1_1_2_0
TDA4X_C_MODELS_PATH ?= $(PSDK_PATH)/tda4x_c_models
TIDL_PATH ?= $(PSDK_PATH)/tidl_j7_00_02_00_00/ti_dl
IVISION_PATH ?= $(PSDK_PATH)/ivision
OPENCV_LIB_PATH ?= /usr/lib/x86_64-linux-gnu

TIARMCGT_ROOT ?= $(PSDK_PATH)/ti-cgt-arm_16.9.4.LTS
CGT7X_ROOT ?= $(PSDK_PATH)/ti-cgt-c7000_1.0.0A18023
CGT6X_ROOT ?= $(PSDK_PATH)/ti-cgt-c6000_8.2.2
GCC_SYSBIOS_ARM_ROOT ?= $(PSDK_PATH)/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf
BIOS_PATH ?= $(PSDK_PATH)/bios_6_73_00_12
XDCTOOLS_PATH ?= $(PSDK_PATH)/xdctools_3_50_07_20_core
PDK_PATH ?= $(PSDK_PATH)/pdk
VISION_APPS_PATH ?= $(PSDK_PATH)/vision_apps

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
