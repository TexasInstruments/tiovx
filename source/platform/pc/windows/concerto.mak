
ifeq ($(TARGET_PLATFORM),PC)
ifeq ($(TARGET_OS),WINDOWS)

include $(PRELUDE)
TARGET      := vx_platform_pc
TARGETTYPE  := library

OS_FILES_REL_PATH = ../../os/win32
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


include $(FINALE)

endif
endif