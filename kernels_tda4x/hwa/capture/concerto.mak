
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), M4 R5F A72 A15))

include $(PRELUDE)
TARGET      := vx_target_kernels_capture_tda4
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(PDK_PATH)/packages
IDIRS       += $(XDCTOOLS_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages
#IDIRS       += $(VISION_APPS_PATH)

DEFS+=SOC_J7

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION
endif

include $(FINALE)
endif

