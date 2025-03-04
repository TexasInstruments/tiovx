
ifeq ($(SOC), j6)
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)
TARGET      := vx_sample_usecases
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

include $(FINALE)

endif
endif