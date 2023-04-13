PSDK_PATH = $(abspath ..)
APP_UTILS_PATH = $(PSDK_PATH)/app_utils

# paths for components shared between tiovx and app_utils are specified in below
# file in app_utils, ex, pdk, cgtools, ...

include $(APP_UTILS_PATH)/tools_path.mak
include $(APP_UTILS_PATH)/build_flags.mak

CUSTOM_KERNEL_PATH ?= $(TIOVX_PATH)/kernels_j7
CUSTOM_PLATFORM_PATH = $(TIOVX_PATH)/source/platform

TIOVX_PATH ?= $(PSDK_PATH)/tiovx
VXLIB_PATH ?= $(PSDK_PATH)/vxlib
J7_C_MODELS_PATH ?= $(PSDK_PATH)/j7_c_models
TIDL_PATH ?= $(PSDK_PATH)/tidl_$(SOC)_08_06_00_10/ti_dl
MMALIB_PATH ?= $(PSDK_PATH)/mmalib_02_06_02_00
IVISION_PATH ?= $(PSDK_PATH)/ivision
IMAGING_PATH ?= $(PSDK_PATH)/imaging

VISION_APPS_PATH ?= $(PSDK_PATH)/vision_apps
MATHLIB_PATH     ?= $(PSDK_PATH)/mathlib_c66x_3_1_2_1

ifneq ($(CUSTOM_KERNEL_PATH),)
include $(CUSTOM_KERNEL_PATH)/custom_tools_path.mak
endif
ifneq ($(CUSTOM_APPLICATION_PATH),)
include $(CUSTOM_APPLICATION_PATH)/custom_tools_path.mak
endif

