include $(PRELUDE)
TARGET      := vx_kernels_hwa
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include

ifeq ($(TARGET_CPU),C66)
SKIPBUILD=1
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
