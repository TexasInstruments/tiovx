ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J7 J721S2))
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F))
ifeq ($(BUILD_HWA_KERNELS),yes)
ifeq ($(BUILD_DISPLAY),yes)

include $(PRELUDE)
TARGET      := vx_target_kernels_display_m2m
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(PDK_PATH)/packages
IDIRS       += $(TIOVX_PATH)/source/include

ifeq ($(TARGET_OS),SYSBIOS)
IDIRS       += $(XDCTOOLS_PATH)/packages
IDIRS       += $(BIOS_PATH)/packages
endif

include $(FINALE)

endif
endif
endif
endif

