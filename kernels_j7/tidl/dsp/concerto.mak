
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C71 C7120 C7504))

include $(PRELUDE)
TARGET      := vx_target_kernels_tidl
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIOVX_PATH)/kernels/ivision/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/tidl/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIDL_PATH)/inc
IDIRS       += $(TIDL_PATH)/custom
IDIRS       += $(VXLIB_PATH)/packages
ifeq ($(RTOS_SDK), mcu_plus_sdk)
  IDIRS       += $(MCU_PLUS_SDK_PATH)/source
  IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers
  IDIRS       += $(MCU_PLUS_SDK_PATH)/source/kernel/dpl
else
  IDIRS       += $(PDK_PATH)/packages
  IDIRS       += $(PDK_PATH)/packages/ti/drv
  IDIRS       += $(PDK_PATH)/packages/ti/drv/udma
  IDIRS       += $(PDK_PATH)/packages/ti/osal
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION -D_TMS320C6X

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64))
DEFS        += __aarch64__
endif

endif

include $(FINALE)

endif
