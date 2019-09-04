
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 R5F C66))

include $(PRELUDE)
TARGET      := vx_target_kernels_j7_arm
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(J7_C_MODELS_PATH)/include

ifeq ($(BUILD_HWA_DMPAC_DOF),yes)
DEFS += BUILD_HWA_DMPAC_DOF
endif

ifeq ($(TARGET_CPU)$(BUILD_VLAB),R5Fyes)
DEFS += VLAB_HWA
endif

include $(FINALE)

endif
