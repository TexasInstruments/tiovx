# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

STATIC_LIBS += vx_kernels_hwa_tests vx_kernels_hwa
STATIC_LIBS += vx_target_kernels_vpac_nf
STATIC_LIBS += vx_target_kernels_dmpac_sde
STATIC_LIBS += vx_target_kernels_vpac_ldc
STATIC_LIBS += vx_conformance_engine bl_filter_lib sde_hw ldc

