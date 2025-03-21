include tools_path.mak
include build_flags.mak

# Project specific build defs (don't change across different combos):
BUILD_DEFS :=
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
ifeq ($(BUILD_TYPE),dev)
BUILD_DEFS += BUILD_DEV
endif
ifeq ($(BUILD_CORE_KERNELS),yes)
BUILD_DEFS += BUILD_CORE_KERNELS
endif
ifeq ($(BUILD_EXT_KERNELS),yes)
BUILD_DEFS += BUILD_EXT_KERNELS
endif
ifeq ($(BUILD_TEST_KERNELS),yes)
BUILD_DEFS += BUILD_TEST_KERNELS
endif

BUILD_DEFS += $(SOC_DEF)

DIRECTORIES :=
DIRECTORIES += source/platform
DIRECTORIES += source/framework
DIRECTORIES += source/vxu
DIRECTORIES += kernels
DIRECTORIES += utils
DIRECTORIES += $(CUSTOM_KERNEL_PATH)
DIRECTORIES += $(CUSTOM_APPLICATION_PATH)
DIRECTORIES += $(CUSTOM_PLATFORM_PATH)

ifeq ($(BUILD_TUTORIAL),yes)
DIRECTORIES += conformance_tests/test_engine
DIRECTORIES += tutorial
endif

DIRECTORIES += tools/sample_use_cases

ifeq ($(BUILD_CONFORMANCE_TEST),yes)
  DIRECTORIES += conformance_tests
endif

# If none of these flags are set, perform a normal build by including all possible cores.
# Setting one of these flags to 1 in the make command will build that core only.
ifeq ($(or $(BUILD_MPU),$(BUILD_R5F),$(BUILD_C7X),$(BUILD_C66)),)
	BUILD_MPU = 1
	BUILD_R5F = 1
	BUILD_C7X = 1
	BUILD_C66 = 1
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
		ifeq ($(BUILD_MPU), 1)
			ifeq ($(BUILD_LINUX_MPU),yes)
				TARGET_COMBOS += $(TARGET_SOC):LINUX:$(MPU_CPU):1:debug:GCC_LINUX_ARM
			endif
			ifeq ($(BUILD_QNX_MPU),yes)
				TARGET_COMBOS += $(TARGET_SOC):QNX:$(MPU_CPU):1:debug:GCC_QNX_ARM
			endif
		endif
		ifeq ($(BUILD_R5F), 1)
			TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R5F:1:debug:TIARMCGT_LLVM
		endif
		ifeq ($(BUILD_C7X), 1)
			TARGET_COMBOS += $(TARGET_SOC):$(RTOS):$(C7X_TARGET):1:debug:CGT7X
		endif
		ifeq ($(BUILD_C66), 1)
			ifeq ($(TARGET_SOC),J721E)
				TARGET_COMBOS += $(TARGET_SOC):$(RTOS):C66:1:debug:CGT6X
			endif
		endif
	endif

	ifeq ($(PROFILE), $(filter $(PROFILE), release all))
		ifeq ($(BUILD_MPU), 1)
			ifeq ($(BUILD_LINUX_MPU),yes)
				TARGET_COMBOS += $(TARGET_SOC):LINUX:$(MPU_CPU):1:release:GCC_LINUX_ARM
			endif
			ifeq ($(BUILD_QNX_MPU),yes)
				TARGET_COMBOS += $(TARGET_SOC):QNX:$(MPU_CPU):1:release:GCC_QNX_ARM
			endif
		endif
		ifeq ($(BUILD_R5F), 1)
			TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R5F:1:release:TIARMCGT_LLVM
		endif
		ifeq ($(BUILD_C7X), 1)
			TARGET_COMBOS += $(TARGET_SOC):$(RTOS):$(C7X_TARGET):1:release:CGT7X
		endif
		ifeq ($(BUILD_C66), 1)
			ifeq ($(TARGET_SOC),J721E)
				TARGET_COMBOS += $(TARGET_SOC):$(RTOS):C66:1:release:CGT6X
			endif
		endif
	endif
endif

ERROR_FLAG = 0
ifeq ($(LDRA_COVERAGE_ENABLED), yes)
	ifeq ($(INSTRUMENT_CORE), )
$(info ERROR - INSTRUMENT_CORE flag missing (A72 A53 R5F C71 C7120 C7504 C7524 C66))
		ERROR_FLAG = 1
	endif
else ifeq ($(INSTRUMENT_FRAMEWORK), yes)
$(info ERROR - LDRA_COVERAGE_ENABLED flag missing)
	ERROR_FLAG = 1
