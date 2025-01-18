
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), FREERTOS SAFERTOS))

include $(PRELUDE)
TARGET      := vx_platform_psdk_j7_rtos
TARGETTYPE  := library

COMMON_FILES_REL_PATH = ../common
TARGET_FILES_REL_PATH = ../../targets

ifeq ($(LDRA_COVERAGE_ENABLED), yes)
include $(TIOVX_PATH)/tiovx_dev/internal_docs/coverage_files/concerto_inc.mak
else
DEFS += HOST_ONLY
DEFS += LDRA_UNTESTABLE_CODE
endif

CSOURCES    := 	tivx_event.c \
				tivx_queue.c \
				tivx_task.c \
				tivx_mutex.c \
				tivx_platform_rtos.c \
				$(COMMON_FILES_REL_PATH)/tivx_init.c \
				$(COMMON_FILES_REL_PATH)/tivx_mem.c \
				$(COMMON_FILES_REL_PATH)/tivx_ipc.c \
				$(COMMON_FILES_REL_PATH)/tivx_platform.c \
				$(COMMON_FILES_REL_PATH)/tivx_host.c \
				$(COMMON_FILES_REL_PATH)/tivx_perf.c \
				$(TARGET_FILES_REL_PATH)/tivx_target_config.c

IDIRS       += $(HOST_ROOT)/source/include
IDIRS       += $(HOST_ROOT)/source/platform/targets
IDIRS       += $(CUSTOM_KERNEL_PATH)/include

ifeq ($(RTOS_SDK), mcu_plus_sdk)
    IDIRS       += $(MCU_PLUS_SDK_PATH)/source
    IDIRS       += $(MCU_PLUS_SDK_PATH)/source/kernel/dpl
else
    IDIRS       += $(PDK_PATH)/packages
    IDIRS       += $(PDK_PATH)/packages/ti/osal
endif
IDIRS       += $(APP_UTILS_PATH)

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
endif

ifeq ($(HOST_COMPILER),GCC)

endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), C66))
CSOURCES += $(TARGET_FILES_REL_PATH)/tivx_target_config_c66.c
SKIPBUILD=0
endif

ifeq ($(TARGET_FAMILY),DSP)
ifneq ($(TARGET_CPU),C66)
CSOURCES += $(TARGET_FILES_REL_PATH)/tivx_target_config_c7.c
SKIPBUILD=0
endif
endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
CSOURCES += $(TARGET_FILES_REL_PATH)/tivx_target_config_mpu1_0.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),R5F)
CSOURCES += $(TARGET_FILES_REL_PATH)/r5f/tivx_target_config_r5f_${SOC}.c
SKIPBUILD=0
endif

include $(FINALE)

endif
