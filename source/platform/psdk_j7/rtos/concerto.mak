
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), FREERTOS SAFERTOS))

include $(PRELUDE)
TARGET      := vx_platform_psdk_j7_rtos
TARGETTYPE  := library

CSOURCES    := 	tivx_event.c \
				tivx_queue.c \
				tivx_task.c \
				tivx_mutex.c \
				tivx_platform_rtos.c \
				../common/tivx_init.c \
				../common/tivx_mem.c \
				../common/tivx_ipc.c \
				../common/tivx_platform.c \
				../common/tivx_host.c \
				../common/tivx_perf.c

IDIRS       += $(HOST_ROOT)/source/include
IDIRS       += $(HOST_ROOT)/source/platform/psdk_j7/common
IDIRS       += $(CUSTOM_KERNEL_PATH)/include
DEFS        += LDRA_UNTESTABLE_CODE
# This is used to signify which sections of code is only applicable
# for the host for code coverage purposes. It has been left defined
# for all cores, but can be wrapped in the appropriate CPU when generating
# code coverage reports.
DEFS        += HOST_ONLY

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
CSOURCES += tivx_target_config_c66.c
SKIPBUILD=0
endif

ifeq ($(TARGET_FAMILY),DSP)
ifneq ($(TARGET_CPU),C66)
CSOURCES += tivx_target_config_c7.c
SKIPBUILD=0
endif
endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
CSOURCES += ../common/tivx_target_config_mpu1_0.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),R5F)
CSOURCES += soc/tivx_target_config_r5f_${SOC}.c
SKIPBUILD=0
endif

include $(FINALE)

endif
