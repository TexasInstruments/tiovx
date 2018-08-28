
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 EVE))

ifeq ($(BUILD_IVISION_KERNELS),yes)

include $(PRELUDE)
TARGET      := vx_target_kernels_ivision
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/ivision/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(XDIAS_PATH)/packages
IDIRS       += $(EVE_SW_PATH)/
IDIRS       += $(EVE_SW_PATH)/common
IDIRS       += $(VXLIB_PATH)/packages

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
endif

include $(FINALE)

endif

endif
