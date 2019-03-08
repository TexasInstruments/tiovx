
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F))

include $(PRELUDE)
TARGET      := vx_target_kernels_display
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

ifeq ($(TARGET_CPU)$(BUILD_VLAB),R5Fyes)
DEFS += VLAB_HWA
endif

include $(FINALE)

endif
