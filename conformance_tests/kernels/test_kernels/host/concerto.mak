
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 R5F))

include $(PRELUDE)
TARGET      := vx_kernels_test_kernels
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/include
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/test_kernels/include

ifeq ($(LDRA_COVERAGE_ENABLED), yes)
include $(TIOVX_PATH)/tiovx_dev/internal_docs/coverage_files/concerto_inc.mak
endif

include $(FINALE)

endif
