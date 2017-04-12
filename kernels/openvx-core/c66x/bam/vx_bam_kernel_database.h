/*==========================================================================*/
/*      Copyright (C) 2009-2017 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/


/**
 *  @file       bam_kernel_database.h
 *
 *  @brief      This file includes the relevant header files for all the kernels and acts as a common
 *  unified interface for all the kernels.
 */

#ifndef BAM_DATABASE_H
#define BAM_DATABASE_H

#include "algframework.h"
#include "ti/vxlib/src/common/VXLIB_bufParams.h"
#include "ti/vxlib/src/vx/VXLIB_absDiff_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_absDiff_i8u_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_absDiff_i16s_i16s_o16s/bam_plugin/BAM_VXLIB_absDiff_i16s_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_cannyNMS_i16s_i16s_i16u_o8u/bam_plugin/BAM_VXLIB_cannyNMS_i16s_i16s_i16u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_doubleThreshold_i16u_i8u/bam_plugin/BAM_VXLIB_doubleThreshold_i16u_i8u.h"
#include "ti/vxlib/src/vx/VXLIB_harrisCornersScore_i16s_i16s_o32f/bam_plugin/BAM_VXLIB_harrisCornersScore_i16s_i16s_o32f.h"
#include "ti/vxlib/src/vx/VXLIB_harrisCornersScore_i32s_i32s_o32f/bam_plugin/BAM_VXLIB_harrisCornersScore_i32s_i32s_o32f.h"
#include "ti/vxlib/src/vx/VXLIB_histogram_i8u_o32u/bam_plugin/BAM_VXLIB_histogram_i8u_o32u.h"
#include "ti/vxlib/src/vx/VXLIB_normL1_i16s_i16s_o16u/bam_plugin/BAM_VXLIB_normL1_i16s_i16s_o16u.h"
#include "ti/vxlib/src/vx/VXLIB_normL2_i16s_i16s_o16u/bam_plugin/BAM_VXLIB_normL2_i16s_i16s_o16u.h"
#include "ti/vxlib/src/vx/VXLIB_sobel_3x3_i8u_o16s_o16s/bam_plugin/BAM_VXLIB_sobel_3x3_i8u_o16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_sobel_5x5_i8u_o16s_o16s/bam_plugin/BAM_VXLIB_sobel_5x5_i8u_o16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_sobel_7x7_i8u_o16s_o16s/bam_plugin/BAM_VXLIB_sobel_7x7_i8u_o16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_sobel_7x7_i8u_o32s_o32s/bam_plugin/BAM_VXLIB_sobel_7x7_i8u_o32s_o32s.h"
#include "ti/vxlib/src/vx/VXLIB_sobelX_3x3_i8u_o16s/bam_plugin/BAM_VXLIB_sobelX_3x3_i8u_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_sobelY_3x3_i8u_o16s/bam_plugin/BAM_VXLIB_sobelY_3x3_i8u_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_tableLookup_i16s_o16s/bam_plugin/BAM_VXLIB_tableLookup_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_tableLookup_i8u_o8u/bam_plugin/BAM_VXLIB_tableLookup_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_integralImage_i8u_o32u/bam_plugin/BAM_VXLIB_integralImage_i8u_o32u.h"
#include "ti/vxlib/src/vx/VXLIB_not_i8u_o8u/bam_plugin/BAM_VXLIB_not_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_and_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_and_i8u_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_or_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_or_i8u_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_xor_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_xor_i8u_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_add_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_add_i8u_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_add_i8u_i8u_o16s/bam_plugin/BAM_VXLIB_add_i8u_i8u_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_add_i8u_i16s_o16s/bam_plugin/BAM_VXLIB_add_i8u_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_add_i16s_i16s_o16s/bam_plugin/BAM_VXLIB_add_i16s_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_subtract_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_subtract_i8u_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_subtract_i8u_i8u_o16s/bam_plugin/BAM_VXLIB_subtract_i8u_i8u_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_subtract_i8u_i16s_o16s/bam_plugin/BAM_VXLIB_subtract_i8u_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_subtract_i16s_i16s_o16s/bam_plugin/BAM_VXLIB_subtract_i16s_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_multiply_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_multiply_i8u_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_multiply_i8u_i8u_o16s/bam_plugin/BAM_VXLIB_multiply_i8u_i8u_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_multiply_i8u_i16s_o16s/bam_plugin/BAM_VXLIB_multiply_i8u_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_multiply_i16s_i16s_o16s/bam_plugin/BAM_VXLIB_multiply_i16s_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_minMaxLoc_i8u/bam_plugin/BAM_VXLIB_minMaxLoc_i8u.h"
#include "ti/vxlib/src/vx/VXLIB_minMaxLoc_i16s/bam_plugin/BAM_VXLIB_minMaxLoc_i16s.h"
#include "ti/vxlib/src/vx/VXLIB_thresholdBinary_i8u_o8u/bam_plugin/BAM_VXLIB_thresholdBinary_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_thresholdRange_i8u_o8u/bam_plugin/BAM_VXLIB_thresholdRange_i8u_o8u.h"

