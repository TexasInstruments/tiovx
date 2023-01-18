ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7120 C7504))

include $(PRELUDE)
TARGET      := vx_target_kernels_tvm
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIOVX_PATH)/kernels/ivision/include
#IDIRS       += $(CUSTOM_KERNEL_PATH)/tvm/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(TIOVX_PATH)/source/include
IDIRS       += $(PDK_PATH)/packages

ifeq ($(TARGET_OS),SYSBIOS)
	IDIRS       += $(XDCTOOLS_PATH)/packages
	IDIRS       += $(BIOS_PATH)/packages
endif

include $(FINALE)

endif
