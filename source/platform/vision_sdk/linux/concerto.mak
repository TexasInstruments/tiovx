
ifeq ($(TARGET_PLATFORM),TDAX)
ifeq ($(TARGET_OS),LINUX)

include $(PRELUDE)
TARGET      := vx_platform_vision_sdk_linux
TARGETTYPE  := library

COMMON_FILES_REL_PATH = ../../os/linux

CSOURCES    := \
    $(COMMON_FILES_REL_PATH)/tivx_event.c \
    tivx_ipc.c tivx_mem.c tivx_init.c     \
    $(COMMON_FILES_REL_PATH)/tivx_mutex.c \
    tivx_platform_common.c                \
    $(COMMON_FILES_REL_PATH)/tivx_queue.c \
    $(COMMON_FILES_REL_PATH)/tivx_task.c tivx_target_config_a15.c

IDIRS       += $(HOST_ROOT)/source/include
IDIRS       += $(HOST_ROOT)/source/platform/vision_sdk/common
IDIRS       += $(XDC_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages
IDIRS       += $(VSDK_PATH)/links_fw
IDIRS       += $(TARGETFS)/usr/include
IDIRS       += $(TARGETFS)/usr

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
endif

ifeq ($(HOST_COMPILER),GCC)

endif

ifeq ($(TARGET_CPU),C66)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),EVE)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),A15)
ifeq ($(TARGET_OS),Bios)
SKIPBUILD=1
endif
endif

ifeq ($(TARGET_CPU),A15)
ifeq ($(TARGET_OS),Linux)
SKIPBUILD=0
endif
endif

ifeq ($(TARGET_CPU),M4)
SKIPBUILD=1
endif

include $(FINALE)

endif
endif
