
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C66))

include $(PRELUDE)
TARGET      := vx_target_kernels_dsp
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/include
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/test_kernels/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages

DEFS += CORE_DSP

ifeq ($(BUILD_BAM),yes)
DEFS += BUILD_BAM
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION
endif

include $(FINALE)

endif

ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J721S2 J784S4 AM62A J722S))

ifeq ($(TARGET_FAMILY),DSP)

ifneq ($(TARGET_CPU),C66)

include $(PRELUDE)
TARGET      := vx_target_kernels_dsp
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
CSOURCES    += ../arm/vx_test_target_target.c
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/include
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/test_kernels/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages

DEFS += CORE_DSP

ifeq ($(BUILD_BAM),yes)
DEFS += BUILD_BAM
endif

DEFS += C6X_MIGRATION _TMS320C6600

include $(FINALE)

endif # ifneq ($(TARGET_CPU),C66)

endif # ifeq ($(TARGET_FAMILY),DSP)

endif # ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J721S2 J784S4 AM62A J722S))
