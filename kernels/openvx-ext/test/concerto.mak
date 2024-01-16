
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72 A53 R5F))
ifeq ($(BUILD_CONFORMANCE_TEST),yes)

include $(PRELUDE)
TARGET      := vx_kernels_openvx_ext_tests
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIOVX_PATH)/conformance_tests
IDIRS       += $(TIOVX_PATH)/conformance_tests/test_tiovx
IDIRS       += $(TIOVX_PATH)/conformance_tests/kernels/include
IDIRS       += $(TIOVX_PATH)/conformance_tests/kernels/test_kernels/include
IDIRS       += $(TIOVX_PATH)/utils/include
IDIRS       += $(APP_UTILS_PATH)/

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_LINARO GCC_WINDOWS GCC_LINUX GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security
endif

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
CFLAGS += --diag_suppress=179
CFLAGS += --diag_suppress=112
CFLAGS += --diag_suppress=552
endif

include $(FINALE)

endif
endif
