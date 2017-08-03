ifeq ($(BUILD_IVISION_KERNELS),yes)
ifeq ($(BUILD_CONFORMANCE_TEST),yes)

include $(PRELUDE)
TARGET      := vx_tiovx_ivision_tests
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/conformance_tests

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
CFLAGS += --diag_suppress=179
CFLAGS += --diag_suppress=112
CFLAGS += --diag_suppress=552
endif

ifeq ($(HOST_COMPILER),GCC)
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security 
endif

ifeq ($(HOST_COMPILER),GCC_LINARO)
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security 
endif

ifeq ($(HOST_COMPILER),GCC_LINUX)
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security 
endif

ifeq ($(HOST_COMPILER),GCC_WINDOWS)
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
endif

ifeq ($(TARGET_CPU),C66)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),EVE)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),A15)
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),M4)
SKIPBUILD=0
endif

include $(FINALE)

endif
endif