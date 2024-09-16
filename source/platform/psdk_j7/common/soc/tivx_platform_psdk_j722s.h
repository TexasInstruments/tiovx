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

/*! \brief Maximum number of targets and thus targetid supported
 *         MUST be <= TIVX_TARGET_MAX_TARGETS_IN_CPU defined in tivx_config.h
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_TARGETS            (40u)

/*! \brief Macro to check the alignment of the size of
 *         the shared memory entry
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_SHM_ENTRY_SIZE_ALIGN      (8U)

/*! \brief HW spinlock ID to use for locking run-time event logger
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_LOCK_LOG_RT_HW_SPIN_LOCK_ID    (253u)


/*! \brief HW spinlock ID to use for locking object descriptor table
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID    (254u)

/*! \brief HW spinlock ID to use for locking data ref queue
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID    (255u)

/*! \brief Max number of targets on a given R5F
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_R5F_MAX            (24U)

/*! \brief Target ID for supported targets
 * \ingroup group_tivx_platform
 */
typedef enum _tivx_target_id_e {

    /*! \brief target ID for DSP1 */
    TIVX_TARGET_ID_DSP1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP1, 0u),

    /*! \brief target ID for DSP_C7_1 */
    TIVX_TARGET_ID_DSP_C7_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 0u),

    /*! \brief target ID for DSP_C7_1_PRI_2 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 1u),

    /*! \brief target ID for DSP_C7_1_PRI_3 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 2u),

    /*! \brief target ID for DSP_C7_1_PRI_4 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 3u),

    /*! \brief target ID for DSP_C7_1_PRI_5 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_5 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 4u),

    /*! \brief target ID for DSP_C7_1_PRI_6 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_6 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 5u),

    /*! \brief target ID for DSP_C7_1_PRI_7 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_7 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 6u),

    /*! \brief target ID for DSP_C7_1_PRI_8 */
    TIVX_TARGET_ID_DSP_C7_1_PRI_8 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_1, 7u),

    /*! \brief target ID for DSP2 */
    TIVX_TARGET_ID_DSP2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP2, 0u),

    /*! \brief target ID for DSP_C7_2 */
    TIVX_TARGET_ID_DSP_C7_2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_2, 0u),

    /*! \brief target ID for DSP_C7_2_PRI_2 */
    TIVX_TARGET_ID_DSP_C7_2_PRI_2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_2, 1u),

    /*! \brief target ID for DSP_C7_2_PRI_3 */
    TIVX_TARGET_ID_DSP_C7_2_PRI_3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_2, 2u),

    /*! \brief target ID for DSP_C7_2_PRI_4 */
    TIVX_TARGET_ID_DSP_C7_2_PRI_4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_2, 3u),

    /*! \brief target ID for DSP_C7_2_PRI_5 */
    TIVX_TARGET_ID_DSP_C7_2_PRI_5 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_2, 4u),

    /*! \brief target ID for DSP_C7_2_PRI_6 */
    TIVX_TARGET_ID_DSP_C7_2_PRI_6 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_2, 5u),

    /*! \brief target ID for DSP_C7_2_PRI_7 */
    TIVX_TARGET_ID_DSP_C7_2_PRI_7 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_2, 6u),

    /*! \brief target ID for DSP_C7_2_PRI_8 */
    TIVX_TARGET_ID_DSP_C7_2_PRI_8 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_2, 7u),

    /*! \brief target ID for MPU-0 */
    TIVX_TARGET_ID_MPU_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MPU_0, 0u),

    /*! \brief target ID for MPU-1 */
    TIVX_TARGET_ID_MPU_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MPU_0, 1u),

    /*! \brief target ID for MPU-0 */
    TIVX_TARGET_ID_MPU_2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MPU_0, 2u),

    /*! \brief target ID for MPU-0 */
    TIVX_TARGET_ID_MPU_3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MPU_0, 3u),

    /*! \brief target ID for MCU1-0 */
    TIVX_TARGET_ID_MCU1_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU1_0, 0u),

    /*! \brief target ID for MCU2-0 */
    TIVX_TARGET_ID_MCU2_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 0u),

    /*! \brief target ID for IPU1-0 */
    TIVX_TARGET_ID_IPU1_0 = TIVX_TARGET_ID_MCU2_0,

    /*! \brief target ID for LDC1 */
    TIVX_TARGET_ID_VPAC_LDC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 1u),

    /*! \brief target ID for MSC1 */
    TIVX_TARGET_ID_VPAC_MSC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 2u),

    /*! \brief target ID for MSC2 */
    TIVX_TARGET_ID_VPAC_MSC2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 3u),

    /*! \brief target ID for VISS1 */
    TIVX_TARGET_ID_VPAC_VISS1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 4u),

    /*! \brief target ID for Display1 */
    TIVX_TARGET_ID_DISPLAY1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 5u),

    /*! \brief target ID for Display2 */
    TIVX_TARGET_ID_DISPLAY2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 6u),

    /*! \brief target ID for CSITX */
    TIVX_TARGET_ID_CSITX = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 7u),

    /*! \brief target ID for CSITX2 */
    TIVX_TARGET_ID_CSITX2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 8u),

    /*! \brief target ID for SDE */
    TIVX_TARGET_ID_DMPAC_SDE = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 9u),

    /*! \brief target ID for DOF */
    TIVX_TARGET_ID_DMPAC_DOF = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 10u),

    /*! \brief target ID for CAPTURE1 */
    TIVX_TARGET_ID_CAPTURE1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 11u),

    /*! \brief target ID for CAPTURE2 */
    TIVX_TARGET_ID_CAPTURE2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 12u),

    /*! \brief target ID for CAPTURE3 */
    TIVX_TARGET_ID_CAPTURE3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 13u),

    /*! \brief target ID for CAPTURE4 */
    TIVX_TARGET_ID_CAPTURE4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 14u),

} tivx_target_id_e;


