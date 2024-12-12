
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))

include $(PRELUDE)
TARGET      := vx_platform_psdk_j7
TARGETTYPE  := library

COMMON_FILES_REL_PATH = ../../os/posix

ifeq ($(LDRA_COVERAGE_ENABLED), yes)
include $(TIOVX_PATH)/tiovx_dev/internal_docs/coverage_files/concerto_inc.mak
else
DEFS += HOST_ONLY
DEFS += LDRA_UNTESTABLE_CODE
endif

CSOURCES    := \
    $(COMMON_FILES_REL_PATH)/tivx_event.c \
    $(COMMON_FILES_REL_PATH)/tivx_mutex.c \
    $(COMMON_FILES_REL_PATH)/tivx_posix_objects.c  \
    $(COMMON_FILES_REL_PATH)/tivx_queue.c \
    $(COMMON_FILES_REL_PATH)/tivx_task.c  \
    ../common/tivx_target_config_mpu1_0.c           \
    ../common/tivx_ipc.c                            \
    ../common/tivx_init.c                           \
    ../common/tivx_host.c                           \
    ../common/tivx_platform.c                       \
    ../common/tivx_mem.c                            \
    tivx_platform_hlos.c                            \
    ../common/tivx_perf.c

IDIRS       += $(TIOVX_PATH)/source/include
IDIRS       += $(TIOVX_PATH)/source/platform/psdk_j7/common
IDIRS       += $(TIOVX_PATH)/source/platform/os/posix
IDIRS       += $(APP_UTILS_PATH)

DEFS += TARGET_HLOS

include $(FINALE)

endif
endif