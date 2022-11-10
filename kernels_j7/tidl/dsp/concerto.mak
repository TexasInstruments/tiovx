
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C71 C7120 C7504))

include $(PRELUDE)
TARGET      := vx_target_kernels_tidl
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIOVX_PATH)/kernels/ivision/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/tidl/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIDL_PATH)/inc
IDIRS       += $(TIDL_PATH)/custom
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(PDK_PATH)/packages

ifeq ($(TARGET_OS),SYSBIOS)
	IDIRS       += $(XDCTOOLS_PATH)/packages
	IDIRS       += $(BIOS_PATH)/packages
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION -D_TMS320C6X

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64))
DEFS        += __aarch64__
endif

endif

include $(FINALE)

endif
