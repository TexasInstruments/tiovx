ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7120 C7504))

include $(PRELUDE)
TARGET      := vx_target_kernels_tvm
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIOVX_PATH)/kernels/ivision/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(TIOVX_PATH)/source/include
ifeq ($(RTOS_SDK), mcu_plus_sdk)
	IDIRS       += $(MCU_PLUS_SDK_PATH)/source
	IDIRS       += $(MCU_PLUS_SDK_PATH)/source/kernel/dpl
else
	IDIRS       += $(PDK_PATH)/packages
	IDIRS       += $(PDK_PATH)/packages/ti/osal
endif
ifeq ($(TARGET_OS),SYSBIOS)
	IDIRS       += $(XDCTOOLS_PATH)/packages
	IDIRS       += $(BIOS_PATH)/packages
endif

include $(FINALE)

endif
