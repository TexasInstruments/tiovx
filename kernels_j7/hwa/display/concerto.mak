
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F))
ifeq ($(BUILD_HWA_KERNELS),yes)
ifeq ($(BUILD_DISPLAY),yes)

include $(PRELUDE)
TARGET      := vx_target_kernels_display
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(PDK_PATH)/packages

ifeq ($(TARGET_OS),SYSBIOS)
IDIRS       += $(XDCTOOLS_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages
endif

ifeq ($(TARGET_CPU)$(BUILD_VLAB),R5Fyes)
DEFS += VLAB_HWA
endif

include $(FINALE)

endif
endif
endif
