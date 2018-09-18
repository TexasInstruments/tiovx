
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72))

include $(PRELUDE)
TARGET      := vx_kernels_hwa_tests
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/conformance_tests
IDIRS       += $(HOST_ROOT)/utils/include

ifeq ($(HOST_COMPILER),GCC_LINUX)
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security
endif

ifeq ($(HOST_COMPILER),GCC_SYSBIOS_ARM)
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security
endif

include $(FINALE)

endif
