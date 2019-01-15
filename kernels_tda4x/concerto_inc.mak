# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

ifeq ($(TARGET_CPU),x86_64)

STATIC_LIBS += vx_kernels_hwa_tests vx_kernels_hwa vx_vxu

STATIC_LIBS += vx_target_kernels_dmpac_sde
STATIC_LIBS += sde_hw

STATIC_LIBS += vx_target_kernels_vpac_ldc
STATIC_LIBS += ldc

ifeq ($(BUILD_HWA_DMPAC_DOF),yes)
STATIC_LIBS += vx_target_kernels_dmpac_dof
STATIC_LIBS += libDOF DOFalgo DOFcommon
SYS_SHARED_LIBS += opencv_imgproc opencv_ml opencv_core
STATIC_LIBS += vx_kernels_hwa_tests
endif

STATIC_LIBS += vx_target_kernels_vpac_msc
STATIC_LIBS += scalar

STATIC_LIBS += vx_target_kernels_vpac_nf
STATIC_LIBS += bl_filter_lib

STATIC_LIBS += vx_target_kernels_vpac_viss
STATIC_LIBS += rawfe nsf4 flexcfa flexcc h3a ee utils

STATIC_LIBS += vx_target_kernels_tda4x_arm

ifeq ($(TARGET_PLATFORM),TDA4X)
STATIC_LIBS += vx_target_kernels_display
STATIC_LIBS += vx_target_kernels_capture
endif

STATIC_LIBS += vx_utils

STATIC_LIBS += vx_tiovx_tidl_tests

STATIC_LIBS += vx_kernels_tidl vx_target_kernels_tidl vx_target_kernels_ivision_common tidl_algo

SYS_SHARED_LIBS += rt dl png

LDIRS+=$(TDA4X_C_MODELS_PATH)/lib/PC/x86_64/LINUX/release
LDIRS+=$(OPENCV_LIB_PATH)
LDIRS+=$(TIDL_PATH)/lib/PC/dsp/$(TARGET_BUILD)

LDIRS       += $(IMAGING_PATH)/out/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
STATIC_LIBS += ti_imaging_dcc

endif
