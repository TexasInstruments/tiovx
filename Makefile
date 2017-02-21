
include tools_path.mak

CONCERTO_ROOT ?= concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := target.mak
BUILD_PLATFORM :=


DIRECTORIES :=
DIRECTORIES += source/platform
DIRECTORIES += source/framework
DIRECTORIES += source/vxu
DIRECTORIES += kernels/openvx-core
DIRECTORIES += tools/sample_use_cases

TARGET_COMBOS :=

BUILD_EMULATION_MODE?=yes
BUILD_TARGET_MODE?=yes
HOST_CORE?=IPU1_0
A15_TARGET_OS?=Linux

BUILD_CONFORMANCE_TEST?=yes

ifeq ($(BUILD_CONFORMANCE_TEST),yes)
  DIRECTORIES += conformance_tests/test_conformance
  DIRECTORIES += conformance_tests/test_engine
  DIRECTORIES += conformance_tests/test_executable
  DIRECTORIES += conformance_tests/test_tiovx
  DIRECTORIES += conformance_tests/test_tiovx_engine
  DIRECTORIES += conformance_tests/test_tiovx_executable
endif

PROFILE?=all

ifeq ($(BUILD_TARGET_MODE),yes)
  ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
  TARGET_COMBOS += TDAX:SYSBIOS:M4:1:debug:TIARMCGT
  TARGET_COMBOS += TDAX:SYSBIOS:C66:1:debug:CGT6X
  TARGET_COMBOS += TDAX:SYSBIOS:EVE:1:debug:ARP32CGT
  TARGET_COMBOS += TDAX:SYSBIOS:A15:1:debug:GCC
  TARGET_COMBOS += TDAX:LINUX:A15:1:debug:GCC
  endif

  ifeq ($(PROFILE), $(filter $(PROFILE), release all))
  TARGET_COMBOS += TDAX:SYSBIOS:M4:1:release:TIARMCGT
  TARGET_COMBOS += TDAX:SYSBIOS:C66:1:release:CGT6X
  TARGET_COMBOS += TDAX:SYSBIOS:EVE:1:release:ARP32CGT
  TARGET_COMBOS += TDAX:SYSBIOS:A15:1:release:GCC
  TARGET_COMBOS += TDAX:LINUX:A15:1:release:GCC
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

include $(CONCERTO_ROOT)/rules.mak
