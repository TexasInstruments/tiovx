


include $(PRELUDE)
TARGET      := vx_target_kernels_openvx_core
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/openvx-core/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(HOST_ROOT)/kernels/openvx-core/c66x

ifeq ($(BUILD_BAM),yes)
DEFS += BUILD_BAM
endif

ifeq ($(TARGET_CPU),X86)
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
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
