
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 R5F))

ifeq ($(BUILD_HWA_DMPAC_DOF),yes)

include $(PRELUDE)
TARGET      := vx_target_kernels_dmpac_dof
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(TDA4X_C_MODELS_PATH)/include

ifeq ($(TARGET_CPU)$(BUILD_VLAB),R5Fyes)
DEFS += VLAB_HWA
endif

include $(FINALE)

endif
# ifeq ($(BUILD_HWA_DMPAC_DOF),yes)

endif
