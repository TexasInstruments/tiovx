PSDK_PATH = $(abspath ..)
APP_UTILS_PATH ?= $(PSDK_PATH)/app_utils

# paths for components shared between app_utils, tiovx, imaging, video_io, ti-perception,
# and vision_apps are specified in below file
include $(APP_UTILS_PATH)/tools_path.mak

# CUSTOM_KERNEL_PATH ?= $(TIOVX_PATH)/kernels_j7
CUSTOM_PLATFORM_PATH = $(TIOVX_PATH)/source/platform

ifneq ($(CUSTOM_KERNEL_PATH),)
include $(CUSTOM_KERNEL_PATH)/custom_tools_path.mak
endif
ifneq ($(CUSTOM_APPLICATION_PATH),)
include $(CUSTOM_APPLICATION_PATH)/custom_tools_path.mak
endif
