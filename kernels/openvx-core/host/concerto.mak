


include $(PRELUDE)
TARGET      := vx_kernels_openvx_core
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/openvx-core/include

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