else ifeq ($(INSTRUMENT_PLATFORM), yes)
$(info ERROR - LDRA_COVERAGE_ENABLED flag missing)
	ERROR_FLAG = 1
else ifneq ($(INSTRUMENT_CORE), )
$(info ERROR - LDRA_COVERAGE_ENABLED flag missing)
	ERROR_FLAG = 1
endif

ifeq ($(ERROR_FLAG), 1)
	TARGET_COMBOS =
endif

CONCERTO_ROOT ?= $(PSDK_BUILDER_PATH)/concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := target.mak
BUILD_PLATFORM :=

# Project specific rules

.PHONY: all vision_apps_utils doxy_docs doxy_docs_design
all: vision_apps_utils

# Note: this has to be moved after "all" in order to ensure vision_apps_utils is processed first
include $(CONCERTO_ROOT)/rules.mak

ifeq ($(BUILD_EMULATION_MODE),yes)
vision_apps_utils:
	BUILD_TARGET_MODE=no $(MAKE) -C $(VISION_APPS_PATH) app_utils_init
	BUILD_TARGET_MODE=no $(MAKE) -C $(VISION_APPS_PATH) cp_to_lib
	BUILD_TARGET_MODE=no $(MAKE) -C $(APP_UTILS_PATH) app_utils_mem
	BUILD_TARGET_MODE=no $(MAKE) -C $(APP_UTILS_PATH) cp_to_lib
else
vision_apps_utils:
endif

SOC_LIST = "j721e" "j721s2" "j784s4" "j742s2" "j722s" "am62a"

doxy_docs:
	cat internal_docs/doxy_cfg_user_guide/user_guide_$(SOC)_linux.cfg > /tmp/user_guide_linux.cfg
	$(PRINT) EXCLUDE = \\ >> /tmp/user_guide_linux.cfg
	for tmp_soc in $(SOC_LIST) ; do \
		tmp=include/TI/soc/tivx_soc_$$tmp_soc.h ; \
		if [ "$$tmp_soc" != $(SOC) ]; then \
			echo           $$tmp \\ >> /tmp/user_guide_linux.cfg ; \
		fi ; \
	done
	$(DOXYGEN) /tmp/user_guide_linux.cfg 2> internal_docs/doxy_cfg_user_guide/doxy_warnings.txt
	$(CLEAN) /tmp/user_guide_linux.cfg
	$(COPY) tiovx_dev/internal_docs/tiovx_release_notes_psdkra_$(SOC).html $(TIOVX_PATH)/tiovx_release_notes.html
	-rm $(TIOVX_PATH)/docs/test_reports/* -f
	$(MKDIR) $(TIOVX_PATH)/docs/test_reports/
	$(MKDIR) $(TIOVX_PATH)/docs/static_analysis/
	$(MKDIR) $(TIOVX_PATH)/docs/bidi/
	$(MKDIR) $(TIOVX_PATH)/docs/manifest/
	$(MKDIR) $(TIOVX_PATH)/docs/user_guide/
	$(MKDIR) $(TIOVX_PATH)/docs/patches/
	$(COPY) tiovx_dev/internal_docs/test_reports/$(SOC)/* $(TIOVX_PATH)/docs/test_reports/.
	$(COPY) internal_docs/doxy_cfg_user_guide/images/*.pdf $(TIOVX_PATH)/docs/user_guide/.
	$(COPY) tiovx_dev/internal_docs/static_analysis_tiovx.xlsx $(TIOVX_PATH)/docs/static_analysis/.
	$(COPY) tiovx_dev/internal_docs/bidi_report_tiovx.xlsx $(TIOVX_PATH)/docs/bidi/.
	$(COPY) tiovx_dev/internal_docs/manifest/TIOVX_manifest.html $(TIOVX_PATH)/docs/manifest/.
	$(COPY) tiovx_dev/internal_docs/patches/* $(TIOVX_PATH)/docs/patches/

doxy_docs_design:
	$(DOXYGEN) tiovx_dev/internal_docs/doxy_cfg_design/design_guide.cfg 2> tiovx_dev/internal_docs/doxy_cfg_design/doxy_warnings.txt

clean_r5f:
	-rm -rf $(TIOVX_PATH)/out/$(TARGET_SOC)/R5F/

clean_c7x:
	-rm -rf $(TIOVX_PATH)/out/$(TARGET_SOC)/C7*/

clean_mpu:
	-rm -rf $(TIOVX_PATH)/out/$(TARGET_SOC)/A*/

clean_c66:
	-rm -rf $(TIOVX_PATH)/out/$(TARGET_SOC)/C66/