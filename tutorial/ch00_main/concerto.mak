
ifeq ($(TARGET_PLATFORM),PC)


include $(PRELUDE)
TARGET      := vx_tutorial_exe
TARGETTYPE  := exe
CSOURCES    := $(call all-c-files)

LDIRS       := $(TIOVX_PATH)/lib/PC/X86/$(TARGET_OS)/$(TARGET_BUILD)
IDIRS       += $(TIOVX_PATH)/tutorial
IDIRS       += $(TIOVX_PATH)/tutorial/ch01_common
IDIRS       += $(TIOVX_PATH)/conformance_tests/

STATIC_LIBS := vx_tutorial
STATIC_LIBS += vx_conformance_engine
STATIC_LIBS += vx_vxu vx_framework
STATIC_LIBS += vx_platform_pc vx_framework 

STATIC_LIBS += vx_kernels_openvx_core vx_target_kernels_openvx_core 

include $(HOST_ROOT)/kernels/concerto_inc.mak

STATIC_LIBS += vx_target_kernels_tutorial

ifeq ($(BUILD_BAM),yes)
STATIC_LIBS += vx_target_kernels_openvx_core_bam vx_target_kernels_openvx_core 
endif

STATIC_LIBS += vx_kernels_host_utils vx_kernels_target_utils

STATIC_LIBS += vx_framework vx_platform_pc

ifeq ($(BUILD_BAM),yes)
STATIC_LIBS += algframework_X86 dmautils_X86 vxlib_bamplugin_X86 
endif

STATIC_LIBS += vxlib_X86 c6xsim_X86_C66




include $(FINALE)

endif