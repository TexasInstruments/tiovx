PSDK_PATH ?= $(abspath ..)
PSDK_BUILDER_PATH ?= $(PSDK_PATH)/sdk_builder
APP_UTILS_PATH ?= $(PSDK_PATH)/app_utils

# paths for components shared between app_utils, tiovx, imaging, video_io, ti-perception,
# and vision_apps are specified in below file
include $(PSDK_BUILDER_PATH)/tools_path.mak

CUSTOM_KERNEL_PATH ?=
CUSTOM_PLATFORM_PATH ?=

ifneq ($(CUSTOM_KERNEL_PATH),)
	include $(CUSTOM_KERNEL_PATH)/custom_tools_path.mak
endif
ifneq ($(CUSTOM_APPLICATION_PATH),)
	include $(CUSTOM_APPLICATION_PATH)/custom_tools_path.mak
endif
