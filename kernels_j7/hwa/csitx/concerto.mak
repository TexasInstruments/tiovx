
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F ))

include $(PRELUDE)
TARGET      := vx_target_kernels_csitx
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(PDK_PATH)/packages
IDIRS       += $(XDCTOOLS_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages

DEFS+=SOC_J721E

include $(FINALE)
endif
