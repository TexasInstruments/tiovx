

ifeq ($(TARGET_PLATFORM),PC)
	ifeq ($(TARGET_OS),LINUX)

		include $(PRELUDE)
		TARGET      := vx_platform_pc
		TARGETTYPE  := library

		OS_FILES_REL_PATH     = ../../common/os/posix
		COMMON_FILES_REL_PATH = ../common
		TARGET_FILES_REL_PATH = ../../common/targets

		COMMON_FILES_BASE_PATH = $(TIOVX_PATH)/source/platform/pc/common


		CSOURCES    := \
			$(OS_FILES_REL_PATH)/tivx_event.c \
			$(OS_FILES_REL_PATH)/tivx_mutex.c \
			$(OS_FILES_REL_PATH)/tivx_posix_objects.c  \
			$(OS_FILES_REL_PATH)/tivx_task.c  \
			$(OS_FILES_REL_PATH)/tivx_queue.c \
			$(COMMON_FILES_REL_PATH)/tivx_mem.c \
			$(COMMON_FILES_REL_PATH)/tivx_ipc.c \
			$(COMMON_FILES_REL_PATH)/tivx_init.c \
			../../common/tivx_host.c \
			$(COMMON_FILES_REL_PATH)/tivx_platform_common.c \
			$(TARGET_FILES_REL_PATH)/tivx_target_config.c \
			$(TARGET_FILES_REL_PATH)/tivx_target_config_pc.c \
			$(TARGET_FILES_REL_PATH)/tivx_target_config_c7.c \
			$(TARGET_FILES_REL_PATH)/tivx_target_config_mpu1_0.c \
			tivx_platform.c

		ifeq ($(SOC),j721e)
			CSOURCES    += $(TARGET_FILES_REL_PATH)/tivx_target_config_c66.c
		endif

		ifeq ($(SOC_FAMILY),SOC_FAMILY_TDA5)
			CSOURCES    += $(TARGET_FILES_REL_PATH)/tivx_target_config_m55.c
			CSOURCES    += $(TARGET_FILES_REL_PATH)/tivx_target_config_r52p.c
		else
			CSOURCES    += $(TARGET_FILES_REL_PATH)/r5f/tivx_target_config_r5f_$(SOC).c
		endif

		DEFS  += LDRA_UNTESTABLE_CODE
		# This is used to signify which sections of code is only applicable
		# for the host for code coverage purposes. It has been left defined
		# for all cores, but can be wrapped in the appropriate CPU when generating
		# code coverage reports.
		DEFS  += HOST_ONLY

		IDIRS += $(TIOVX_PATH)/source/include $(COMMON_FILES_BASE_PATH)
		IDIRS += $(APP_UTILS_PATH)
		IDIRS += $(VISION_APPS_PATH)/platform/$(SOC)/rtos
		IDIRS += $(TIOVX_PATH)/source/platform/common/os/posix
		IDIRS += $(TIOVX_PATH)/source/platform/common/targets
		IDIRS += $(TIOVX_PATH)/source/platform/common

		ifeq ($(RTOS_SDK), mcu_plus_sdk)
			IDIRS += $(MCU_PLUS_SDK_PATH)/source
			DEFS  += MCU_PLUS_SDK
		endif

		ifeq ($(RTOS_SDK), mcu_sdk)
			IDIRS += $(MCU_SDK_PATH)/source/device/tda54/ti_sdk_config/default/Hal_Cfg
			IDIRS += $(MCU_SDK_PATH)/source/compiler/hostemu-gcc-linux
			IDIRS += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/include
			IDIRS += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/vPC/include
			IDIRS += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/v0/include
			IDIRS += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/vPC/include
			IDIRS += $(MCU_SDK_PATH)/source/compatibility/dpl/include
			IDIRS += $(MCU_SDK_PATH)/source/arch/include
			DEFS  += MCU_SDK
		endif

		DEFS  += _DISABLE_TIDL
		IDIRS += $(CUSTOM_KERNEL_PATH)/include

		include $(FINALE)

	endif
endif
