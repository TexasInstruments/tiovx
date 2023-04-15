
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72 A53 R5F))
ifeq ($(BUILD_HWA_KERNELS),yes)

include $(PRELUDE)
TARGET      := vx_kernels_hwa
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(J7_C_MODELS_PATH)/include
IDIRS       += $(TIOVX_PATH)/source/include

ifeq ($(BUILD_DMPAC_DOF),yes)
DEFS += BUILD_DMPAC_DOF
endif

ifeq ($(BUILD_DMPAC_SDE),yes)
DEFS += BUILD_DMPAC_SDE
endif

ifeq ($(BUILD_VPAC_VISS),yes)
DEFS += BUILD_VPAC_VISS
endif

ifeq ($(BUILD_VPAC_MSC),yes)
DEFS += BUILD_VPAC_MSC
endif

ifeq ($(BUILD_VPAC_LDC),yes)
DEFS += BUILD_VPAC_LDC
endif

ifeq ($(BUILD_VPAC_NF),yes)
DEFS += BUILD_VPAC_NF
endif

include $(FINALE)

endif
endif
