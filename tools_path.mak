
TIOVX_PATH = $(abspath .)

VSDK_INSTALL_PATH ?= $(abspath ../../../)
VSDK_TOOLS_PATH ?= $(VSDK_INSTALL_PATH)/ti_components

CROSS_COMPILE_LINARO := arm-linux-gnueabihf-
CROSS_COMPILE := arm-none-eabi-

BUILD_OS ?= Linux
ifeq ($(OS),Windows_NT)
    BUILD_OS=Windows_NT
endif

ifeq ($(BUILD_OS),Windows_NT)
XDC_PATH ?= $(VSDK_TOOLS_PATH)/os_tools/windows/xdctools_3_32_00_06_core
TIARMCGT_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/windows/ti-cgt-arm_5.2.5
GCC_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/windows/gcc-arm-none-eabi-4_9-2015q3
CGT6X_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/windows/C6000_7.4.2
ARP32CGT_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/windows/arp32_1.0.7
GCC_WINDOWS_ROOT ?= C:/CodeBlocks/MinGW
endif

ifeq ($(BUILD_OS),Linux)
XDC_PATH ?= $(VSDK_TOOLS_PATH)/os_tools/linux/xdctools_3_32_00_06_core
TIARMCGT_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/linux/ti-cgt-arm_5.2.5
GCC_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/linux/gcc-arm-none-eabi-4_9-2015q3
GCC_ROOT_LINARO ?= $(VSDK_TOOLS_PATH)/os_tools/linux/linaro/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf
CGT6X_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/linux/C6000_7.4.2
ARP32CGT_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/linux/arp32_1.0.7
GCC_LINUX_ROOT ?= /usr/
endif

DMAUTILS_PATH ?= $(VSDK_TOOLS_PATH)/algorithms/dsp_apps/dmautils
ALGFRAMEWORK_PATH ?= $(VSDK_TOOLS_PATH)/algorithms/dsp_apps/algframework
VXLIB_PATH ?= $(VSDK_TOOLS_PATH)/algorithms/vxlib_c66x_1_1_0_0
EVE_SW_PATH ?= $(VSDK_TOOLS_PATH)/algorithms/eve_sw_01_14_00_00
XDIAS_PATH ?= $(VSDK_TOOLS_PATH)/codecs/xdais_7_24_00_04
EDMA3_LLD_PATH ?= $(VSDK_TOOLS_PATH)/drivers/edma3_lld_02_12_00_20
BIOS_PATH ?= $(VSDK_TOOLS_PATH)/os_tools/bios_6_46_00_23

PDK_PATH ?= $(VSDK_INSTALL_PATH)/ti_components/drivers/pdk
VSDK_PATH ?= $(VSDK_INSTALL_PATH)/vision_sdk
TARGETFS ?= $(VSDK_INSTALL_PATH)/vision_sdk/hlos/linux/targetfs




