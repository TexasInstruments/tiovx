

include $(PRELUDE)
TARGET      := vx_sample_usecases
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

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