/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP1, (vx_enum)TIVX_TARGET_ID_DSP1},                                   \
    {TIVX_TARGET_DSP_C7_1, (vx_enum)TIVX_TARGET_ID_DSP_C7_1},                           \
    {TIVX_TARGET_DSP_C7_1_PRI_2, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_2},               \
    {TIVX_TARGET_DSP_C7_1_PRI_3, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_3},               \
    {TIVX_TARGET_DSP_C7_1_PRI_4, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_4},               \
    {TIVX_TARGET_DSP_C7_1_PRI_5, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_5},               \
    {TIVX_TARGET_DSP_C7_1_PRI_6, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_6},               \
    {TIVX_TARGET_DSP_C7_1_PRI_7, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_7},               \
    {TIVX_TARGET_DSP_C7_1_PRI_8, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_8},               \
    {TIVX_TARGET_DSP2, (vx_enum)TIVX_TARGET_ID_DSP2},                                   \
    {TIVX_TARGET_DSP_C7_2, (vx_enum)TIVX_TARGET_ID_DSP_C7_2},                           \
    {TIVX_TARGET_DSP_C7_2_PRI_2, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_2},               \
    {TIVX_TARGET_DSP_C7_2_PRI_3, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_3},               \
    {TIVX_TARGET_DSP_C7_2_PRI_4, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_4},               \
    {TIVX_TARGET_DSP_C7_2_PRI_5, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_5},               \
    {TIVX_TARGET_DSP_C7_2_PRI_6, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_6},               \
    {TIVX_TARGET_DSP_C7_2_PRI_7, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_7},               \
    {TIVX_TARGET_DSP_C7_2_PRI_8, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_8},               \
    {TIVX_TARGET_MCU1_0, (vx_enum)TIVX_TARGET_ID_MCU1_0},                               \
    {TIVX_TARGET_IPU1_0, (vx_enum)TIVX_TARGET_ID_IPU1_0},                               \
    {TIVX_TARGET_MCU2_0, (vx_enum)TIVX_TARGET_ID_MCU2_0},                               \
    {TIVX_TARGET_MPU_0, (vx_enum)TIVX_TARGET_ID_MPU_0},                                 \
    {TIVX_TARGET_MPU_1, (vx_enum)TIVX_TARGET_ID_MPU_1},                                 \
    {TIVX_TARGET_MPU_2, (vx_enum)TIVX_TARGET_ID_MPU_2},                                 \
    {TIVX_TARGET_MPU_3, (vx_enum)TIVX_TARGET_ID_MPU_3},                                 \
    {TIVX_TARGET_VPAC_LDC1, (vx_enum)TIVX_TARGET_ID_VPAC_LDC1},                         \
    {TIVX_TARGET_VPAC_MSC1, (vx_enum)TIVX_TARGET_ID_VPAC_MSC1},                         \
    {TIVX_TARGET_VPAC_MSC2, (vx_enum)TIVX_TARGET_ID_VPAC_MSC2},                         \
    {TIVX_TARGET_VPAC_VISS1, (vx_enum)TIVX_TARGET_ID_VPAC_VISS1},                       \
    {TIVX_TARGET_CAPTURE1, (vx_enum)TIVX_TARGET_ID_CAPTURE1},                           \
    {TIVX_TARGET_CAPTURE2, (vx_enum)TIVX_TARGET_ID_CAPTURE2},                           \
    {TIVX_TARGET_DISPLAY1, (vx_enum)TIVX_TARGET_ID_DISPLAY1},                           \
    {TIVX_TARGET_DISPLAY2, (vx_enum)TIVX_TARGET_ID_DISPLAY2},                           \
    {TIVX_TARGET_CSITX, (vx_enum)TIVX_TARGET_ID_CSITX},                                 \
    {TIVX_TARGET_CSITX2, (vx_enum)TIVX_TARGET_ID_CSITX2},                               \
    {TIVX_TARGET_CAPTURE3, (vx_enum)TIVX_TARGET_ID_CAPTURE3},                           \
    {TIVX_TARGET_CAPTURE4, (vx_enum)TIVX_TARGET_ID_CAPTURE4},                           \
    {TIVX_TARGET_DMPAC_SDE, (vx_enum)TIVX_TARGET_ID_DMPAC_SDE},                         \
    {TIVX_TARGET_DMPAC_DOF, (vx_enum)TIVX_TARGET_ID_DMPAC_DOF},                         \
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
void tivxPlatformSetHostTargetId(tivx_target_id_e host_target_id);

#ifdef __cplusplus
}
#endif

#endif
