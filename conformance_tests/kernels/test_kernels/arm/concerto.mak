
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C66 A15 M4 A72 R5F))

include $(PRELUDE)
TARGET      := vx_target_kernels_source_sink
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/include
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/test_kernels/include
IDIRS       += $(VXLIB_PATH)/packages

include $(FINALE)

endif
