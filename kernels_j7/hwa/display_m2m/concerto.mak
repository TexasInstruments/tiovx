
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 M4 R5F ))

include $(PRELUDE)
TARGET      := vx_target_kernels_display_m2m
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
# < DEVELOPER_TODO: Add any custom include paths using 'IDIRS' >
# < DEVELOPER_TODO: Add any custom preprocessor defines or build options needed using
#                   'CFLAGS'. >
# < DEVELOPER_TODO: Adjust which cores this library gets built on using 'SKIPBUILD'. >

ifeq ($(TARGET_CPU),C66)
DEFS += CORE_DSP
endif

ifeq ($(BUILD_BAM),yes)
DEFS += BUILD_BAM
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION
endif

include $(FINALE)
endif

