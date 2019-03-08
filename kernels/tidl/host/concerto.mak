ifeq ($(BUILD_SDK), $(filter $(BUILD_SDK), vsdk psdk))
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 A15 M4))

include $(PRELUDE)
TARGET      := vx_kernels_tidl
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/tidl/include
IDIRS       += $(XDIAS_PATH)/packages
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIOVX_PATH)/utils/include
IDIRS       += $(EVE_SW_PATH)/
IDIRS       += $(EVE_SW_PATH)/common
IDIRS       += $(TIDL_PATH)/inc

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_WINDOWS GCC_LINUX GCC_SYSBIOS_ARM GCC_LINUX_ARM))
CFLAGS += -Wno-unused-function
endif

ifeq ($(TARGET_PLATFORM),PC)
CFLAGS += -DHOST_EMULATION
endif

include $(FINALE)

endif
endif
