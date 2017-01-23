
include tools_path.mak

CONCERTO_ROOT ?= concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := target.mak
BUILD_PLATFORM :=


DIRECTORIES :=
DIRECTORIES += conformance_tests/test_conformance
DIRECTORIES += conformance_tests/test_engine
DIRECTORIES += conformance_tests/test_executable
DIRECTORIES += source/platform
DIRECTORIES += source/framework
DIRECTORIES += source/vxu
DIRECTORIES += kernels/openvx-core
DIRECTORIES += tools/sample_use_cases

TARGET_COMBOS :=

BUILD_EMULATION_MODE?=no
BUILD_TARGET_MODE?=yes

PROFILE?=all

ifeq ($(BUILD_TARGET_MODE),yes)
  ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
  TARGET_COMBOS += TDAX:SYSBIOS:M4:1:debug:TIARMCGT
  TARGET_COMBOS += TDAX:SYSBIOS:C66:1:debug:CGT6X
  TARGET_COMBOS += TDAX:SYSBIOS:EVE:1:debug:ARP32CGT
  TARGET_COMBOS += TDAX:SYSBIOS:A15:1:debug:GCC
  endif

  ifeq ($(PROFILE), $(filter $(PROFILE), release all))
  TARGET_COMBOS += TDAX:SYSBIOS:M4:1:release:TIARMCGT
  TARGET_COMBOS += TDAX:SYSBIOS:C66:1:release:CGT6X
  TARGET_COMBOS += TDAX:SYSBIOS:EVE:1:release:ARP32CGT
  TARGET_COMBOS += TDAX:SYSBIOS:A15:1:release:GCC
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
