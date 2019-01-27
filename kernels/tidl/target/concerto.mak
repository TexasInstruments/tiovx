ifeq ($(BUILD_SDK), $(filter $(BUILD_SDK), vsdk psdk))
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 C66 EVE))
include $(PRELUDE)
TARGET      := vx_target_kernels_tidl
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/tidl/include
IDIRS       += $(HOST_ROOT)/kernels/ivision/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(XDIAS_PATH)/packages
IDIRS       += $(IVISION_PATH)
IDIRS       += $(EVE_SW_PATH)/
IDIRS       += $(EVE_SW_PATH)/common
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(TIDL_PATH)/inc
IDIRS       += $(SDK_PLATFORM_IF_PATH)

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C66))
CFLAGS      += -DBUILD_DSP
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), EVE))
CFLAGS      += -DBUILD_ARP32
endif

include $(FINALE)

endif
endif
