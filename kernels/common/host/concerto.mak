
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 R5F))

include $(PRELUDE)
TARGET      := vx_kernels_host_utils
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

include $(FINALE)

endif
