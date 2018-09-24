
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72 R5F))

include $(PRELUDE)
TARGET      := vx_kernels_host_utils
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

include $(FINALE)

endif
