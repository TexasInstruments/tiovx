/*
 *******************************************************************************
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com/
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

#include <stdbool.h>

/*! \brief Maximum number of targets and thus targetid supported
 *         MUST be <= TIVX_TARGET_MAX_TARGETS_IN_CPU defined in tivx_config.h
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_TARGETS            (78u)

/*! \brief Maximum number obj descriptors that are present in shared memory
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST  (4096u)

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

    /*! \brief target ID for DSP_C7_3 */
    TIVX_TARGET_ID_DSP_C7_3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_3, 0u),

    /*! \brief target ID for DSP_C7_3_PRI_2 */
    TIVX_TARGET_ID_DSP_C7_3_PRI_2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_3, 1u),

    /*! \brief target ID for DSP_C7_3_PRI_3 */
    TIVX_TARGET_ID_DSP_C7_3_PRI_3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_3, 2u),

    /*! \brief target ID for DSP_C7_3_PRI_4 */
    TIVX_TARGET_ID_DSP_C7_3_PRI_4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_3, 3u),

    /*! \brief target ID for DSP_C7_3_PRI_5 */
    TIVX_TARGET_ID_DSP_C7_3_PRI_5 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_3, 4u),

    /*! \brief target ID for DSP_C7_3_PRI_6 */
    TIVX_TARGET_ID_DSP_C7_3_PRI_6 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_3, 5u),

    /*! \brief target ID for DSP_C7_3_PRI_7 */
    TIVX_TARGET_ID_DSP_C7_3_PRI_7 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_3, 6u),

    /*! \brief target ID for DSP_C7_3_PRI_8 */
    TIVX_TARGET_ID_DSP_C7_3_PRI_8 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_3, 7u),

    /*! \brief target ID for DSP_C7_4 */
    TIVX_TARGET_ID_DSP_C7_4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_4, 0u),

    /*! \brief target ID for DSP_C7_4_PRI_2 */
    TIVX_TARGET_ID_DSP_C7_4_PRI_2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_4, 1u),

    /*! \brief target ID for DSP_C7_4_PRI_3 */
    TIVX_TARGET_ID_DSP_C7_4_PRI_3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_4, 2u),

    /*! \brief target ID for DSP_C7_4_PRI_4 */
    TIVX_TARGET_ID_DSP_C7_4_PRI_4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_4, 3u),

    /*! \brief target ID for DSP_C7_4_PRI_5 */
    TIVX_TARGET_ID_DSP_C7_4_PRI_5 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_4, 4u),

    /*! \brief target ID for DSP_C7_4_PRI_6 */
    TIVX_TARGET_ID_DSP_C7_4_PRI_6 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_4, 5u),

    /*! \brief target ID for DSP_C7_4_PRI_7 */
    TIVX_TARGET_ID_DSP_C7_4_PRI_7 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_4, 6u),

    /*! \brief target ID for DSP_C7_4_PRI_8 */
    TIVX_TARGET_ID_DSP_C7_4_PRI_8 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_DSP_C7_4, 7u),

    /*! \brief target ID for A72-0 */
    TIVX_TARGET_ID_A72_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_A72_0, 0u),

    /*! \brief target ID for A72-0 */
    TIVX_TARGET_ID_A72_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_A72_0, 1u),

    /*! \brief target ID for A72-0 */
    TIVX_TARGET_ID_A72_2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_A72_0, 2u),

    /*! \brief target ID for A72-0 */
    TIVX_TARGET_ID_A72_3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_A72_0, 3u),

    /*! \brief target ID for MCU2-0 */
    TIVX_TARGET_ID_MCU2_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 0u),

    /*! \brief target ID for IPU1-0 */
    TIVX_TARGET_ID_IPU1_0 = TIVX_TARGET_ID_MCU2_0,

    /*! \brief target ID for VPAC1 NF */
    TIVX_TARGET_ID_VPAC_NF = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 1u),

    /*! \brief target ID for VPAC1 LDC1 */
    TIVX_TARGET_ID_VPAC_LDC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 2u),

    /*! \brief target ID for VPAC1 MSC1 */
    TIVX_TARGET_ID_VPAC_MSC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 3u),

    /*! \brief target ID for VPAC1 MSC2 */
    TIVX_TARGET_ID_VPAC_MSC2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 4u),

    /*! \brief target ID for VPAC1 VISS1 */
    TIVX_TARGET_ID_VPAC_VISS1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 5u),

    /*! \brief target ID for Capture1 */
    TIVX_TARGET_ID_CAPTURE1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 6u),

    /*! \brief target ID for Capture2 */
    TIVX_TARGET_ID_CAPTURE2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 7u),

    /*! \brief target ID for Display1 */
    TIVX_TARGET_ID_DISPLAY1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 8u),

    /*! \brief target ID for Display2 */
    TIVX_TARGET_ID_DISPLAY2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 9u),

    /*! \brief target ID for CSITX */
    TIVX_TARGET_ID_CSITX = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 10u),

    /*! \brief target ID for Capture3 */
    TIVX_TARGET_ID_CAPTURE3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 11u),

    /*! \brief target ID for Capture4 */
    TIVX_TARGET_ID_CAPTURE4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 12u),

    /*! \brief target ID for Capture5 */
    TIVX_TARGET_ID_CAPTURE5 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 13u),

    /*! \brief target ID for Capture6 */
    TIVX_TARGET_ID_CAPTURE6 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 14u),

    /*! \brief target ID for Capture7 */
    TIVX_TARGET_ID_CAPTURE7 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 15u),

    /*! \brief target ID for Capture8 */
    TIVX_TARGET_ID_CAPTURE8 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 16u),

    /*! \brief target ID for Capture9 */
    TIVX_TARGET_ID_CAPTURE9 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 17u),

    /*! \brief target ID for Capture10 */
    TIVX_TARGET_ID_CAPTURE10 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 18u),

    /*! \brief target ID for Capture11 */
    TIVX_TARGET_ID_CAPTURE11 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 19u),

    /*! \brief target ID for Capture12 */
    TIVX_TARGET_ID_CAPTURE12 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 20u),

    /*! \brief target ID for Display M2M1 */
    TIVX_TARGET_ID_DISPLAY_M2M1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 21u),
    
    /*! \brief target ID for Display M2M2 */
    TIVX_TARGET_ID_DISPLAY_M2M2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 22u),
    
    /*! \brief target ID for Display M2M3 */
    TIVX_TARGET_ID_DISPLAY_M2M3 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 23u),
    
    /*! \brief target ID for Display M2M4 */
    TIVX_TARGET_ID_DISPLAY_M2M4 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 24u),

    /*! \brief target ID for CSITX2 */
    TIVX_TARGET_ID_CSITX2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_0, 25u),

    /*! \brief target ID for MCU2-1 */
    TIVX_TARGET_ID_MCU2_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_1, 0u),

    /*! \brief target ID for IPU1-1 */
    TIVX_TARGET_ID_IPU1_1 = TIVX_TARGET_ID_MCU2_1,

    /*! \brief target ID for SDE */
    TIVX_TARGET_ID_DMPAC_SDE = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_1, 1u),

    /*! \brief target ID for DOF */
    TIVX_TARGET_ID_DMPAC_DOF = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU2_1, 2u),

    /*! \brief target ID for MCU3_0 */
    TIVX_TARGET_ID_MCU3_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU3_0, 0u),

    /*! \brief target ID for MCU3_1 */
    TIVX_TARGET_ID_MCU3_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU3_1, 0u),

    /*! \brief target ID for MCU4_0 */
    TIVX_TARGET_ID_MCU4_0 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU4_0, 0u),

    /*! \brief target ID for VPAC2 NF */
    TIVX_TARGET_ID_VPAC2_NF = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU4_0, 1u),

    /*! \brief target ID for VPAC2 LDC1 */
    TIVX_TARGET_ID_VPAC2_LDC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU4_0, 2u),

    /*! \brief target ID for VPAC2 MSC1 */
    TIVX_TARGET_ID_VPAC2_MSC1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU4_0, 3u),

    /*! \brief target ID for VPAC2 MSC2 */
    TIVX_TARGET_ID_VPAC2_MSC2 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU4_0, 4u),

    /*! \brief target ID for VPAC2 VISS1 */
    TIVX_TARGET_ID_VPAC2_VISS1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU4_0, 5u),

    /*! \brief target ID for MCU4_1 */
    TIVX_TARGET_ID_MCU4_1 = TIVX_MAKE_TARGET_ID(TIVX_CPU_ID_MCU4_1, 0u),
    
} tivx_target_id_e;