extern BAM_KernelDBdef gBAM_TI_kernelDBdef;

typedef enum _bam_ti_kernelid
{
    BAM_TI_KERNELID_UNDEFINED = -1,

    BAM_KERNELID_DMAREAD_AUTOINCREMENT = 0,
    BAM_KERNELID_DMAWRITE_AUTOINCREMENT = 1,
    BAM_KERNELID_DMAREAD_ONESHOT = 2,
    BAM_KERNELID_DMAWRITE_ONESHOT = 3,
    BAM_KERNELID_VXLIB_ABSDIFF_I8U_I8U_O8U = 4,
    BAM_KERNELID_VXLIB_ABSDIFF_I16S_I16S_O16S = 5,
    BAM_KERNELID_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U = 6,
    BAM_KERNELID_VXLIB_DOUBLETHRESHOLD_I16S_I8U = 7,
    BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I16S_I16S_O32F = 8,
    BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I32S_I32S_O32F = 9,
    BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U = 10,
    BAM_KERNELID_VXLIB_NORML1_I16S_I16S_O16U = 11,
    BAM_KERNELID_VXLIB_NORML2_I16S_I16S_O16U = 12,
    BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S = 13,
    BAM_KERNELID_VXLIB_SOBEL_5X5_I8U_O16S_O16S = 14,
    BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O16S_O16S = 15,
    BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O32S_O32S = 16,
    BAM_KERNELID_VXLIB_SOBELX_3X3_I8U_O16S = 17,
    BAM_KERNELID_VXLIB_SOBELY_3X3_I8U_O16S = 18,
    BAM_KERNELID_VXLIB_TABLELOOKUP_I16S_O16S = 19,
    BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U = 20,
    BAM_KERNELID_VXLIB_INTEGRALIMAGE_I8U_O32U = 21,
    BAM_KERNELID_VXLIB_NOT_I8U_O8U = 22,
    BAM_KERNELID_VXLIB_AND_I8U_I8U_O8U = 23,
    BAM_KERNELID_VXLIB_OR_I8U_I8U_O8U = 24,
    BAM_KERNELID_VXLIB_XOR_I8U_I8U_O8U = 25,
    BAM_KERNELID_VXLIB_ADD_I8U_I8U_O8U = 26,
    BAM_KERNELID_VXLIB_ADD_I8U_I8U_O16S = 27,
    BAM_KERNELID_VXLIB_ADD_I8U_I16S_O16S = 28,
    BAM_KERNELID_VXLIB_ADD_I16S_I16S_O16S = 29,
    BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O8U = 30,
    BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O16S = 31,
    BAM_KERNELID_VXLIB_SUBTRACT_I8U_I16S_O16S = 32,
    BAM_KERNELID_VXLIB_SUBTRACT_I16S_I16S_O16S = 33,
    BAM_KERNELID_VXLIB_MULTIPLY_I8U_I8U_O8U = 34,
    BAM_KERNELID_VXLIB_MULTIPLY_I8U_I8U_O16S = 35,
    BAM_KERNELID_VXLIB_MULTIPLY_I8U_I16S_O16S = 36,
    BAM_KERNELID_VXLIB_MULTIPLY_I16S_I16S_O16S = 37,
    BAM_KERNELID_VXLIB_MINMAXLOC_I8U = 38,
    BAM_KERNELID_VXLIB_MINMAXLOC_I16S = 39,
    BAM_KERNELID_VXLIB_THRESHOLDBINARY_I8U_O8U = 40,
    BAM_KERNELID_VXLIB_THRESHOLDRANGE_I8U_O8U = 41,
    BAM_KERNELID_MAX = 0X7FFFFFFF
} BAM_TI_KernelID;

#endif
