


include $(PRELUDE)
TARGET      := vx_target_kernels_ivision
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/ivision/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(XDIAS_PATH)/packages
IDIRS       += $(EVE_SW_PATH)/
IDIRS       += $(EVE_SW_PATH)/common


ifeq ($(TARGET_CPU),X86)
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
endif

ifeq ($(TARGET_CPU),C66)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),EVE)
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),A15)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),M4)
SKIPBUILD=1
endif


include $(FINALE)
