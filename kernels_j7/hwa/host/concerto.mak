
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72 R5F))
ifeq ($(BUILD_HWA_KERNELS),yes)

include $(PRELUDE)
TARGET      := vx_kernels_hwa
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(J7_C_MODELS_PATH)/include
IDIRS       += $(TIOVX_PATH)/source/include

ifeq ($(BUILD_VPAC3),yes)
DEFS        += VPAC3
endif

ifeq ($(BUILD_HWA_DMPAC_DOF),yes)
DEFS += BUILD_HWA_DMPAC_DOF
endif

include $(FINALE)

endif
endif
