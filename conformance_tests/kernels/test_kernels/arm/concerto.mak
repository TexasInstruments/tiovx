
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 C66 A72 A53 R5F C71 C7120 C7504 C7524))

	include $(PRELUDE)
	TARGET      := vx_target_kernels_source_sink
	TARGETTYPE  := library
	CSOURCES    := $(call all-c-files)
	IDIRS       += $(TIOVX_PATH)/conformance_tests/kernels/include
	IDIRS       += $(TIOVX_PATH)/conformance_tests/kernels/test_kernels/include
	IDIRS       += $(VXLIB_PATH)/packages
	IDIRS       += $(TIOVX_PATH)/source/include
	IDIRS       += $(TIOVX_PATH)/source/platform/common/targets
	IDIRS       += $(TIOVX_PATH)/source/platform/common/os/posix
	IDIRS       += $(PSDK_PATH)/app_utils

	ifeq ($(RTOS_SDK), mcu_plus_sdk)
		IDIRS   += $(APP_UTILS_PATH)/utils/rtos/src
	else
		IDIRS   += $(PDK_PATH)/packages/ti/osal/soc
	endif

	ifeq ($(LDRA_COVERAGE_ENABLED), yes)
		include $(TIOVX_PATH)/tiovx_dev/internal_docs/coverage_files/concerto_inc.mak
	endif

	include $(FINALE)

endif