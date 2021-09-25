
ifeq ($(TARGET_PLATFORM),TDAX)
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))

include $(PRELUDE)
TARGET      := vx_platform_vision_sdk_linux
TARGETTYPE  := library

COMMON_FILES_REL_PATH = ../../os/posix

CSOURCES    := \
    $(COMMON_FILES_REL_PATH)/tivx_event.c \
    tivx_ipc.c tivx_mem.c tivx_init.c     \
    $(COMMON_FILES_REL_PATH)/tivx_mutex.c \
    tivx_platform_common.c                \
	tivx_host.c                           \
    $(COMMON_FILES_REL_PATH)/tivx_queue.c \
    $(COMMON_FILES_REL_PATH)/tivx_task.c tivx_target_config_a15.c

IDIRS       += $(HOST_ROOT)/source/include
IDIRS       += $(HOST_ROOT)/source/platform/vision_sdk/common
IDIRS       += $(XDC_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages
IDIRS       += $(SDK_PLATFORM_IF_PATH)/
IDIRS       += $(SDK_PLATFORM_IF_PATH)/src/hlos/osa/include
IDIRS       += $(TARGETFS)/usr/include
IDIRS       += $(TARGETFS)/usr
IDIRS       += $(CMEM_PATH)/ludev/include/ti

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
