
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72 A53 R5F))
ifeq ($(BUILD_VIDEO_IO_KERNELS),yes)

include $(PRELUDE)
TARGET      := vx_kernels_video_io
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(CUSTOM_KERNEL_PATH)/video_io/include
IDIRS       += $(TIOVX_PATH)/source/include

ifeq ($(BUILD_CAPTURE),yes)
DEFS += BUILD_CAPTURE
endif

ifeq ($(BUILD_CSITX),yes)
DEFS += BUILD_CSITX
endif

ifeq ($(BUILD_DISPLAY),yes)
DEFS += BUILD_DISPLAY
endif

include $(FINALE)

endif
endif
