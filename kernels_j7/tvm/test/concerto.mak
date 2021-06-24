ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), A72))

ifeq ($(BUILD_CONFORMANCE_TEST),yes)

include $(PRELUDE)
TARGET      := vx_tiovx_tvm_tests
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIOVX_PATH)/conformance_tests
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIDL_PATH)/inc
IDIRS       += $(TIOVX_PATH)/utils/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/tidl/include

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
CFLAGS += --diag_suppress=179
CFLAGS += --diag_suppress=112
CFLAGS += --diag_suppress=552
endif

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_LINARO GCC_WINDOWS GCC_LINUX GCC_SYSBIOS_ARM GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security
endif

include $(FINALE)

endif

endif
