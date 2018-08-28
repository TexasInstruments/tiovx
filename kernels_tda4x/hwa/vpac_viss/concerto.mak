
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 R5F))

include $(PRELUDE)
TARGET      := vx_target_kernels_vpac_viss
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(TDA4X_C_MODELS_PATH)/include
IDIRS       += $(VXLIB_PATH)/packages

# Temporary until viss is fixed for file read
ifeq ($(TARGET_CPU),R5F)
SKIPBUILD=1
endif

include $(FINALE)

endif
