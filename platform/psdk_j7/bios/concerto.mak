
ifeq ($(TARGET_PLATFORM), J7)
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 R5F C66 C71))
ifeq ($(TARGET_OS),SYSBIOS)

include $(PRELUDE)
TARGET      := vx_platform_psdk_j7_bios
TARGETTYPE  := library

CSOURCES    := 	tivx_event.c \
				tivx_queue.c \
				tivx_task.c \
				tivx_mutex.c \
				tivx_platform_bios.c \
				../common/tivx_init.c \
				../common/tivx_mem.c \
				../common/tivx_ipc.c \
				../common/tivx_platform.c \
				../common/tivx_platform_common.c \
				../common/tivx_host.c

IDIRS       += $(HOST_ROOT)/source/include
IDIRS       += $(CUSTOM_PLATFORM_PATH)/psdk_j7/common
IDIRS       += $(CUSTOM_KERNEL_PATH)/include
IDIRS       += $(PDK_PATH)/packages
IDIRS       += $(XDCTOOLS_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages
IDIRS       += $(VISION_APPS_PATH)

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
endif

ifeq ($(HOST_COMPILER),GCC)

endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), C71 C66))
CSOURCES += tivx_target_config_dsp.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),A72)
CSOURCES += ../common/tivx_target_config_mpu1_0.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),R5F)
CSOURCES += tivx_target_config_mcu2_0.c
SKIPBUILD=0
endif

include $(FINALE)

endif
endif
endif