/*! \brief Mapping of Target names with Target Ids
 *   Used to initialize internal structure
 *
 * \ingroup group_tivx_platform
 */
#define TIVX_TARGET_INFO                                                       \
{                                                                              \
    {TIVX_TARGET_DSP_C7_1, (vx_enum)TIVX_TARGET_ID_DSP_C7_1},                           \
    {TIVX_TARGET_DSP_C7_1_PRI_2, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_2},               \
    {TIVX_TARGET_DSP_C7_1_PRI_3, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_3},               \
    {TIVX_TARGET_DSP_C7_1_PRI_4, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_4},               \
    {TIVX_TARGET_DSP_C7_1_PRI_5, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_5},               \
    {TIVX_TARGET_DSP_C7_1_PRI_6, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_6},               \
    {TIVX_TARGET_DSP_C7_1_PRI_7, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_7},               \
    {TIVX_TARGET_DSP_C7_1_PRI_8, (vx_enum)TIVX_TARGET_ID_DSP_C7_1_PRI_8},               \
    {TIVX_TARGET_IPU1_0, (vx_enum)TIVX_TARGET_ID_IPU1_0},                               \
    {TIVX_TARGET_MCU2_0, (vx_enum)TIVX_TARGET_ID_MCU2_0},                               \
    {TIVX_TARGET_A72_0, (vx_enum)TIVX_TARGET_ID_A72_0},                                 \
    {TIVX_TARGET_A72_1, (vx_enum)TIVX_TARGET_ID_A72_1},                                 \
    {TIVX_TARGET_A72_2, (vx_enum)TIVX_TARGET_ID_A72_2},                                 \
    {TIVX_TARGET_A72_3, (vx_enum)TIVX_TARGET_ID_A72_3},                                 \
    {TIVX_TARGET_VPAC_NF, (vx_enum)TIVX_TARGET_ID_VPAC_NF},                             \
    {TIVX_TARGET_VPAC2_NF, (vx_enum)TIVX_TARGET_ID_VPAC2_NF},                           \
    {TIVX_TARGET_VPAC_LDC1, (vx_enum)TIVX_TARGET_ID_VPAC_LDC1},                         \
    {TIVX_TARGET_VPAC2_LDC1, (vx_enum)TIVX_TARGET_ID_VPAC2_LDC1},                       \
    {TIVX_TARGET_VPAC_MSC1, (vx_enum)TIVX_TARGET_ID_VPAC_MSC1},                         \
    {TIVX_TARGET_VPAC2_MSC1, (vx_enum)TIVX_TARGET_ID_VPAC2_MSC1},                       \
    {TIVX_TARGET_VPAC_MSC2, (vx_enum)TIVX_TARGET_ID_VPAC_MSC2},                         \
    {TIVX_TARGET_VPAC2_MSC2, (vx_enum)TIVX_TARGET_ID_VPAC2_MSC2},                       \
    {TIVX_TARGET_VPAC_VISS1, (vx_enum)TIVX_TARGET_ID_VPAC_VISS1},                       \
    {TIVX_TARGET_VPAC2_VISS1, (vx_enum)TIVX_TARGET_ID_VPAC2_VISS1},                     \
    {TIVX_TARGET_CAPTURE1, (vx_enum)TIVX_TARGET_ID_CAPTURE1},                           \
    {TIVX_TARGET_CAPTURE2, (vx_enum)TIVX_TARGET_ID_CAPTURE2},                           \
    {TIVX_TARGET_DISPLAY1, (vx_enum)TIVX_TARGET_ID_DISPLAY1},                           \
    {TIVX_TARGET_DISPLAY2, (vx_enum)TIVX_TARGET_ID_DISPLAY2},                           \
    {TIVX_TARGET_CSITX, (vx_enum)TIVX_TARGET_ID_CSITX},                                 \
    {TIVX_TARGET_CAPTURE3, (vx_enum)TIVX_TARGET_ID_CAPTURE3},                           \
    {TIVX_TARGET_CAPTURE4, (vx_enum)TIVX_TARGET_ID_CAPTURE4},                           \
    {TIVX_TARGET_CAPTURE5, (vx_enum)TIVX_TARGET_ID_CAPTURE5},                           \
    {TIVX_TARGET_CAPTURE6, (vx_enum)TIVX_TARGET_ID_CAPTURE6},                           \
    {TIVX_TARGET_CAPTURE7, (vx_enum)TIVX_TARGET_ID_CAPTURE7},                           \
    {TIVX_TARGET_CAPTURE8, (vx_enum)TIVX_TARGET_ID_CAPTURE8},                           \
    {TIVX_TARGET_CAPTURE9, (vx_enum)TIVX_TARGET_ID_CAPTURE9},                           \
    {TIVX_TARGET_CAPTURE10, (vx_enum)TIVX_TARGET_ID_CAPTURE10},                         \
    {TIVX_TARGET_CAPTURE11, (vx_enum)TIVX_TARGET_ID_CAPTURE11},                         \
    {TIVX_TARGET_CAPTURE12, (vx_enum)TIVX_TARGET_ID_CAPTURE12},                         \
    {TIVX_TARGET_DISPLAY_M2M1, (vx_enum)TIVX_TARGET_ID_DISPLAY_M2M1},                   \
    {TIVX_TARGET_DISPLAY_M2M2, (vx_enum)TIVX_TARGET_ID_DISPLAY_M2M2},                   \
    {TIVX_TARGET_DISPLAY_M2M3, (vx_enum)TIVX_TARGET_ID_DISPLAY_M2M3},                   \
    {TIVX_TARGET_DISPLAY_M2M4, (vx_enum)TIVX_TARGET_ID_DISPLAY_M2M4},                   \
    {TIVX_TARGET_CSITX2, (vx_enum)TIVX_TARGET_ID_CSITX2},                               \
    {TIVX_TARGET_IPU1_1, (vx_enum)TIVX_TARGET_ID_IPU1_1},                               \
    {TIVX_TARGET_MCU2_1, (vx_enum)TIVX_TARGET_ID_MCU2_1},                               \
    {TIVX_TARGET_DMPAC_SDE, (vx_enum)TIVX_TARGET_ID_DMPAC_SDE},                         \
    {TIVX_TARGET_DMPAC_DOF, (vx_enum)TIVX_TARGET_ID_DMPAC_DOF},                         \
    {TIVX_TARGET_MCU3_0, (vx_enum)TIVX_TARGET_ID_MCU3_0},                               \
    {TIVX_TARGET_MCU3_1, (vx_enum)TIVX_TARGET_ID_MCU3_1},                               \
    {TIVX_TARGET_MCU4_0, (vx_enum)TIVX_TARGET_ID_MCU4_0},                               \
    {TIVX_TARGET_MCU4_1, (vx_enum)TIVX_TARGET_ID_MCU4_1},                               \
    {TIVX_TARGET_DSP_C7_2, (vx_enum)TIVX_TARGET_ID_DSP_C7_2},                           \
    {TIVX_TARGET_DSP_C7_2_PRI_2, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_2},               \
    {TIVX_TARGET_DSP_C7_2_PRI_3, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_3},               \
    {TIVX_TARGET_DSP_C7_2_PRI_4, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_4},               \
    {TIVX_TARGET_DSP_C7_2_PRI_5, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_5},               \
    {TIVX_TARGET_DSP_C7_2_PRI_6, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_6},               \
    {TIVX_TARGET_DSP_C7_2_PRI_7, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_7},               \
    {TIVX_TARGET_DSP_C7_2_PRI_8, (vx_enum)TIVX_TARGET_ID_DSP_C7_2_PRI_8},               \
    {TIVX_TARGET_DSP_C7_3, (vx_enum)TIVX_TARGET_ID_DSP_C7_3},                           \
    {TIVX_TARGET_DSP_C7_3_PRI_2, (vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_2},               \
    {TIVX_TARGET_DSP_C7_3_PRI_3, (vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_3},               \
    {TIVX_TARGET_DSP_C7_3_PRI_4, (vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_4},               \
    {TIVX_TARGET_DSP_C7_3_PRI_5, (vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_5},               \
    {TIVX_TARGET_DSP_C7_3_PRI_6, (vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_6},               \
    {TIVX_TARGET_DSP_C7_3_PRI_7, (vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_7},               \
    {TIVX_TARGET_DSP_C7_3_PRI_8, (vx_enum)TIVX_TARGET_ID_DSP_C7_3_PRI_8},               \
    {TIVX_TARGET_DSP_C7_4, (vx_enum)TIVX_TARGET_ID_DSP_C7_4},                           \
    {TIVX_TARGET_DSP_C7_4_PRI_2, (vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_2},               \
    {TIVX_TARGET_DSP_C7_4_PRI_3, (vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_3},               \
    {TIVX_TARGET_DSP_C7_4_PRI_4, (vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_4},               \
    {TIVX_TARGET_DSP_C7_4_PRI_5, (vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_5},               \
    {TIVX_TARGET_DSP_C7_4_PRI_6, (vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_6},               \
    {TIVX_TARGET_DSP_C7_4_PRI_7, (vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_7},               \
    {TIVX_TARGET_DSP_C7_4_PRI_8, (vx_enum)TIVX_TARGET_ID_DSP_C7_4_PRI_8},               \
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
