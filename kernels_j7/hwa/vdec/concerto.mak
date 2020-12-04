
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F))
ifeq ($(BUILD_HWA_KERNELS),yes)

include $(PRELUDE)
TARGET      := vx_target_kernels_vdec
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/include
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(VIDEO_CODEC_PATH)
IDIRS       += $(VIDEO_CODEC_PATH)/ti-img-encode-decode/timmlib/include
IDIRS       += $(VISION_APPS_PATH)

include $(FINALE)

endif
endif

