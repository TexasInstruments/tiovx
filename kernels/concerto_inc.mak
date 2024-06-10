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
else
STATIC_LIBS += $(C7X_VERSION)-host-emulation
endif

SYS_SHARED_LIBS += rt dl png z

LDIRS+=$(RECOMPUTE_PATH)/host_emulation

LDIRS       += $(APP_UTILS_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
STATIC_LIBS += app_utils_mem
LDIRS       += $(VISION_APPS_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
STATIC_LIBS += app_utils_init

endif #ifeq ($(TARGET_CPU),x86_64)


ifneq ($(CUSTOM_KERNEL_PATH),)
include $(CUSTOM_KERNEL_PATH)/concerto_inc.mak
endif

ifneq ($(CUSTOM_APPLICATION_PATH),)
include $(CUSTOM_APPLICATION_PATH)/concerto_inc.mak
endif
