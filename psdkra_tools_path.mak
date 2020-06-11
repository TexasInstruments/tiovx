
TIOVX_PATH ?= $(abspath .)
PSDK_PATH ?= $(abspath ..)

CUSTOM_KERNEL_PATH ?= $(TIOVX_PATH)/tiovx_dev/kernels_j7
CUSTOM_PLATFORM_PATH = $(TIOVX_PATH)/tiovx_dev/platform

VXLIB_PATH ?= $(PSDK_PATH)/vxlib_c66x_1_1_5_0
J7_C_MODELS_PATH ?= $(PSDK_PATH)/j7_c_models
TIDL_PATH ?= $(PSDK_PATH)/tidl_j7_01_02_00_08/ti_dl
IVISION_PATH ?= $(PSDK_PATH)/ivision
IMAGING_PATH ?= $(PSDK_PATH)/imaging
MMALIB_PATH ?= $(PSDK_PATH)/mmalib_01_02_00_02

TIARMCGT_ROOT ?= $(PSDK_PATH)/ti-cgt-arm_20.2.0.LTS
CGT7X_ROOT ?= $(PSDK_PATH)/ti-cgt-c7000_1.3.0.STS
CGT6X_ROOT ?= $(PSDK_PATH)/ti-cgt-c6000_8.3.2
GCC_SYSBIOS_ARM_ROOT ?= $(PSDK_PATH)/gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf
GCC_LINUX_ARM_ROOT ?= $(PSDK_PATH)/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu
BIOS_PATH ?= $(PSDK_PATH)/bios_6_82_01_19
XDCTOOLS_PATH ?= $(PSDK_PATH)/xdctools_3_61_00_16_core
PDK_PATH ?= $(PSDK_PATH)/pdk
PDK_QNX_PATH ?= $(PSDK_PATH)/psdkqa/pdk
VISION_APPS_PATH ?= $(PSDK_PATH)/vision_apps
VIDEO_CODEC_PATH ?= $(PSDK_PATH)/video_codec
MATHLIB_PATH     ?= $(PSDK_PATH)/mathlib_c66x_3_1_2_1

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
