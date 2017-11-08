# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

ifneq ($(CUSTOM_KERNEL_PATH),)
include $(CUSTOM_KERNEL_PATH)/concerto_inc.mak
endif

ifneq ($(CUSTOM_APPLICATION_PATH),)
include $(CUSTOM_APPLICATION_PATH)/concerto_inc.mak
endif

ifeq ($(BUILD_IVISION_KERNELS),yes)
STATIC_LIBS += vx_tiovx_ivision_tests vx_kernels_ivision
endif
