# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

ifeq ($(TARGET_CPU),x86_64)

	STATIC_LIBS += vx_vxu

	STATIC_LIBS += vx_utils
	STATIC_LIBS += vx_target_kernels_ivision_common

	ifeq ($(SOC), j722s)
		STATIC_LIBS += C7524-MMA2_256-host-emulation
	else ifeq ($(SOC), tda54)
		STATIC_LIBS += C7604-MMA3_1024-host-emulation
	else
		STATIC_LIBS += $(C7X_VERSION)-host-emulation
	endif

	SYS_SHARED_LIBS += rt dl png z

	LDIRS       += $(CGT7X_ROOT)/host_emulation
	LDIRS       += $(APP_UTILS_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
	LDIRS       += $(VISION_APPS_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)

	STATIC_LIBS += app_utils_mem
	STATIC_LIBS += app_utils_init

	ifeq ($(SOC_FAMILY), SOC_FAMILY_TDA5))
		STATIC_LIBS += app_utils_init_vdk_stub
		STATIC_LIBS += app_utils_console_io
		STATIC_LIBS += app_utils_timer
		STATIC_LIBS += app_utils_ipc
		STATIC_LIBS += app_utils_misc
		STATIC_LIBS += app_utils_pc_osal
	endif

	ifeq ($(SOC_FAMILY), SOC_FAMILY_TDA5)
		ifeq ($(TARGET_BUILD),debug)
			LDIRS += $(MCU_SDK_PATH)/build/$(SOC)/lib/Debug
		else
			LDIRS += $(MCU_SDK_PATH)/build/$(SOC)/lib/Release
		endif
		ADDITIONAL_STATIC_LIBS += libarch-ti_sdk_cfg_default_hostemu_gcc-linux.a
		ADDITIONAL_STATIC_LIBS += libdrivers-ti_sdk_cfg_default_hostemu_gcc-linux.a
		ADDITIONAL_STATIC_LIBS += libhal-ti_sdk_cfg_default_hostemu_gcc-linux.a
		ADDITIONAL_STATIC_LIBS += libtda54_hostemu_gcc-linux.a
		ADDITIONAL_STATIC_LIBS += libti_sdk_cfg_default_hostemu_gcc-linux.a
		ADDITIONAL_STATIC_LIBS += libutils_nortos-ti_sdk_cfg_default_hostemu_gcc-linux.a
	endif # ifeq ($(SOC_FAMILY), SOC_FAMILY_TDA5)

endif #ifeq ($(TARGET_CPU),x86_64)


ifneq ($(CUSTOM_KERNEL_PATH),)
	include $(CUSTOM_KERNEL_PATH)/concerto_inc.mak
endif

ifneq ($(CUSTOM_APPLICATION_PATH),)
	include $(CUSTOM_APPLICATION_PATH)/concerto_inc.mak
endif
