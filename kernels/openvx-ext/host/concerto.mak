
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72 A53))

include $(PRELUDE)
TARGET      := vx_kernels_openvx_ext
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       := $(HOST_ROOT)/kernels/include
IDIRS       += $(HOST_ROOT)/kernels/openvx-ext/include

include $(FINALE)
endif

