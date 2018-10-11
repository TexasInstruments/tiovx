
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 EVE))

include $(PRELUDE)
TARGET      := vx_target_kernels_ivision_common
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

IDIRS       += $(HOST_ROOT)/kernels/ivision/include
IDIRS       += $(XDIAS_PATH)/packages
IDIRS       += $(EVE_SW_PATH)/
IDIRS       += $(EVE_SW_PATH)/common
IDIRS       += $(IVISION_PATH)/

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
endif

ifeq ($(HOST_COMPILER),GCC_LINUX)
CFLAGS += -Wno-switch
endif


include $(FINALE)

endif
