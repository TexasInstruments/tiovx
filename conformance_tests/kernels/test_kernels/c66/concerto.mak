
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

ifeq ($(TARGET_FAMILY),DSP)

ifneq ($(TARGET_CPU),C66)

include $(PRELUDE)
TARGET      := vx_target_kernels_dsp
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
CSOURCES    += ../arm/vx_test_target_target.c
CSOURCES    += ../arm/vx_tiovx_overhead_target.c
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/include
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/test_kernels/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(TIOVX_PATH)/source/include
IDIRS       += $(TIOVX_PATH)/source/platform/psdk_j7/common
IDIRS       += $(PSDK_PATH)/app_utils

ifeq ($(RTOS_SDK), mcu_plus_sdk)
IDIRS       += $(APP_UTILS_PATH)/utils/rtos/src
else
ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J742S2))
IDIRS       += $(PDK_PATH)/packages/ti/osal/soc
else
IDIRS       += $(PDK_PATH)/packages/ti/osal/soc/${SOC}
endif
endif

DEFS += CORE_DSP

ifeq ($(BUILD_BAM),yes)
DEFS += BUILD_BAM
endif

DEFS += C6X_MIGRATION _TMS320C6600

include $(FINALE)

endif # ifneq ($(TARGET_CPU),C66)

endif # ifeq ($(TARGET_FAMILY),DSP)
