
TIOVX_PATH = $(abspath .)

VSDK_INSTALL_PATH ?= $(abspath ../../../)
VSDK_TOOLS_PATH ?= $(VSDK_INSTALL_PATH)/ti_components
#VSDK_TOOLS_PATH ?= /datalocal/ti_components

CROSS_COMPILE_LINARO := arm-linux-gnueabihf-
CROSS_COMPILE := arm-none-eabi-

BUILD_OS ?= Linux
ifeq ($(OS),Windows_NT)
    BUILD_OS=Windows_NT
endif

ifeq ($(BUILD_OS),Windows_NT)
XDC_PATH ?= $(VSDK_TOOLS_PATH)/os_tools/windows/xdctools_3_32_01_22_core
TIARMCGT_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/windows/ti-cgt-arm_16.9.2.LTS
GCC_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/windows/gcc-arm-none-eabi-4_9-2015q3
CGT6X_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/windows/C6000_7.4.2
ARP32CGT_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/windows/arp32_1.0.7
GCC_WINDOWS_ROOT ?= C:/CodeBlocks/MinGW
endif

ifeq ($(BUILD_OS),Linux)
XDC_PATH ?= $(VSDK_TOOLS_PATH)/os_tools/linux/xdctools_3_32_01_22_core
TIARMCGT_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/linux/ti-cgt-arm_16.9.2.LTS
GCC_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/linux/gcc-arm-none-eabi-4_9-2015q3
GCC_ROOT_LINARO ?= $(VSDK_TOOLS_PATH)/os_tools/linux/linaro/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf
CGT6X_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/linux/C6000_7.4.2
ARP32CGT_ROOT ?= $(VSDK_TOOLS_PATH)/cg_tools/linux/arp32_1.0.7
GCC_LINUX_ROOT ?= /usr/
endif

VXLIB_PATH ?= $(VSDK_TOOLS_PATH)/algorithms/vxlib_c66x_1_1_0_0
EVE_SW_PATH ?= $(VSDK_TOOLS_PATH)/algorithms/eve_sw_01_16_00_00
XDIAS_PATH ?= $(VSDK_TOOLS_PATH)/codecs/xdais_7_24_00_04
EDMA3_LLD_PATH ?= $(VSDK_TOOLS_PATH)/drivers/edma3_lld_02_12_00_20
BIOS_PATH ?= $(VSDK_TOOLS_PATH)/os_tools/bios_6_46_04_53

DMAUTILS_PATH ?= $(VSDK_TOOLS_PATH)/algorithms/REL.DMAUTILS.00.08.00.02/dmautils
ALGFRAMEWORK_PATH ?= $(VSDK_TOOLS_PATH)/algorithms/REL.ALGFRAMEWORK.00.02.06.01/algframework
PDK_PATH ?= $(VSDK_INSTALL_PATH)/ti_components/drivers/pdk
VSDK_PATH ?= $(VSDK_INSTALL_PATH)/vision_sdk
CMEM_PATH ?= $(VSDK_INSTALL_PATH)/ti_components/os_tools/linux/kernel/cmem
TARGETFS ?= $(VSDK_INSTALL_PATH)/ti_components/os_tools/linux/targetfs




