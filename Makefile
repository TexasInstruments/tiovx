
# Valid values are: vsdk psdk
BUILD_SDK?=vsdk

include $(BUILD_SDK)_tools_path.mak


BUILD_TARGET_MODE?=yes
BUILD_EMULATION_MODE?=no
# valid values: X86 x86_64 all
BUILD_EMULATION_ARCH?=all

BUILD_CONFORMANCE_TEST?=yes
BUILD_TUTORIAL?=yes
BUILD_BAM?=yes
BUILD_LINUX_A15?=yes
BUILD_EVE?=yes
BUILD_IGNORE_LIB_ORDER?=no

# Kernel Library Extensions
BUILD_IVISION_KERNELS?=yes

PROFILE?=all

DIRECTORIES :=
DIRECTORIES += source/platform
DIRECTORIES += source/framework
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

ifeq ($(BUILD_TARGET_MODE),yes)
  ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
	TARGET_COMBOS += TDAX:SYSBIOS:M4:1:debug:TIARMCGT
	TARGET_COMBOS += TDAX:SYSBIOS:C66:1:debug:CGT6X
	ifeq ($(BUILD_EVE),yes)
	TARGET_COMBOS += TDAX:SYSBIOS:EVE:1:debug:ARP32CGT
	endif
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
	ifeq ($(BUILD_EVE),yes)
	TARGET_COMBOS += TDAX:SYSBIOS:EVE:1:release:ARP32CGT
	endif
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
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), x86_64 all))
            TARGET_COMBOS += PC:WINDOWS:x86_64:1:debug:GCC_WINDOWS
        endif
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), X86 all))
            TARGET_COMBOS += PC:WINDOWS:X86:1:debug:GCC_WINDOWS
        endif
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), x86_64 all))
            TARGET_COMBOS += PC:WINDOWS:x86_64:1:release:GCC_WINDOWS
        endif
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), X86 all))
            TARGET_COMBOS += PC:WINDOWS:X86:1:release:GCC_WINDOWS
        endif
    endif
  else
    ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), x86_64 all))
            TARGET_COMBOS += PC:LINUX:x86_64:1:debug:GCC_LINUX
        endif
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), X86 all))
            TARGET_COMBOS += PC:LINUX:X86:1:debug:GCC_LINUX
        endif
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), x86_64 all))
            TARGET_COMBOS += PC:LINUX:x86_64:1:release:GCC_LINUX
        endif
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), X86 all))
            TARGET_COMBOS += PC:LINUX:X86:1:release:GCC_LINUX
        endif
    endif
  endif
endif

CONCERTO_ROOT ?= concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := concerto/target.mak
BUILD_PLATFORM :=

include $(CONCERTO_ROOT)/rules.mak
