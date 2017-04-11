/*==========================================================================*/
/*      Copyright (C) 2009-2017 Texas Instruments Incorporated.             */
/*                      All Rights Reserved                                 */
/*==========================================================================*/


/**
 *  @file       bam_kernel_database.c
 *
 *  @brief      This file defines the table of interface of all kernels supported in BAM
 */

#include "vx_bam_kernel_database.h"
#include "bam_dma_one_shot_node.h"

/**
 * Kernel Database - used for graph construction
 *
 * WARNING: the order the entries are listed must match the same order as the enum BAM_TI_KernelID in file bam_kernel_database.h
 *
 */
static BAM_KernelHostDBdef bamKernelHostDB[] =
{
    { &gBAM_TI_dmaAutoIncrementReadKernel, &gBAM_TI_dmaReadAutoIncrementKernelHelperFunc, "ti_dma_read_autoincrement", BAM_KERNELID_DMAREAD_AUTOINCREMENT},
    { &gBAM_TI_dmaAutoIncrementWriteKernel, &gBAM_TI_dmaWriteAutoIncrementKernelHelperFunc, "ti_dma_write_autoincrement", BAM_KERNELID_DMAWRITE_AUTOINCREMENT},
    { &gBAM_TI_dmaOneShotReadKernel, &gBAM_TI_dmaReadOneShotKernelHelperFunc, "ti_dma_read_oneshot", BAM_KERNELID_DMAREAD_ONESHOT},
    { &gBAM_TI_dmaOneShotWriteKernel, &gBAM_TI_dmaWriteOneShotKernelHelperFunc, "ti_dma_write_oneshot", BAM_KERNELID_DMAWRITE_ONESHOT},
    { &gBAM_VXLIB_absDiff_i8u_i8u_o8u_kernel, &gBAM_VXLIB_absDiff_i8u_i8u_o8u_helperFunc, "vxlib_absDiff_i8u_i8u_o8u", BAM_KERNELID_VXLIB_ABSDIFF_I8U_I8U_O8U },
    { &gBAM_VXLIB_absDiff_i16s_i16s_o16s_kernel, &gBAM_VXLIB_absDiff_i16s_i16s_o16s_helperFunc, "vxlib_absDiff_i16s_i16s_o16s", BAM_KERNELID_VXLIB_ABSDIFF_I16S_I16S_O16S },
    { &gBAM_VXLIB_cannyNMS_i16s_i16s_i16u_o8u_kernel, &gBAM_VXLIB_cannyNMS_i16s_i16s_i16u_o8u_helperFunc, "vxlib_cannyNMS_i16s_i16s_i16u_o8u", BAM_KERNELID_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U },
    { &gBAM_VXLIB_doubleThreshold_i16u_i8u_kernel, &gBAM_VXLIB_doubleThreshold_i16u_i8u_helperFunc, "vxlib_doubleThreshold_i16u_i8u", BAM_KERNELID_VXLIB_DOUBLETHRESHOLD_I16S_I8U },
    { &gBAM_VXLIB_harrisCornersScore_i16s_i16s_o32f_kernel, &gBAM_VXLIB_harrisCornersScore_i16s_i16s_o32f_helperFunc, "vxlib_harrisCornersScore_i16s_i16s_o32f", BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I16S_I16S_O32F },
    { &gBAM_VXLIB_harrisCornersScore_i32s_i32s_o32f_kernel, &gBAM_VXLIB_harrisCornersScore_i32s_i32s_o32f_helperFunc, "vxlib_harrisCornersScore_i32s_i32s_o32f", BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I32S_I32S_O32F },
    { &gBAM_VXLIB_histogram_i8u_o32u_kernel, &gBAM_VXLIB_histogram_i8u_o32u_helperFunc, "vxlib_histogram_i8u_o32u", BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U },
    { &gBAM_VXLIB_normL1_i16s_i16s_o16u_kernel, &gBAM_VXLIB_normL1_i16s_i16s_o16u_helperFunc, "vxlib_normL1_i16s_i16s_o16u", BAM_KERNELID_VXLIB_NORML1_I16S_I16S_O16U },
    { &gBAM_VXLIB_normL2_i16s_i16s_o16u_kernel, &gBAM_VXLIB_normL2_i16s_i16s_o16u_helperFunc, "vxlib_normL2_i16s_i16s_o16u", BAM_KERNELID_VXLIB_NORML2_I16S_I16S_O16U },
    { &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_helperFunc, "vxlib_sobel_3x3_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobel_5x5_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_5x5_i8u_o16s_o16s_helperFunc, "vxlib_sobel_5x5_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_5X5_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobel_7x7_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_7x7_i8u_o16s_o16s_helperFunc, "vxlib_sobel_7x7_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobel_7x7_i8u_o32s_o32s_kernel, &gBAM_VXLIB_sobel_7x7_i8u_o32s_o32s_helperFunc, "vxlib_sobel_7x7_i8u_o32s_o32s", BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O32S_O32S },
    { &gBAM_VXLIB_sobelX_3x3_i8u_o16s_kernel, &gBAM_VXLIB_sobelX_3x3_i8u_o16s_helperFunc, "vxlib_sobelX_3x3_i8u_o16s", BAM_KERNELID_VXLIB_SOBELX_3X3_I8U_O16S },
    { &gBAM_VXLIB_sobelY_3x3_i8u_o16s_kernel, &gBAM_VXLIB_sobelY_3x3_i8u_o16s_helperFunc, "vxlib_sobelY_3x3_i8u_o16s", BAM_KERNELID_VXLIB_SOBELY_3X3_I8U_O16S },
    { &gBAM_VXLIB_tableLookup_i16s_o16s_kernel, &gBAM_VXLIB_tableLookup_i16s_o16s_helperFunc, "vxlib_tableLookup_i16s_o16s", BAM_KERNELID_VXLIB_TABLELOOKUP_I16S_O16S },
    { &gBAM_VXLIB_tableLookup_i8u_o8u_kernel, &gBAM_VXLIB_tableLookup_i8u_o8u_helperFunc, "vxlib_tableLookup_i8u_o8u", BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U },
    { &gBAM_VXLIB_integralImage_i8u_o32u_kernel, &gBAM_VXLIB_integralImage_i8u_o32u_helperFunc, "vxlib_integralImage_i8u_o32u", BAM_KERNELID_VXLIB_INTEGRALIMAGE_I8U_O32U },
    { &gBAM_VXLIB_not_i8u_o8u_kernel, &gBAM_VXLIB_not_i8u_o8u_helperFunc, "vxlib_not_i8u_o8u", BAM_KERNELID_VXLIB_NOT_I8U_O8U },
    { &gBAM_VXLIB_and_i8u_i8u_o8u_kernel, &gBAM_VXLIB_and_i8u_i8u_o8u_helperFunc, "vxlib_and_i8u_i8u_o8u", BAM_KERNELID_VXLIB_AND_I8U_I8U_O8U },
    { &gBAM_VXLIB_or_i8u_i8u_o8u_kernel, &gBAM_VXLIB_or_i8u_i8u_o8u_helperFunc, "vxlib_or_i8u_i8u_o8u", BAM_KERNELID_VXLIB_OR_I8U_I8U_O8U },
    { &gBAM_VXLIB_xor_i8u_i8u_o8u_kernel, &gBAM_VXLIB_xor_i8u_i8u_o8u_helperFunc, "vxlib_xor_i8u_i8u_o8u", BAM_KERNELID_VXLIB_XOR_I8U_I8U_O8U },
    { &gBAM_VXLIB_add_i8u_i8u_o8u_kernel, &gBAM_VXLIB_add_i8u_i8u_o8u_helperFunc, "vxlib_add_i8u_i8u_o8u", BAM_KERNELID_VXLIB_ADD_I8U_I8U_O8U },
    { &gBAM_VXLIB_add_i8u_i8u_o16s_kernel, &gBAM_VXLIB_add_i8u_i8u_o16s_helperFunc, "vxlib_add_i8u_i8u_o16s", BAM_KERNELID_VXLIB_ADD_I8U_I8U_O16S },
    { &gBAM_VXLIB_add_i8u_i16s_o16s_kernel, &gBAM_VXLIB_add_i8u_i16s_o16s_helperFunc, "vxlib_add_i8u_i16s_o16s", BAM_KERNELID_VXLIB_ADD_I8U_I16S_O16S },
    { &gBAM_VXLIB_add_i16s_i16s_o16s_kernel, &gBAM_VXLIB_add_i16s_i16s_o16s_helperFunc, "vxlib_add_i16s_i16s_o16s", BAM_KERNELID_VXLIB_ADD_I16S_I16S_O16S },
    { &gBAM_VXLIB_subtract_i8u_i8u_o8u_kernel, &gBAM_VXLIB_subtract_i8u_i8u_o8u_helperFunc, "vxlib_subtract_i8u_i8u_o8u", BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O8U },
    { &gBAM_VXLIB_subtract_i8u_i8u_o16s_kernel, &gBAM_VXLIB_subtract_i8u_i8u_o16s_helperFunc, "vxlib_subtract_i8u_i8u_o16s", BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O16S },
    { &gBAM_VXLIB_subtract_i8u_i16s_o16s_kernel, &gBAM_VXLIB_subtract_i8u_i16s_o16s_helperFunc, "vxlib_subtract_i8u_i16s_o16s", BAM_KERNELID_VXLIB_SUBTRACT_I8U_I16S_O16S },
    { &gBAM_VXLIB_subtract_i16s_i16s_o16s_kernel, &gBAM_VXLIB_subtract_i16s_i16s_o16s_helperFunc, "vxlib_subtract_i16s_i16s_o16s", BAM_KERNELID_VXLIB_SUBTRACT_I16S_I16S_O16S },
    { &gBAM_VXLIB_multiply_i8u_i8u_o8u_kernel, &gBAM_VXLIB_multiply_i8u_i8u_o8u_helperFunc, "vxlib_multiply_i8u_i8u_o8u", BAM_KERNELID_VXLIB_MULTIPLY_I8U_I8U_O8U },
    { &gBAM_VXLIB_multiply_i8u_i8u_o16s_kernel, &gBAM_VXLIB_multiply_i8u_i8u_o16s_helperFunc, "vxlib_multiply_i8u_i8u_o16s", BAM_KERNELID_VXLIB_MULTIPLY_I8U_I8U_O16S },
    { &gBAM_VXLIB_multiply_i8u_i16s_o16s_kernel, &gBAM_VXLIB_multiply_i8u_i16s_o16s_helperFunc, "vxlib_multiply_i8u_i16s_o16s", BAM_KERNELID_VXLIB_MULTIPLY_I8U_I16S_O16S },
    { &gBAM_VXLIB_multiply_i16s_i16s_o16s_kernel, &gBAM_VXLIB_multiply_i16s_i16s_o16s_helperFunc, "vxlib_multiply_i16s_i16s_o16s", BAM_KERNELID_VXLIB_MULTIPLY_I16S_I16S_O16S }
};

/**
 * Kernel Database - used for graph execution
 *
 *  WARNING: the order the entries are listed must match the same order as the enum BAM_TI_KernelID in file bam_kernel_database.h
 *
 */
static BAM_KernelExecFuncDBdef bamKernelExecFuncDB[] =
{
    { &gBAM_TI_dmaAutoIncrementReadKernel, &gBAM_TI_dmaReadAutoIncrementKernelExecFunc, "ti_dma_read_autoincrement", BAM_KERNELID_DMAREAD_AUTOINCREMENT},
    { &gBAM_TI_dmaAutoIncrementWriteKernel, &gBAM_TI_dmaWriteAutoIncrementKernelExecFunc, "ti_dma_write_autoincrement", BAM_KERNELID_DMAWRITE_AUTOINCREMENT},
    { &gBAM_TI_dmaOneShotReadKernel, &gBAM_TI_dmaReadOneShotKernelExecFunc, "ti_dma_read_oneshot", BAM_KERNELID_DMAREAD_ONESHOT},
    { &gBAM_TI_dmaOneShotWriteKernel, &gBAM_TI_dmaWriteOneShotKernelExecFunc, "ti_dma_write_oneshot", BAM_KERNELID_DMAWRITE_ONESHOT},
    { &gBAM_VXLIB_absDiff_i8u_i8u_o8u_kernel, &gBAM_VXLIB_absDiff_i8u_i8u_o8u_execFunc, "vxlib_absDiff_i8u_i8u_o8u", BAM_KERNELID_VXLIB_ABSDIFF_I8U_I8U_O8U },
    { &gBAM_VXLIB_absDiff_i16s_i16s_o16s_kernel, &gBAM_VXLIB_absDiff_i16s_i16s_o16s_execFunc, "vxlib_absDiff_i16s_i16s_o16s", BAM_KERNELID_VXLIB_ABSDIFF_I16S_I16S_O16S },
    { &gBAM_VXLIB_cannyNMS_i16s_i16s_i16u_o8u_kernel, &gBAM_VXLIB_cannyNMS_i16s_i16s_i16u_o8u_execFunc, "vxlib_cannyNMS_i16s_i16s_i16u_o8u", BAM_KERNELID_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U },
    { &gBAM_VXLIB_doubleThreshold_i16u_i8u_kernel, &gBAM_VXLIB_doubleThreshold_i16u_i8u_execFunc, "vxlib_doubleThreshold_i16u_i8u", BAM_KERNELID_VXLIB_DOUBLETHRESHOLD_I16S_I8U },
    { &gBAM_VXLIB_harrisCornersScore_i16s_i16s_o32f_kernel, &gBAM_VXLIB_harrisCornersScore_i16s_i16s_o32f_execFunc, "vxlib_harrisCornersScore_i16s_i16s_o32f", BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I16S_I16S_O32F },
    { &gBAM_VXLIB_harrisCornersScore_i32s_i32s_o32f_kernel, &gBAM_VXLIB_harrisCornersScore_i32s_i32s_o32f_execFunc, "vxlib_harrisCornersScore_i32s_i32s_o32f", BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I32S_I32S_O32F },
    { &gBAM_VXLIB_histogram_i8u_o32u_kernel, &gBAM_VXLIB_histogram_i8u_o32u_execFunc, "vxlib_histogram_i8u_o32u", BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U },
    { &gBAM_VXLIB_normL1_i16s_i16s_o16u_kernel, &gBAM_VXLIB_normL1_i16s_i16s_o16u_execFunc, "vxlib_normL1_i16s_i16s_o16u", BAM_KERNELID_VXLIB_NORML1_I16S_I16S_O16U },
    { &gBAM_VXLIB_normL2_i16s_i16s_o16u_kernel, &gBAM_VXLIB_normL2_i16s_i16s_o16u_execFunc, "vxlib_normL2_i16s_i16s_o16u", BAM_KERNELID_VXLIB_NORML2_I16S_I16S_O16U },
    { &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_execFunc, "vxlib_sobel_3x3_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobel_5x5_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_5x5_i8u_o16s_o16s_execFunc, "vxlib_sobel_5x5_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_5X5_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobel_7x7_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_7x7_i8u_o16s_o16s_execFunc, "vxlib_sobel_7x7_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobel_7x7_i8u_o32s_o32s_kernel, &gBAM_VXLIB_sobel_7x7_i8u_o32s_o32s_execFunc, "vxlib_sobel_7x7_i8u_o32s_o32s", BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O32S_O32S },
    { &gBAM_VXLIB_sobelX_3x3_i8u_o16s_kernel, &gBAM_VXLIB_sobelX_3x3_i8u_o16s_execFunc, "vxlib_sobelX_3x3_i8u_o16s", BAM_KERNELID_VXLIB_SOBELX_3X3_I8U_O16S },
    { &gBAM_VXLIB_sobelY_3x3_i8u_o16s_kernel, &gBAM_VXLIB_sobelY_3x3_i8u_o16s_execFunc, "vxlib_sobelY_3x3_i8u_o16s", BAM_KERNELID_VXLIB_SOBELY_3X3_I8U_O16S },
    { &gBAM_VXLIB_tableLookup_i16s_o16s_kernel, &gBAM_VXLIB_tableLookup_i16s_o16s_execFunc, "vxlib_tableLookup_i16s_o16s", BAM_KERNELID_VXLIB_TABLELOOKUP_I16S_O16S },
    { &gBAM_VXLIB_tableLookup_i8u_o8u_kernel, &gBAM_VXLIB_tableLookup_i8u_o8u_execFunc, "vxlib_tableLookup_i8u_o8u", BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U },
    { &gBAM_VXLIB_integralImage_i8u_o32u_kernel, &gBAM_VXLIB_integralImage_i8u_o32u_execFunc, "vxlib_integralImage_i8u_o32u", BAM_KERNELID_VXLIB_INTEGRALIMAGE_I8U_O32U },
    { &gBAM_VXLIB_not_i8u_o8u_kernel, &gBAM_VXLIB_not_i8u_o8u_execFunc, "vxlib_not_i8u_o8u", BAM_KERNELID_VXLIB_NOT_I8U_O8U },
    { &gBAM_VXLIB_and_i8u_i8u_o8u_kernel, &gBAM_VXLIB_and_i8u_i8u_o8u_execFunc, "vxlib_and_i8u_i8u_o8u", BAM_KERNELID_VXLIB_AND_I8U_I8U_O8U },
    { &gBAM_VXLIB_or_i8u_i8u_o8u_kernel, &gBAM_VXLIB_or_i8u_i8u_o8u_execFunc, "vxlib_or_i8u_i8u_o8u", BAM_KERNELID_VXLIB_OR_I8U_I8U_O8U },
    { &gBAM_VXLIB_xor_i8u_i8u_o8u_kernel, &gBAM_VXLIB_xor_i8u_i8u_o8u_execFunc, "vxlib_xor_i8u_i8u_o8u", BAM_KERNELID_VXLIB_XOR_I8U_I8U_O8U },
    { &gBAM_VXLIB_add_i8u_i8u_o8u_kernel, &gBAM_VXLIB_add_i8u_i8u_o8u_execFunc, "vxlib_add_i8u_i8u_o8u", BAM_KERNELID_VXLIB_ADD_I8U_I8U_O8U },
    { &gBAM_VXLIB_add_i8u_i8u_o16s_kernel, &gBAM_VXLIB_add_i8u_i8u_o16s_execFunc, "vxlib_add_i8u_i8u_o16s", BAM_KERNELID_VXLIB_ADD_I8U_I8U_O16S },
    { &gBAM_VXLIB_add_i8u_i16s_o16s_kernel, &gBAM_VXLIB_add_i8u_i16s_o16s_execFunc, "vxlib_add_i8u_i16s_o16s", BAM_KERNELID_VXLIB_ADD_I8U_I16S_O16S },
    { &gBAM_VXLIB_add_i16s_i16s_o16s_kernel, &gBAM_VXLIB_add_i16s_i16s_o16s_execFunc, "vxlib_add_i16s_i16s_o16s", BAM_KERNELID_VXLIB_ADD_I16S_I16S_O16S },
    { &gBAM_VXLIB_subtract_i8u_i8u_o8u_kernel, &gBAM_VXLIB_subtract_i8u_i8u_o8u_execFunc, "vxlib_subtract_i8u_i8u_o8u", BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O8U },
    { &gBAM_VXLIB_subtract_i8u_i8u_o16s_kernel, &gBAM_VXLIB_subtract_i8u_i8u_o16s_execFunc, "vxlib_subtract_i8u_i8u_o16s", BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O16S },
    { &gBAM_VXLIB_subtract_i8u_i16s_o16s_kernel, &gBAM_VXLIB_subtract_i8u_i16s_o16s_execFunc, "vxlib_subtract_i8u_i16s_o16s", BAM_KERNELID_VXLIB_SUBTRACT_I8U_I16S_O16S },
    { &gBAM_VXLIB_subtract_i16s_i16s_o16s_kernel, &gBAM_VXLIB_subtract_i16s_i16s_o16s_execFunc, "vxlib_subtract_i16s_i16s_o16s", BAM_KERNELID_VXLIB_SUBTRACT_I16S_I16S_O16S },
    { &gBAM_VXLIB_multiply_i8u_i8u_o8u_kernel, &gBAM_VXLIB_multiply_i8u_i8u_o8u_execFunc, "vxlib_multiply_i8u_i8u_o8u", BAM_KERNELID_VXLIB_MULTIPLY_I8U_I8U_O8U },
    { &gBAM_VXLIB_multiply_i8u_i8u_o16s_kernel, &gBAM_VXLIB_multiply_i8u_i8u_o16s_execFunc, "vxlib_multiply_i8u_i8u_o16s", BAM_KERNELID_VXLIB_MULTIPLY_I8U_I8U_O16S },
    { &gBAM_VXLIB_multiply_i8u_i16s_o16s_kernel, &gBAM_VXLIB_multiply_i8u_i16s_o16s_execFunc, "vxlib_multiply_i8u_i16s_o16s", BAM_KERNELID_VXLIB_MULTIPLY_I8U_I16S_O16S },
    { &gBAM_VXLIB_multiply_i16s_i16s_o16s_kernel, &gBAM_VXLIB_multiply_i16s_i16s_o16s_execFunc, "vxlib_multiply_i16s_i16s_o16s", BAM_KERNELID_VXLIB_MULTIPLY_I16S_I16S_O16S }
};

BAM_KernelDBdef gBAM_TI_kernelDBdef =
{
    sizeof(bamKernelExecFuncDB) / sizeof(bamKernelExecFuncDB[0]),
    bamKernelHostDB,
    bamKernelExecFuncDB
};

