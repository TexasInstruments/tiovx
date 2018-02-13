# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

ifeq ($(TARGET_CPU),x86_64)

STATIC_LIBS += vx_kernels_hwa_tests vx_kernels_hwa

ifeq ($(BUILD_HWA_DMPAC_SDE),yes)
STATIC_LIBS += vx_target_kernels_dmpac_sde
STATIC_LIBS += sde_hw
endif
ifeq ($(BUILD_HWA_VPAC_LDC),yes)
STATIC_LIBS += vx_target_kernels_vpac_ldc
STATIC_LIBS += ldc
endif
ifeq ($(BUILD_HWA_DMPAC_DOF),yes)
STATIC_LIBS += vx_target_kernels_dmpac_dof
STATIC_LIBS += libDOF DOFalgo DOFcommon
STATIC_LIBS += opencv_highgui opencv_imgproc opencv_ml opencv_core
STATIC_LIBS += vx_kernels_hwa_tests
endif
ifeq ($(BUILD_HWA_VPAC_MSC),yes)
STATIC_LIBS += vx_target_kernels_vpac_msc
STATIC_LIBS += scalar
endif
ifeq ($(BUILD_HWA_VPAC_NF),yes)
STATIC_LIBS += vx_target_kernels_vpac_nf
STATIC_LIBS += bl_filter_lib
endif

ifeq ($(BUILD_HWA_VPAC_VISS),yes)
STATIC_LIBS += vx_target_kernels_vpac_viss
STATIC_LIBS += rawfe nsf4 flexcfa flexcc h3a ee utils
endif

STATIC_LIBS += vx_vxu
STATIC_LIBS += vx_target_kernels_tda4x_arm
STATIC_LIBS += vx_utils

ifeq ($(BUILD_CONFORMANCE_TEST),yes)
STATIC_LIBS += vx_conformance_engine
endif
STATIC_LIBS += rt dl png

LDIRS+=$(TDA4X_C_MODELS_PATH)/lib/PC/x86_64/LINUX/release
LDIRS+=$(OPENCV_LIB_PATH)

endif
