# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

STATIC_LIBS += vx_kernels_test_kernels_tests vx_kernels_test_kernels
STATIC_LIBS += vx_target_kernels_c66
ifeq ($(BUILD_BAM),yes)
STATIC_LIBS += vx_target_kernels_c66_bam
endif
STATIC_LIBS += vx_conformance_engine
# < DEVELOPER_TODO: Add any additional dependent libraries >

