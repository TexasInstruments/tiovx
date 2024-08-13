
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))

include $(PRELUDE)
TARGET      := vx_platform_psdk_j7
TARGETTYPE  := library

COMMON_FILES_REL_PATH = ../../os/posix

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
DEFS        += LDRA_UNTESTABLE_CODE
# This is used to signify which sections of code is only applicable
# for the host for code coverage purposes. It has been left defined
# for all cores, but can be wrapped in the appropriate CPU when generating
# code coverage reports.
DEFS        += HOST_ONLY

DEFS += TARGET_HLOS

include $(FINALE)

endif
endif