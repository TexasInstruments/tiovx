
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72 A53 R5F))
ifeq ($(BUILD_CONFORMANCE_TEST),yes)
ifeq ($(BUILD_VIDEO_IO_KERNELS),yes)

include $(PRELUDE)
TARGET      := vx_kernels_video_io_tests
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIOVX_PATH)/conformance_tests
IDIRS       += $(TIOVX_PATH)/conformance_tests/test_tiovx
IDIRS       += $(TIOVX_PATH)/utils/include
IDIRS       += $(APP_UTILS_PATH)/
IDIRS       += $(IMAGING_PATH)/

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

ifeq ($(BUILD_CAPTURE),yes)
DEFS += BUILD_CAPTURE
endif

ifeq ($(BUILD_CSITX),yes)
DEFS += BUILD_CSITX
endif

ifeq ($(BUILD_DISPLAY),yes)
DEFS += BUILD_DISPLAY
endif

include $(FINALE)

endif
endif
endif
