
ifeq ($(TARGET_PLATFORM),TDAX)
ifeq ($(TARGET_OS),SYSBIOS)

include $(PRELUDE)
TARGET      := vx_platform_vision_sdk_bios
TARGETTYPE  := library
CSOURCES    := tivx_event.c tivx_init.c tivx_ipc.c tivx_mem.c \
               tivx_mutex.c tivx_platform.c tivx_platform_common.c \
               tivx_queue.c tivx_task.c tivx_host.c
IDIRS       += $(HOST_ROOT)/source/include
IDIRS       += $(HOST_ROOT)/source/platform/vision_sdk/common
IDIRS       += $(PDK_PATH)/packages/ti/drv/vps/include
IDIRS       += $(XDC_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages
IDIRS       += $(SDK_PLATFORM_IF_PATH)

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
endif

ifeq ($(HOST_COMPILER),GCC)

endif

ifeq ($(TARGET_CPU),C66)
CSOURCES += tivx_target_config_dsp_eve.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),EVE)
CSOURCES += tivx_target_config_dsp_eve.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),A15)
CSOURCES += tivx_target_config_a15.c
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),M4)
CSOURCES += tivx_target_config_ipu1_0.c
SKIPBUILD=0
endif

include $(FINALE)

endif
endif
