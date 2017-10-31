# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

STATIC_LIBS += vx_kernels_hwa_tests vx_kernels_hwa

ifeq ($(BUILD_HWA_VPAC_NF),yes)
STATIC_LIBS += vx_target_kernels_vpac_nf
STATIC_LIBS += bl_filter_lib
endif
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
STATIC_LIBS += libDOF algo common
STATIC_LIBS += opencv_highgui opencv_imgcodecs opencv_imgproc opencv_ml opencv_core opencv_hal
STATIC_LIBS += libjasper libjpeg libpng libtiff zlib

LDIRS+=$(DMPAC_DOF_PATH)/build/src/algo
LDIRS+=$(DMPAC_DOF_PATH)/build/src/common
LDIRS+=$(DMPAC_DOF_PATH)/build/extra/opencv/sources/3rdparty/lib
LDIRS+=$(DMPAC_DOF_PATH)/build/extra/opencv/sources/lib
endif

STATIC_LIBS += vx_conformance_engine
STATIC_LIBS += dl


