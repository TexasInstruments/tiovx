
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72 R5F))

include $(PRELUDE)
TARGET      := vx_tutorial
TARGETTYPE  := library

IDIRS       += $(TIOVX_PATH)/utils/include
IDIRS       += $(TIOVX_PATH)/tutorial
IDIRS       += $(TIOVX_PATH)/tutorial/ch01_common

CH01_SOURCES := \
    ch01_common/vx_tutorial.c \

CH02_SOURCES := \
	ch02_image/vx_tutorial_image.c \
	ch02_image/vx_tutorial_image_load_save.c \
	ch02_image/vx_tutorial_image_query.c \
	ch02_image/vx_tutorial_image_crop_roi.c \
	ch02_image/vx_tutorial_image_extract_channel.c \
	ch02_image/vx_tutorial_image_color_convert.c \
	ch02_image/vx_tutorial_image_histogram.c \

CH03_SOURCES := \
	ch03_graph/vx_tutorial_graph.c \
	ch03_graph/vx_tutorial_graph_image_gradients.c \
	ch03_graph/vx_tutorial_graph_image_gradients_pytiovx_uc.c \
	ch03_graph/vx_tutorial_graph_user_kernel.c \
	ch03_graph/phase_rgb_user_kernel.c \
	ch03_graph/vx_tutorial_graph_user_kernel_pytiovx.c \
	ch03_graph/vx_tutorial_graph_user_kernel_pytiovx_uc.c \

CH04_SOURCES := \
	ch04_graph_pipeline/vx_tutorial_graph_pipeline.c \
	ch04_graph_pipeline/vx_tutorial_graph_pipeline_two_nodes.c \

IDIRS       += $(TIOVX_PATH)/kernels/tidl/include
IDIRS       += $(TIOVX_PATH)/conformance_tests/kernels/include
IDIRS       += $(TIOVX_PATH)/utils/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIDL_PATH)/inc
IDIRS       += $(XDIAS_PATH)/packages
IDIRS       += $(EVE_SW_PATH)/common

CH05_SOURCES := \
	ch05_test_mcu/vx_tutorial_mcu_demo.c \

ifeq ($(TARGET_PLATFORM),PC)
CFLAGS += -DHOST_EMULATION
endif

CSOURCES    := \
	$(CH01_SOURCES) \
	$(CH02_SOURCES) \
	$(CH03_SOURCES) \
	$(CH04_SOURCES) \
	$(CH05_SOURCES) \

DEFS += _DISABLE_TIDL

include $(FINALE)

endif
