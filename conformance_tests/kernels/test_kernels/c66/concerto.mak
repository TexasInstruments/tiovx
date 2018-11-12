include $(PRELUDE)
TARGET      := vx_target_kernels_c66
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/include
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/test_kernels/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
# < DEVELOPER_TODO: Add any custom include paths using 'IDIRS' >
# < DEVELOPER_TODO: Add any custom preprocessor defines or build options needed using
#                   'CFLAGS'. >
# < DEVELOPER_TODO: Adjust which cores this library gets built on using 'SKIPBUILD'. >

ifeq ($(TARGET_CPU),C66)
DEFS += CORE_DSP
endif

ifeq ($(BUILD_BAM),yes)
DEFS += BUILD_BAM
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION
endif

ifeq ($(TARGET_CPU),C66)
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),EVE)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),A15)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),M4)
SKIPBUILD=1
endif

include $(FINALE)

