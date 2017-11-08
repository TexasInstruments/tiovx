
TIOVX_PATH = $(abspath .)

PLSDK_PATH := /home/x0157990/ti-processor-sdk-linux-am57xx-evm-04.00.00.04
PRSDK_PATH := /home/x0157990/ti-processor-sdk-rtos-am57xx-evm-04-00-00-04

CROSS_COMPILE_LINARO := arm-linux-gnueabihf-
CROSS_COMPILE := arm-none-eabi-

BUILD_OS ?= Linux
ifeq ($(OS),Windows_NT)
    BUILD_OS=Windows_NT
endif

ifeq ($(BUILD_OS),Windows_NT)
GCC_WINDOWS_ROOT ?= C:/CodeBlocks/MinGW
endif

ifneq ($(CUSTOM_KERNEL_PATH),)
include $(CUSTOM_KERNEL_PATH)/custom_tools_path.mak
endif

ifneq ($(CUSTOM_APPLICATION_PATH),)
include $(CUSTOM_APPLICATION_PATH)/custom_tools_path.mak
endif

XDC_PATH := $(PRSDK_PATH)/xdctools_3_32_01_22_core/
TIARMCGT_ROOT := $(PRSDK_PATH)/ti-cgt-arm_16.9.2.LTS

GCC_ROOT := $(PRSDK_PATH)/gcc-arm-none-eabi-4_9-2015q3
GCC_ROOT_LINARO := /home/x0157990/linaro/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf
CGT6X_ROOT := $(PRSDK_PATH)/ti-cgt-c6000_8.1.3/
GCC_LINUX_ROOT ?= /usr/


VXLIB_PATH ?= $(PRSDK_PATH)/vxlib_c66x_1_1_0_0
XDIAS_PATH := $(PRSDK_PATH)/xdais_7_24_00_04
EDMA3_LLD_PATH := $(PRSDK_PATH)/edma3_lld_2_12_04_28/
BIOS_PATH := $(PRSDK_PATH)/bios_6_46_05_55/

DMAUTILS_PATH ?= $(PRSDK_PATH)/REL.DMAUTILS.00.08.00.02/dmautils
ALGFRAMEWORK_PATH ?= $(PRSDK_PATH)/REL.ALGFRAMEWORK.02.07.00.00/algframework
PDK_PATH := $(PRSDK_PATH)/pdk_am57xx_1_0_7
CMEM_PATH ?= $(PLSDK_INSTALL_PATH)/board-support/extra-drivers/cmem-mod-4.14.00.00+gitAUTOINC+b514a99ac4/
TARGETFS := $(PLSDK_PATH)/linux-devkit/sysroots/armv7ahf-neon-linux-gnueabi/
SDK_PLATFORM_IF_PATH ?= /home/x0157990/am57/openvx/ex41_forwardmsg/vx
