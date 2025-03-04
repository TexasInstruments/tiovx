
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))

include $(PRELUDE)
TARGET      := vx_platform_board_hlos
TARGETTYPE  := library

OS_FILES_REL_PATH = ../../common/os/posix
COMMON_FILES_REL_PATH = ../common
TARGET_FILES_REL_PATH = ../../common/targets

ifeq ($(LDRA_COVERAGE_ENABLED), yes)
include $(TIOVX_PATH)/tiovx_dev/internal_docs/coverage_files/concerto_inc.mak
else
DEFS += HOST_ONLY
DEFS += LDRA_UNTESTABLE_CODE
endif

CSOURCES    := \
    $(OS_FILES_REL_PATH)/tivx_event.c \
    $(OS_FILES_REL_PATH)/tivx_mutex.c \
    $(OS_FILES_REL_PATH)/tivx_posix_objects.c  \
    $(OS_FILES_REL_PATH)/tivx_queue.c \
    $(OS_FILES_REL_PATH)/tivx_task.c  \
    $(TARGET_FILES_REL_PATH)/tivx_target_config_mpu1_0.c           \
    $(COMMON_FILES_REL_PATH)/tivx_ipc.c                            \
    $(COMMON_FILES_REL_PATH)/tivx_init.c                           \
	../../common/tivx_host.c \
    $(COMMON_FILES_REL_PATH)/tivx_platform.c                       \
    $(COMMON_FILES_REL_PATH)/tivx_mem.c                            \
    tivx_platform_hlos.c                            \
    $(COMMON_FILES_REL_PATH)/tivx_perf.c  \
    $(TARGET_FILES_REL_PATH)/tivx_target_config.c

IDIRS       += $(TIOVX_PATH)/source/include
IDIRS       += $(TIOVX_PATH)/source/platform/common/targets
IDIRS       += $(TIOVX_PATH)/source/platform/common/os/posix
IDIRS       += $(TIOVX_PATH)/source/platform/board/common
IDIRS       += $(APP_UTILS_PATH)

DEFS += TARGET_HLOS

include $(FINALE)

endif
endif