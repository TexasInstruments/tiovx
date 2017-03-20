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
#include "ti/vxlib/src/vx/VXLIB_histogram_i8u_o32u/bam_plugin/BAM_VXLIB_histogram_i8u_o32u.h"
#include "ti/vxlib/src/vx/VXLIB_sobel_3x3_i8u_o16s_o16s/bam_plugin/BAM_VXLIB_sobel_3x3_i8u_o16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_sobelX_3x3_i8u_o16s/bam_plugin/BAM_VXLIB_sobelX_3x3_i8u_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_sobelY_3x3_i8u_o16s/bam_plugin/BAM_VXLIB_sobelY_3x3_i8u_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_tableLookup_i16s_o16s/bam_plugin/BAM_VXLIB_tableLookup_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_tableLookup_i8u_o8u/bam_plugin/BAM_VXLIB_tableLookup_i8u_o8u.h"

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
    BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U = 6,
    BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S = 7,
    BAM_KERNELID_VXLIB_SOBELX_3X3_I8U_O16S = 8,
    BAM_KERNELID_VXLIB_SOBELY_3X3_I8U_O16S = 9,
    BAM_KERNELID_VXLIB_TABLELOOKUP_I16S_O16S = 10,
    BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U = 11,
    BAM_KERNELID_MAX =                           0X7FFFFFFF
} BAM_TI_KernelID;

#endif
