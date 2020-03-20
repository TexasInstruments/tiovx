
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72 R5F))
ifeq ($(BUILD_CONFORMANCE_TEST),yes)

include $(PRELUDE)
TARGET      := vx_kernels_hwa_tests
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/conformance_tests
IDIRS       += $(HOST_ROOT)/utils/include
IDIRS       += $(VISION_APPS_PATH)/
IDIRS       += $(IMAGING_PATH)/

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_LINARO GCC_WINDOWS GCC_LINUX GCC_SYSBIOS_ARM GCC_LINUX_ARM))
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


ifeq ($(BUILD_CT_TIOVX_HWA_NEGATIVE_TESTS),yes)
CFLAGS      += -DBUILD_CT_TIOVX_HWA_NEGATIVE_TESTS
endif

include $(FINALE)

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
vision_apps_utils:
	BUILD_TARGET_MODE=no make -C $(VISION_APPS_PATH) app_utils_iss cp_to_lib
endif

endif
endif
