
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72))

ifeq ($(BUILD_IVISION_KERNELS),yes)

include $(PRELUDE)
TARGET      := vx_kernels_ivision
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/ivision/include

include $(FINALE)

endif

endif
