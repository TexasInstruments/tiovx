
include tools_path.mak

CONCERTO_ROOT ?= concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := target.mak
BUILD_PLATFORM :=


DIRECTORIES :=
#DIRECTORIES += conformance_tests/test_conformance
#DIRECTORIES += conformance_tests/test_engine
DIRECTORIES += source/platform
DIRECTORIES += source/framework
DIRECTORIES += source/vxu
DIRECTORIES += kernels/openvx-core
#DIRECTORIES += tools/sample_use_cases

TARGET_COMBOS :=

PROFILE=release

TARGET_COMBOS += TDAX:SYSBIOS:M4:1:$(PROFILE):TIARMCGT
TARGET_COMBOS += TDAX:SYSBIOS:C66:1:$(PROFILE):CGT6X
TARGET_COMBOS += TDAX:SYSBIOS:EVE:1:$(PROFILE):ARP32CGT
TARGET_COMBOS += TDAX:SYSBIOS:A15:1:$(PROFILE):GCC

include $(CONCERTO_ROOT)/rules.mak
