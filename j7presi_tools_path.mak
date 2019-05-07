
TIOVX_PATH = $(abspath .)
PSDK_PATH = $(abspath ..)

CUSTOM_KERNEL_PATH = $(TIOVX_PATH)/tiovx_dev/kernels_j7
CUSTOM_PLATFORM_PATH = $(TIOVX_PATH)/tiovx_dev/platform

VXLIB_PATH ?= $(PSDK_PATH)/vxlib_c66x_1_1_3_0
J7_C_MODELS_PATH ?= $(PSDK_PATH)/j7_c_models
TIDL_PATH ?= $(PSDK_PATH)/tidl_j7_00_08_00_00/ti_dl
IVISION_PATH ?= $(PSDK_PATH)/ivision
IMAGING_PATH ?= $(PSDK_PATH)/imaging
MMALIB_PATH ?= $(PSDK_PATH)/mmalib_00_09_00_00

TIARMCGT_ROOT ?= $(PSDK_PATH)/ti-cgt-arm_18.12.1.LTS
CGT7X_ROOT ?= $(PSDK_PATH)/ti-cgt-c7000_1.0.0
CGT6X_ROOT ?= $(PSDK_PATH)/ti-cgt-c6000_8.3.2
GCC_SYSBIOS_ARM_ROOT ?= $(PSDK_PATH)/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf
GCC_LINUX_ARM_ROOT ?= $(PSDK_PATH)/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-linux-gnu
BIOS_PATH ?= $(PSDK_PATH)/bios_6_76_01_08_eng
XDCTOOLS_PATH ?= $(PSDK_PATH)/xdctools_3_55_01_14_core_eng
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
