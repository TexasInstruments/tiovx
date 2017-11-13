include $(PRELUDE)
TARGET      := vx_target_kernels_arm
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(TDA4x_C_MODELS_PATH)/include

ifeq ($(BUILD_HWA_VPAC_NF),yes)
DEFS += BUILD_HWA_VPAC_NF
endif
ifeq ($(BUILD_HWA_DMPAC_SDE),yes)
DEFS += BUILD_HWA_DMPAC_SDE
endif
ifeq ($(BUILD_HWA_VPAC_LDC),yes)
DEFS += BUILD_HWA_VPAC_LDC
endif
ifeq ($(BUILD_HWA_VPAC_MSC),yes)
DEFS += BUILD_HWA_VPAC_MSC
endif
ifeq ($(BUILD_HWA_VPAC_VISS),yes)
DEFS += BUILD_HWA_VPAC_VISS
endif
ifeq ($(BUILD_HWA_DMPAC_DOF),yes)
DEFS += BUILD_HWA_DMPAC_DOF
endif

ifeq ($(TARGET_CPU),C66)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),EVE)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),A15)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),M4)
SKIPBUILD=1
endif

include $(FINALE)

