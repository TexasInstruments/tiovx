
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72))

include $(PRELUDE)
TARGET      := vx_kernels_hwa
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(TDA4X_C_MODELS_PATH)/include

ifeq ($(BUILD_HWA_DMPAC_DOF),yes)
DEFS += BUILD_HWA_DMPAC_DOF
endif

include $(FINALE)

endif
