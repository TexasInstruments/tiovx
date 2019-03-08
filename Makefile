
# Valid values are: vsdk psdk
BUILD_SDK?=vsdk

include $(BUILD_SDK)_tools_path.mak
include build_flags_$(BUILD_SDK).mak

# Project specific build defs (don't change across different combos):
BUILD_DEFS :=
ifeq ($(BUILD_IVISION_KERNELS),yes)
BUILD_DEFS += BUILD_IVISION_KERNELS
endif
ifneq ($(CUSTOM_KERNEL_PATH),)
BUILD_DEFS += CUSTOM_KERNEL_PATH
endif
ifneq ($(CUSTOM_APPLICATION_PATH),)
BUILD_DEFS += CUSTOM_APPLICATION_PATH
endif
ifeq ($(BUILD_TUTORIAL),yes)
BUILD_DEFS += BUILD_TUTORIAL
endif
ifeq ($(BUILD_CONFORMANCE_TEST),yes)
BUILD_DEFS += BUILD_CONFORMANCE_TEST
endif


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
		TARGET_COMBOS += TDAX:LINUX:A15:1:debug:GCC_LINUX_ARM
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
		TARGET_COMBOS += TDAX:LINUX:A15:1:release:GCC_LINUX_ARM
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
BUILD_TARGET := target.mak
BUILD_PLATFORM :=

include $(CONCERTO_ROOT)/rules.mak

# Project specific rules

.PHONY: all
all:

doxy_docs:
	$(DOXYGEN) tiovx_dev/internal_docs/doxy_cfg_user_guide/user_guide_linux.cfg 2> tiovx_dev/internal_docs/doxy_cfg_user_guide/doxy_warnings.txt

doxy_docs_pytiovx:
	$(DOXYGEN) tiovx_dev/internal_docs/doxy_cfg_pytiovx/pytiovx_guide_linux.cfg 2> tiovx_dev/internal_docs/doxy_cfg_pytiovx/doxy_warnings.txt

doxy_docs_tutorial:
	$(DOXYGEN) tiovx_dev/internal_docs/doxy_cfg_tutorial_guide/tutorial_guide_linux.cfg 2> tiovx_dev/internal_docs/doxy_cfg_tutorial_guide/doxy_warnings.txt

doxy_docs_design:
	$(DOXYGEN) tiovx_dev/internal_docs/doxy_cfg_design/design_guide.cfg 2> tiovx_dev/internal_docs/doxy_cfg_design/doxy_warnings.txt
