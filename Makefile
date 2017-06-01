
include tools_path.mak

BUILD_EMULATION_MODE?=no
BUILD_TARGET_MODE?=yes

BUILD_CONFORMANCE_TEST?=yes
BUILD_IVISION_KERNELS?=yes
BUILD_BAM?=yes
BUILD_TUTORIAL?=yes
BUILD_LINUX_A15?=no

PROFILE?=all

DIRECTORIES :=
DIRECTORIES += source/platform
DIRECTORIES += source/framework
DIRECTORIES += source/vxu
DIRECTORIES += kernels/openvx-core
ifeq ($(BUILD_IVISION_KERNELS),yes)
DIRECTORIES += kernels/ivision
endif
ifeq ($(BUILD_TUTORIAL),yes)
DIRECTORIES += tutorial
endif
DIRECTORIES += tools/sample_use_cases

ifeq ($(BUILD_CONFORMANCE_TEST),yes)
  DIRECTORIES += conformance_tests/test_conformance
  DIRECTORIES += conformance_tests/test_engine
  DIRECTORIES += conformance_tests/test_executable
  DIRECTORIES += conformance_tests/test_tiovx
  ifeq ($(BUILD_IVISION_KERNELS),yes)
  DIRECTORIES += conformance_tests/test_ivision
  endif
endif

TARGET_COMBOS :=

ifeq ($(BUILD_TARGET_MODE),yes)
  ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
  TARGET_COMBOS += TDAX:SYSBIOS:M4:1:debug:TIARMCGT
  TARGET_COMBOS += TDAX:SYSBIOS:C66:1:debug:CGT6X
  TARGET_COMBOS += TDAX:SYSBIOS:EVE:1:debug:ARP32CGT
  TARGET_COMBOS += TDAX:SYSBIOS:A15:1:debug:GCC
    ifneq ($(OS),Windows_NT)
	ifeq ($(BUILD_LINUX_A15),yes)
    TARGET_COMBOS += TDAX:LINUX:A15:1:debug:GCC_LINARO
	endif
    endif
  endif

  ifeq ($(PROFILE), $(filter $(PROFILE), release all))
  TARGET_COMBOS += TDAX:SYSBIOS:M4:1:release:TIARMCGT
  TARGET_COMBOS += TDAX:SYSBIOS:C66:1:release:CGT6X
  TARGET_COMBOS += TDAX:SYSBIOS:EVE:1:release:ARP32CGT
  TARGET_COMBOS += TDAX:SYSBIOS:A15:1:release:GCC
    ifneq ($(OS),Windows_NT)
	ifeq ($(BUILD_LINUX_A15),yes)
    TARGET_COMBOS += TDAX:LINUX:A15:1:release:GCC_LINARO
	endif
    endif
  endif
endif

ifeq ($(BUILD_EMULATION_MODE),yes)
  ifeq ($(OS),Windows_NT)
    ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
    TARGET_COMBOS += PC:WINDOWS:X86:1:debug:GCC_WINDOWS
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
    TARGET_COMBOS += PC:WINDOWS:X86:1:release:GCC_WINDOWS
    endif
  else
    ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
    TARGET_COMBOS += PC:LINUX:X86:1:debug:GCC_LINUX
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
    TARGET_COMBOS += PC:LINUX:X86:1:release:GCC_LINUX
    endif
  endif
endif

CONCERTO_ROOT ?= concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := concerto/target.mak
BUILD_PLATFORM :=

include $(CONCERTO_ROOT)/rules.mak
