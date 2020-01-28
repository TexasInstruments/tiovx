/*
 *******************************************************************************
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_PLATFORM_VISION_SDK_H_
#define TIVX_PLATFORM_VISION_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Vision SDK Platform APIs
 */

#include <TI/j7.h>
#include <stdbool.h>

/*! \brief Maximum number of targets and thus targetid supported
 *         MUST be <= TIVX_TARGET_MAX_TARGETS_IN_CPU defined in tivx_config.h
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_TARGETS            (25u)

/*! \brief Maximum number obj descriptors that are present in shared memory
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST  (1024u)

/*! \brief Macro to check the alignment of the size of
 *         the shared memory entry
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_SHM_ENTRY_SIZE_ALIGN      (8U)

/*! \brief HW spinlock ID to use for locking data ref queue
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID    (255u)

/*! \brief Macros for build time check
 * \ingroup group_tivx_platform
 */
#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define BUILD_ASSERT(e) \
     enum { ASSERT_CONCAT(assert_line_, __LINE__) = (bool)1/(!!(e)) }

/*! \brief Target ID for supported targets
 * \ingroup group_tivx_platform
 */
typedef enum _tivx_target_id_e {

    /*! \brief target ID for DSP1 */
    TIVX_TARGET_ID_DSP1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 0u),

    /*! \brief target ID for DSP2 */
    TIVX_TARGET_ID_DSP2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP2, 0u),

    /*! \brief target ID for DSP_C7_1 */
    TIVX_TARGET_ID_DSP_C7_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 0u),

    /*! \brief target ID for A72-0 */
    TIVX_TARGET_ID_A72_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_A72_0, 0u),

    /*! \brief target ID for A72-0 */
    TIVX_TARGET_ID_A72_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_A72_0, 1u),

    /*! \brief target ID for A72-0 */
    TIVX_TARGET_ID_A72_2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_A72_0, 2u),

    /*! \brief target ID for A72-0 */
    TIVX_TARGET_ID_A72_3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_A72_0, 3u),

    /*! \brief target ID for IPU1-0 */
    TIVX_TARGET_ID_IPU1_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 0u),

    /*! \brief target ID for NF */
    TIVX_TARGET_ID_VPAC_NF = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 1u),

    /*! \brief target ID for LDC1 */
    TIVX_TARGET_ID_VPAC_LDC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 2u),

    /*! \brief target ID for MSC1 */
    TIVX_TARGET_ID_VPAC_MSC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 3u),

    /*! \brief target ID for MSC2 */
    TIVX_TARGET_ID_VPAC_MSC2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 4u),

    /*! \brief target ID for SDE */
    TIVX_TARGET_ID_DMPAC_SDE = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 5u),

    /*! \brief target ID for DOF */
    TIVX_TARGET_ID_DMPAC_DOF = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 6u),

    /*! \brief target ID for VISS1 */
    TIVX_TARGET_ID_VPAC_VISS1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 7u),

    /*! \brief target ID for Capture1 */
    TIVX_TARGET_ID_CAPTURE1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 8u),

    /*! \brief target ID for Capture2 */
    TIVX_TARGET_ID_CAPTURE2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 9u),

    /*! \brief target ID for Display1 */
    TIVX_TARGET_ID_DISPLAY1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 10u),

    /*! \brief target ID for Display2 */
    TIVX_TARGET_ID_DISPLAY2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 11u),

    /*! \brief target ID for VDEC1 */
    TIVX_TARGET_ID_VDEC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 12u),

    /*! \brief target ID for VDEC2 */
    TIVX_TARGET_ID_VDEC2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 13u),

    /*! \brief target ID for VENC1 */
    TIVX_TARGET_ID_VENC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 14u),

    /*! \brief target ID for VENC2 */
    TIVX_TARGET_ID_VENC2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 15u),
    
    /*! \brief target ID for CSITX */
    TIVX_TARGET_ID_CSITX = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_IPU1_0, 16u),

} tivx_target_id_e;


/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP1, (vx_enum)TIVX_TARGET_ID_DSP1},                                   \
    {TIVX_TARGET_DSP2, (vx_enum)TIVX_TARGET_ID_DSP2},                                   \
    {TIVX_TARGET_DSP_C7_1, (vx_enum)TIVX_TARGET_ID_DSP_C7_1},                           \
    {TIVX_TARGET_IPU1_0, (vx_enum)TIVX_TARGET_ID_IPU1_0},                               \
    {TIVX_TARGET_A72_0, (vx_enum)TIVX_TARGET_ID_A72_0},                                 \
    {TIVX_TARGET_A72_1, (vx_enum)TIVX_TARGET_ID_A72_1},                                 \
    {TIVX_TARGET_A72_2, (vx_enum)TIVX_TARGET_ID_A72_2},                                 \
    {TIVX_TARGET_A72_3, (vx_enum)TIVX_TARGET_ID_A72_3},                                 \
    {TIVX_TARGET_VPAC_NF, (vx_enum)TIVX_TARGET_ID_VPAC_NF},                             \
    {TIVX_TARGET_VPAC_LDC1, (vx_enum)TIVX_TARGET_ID_VPAC_LDC1},                         \
    {TIVX_TARGET_VPAC_MSC1, (vx_enum)TIVX_TARGET_ID_VPAC_MSC1},                         \
    {TIVX_TARGET_VPAC_MSC2, (vx_enum)TIVX_TARGET_ID_VPAC_MSC2},                         \
    {TIVX_TARGET_DMPAC_SDE, (vx_enum)TIVX_TARGET_ID_DMPAC_SDE},                         \
    {TIVX_TARGET_DMPAC_DOF, (vx_enum)TIVX_TARGET_ID_DMPAC_DOF},                         \
    {TIVX_TARGET_VPAC_VISS1, (vx_enum)TIVX_TARGET_ID_VPAC_VISS1},                       \
    {TIVX_TARGET_CAPTURE1, (vx_enum)TIVX_TARGET_ID_CAPTURE1},                           \
    {TIVX_TARGET_CAPTURE2, (vx_enum)TIVX_TARGET_ID_CAPTURE2},                           \
    {TIVX_TARGET_DISPLAY1, (vx_enum)TIVX_TARGET_ID_DISPLAY1},                           \
    {TIVX_TARGET_DISPLAY2, (vx_enum)TIVX_TARGET_ID_DISPLAY2},                           \
    {TIVX_TARGET_VDEC1, (vx_enum)TIVX_TARGET_ID_VDEC1},                                 \
    {TIVX_TARGET_VDEC2, (vx_enum)TIVX_TARGET_ID_VDEC2},                                 \
    {TIVX_TARGET_VENC1, (vx_enum)TIVX_TARGET_ID_VENC1},                                 \
    {TIVX_TARGET_VENC2, (vx_enum)TIVX_TARGET_ID_VENC2},                                 \
    {TIVX_TARGET_CSITX, (vx_enum)TIVX_TARGET_ID_CSITX},                                 \
    /* TIVX_TARGET_HOST will be filled later during tivxHostInit()             \
     * by calling function tivxPlatformSetHostTargetId                         \
     */                                                                        \
    {TIVX_TARGET_HOST, (vx_enum)TIVX_TARGET_ID_INVALID}                                 \
}

/*! \brief Set target ID for HOST.
 *
 *         Called during tivxHostInit()
 *
 * \ingroup group_tivx_platform
 */
void tivxPlatformSetHostTargetId(tivx_target_id_e host_targe_id);

#ifdef __cplusplus
}
#endif

#endif
