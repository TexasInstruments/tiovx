
ifeq ($(TARGET_PLATFORM),PC)


include $(PRELUDE)
TARGET      := vx_tutorial_exe
TARGETTYPE  := exe
CSOURCES    := $(call all-c-files)

LDIRS       := $(TIOVX_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
# Uncomment the line below if you want have rebuilt TI-DL library in the original package.
# otherwise the linker will use the libraries located in $(TIOVX_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
#LDIRS       += $(TIDL_PATH)/lib/PC/dsp/$(TARGET_BUILD)
IDIRS       += $(TIOVX_PATH)/tutorial
IDIRS       += $(TIOVX_PATH)/tutorial/ch01_common
IDIRS       += $(VISION_APPS_PATH)

STATIC_LIBS := vx_tutorial
STATIC_LIBS += vx_vxu vx_framework
STATIC_LIBS += vx_platform_pc vx_framework

STATIC_LIBS += vx_kernels_openvx_core vx_target_kernels_openvx_core
STATIC_LIBS += vx_kernels_openvx_ext vx_target_kernels_openvx_ext


include $(HOST_ROOT)/kernels/concerto_inc.mak
include $(HOST_ROOT)/conformance_tests/kernels/concerto_inc.mak

STATIC_LIBS += vx_target_kernels_tutorial
ifeq ($(BUILD_BAM),yes)
STATIC_LIBS += vx_target_kernels_openvx_core_bam vx_target_kernels_openvx_core
endif


STATIC_LIBS += vx_kernels_host_utils vx_kernels_target_utils

STATIC_LIBS += vx_framework vx_platform_pc
ifeq ($(BUILD_BAM),yes)
STATIC_LIBS += algframework_$(TARGET_CPU) dmautils_$(TARGET_CPU) vxlib_bamplugin_$(TARGET_CPU)
endif

STATIC_LIBS += vxlib_$(TARGET_CPU) c6xsim_$(TARGET_CPU)_C66

STATIC_LIBS += vx_utils

ifeq ($(SOC), am62a)
SKIPBUILD=1
endif

include $(FINALE)

endif
