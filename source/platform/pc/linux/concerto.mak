

ifeq ($(TARGET_PLATFORM),PC)
ifeq ($(TARGET_OS),LINUX)

include $(PRELUDE)
TARGET      := vx_platform_pc
TARGETTYPE  := library

OS_FILES_REL_PATH = ../../os/posix
COMMON_FILES_REL_PATH = ../common

COMMON_FILES_BASE_PATH = $(TIOVX_PATH)/source/platform/pc/common


CSOURCES    := \
	$(OS_FILES_REL_PATH)/tivx_event.c \
	$(OS_FILES_REL_PATH)/tivx_mutex.c \
	$(OS_FILES_REL_PATH)/tivx_task.c  \
	$(OS_FILES_REL_PATH)/tivx_queue.c \
	$(COMMON_FILES_REL_PATH)/tivx_mem.c \
	$(COMMON_FILES_REL_PATH)/tivx_ipc.c \
	$(COMMON_FILES_REL_PATH)/tivx_init.c \
	$(COMMON_FILES_REL_PATH)/tivx_host.c \
	$(COMMON_FILES_REL_PATH)/tivx_target_config_pc.c \
	$(COMMON_FILES_REL_PATH)/tivx_platform_common.c \
    tivx_platform.c

IDIRS += $(TIOVX_PATH)/source/include $(COMMON_FILES_BASE_PATH)
IDIRS += $(VISION_APPS_PATH)

ifneq ($(BUILD_SDK), $(filter $(BUILD_SDK), vsdk psdk))
DEFS += _DISABLE_TIDL
IDIRS += $(CUSTOM_KERNEL_PATH)/include
endif

include $(FINALE)

endif
endif
