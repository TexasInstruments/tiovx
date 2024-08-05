
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C66 A15 M4 A72 A53 R5F))

include $(PRELUDE)
TARGET      := vx_target_kernels_source_sink
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIOVX_PATH)/conformance_tests/kernels/include
IDIRS       += $(TIOVX_PATH)/conformance_tests/kernels/test_kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(TIOVX_PATH)/source/include
IDIRS       += $(TIOVX_PATH)/source/platform/psdk_j7/common
IDIRS       += $(PSDK_PATH)/app_utils
IDIRS       += $(TIOVX_PATH)/source/platform/os/posix

ifeq ($(RTOS_SDK), mcu_plus_sdk)
IDIRS       += $(APP_UTILS_PATH)/utils/rtos/src
else
IDIRS       += $(PDK_PATH)/packages/ti/osal/soc/$(SOC)
endif

include $(FINALE)

endif
