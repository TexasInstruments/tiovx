
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72))

include $(PRELUDE)
TARGET      := vx_hwa_target_kernels
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(VXLIB_PATH)/packages

include $(FINALE)
endif

