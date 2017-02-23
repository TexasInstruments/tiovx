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
    { &gBAM_VXLIB_histogram_i8u_o32u_kernel, &gBAM_VXLIB_histogram_i8u_o32u_helperFunc, "vxlib_histogram_i8u_o32u", BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U },
    { &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_helperFunc, "vxlib_sobel_3x3_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobelX_3x3_i8u_o16s_kernel, &gBAM_VXLIB_sobelX_3x3_i8u_o16s_helperFunc, "vxlib_sobelX_3x3_i8u_o16s", BAM_KERNELID_VXLIB_SOBELX_3X3_I8U_O16S },
    { &gBAM_VXLIB_sobelY_3x3_i8u_o16s_kernel, &gBAM_VXLIB_sobelY_3x3_i8u_o16s_helperFunc, "vxlib_sobelY_3x3_i8u_o16s", BAM_KERNELID_VXLIB_SOBELY_3X3_I8U_O16S },
    { &gBAM_VXLIB_tableLookup_i16s_o16s_kernel, &gBAM_VXLIB_tableLookup_i16s_o16s_helperFunc, "vxlib_tableLookup_i16s_o16s", BAM_KERNELID_VXLIB_TABLELOOKUP_I16S_O16S },
    { &gBAM_VXLIB_tableLookup_i8u_o8u_kernel, &gBAM_VXLIB_tableLookup_i8u_o8u_helperFunc, "vxlib_tableLookup_i8u_o8u", BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U },
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
    { &gBAM_VXLIB_histogram_i8u_o32u_kernel, &gBAM_VXLIB_histogram_i8u_o32u_execFunc, "vxlib_histogram_i8u_o32u", BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U },
    { &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_execFunc, "vxlib_sobel_3x3_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobelX_3x3_i8u_o16s_kernel, &gBAM_VXLIB_sobelX_3x3_i8u_o16s_execFunc, "vxlib_sobelX_3x3_i8u_o16s", BAM_KERNELID_VXLIB_SOBELX_3X3_I8U_O16S },
    { &gBAM_VXLIB_sobelY_3x3_i8u_o16s_kernel, &gBAM_VXLIB_sobelY_3x3_i8u_o16s_execFunc, "vxlib_sobelY_3x3_i8u_o16s", BAM_KERNELID_VXLIB_SOBELY_3X3_I8U_O16S },
    { &gBAM_VXLIB_tableLookup_i16s_o16s_kernel, &gBAM_VXLIB_tableLookup_i16s_o16s_execFunc, "vxlib_tableLookup_i16s_o16s", BAM_KERNELID_VXLIB_TABLELOOKUP_I16S_O16S },
    { &gBAM_VXLIB_tableLookup_i8u_o8u_kernel, &gBAM_VXLIB_tableLookup_i8u_o8u_execFunc, "vxlib_tableLookup_i8u_o8u", BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U },
};

BAM_KernelDBdef gBAM_TI_kernelDBdef =
{
    sizeof(bamKernelExecFuncDB) / sizeof(bamKernelExecFuncDB[0]),
    bamKernelHostDB,
    bamKernelExecFuncDB
};

