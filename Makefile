# Valid values are: j7presi
BUILD_SDK?=tiovx_dev/j7presi

include $(BUILD_SDK)_tools_path.mak
include $(TIOVX_PATH)/build_flags.mak

DIRECTORIES :=
DIRECTORIES += source/platform
DIRECTORIES += source/framework
DIRECTORIES += $(CUSTOM_PLATFORM_PATH)
DIRECTORIES += source/vxu
DIRECTORIES += kernels
DIRECTORIES += utils
DIRECTORIES += $(CUSTOM_KERNEL_PATH)
DIRECTORIES += $(CUSTOM_APPLICATION_PATH)

ifeq ($(BUILD_TUTORIAL),yes)
DIRECTORIES += conformance_tests/test_engine
DIRECTORIES += tutorial
endif

DIRECTORIES += tools/sample_use_cases

ifeq ($(BUILD_CONFORMANCE_TEST),yes)
  DIRECTORIES += conformance_tests
endif

TARGET_COMBOS :=

ifeq ($(BUILD_EMULATION_MODE),yes)
    ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
        TARGET_COMBOS += PC:LINUX:x86_64:1:debug:GCC_LINUX
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
        TARGET_COMBOS += PC:LINUX:x86_64:1:release:GCC_LINUX
    endif
endif

ifeq ($(BUILD_TARGET_MODE),yes)
    ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
        TARGET_COMBOS += TDA4X:SYSBIOS:R5F:1:debug:TIARMCGT
        TARGET_COMBOS += TDA4X:SYSBIOS:A72:1:debug:GCC_SYSBIOS_ARM
        TARGET_COMBOS += TDA4X:SYSBIOS:C66:1:debug:CGT6X
        TARGET_COMBOS += TDA4X:SYSBIOS:C71:1:debug:CGT7X
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
        TARGET_COMBOS += TDA4X:SYSBIOS:R5F:1:release:TIARMCGT
        TARGET_COMBOS += TDA4X:SYSBIOS:A72:1:release:GCC_SYSBIOS_ARM
        TARGET_COMBOS += TDA4X:SYSBIOS:C66:1:release:CGT6X
        TARGET_COMBOS += TDA4X:SYSBIOS:C71:1:release:CGT7X
    endif
endif

CONCERTO_ROOT ?= concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := concerto/target.mak
BUILD_PLATFORM :=

include $(CONCERTO_ROOT)/rules.mak
