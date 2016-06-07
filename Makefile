
CONCERTO_ROOT ?= concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := target.mak
BUILD_PLATFORM :=


TI_TOOLS_ROOT := D:/ked/ADAS/code/ti_components/cg_tools/windows

TIARMCGT_ROOT := $(TI_TOOLS_ROOT)/ti-cgt-arm_5.2.5
GCC_ROOT := $(TI_TOOLS_ROOT)/gcc-arm-none-eabi-4_7-2013q3

CROSS_COMPILE := arm-none-eabi-

DIRECTORIES :=
#DIRECTORIES += conformance_tests/test_conformance
#DIRECTORIES += conformance_tests/test_engine
DIRECTORIES += source/framework
DIRECTORIES += source/vxu
DIRECTORIES += source/helper

TARGET_COMBOS :=

TARGET_COMBOS += TDAX:SYSBIOS:M4:1:debug:TIARMCGT
TARGET_COMBOS += TDAX:SYSBIOS:A15:1:debug:GCC

include $(CONCERTO_ROOT)/rules.mak
