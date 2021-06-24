ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), A72))

include $(PRELUDE)
TARGET      := vx_kernels_tvm
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/tvm/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIDL_PATH)/inc

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_LINARO GCC_WINDOWS GCC_LINUX GCC_SYSBIOS_ARM GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-unused-function
endif

include $(FINALE)

endif
