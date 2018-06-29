/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


/**
 *  @file       tivx_bam_kernel_database.h
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
#include "ti/vxlib/src/vx/VXLIB_box_3x3_i8u_o8u/bam_plugin/BAM_VXLIB_box_3x3_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_dilate_3x3_i8u_o8u/bam_plugin/BAM_VXLIB_dilate_3x3_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_erode_3x3_i8u_o8u/bam_plugin/BAM_VXLIB_erode_3x3_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_gaussian_3x3_i8u_o8u/bam_plugin/BAM_VXLIB_gaussian_3x3_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_median_3x3_i8u_o8u/bam_plugin/BAM_VXLIB_median_3x3_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_addSquare_i8u_i16s_o16s/bam_plugin/BAM_VXLIB_addSquare_i8u_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_addWeight_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_addWeight_i8u_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_convertDepth_i8u_o16s/bam_plugin/BAM_VXLIB_convertDepth_i8u_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_convertDepth_i16s_o8u/bam_plugin/BAM_VXLIB_convertDepth_i16s_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_channelExtract_1of2_i8u_o8u/bam_plugin/BAM_VXLIB_channelExtract_1of2_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_channelExtract_1of3_i8u_o8u/bam_plugin/BAM_VXLIB_channelExtract_1of3_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_channelExtract_1of4_i8u_o8u/bam_plugin/BAM_VXLIB_channelExtract_1of4_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_channelCombine_2to1_i8u_o8u/bam_plugin/BAM_VXLIB_channelCombine_2to1_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_channelCombine_3to1_i8u_o8u/bam_plugin/BAM_VXLIB_channelCombine_3to1_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_channelCombine_4to1_i8u_o8u/bam_plugin/BAM_VXLIB_channelCombine_4to1_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_channelCombine_yuyv_i8u_o8u/bam_plugin/BAM_VXLIB_channelCombine_yuyv_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_channelCopy_1to1_i8u_o8u/bam_plugin/BAM_VXLIB_channelCopy_1to1_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_RGBtoYUV4_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_RGBtoYUV4_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_RGBtoRGBX_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_RGBtoRGBX_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_RGBtoNV12_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_RGBtoNV12_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_RGBtoIYUV_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_RGBtoIYUV_i8u_o8u.h"

#include "ti/vxlib/src/vx/VXLIB_colorConvert_RGBXtoYUV4_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_RGBXtoYUV4_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_RGBXtoRGB_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_RGBXtoRGB_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_RGBXtoNV12_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_RGBXtoNV12_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_RGBXtoIYUV_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_RGBXtoIYUV_i8u_o8u.h"

#include "ti/vxlib/src/vx/VXLIB_colorConvert_NVXXtoRGB_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u.h"

#include "ti/vxlib/src/vx/VXLIB_colorConvert_IYUVtoRGB_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_IYUVtoYUV4_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_IYUVtoYUV4_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_IYUVtoNV12_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_IYUVtoNV12_i8u_o8u.h"

#include "ti/vxlib/src/vx/VXLIB_colorConvert_YUVXtoRGB_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_colorConvert_YUVXtoNV12_i8u_o8u/bam_plugin/BAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u.h"

#include "ti/vxlib/src/vx/VXLIB_convolve_i8u_c16s_o8u/bam_plugin/BAM_VXLIB_convolve_i8u_c16s_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_convolve_i8u_c16s_o16s/bam_plugin/BAM_VXLIB_convolve_i8u_c16s_o16s.h"

#include "ti/vxlib/src/vx/VXLIB_magnitude_i16s_i16s_o16s/bam_plugin/BAM_VXLIB_magnitude_i16s_i16s_o16s.h"
#include "ti/vxlib/src/vx/VXLIB_phase_i16s_i16s_o8u/bam_plugin/BAM_VXLIB_phase_i16s_i16s_o8u.h"

#include "ti/vxlib/src/vx/VXLIB_erode_MxN_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_erode_MxN_i8u_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_dilate_MxN_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_dilate_MxN_i8u_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_median_MxN_i8u_i8u_o8u/bam_plugin/BAM_VXLIB_median_MxN_i8u_i8u_o8u.h"

#include "ti/vxlib/src/vx/VXLIB_scaleImageNearest_i8u_o8u/bam_plugin/BAM_VXLIB_scaleImageNearest_i8u_o8u.h"
#include "ti/vxlib/src/vx/VXLIB_halfScaleGaussian_5x5_i8u_o8u/bam_plugin/BAM_VXLIB_halfScaleGaussian_5x5_i8u_o8u.h"

#include "ti/vxlib/src/vx/VXLIB_histogramSimple_i8u_o32u/bam_plugin/BAM_VXLIB_histogramSimple_i8u_o32u.h"

#include "ti/vxlib/src/vx/VXLIB_meanStdDev_i8u_o32f/bam_plugin/BAM_VXLIB_meanStdDev_i8u_o32f.h"


extern BAM_KernelDBdef gBAM_TI_kernelDBdef;

typedef enum _bam_ti_kernelid
{
    BAM_TI_KERNELID_UNDEFINED = -1,

    BAM_KERNELID_DMAREAD_AUTOINCREMENT = 0,
    BAM_KERNELID_DMAWRITE_AUTOINCREMENT = 1,
    BAM_KERNELID_DMAREAD_NULL = 2,
    BAM_KERNELID_DMAWRITE_NULL = 3,
    BAM_KERNELID_VXLIB_ABSDIFF_I16S_I16S_O16S = 4,
    BAM_KERNELID_VXLIB_ABSDIFF_I8U_I8U_O8U = 5,
    BAM_KERNELID_VXLIB_ADD_I16S_I16S_O16S = 6,
    BAM_KERNELID_VXLIB_ADD_I8U_I16S_O16S = 7,
    BAM_KERNELID_VXLIB_ADD_I8U_I8U_O16S = 8,
    BAM_KERNELID_VXLIB_ADD_I8U_I8U_O8U = 9,
    BAM_KERNELID_VXLIB_ADDSQUARE_I8U_I16S_O16S = 10,
    BAM_KERNELID_VXLIB_ADDWEIGHT_I8U_I8U_O8U = 11,
    BAM_KERNELID_VXLIB_AND_I8U_I8U_O8U = 12,
    BAM_KERNELID_VXLIB_BOX_3X3_I8U_O8U = 13,
    BAM_KERNELID_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U = 14,
    BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U = 15,
    BAM_KERNELID_VXLIB_CHANNEL_COMBINE_2TO1_I8U_O8U = 16,
    BAM_KERNELID_VXLIB_CHANNEL_COMBINE_3TO1_I8U_O8U = 17,
    BAM_KERNELID_VXLIB_CHANNEL_COMBINE_4TO1_I8U_O8U = 18,
    BAM_KERNELID_VXLIB_CHANNEL_COMBINE_YUYV_I8U_O8U = 19,
    BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF2_I8U_O8U = 20,
    BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF3_I8U_O8U = 21,
    BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF4_I8U_O8U = 22,
    BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoNV12_I8U_O8U = 23,
    BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoRGB_I8U_O8U = 24,
    BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoRGBX_I8U_O8U = 25,
    BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoYUV4_I8U_O8U = 26,
    BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoIYUV_I8U_O8U = 27,
    BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoRGB_I8U_O8U = 28,
    BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoRGBX_I8U_O8U = 29,
    BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoYUV4_I8U_O8U = 30,
    BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoIYUV_I8U_O8U = 31,
    BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoNV12_I8U_O8U = 32,
    BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoRGBX_I8U_O8U = 33,
    BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoYUV4_I8U_O8U = 34,
    BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoIYUV_I8U_O8U = 35,
    BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoNV12_I8U_O8U = 36,
    BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoRGB_I8U_O8U = 37,
    BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoYUV4_I8U_O8U = 38,
    BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoIYUV_I8U_O8U = 39,
    BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoNV12_I8U_O8U = 40,
    BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoRGB_I8U_O8U = 41,
    BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoRGBX_I8U_O8U = 42,
    BAM_KERNELID_VXLIB_CONVERTDEPTH_I16S_O8U = 43,
    BAM_KERNELID_VXLIB_CONVERTDEPTH_I8U_O16S = 44,
    BAM_KERNELID_VXLIB_CONVOLVE_I8U_C16S_O16S = 45,
    BAM_KERNELID_VXLIB_CONVOLVE_I8U_C16S_O8U = 46,
    BAM_KERNELID_VXLIB_DILATE_3X3_I8U_O8U = 47,
    BAM_KERNELID_VXLIB_DILATE_MXN_I8U_I8U_O8U = 48,
    BAM_KERNELID_VXLIB_DOUBLETHRESHOLD_I16S_I8U = 49,
    BAM_KERNELID_VXLIB_ERODE_3X3_I8U_O8U = 50,
    BAM_KERNELID_VXLIB_ERODE_MXN_I8U_I8U_O8U = 51,
    BAM_KERNELID_VXLIB_GAUSSIAN_3X3_I8U_O8U = 52,
    BAM_KERNELID_VXLIB_HALFSCALEGAUSSIAN_5x5_I8U_O8U = 53,
    BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I16S_I16S_O32F = 54,
    BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I32S_I32S_O32F = 55,
    BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U = 56,
    BAM_KERNELID_VXLIB_HISTOGRAMSIMPLE_I8U_O32U = 57,
    BAM_KERNELID_VXLIB_INTEGRALIMAGE_I8U_O32U = 58,
    BAM_KERNELID_VXLIB_MAGNITUDE_I16S_I16S_O16S = 59,
    BAM_KERNELID_VXLIB_MEAN_STDDEV_I8U_O32F = 60,
    BAM_KERNELID_VXLIB_MEDIAN_3X3_I8U_O8U = 61,
    BAM_KERNELID_VXLIB_MEDIAN_MXN_I8U_I8U_O8U = 62,
    BAM_KERNELID_VXLIB_MINMAXLOC_I16S = 63,
    BAM_KERNELID_VXLIB_MINMAXLOC_I8U = 64,
    BAM_KERNELID_VXLIB_MULTIPLY_I16S_I16S_O16S = 65,
    BAM_KERNELID_VXLIB_MULTIPLY_I8U_I16S_O16S = 66,
    BAM_KERNELID_VXLIB_MULTIPLY_I8U_I8U_O16S = 67,
    BAM_KERNELID_VXLIB_MULTIPLY_I8U_I8U_O8U = 68,
    BAM_KERNELID_VXLIB_NORML1_I16S_I16S_O16U = 69,
    BAM_KERNELID_VXLIB_NORML2_I16S_I16S_O16U = 70,
    BAM_KERNELID_VXLIB_NOT_I8U_O8U = 71,
    BAM_KERNELID_VXLIB_OR_I8U_I8U_O8U = 72,
    BAM_KERNELID_VXLIB_PHASE_I16S_I16S_O8U = 73,
    BAM_KERNELID_VXLIB_SCALEIMAGENEAREST_I8U_O8U = 74,
    BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S = 75,
    BAM_KERNELID_VXLIB_SOBEL_5X5_I8U_O16S_O16S = 76,
    BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O16S_O16S = 77,
    BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O32S_O32S = 78,
    BAM_KERNELID_VXLIB_SOBELX_3X3_I8U_O16S = 79,
    BAM_KERNELID_VXLIB_SOBELY_3X3_I8U_O16S = 80,
    BAM_KERNELID_VXLIB_SUBTRACT_I16S_I16S_O16S = 81,
    BAM_KERNELID_VXLIB_SUBTRACT_I8U_I16S_O16S = 82,
    BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O16S = 83,
    BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O8U = 84,
    BAM_KERNELID_VXLIB_TABLELOOKUP_I16S_O16S = 85,
    BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U = 86,
    BAM_KERNELID_VXLIB_THRESHOLDBINARY_I8U_O8U = 87,
    BAM_KERNELID_VXLIB_THRESHOLDRANGE_I8U_O8U = 88,
    BAM_KERNELID_VXLIB_XOR_I8U_I8U_O8U = 89,
    BAM_KERNELID_MAX = 0X7FFFFFFF
} BAM_TI_KernelID;

#endif
