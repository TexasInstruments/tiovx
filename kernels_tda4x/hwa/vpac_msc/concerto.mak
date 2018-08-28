
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 R5F))

include $(PRELUDE)
TARGET      := vx_target_kernels_vpac_msc
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(HOST_ROOT)/kernels/openvx-core/include
IDIRS       += $(TDA4X_C_MODELS_PATH)/include
IDIRS       += $(VXLIB_PATH)/packages

include $(FINALE)

endif
