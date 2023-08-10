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
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R5F:1:debug:TIARMCGT_LLVM
        ifeq ($(TARGET_SOC),J721E)
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):C66:1:debug:CGT6X
        endif
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):$(C7X_TARGET):1:debug:CGT7X
        ifeq ($(BUILD_LINUX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):LINUX:$(MPU_CPU):1:debug:GCC_LINUX_ARM
        endif
        ifeq ($(BUILD_QNX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):QNX:$(MPU_CPU):1:debug:GCC_QNX_ARM
        endif
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R5F:1:release:TIARMCGT_LLVM
        ifeq ($(TARGET_SOC),J721E)
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):C66:1:release:CGT6X
        endif
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):$(C7X_TARGET):1:release:CGT7X
        ifeq ($(BUILD_LINUX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):LINUX:$(MPU_CPU):1:release:GCC_LINUX_ARM
        endif
        ifeq ($(BUILD_QNX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):QNX:$(MPU_CPU):1:release:GCC_QNX_ARM
        endif
    endif
endif

CONCERTO_ROOT ?= $(PSDK_BUILDER_PATH)/concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := target.mak
BUILD_PLATFORM :=

include $(CONCERTO_ROOT)/rules.mak

# Project specific rules

.PHONY: all vision_apps_utils doxy_docs doxy_docs_design
all: vision_apps_utils

ifeq ($(BUILD_EMULATION_MODE),yes)
vision_apps_utils:
	BUILD_TARGET_MODE=no $(MAKE) -C $(VISION_APPS_PATH) app_utils_init
	BUILD_TARGET_MODE=no $(MAKE) -C $(VISION_APPS_PATH) cp_to_lib
	BUILD_TARGET_MODE=no $(MAKE) -C $(APP_UTILS_PATH) app_utils_mem
	BUILD_TARGET_MODE=no $(MAKE) -C $(APP_UTILS_PATH) cp_to_lib
else
vision_apps_utils:

endif

doxy_docs:
	cat tiovx_dev/internal_docs/doxy_cfg_user_guide/user_guide_$(SOC)_linux.cfg > /tmp/user_guide_$(SOC)_linux.cfg
	$(PRINT) EXCLUDE = \\ >> /tmp/user_guide_$(SOC)_linux.cfg
ifeq ($(SOC),j721e)
	$(PRINT)           include/TI/soc/tivx_soc_j721s2.h \\ >> /tmp/user_guide_j721e_linux.cfg
	$(PRINT)           include/TI/soc/tivx_soc_j784s4.h \\ >> /tmp/user_guide_j721e_linux.cfg
endif
ifeq ($(SOC),j721s2)
	$(PRINT)           include/TI/soc/tivx_soc_j721e.h  \\ >> /tmp/user_guide_j721s2_linux.cfg
	$(PRINT)           include/TI/soc/tivx_soc_j784s4.h \\ >> /tmp/user_guide_j721s2_linux.cfg
endif
ifeq ($(SOC),j784s4)
	$(PRINT)           include/TI/soc/tivx_soc_j721e.h  \\ >> /tmp/user_guide_j784s4_linux.cfg
	$(PRINT)           include/TI/soc/tivx_soc_j721s2.h \\ >> /tmp/user_guide_j784s4_linux.cfg
endif
	$(PRINT)           include/TI/tivx_soc_j6.h >> /tmp/user_guide_$(SOC)_linux.cfg
	$(DOXYGEN) /tmp/user_guide_$(SOC)_linux.cfg 2> tiovx_dev/internal_docs/doxy_cfg_user_guide/doxy_warnings.txt
	$(CLEAN) /tmp/user_guide_$(SOC)_linux.cfg
	$(COPY) tiovx_dev/internal_docs/tiovx_release_notes_psdkra_$(SOC).html $(TIOVX_PATH)/tiovx_release_notes.html
	-rm $(TIOVX_PATH)/docs/test_reports/* -f
	$(MKDIR) $(TIOVX_PATH)/docs/test_reports/
	$(MKDIR) $(TIOVX_PATH)/docs/static_analysis/
	$(MKDIR) $(TIOVX_PATH)/docs/bidi/
	$(MKDIR) $(TIOVX_PATH)/docs/manifest/
	$(MKDIR) $(TIOVX_PATH)/docs/user_guide/
	$(COPY) tiovx_dev/internal_docs/test_reports/$(SOC)/* $(TIOVX_PATH)/docs/test_reports/.
	$(COPY) tiovx_dev/internal_docs/doxy_cfg_user_guide/images/*.pdf $(TIOVX_PATH)/docs/user_guide/.
	$(COPY) tiovx_dev/internal_docs/static_analysis_tiovx.xlsx $(TIOVX_PATH)/docs/static_analysis/.
	$(COPY) tiovx_dev/internal_docs/bidi_report_tiovx.xlsx $(TIOVX_PATH)/docs/bidi/.
	$(COPY) tiovx_dev/internal_docs/manifest/TIOVX_manifest.html $(TIOVX_PATH)/docs/manifest/.

doxy_docs_design:
	$(DOXYGEN) tiovx_dev/internal_docs/doxy_cfg_design/design_guide.cfg 2> tiovx_dev/internal_docs/doxy_cfg_design/doxy_warnings.txt
