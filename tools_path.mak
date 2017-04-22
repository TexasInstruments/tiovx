# Copyright (C) 2011 Texas Insruments, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

TIOVX_PATH := $(abspath .)

VSDK_INSTALL_PATH := $(abspath ../../../)
VSDK_TOOLS_PATH := $(VSDK_INSTALL_PATH)/ti_components

CROSS_COMPILE_LINARO := arm-linux-gnueabihf-
CROSS_COMPILE := arm-none-eabi-

BUILD_OS ?= Linux
ifeq ($(OS),Windows_NT)
    BUILD_OS=Windows_NT
endif

ifeq ($(BUILD_OS),Windows_NT)
XDC_PATH := $(VSDK_TOOLS_PATH)/os_tools/windows/xdctools_3_32_00_06_core
TIARMCGT_ROOT := $(VSDK_TOOLS_PATH)/cg_tools/windows/ti-cgt-arm_5.2.5
GCC_ROOT := $(VSDK_TOOLS_PATH)/cg_tools/windows/gcc-arm-none-eabi-4_9-2015q3
CGT6X_ROOT := $(VSDK_TOOLS_PATH)/cg_tools/windows/C6000_7.4.2
ARP32CGT_ROOT := $(VSDK_TOOLS_PATH)/cg_tools/windows/arp32_1.0.7
GCC_WINDOWS_ROOT := C:/CodeBlocks/MinGW
endif

ifeq ($(BUILD_OS),Linux)
XDC_PATH := $(VSDK_TOOLS_PATH)/os_tools/linux/xdctools_3_32_00_06_core
TIARMCGT_ROOT := $(VSDK_TOOLS_PATH)/cg_tools/linux/ti-cgt-arm_5.2.5

GCC_ROOT := $(VSDK_TOOLS_PATH)/cg_tools/linux/gcc-arm-none-eabi-4_9-2015q3
GCC_ROOT_LINARO := $(VSDK_TOOLS_PATH)/os_tools/linux/linaro/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf

CGT6X_ROOT := $(VSDK_TOOLS_PATH)/cg_tools/linux/C6000_7.4.2
ARP32CGT_ROOT := $(VSDK_TOOLS_PATH)/cg_tools/linux/arp32_1.0.7
GCC_LINUX_ROOT := /usr/
endif

BIOS_PATH := $(VSDK_TOOLS_PATH)/os_tools/bios_6_46_00_23
VXLIB_PATH := $(VSDK_TOOLS_PATH)/algorithms_codecs/vxlib_c66x_src_1_0_0_0
XDIAS_PATH := $(VSDK_TOOLS_PATH)/algorithms_codecs/xdais_7_24_00_04
EVE_SW_PATH := $(VSDK_TOOLS_PATH)/algorithms_codecs/eve_sw_01_14_00_00

# This path is temporary for our development since the above release doesn't have bam_plugins yet
VXLIB2_PATH := $(VSDK_TOOLS_PATH)/algorithms_codecs/vxlib
ALGFRAMEWORK_PATH := $(VSDK_TOOLS_PATH)/algorithms_codecs/dsp_apps/algframework
DMAUTILS_PATH := $(VSDK_TOOLS_PATH)/algorithms_codecs/dsp_apps/dmautils
EDMA3_LLD_PATH := $(VSDK_TOOLS_PATH)/drivers/edma3_lld_02_12_00_20

STW_PATH := $(VSDK_INSTALL_PATH)/ti_components/drivers/starterware/starterware_
VSDK_PATH := $(VSDK_INSTALL_PATH)/vision_sdk
TARGETFS := $(VSDK_TOOLS_PATH)/../user/openvx-nightly/targetfs

