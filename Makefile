
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

TARGET_COMBOS :=

TARGET_COMBOS += TDAX:SYSBIOS:M4:1:debug:TIARMCGT
TARGET_COMBOS += TDAX:SYSBIOS:C66:1:debug:CGT6X
TARGET_COMBOS += TDAX:SYSBIOS:EVE:1:debug:ARP32CGT
TARGET_COMBOS += TDAX:SYSBIOS:A15:1:debug:GCC

include $(CONCERTO_ROOT)/rules.mak
