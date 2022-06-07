
ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J7 J721S2 J784S4 AM62A))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 R5F C66 C71 C7120 C7504))
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), SYSBIOS FREERTOS SAFERTOS))

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
				../common/tivx_host.c

IDIRS       += $(TIOVX_PATH)/source/include
IDIRS       += $(CUSTOM_PLATFORM_PATH)/psdk_j7/common
IDIRS       += $(CUSTOM_KERNEL_PATH)/include
IDIRS       += $(PDK_PATH)/packages
IDIRS       += $(VISION_APPS_PATH)

ifeq ($(TARGET_OS),SYSBIOS)
IDIRS       += $(XDCTOOLS_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages
endif

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
endif

ifeq ($(HOST_COMPILER),GCC)

endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), C66))
CSOURCES += tivx_target_config_c66.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), C71 C7120 C7504))
CSOURCES += tivx_target_config_c7.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),A72)
CSOURCES += ../common/tivx_target_config_mpu1_0.c
SKIPBUILD=0
endif

ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J7 J721S2 J784S4 AM62A))
ifeq ($(TARGET_CPU),R5F)
CSOURCES += tivx_target_config_r5f.c
SKIPBUILD=0
endif
endif

include $(FINALE)

endif
endif
endif
