
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C66))

include $(PRELUDE)
TARGET      := vx_target_kernels_tidl
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/ivision/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/tidl/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIDL_PATH)/inc
IDIRS       += $(VXLIB_PATH)/packages

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
endif

include $(FINALE)

endif
