
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72))

include $(PRELUDE)
TARGET      := vx_sample_usecases
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

include $(FINALE)

endif
