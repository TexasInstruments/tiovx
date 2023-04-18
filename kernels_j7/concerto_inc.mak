# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

ifeq ($(TARGET_CPU),x86_64)

STATIC_LIBS += vx_vxu

STATIC_LIBS += vx_utils
STATIC_LIBS += vx_tiovx_tidl_tests
STATIC_LIBS += vx_kernels_tidl vx_target_kernels_tidl vx_target_kernels_ivision_common tidl_algo tidl_priv_algo tidl_obj_algo tidl_custom tidl_avx_kernels
STATIC_LIBS += $(C7X_VERSION)-host-emulation

# Uncomment below to link to TIDL/MMALIB in host emulation mode instead of natural C mode on PC
# STATIC_LIBS += mmalib_x86_64 mmalib_cn_x86_64 common_x86_64
# ADDITIONAL_STATIC_LIBS += dmautils.lib udma.lib sciclient.lib ti.csl.lib ti.osal.lib

SYS_SHARED_LIBS += rt dl png z

LDIRS+=$(CGT7X_ROOT)/host_emulation
LDIRS+=$(MMALIB_PATH)/lib/$(C7X_VERSION)/$(TARGET_BUILD)
LDIRS+=$(TIDL_PATH)/lib/$(SOC)/PC/algo/$(TARGET_BUILD)

ifeq ($(RTOS_SDK), mcu_plus_sdk)
LDIRS+= $(MCU_PLUS_SDK_PATH)/source/drivers/dmautils/lib/
else
LDIRS+= $(PDK_PATH)/packages/ti/drv/udma/lib/$(SOC)_hostemu/c7x-hostemu/$(TARGET_BUILD)
LDIRS+= $(PDK_PATH)/packages/ti/csl/lib/$(SOC)/c7x-hostemu/$(TARGET_BUILD)
LDIRS+= $(PDK_PATH)/packages/ti/drv/sciclient/lib/$(SOC)_hostemu/c7x-hostemu/$(TARGET_BUILD)
LDIRS+= $(PDK_PATH)/packages/ti/osal/lib/nonos/$(SOC)/c7x-hostemu/$(TARGET_BUILD)
endif

LDIRS       += $(APP_UTILS_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
STATIC_LIBS += app_utils_mem
LDIRS       += $(VISION_APPS_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
STATIC_LIBS += app_utils_init

PDK_LIBS =

ifeq ($(RTOS_SDK), mcu_plus_sdk)
	PDK_LIBS += dmautils.am62ax.c75x.ti-c7x-hostemu.$(TARGET_BUILD).lib
else
	PDK_LIBS += dmautils.lib
	PDK_LIBS += ti.csl.lib
endif

ifneq ($(SOC),am62a)
PDK_LIBS += udma.lib
PDK_LIBS += sciclient.lib
PDK_LIBS += ti.osal.lib
endif

MMA_LIBS =
MMA_LIBS += mmalib_cn_x86_64
MMA_LIBS += mmalib_x86_64
MMA_LIBS += common_x86_64

ADDITIONAL_STATIC_LIBS += $(PDK_LIBS)

STATIC_LIBS += $(MMA_LIBS)

endif #ifeq ($(TARGET_CPU),x86_64)
