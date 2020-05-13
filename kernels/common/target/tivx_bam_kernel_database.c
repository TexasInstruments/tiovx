/*
*
* Copyright (c) 2017 - 2019 Texas Instruments Incorporated
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
 *  @file       tivx_bam_kernel_database.c
 *
 *  @brief      This file defines the table of interface of all kernels supported in BAM
 */

#include <TI/tivx.h>
#include <TI/tivx_config.h>
#include "tivx_bam_kernel_wrapper.h"
#include "bam_dma_one_shot_node.h"

/**
 * Kernel Database - used for graph execution
 */
#define TIVX_MAX_DSP_BAM_PLUGINS BAM_KERNELID_EXTERNAL_START + TIVX_MAX_DSP_BAM_USER_PLUGINS

static BAM_KernelHostDBdef bamKernelHostDB[TIVX_MAX_DSP_BAM_PLUGINS];
static BAM_KernelExecFuncDBdef bamKernelExecFuncDB[TIVX_MAX_DSP_BAM_PLUGINS];
static uint32_t gNumActivePlugins;
static BAM_KernelInfo dummyKernelInfo;

/**
 * BAM Kernel Database - used for builtin BAM kernels
 */
static BAM_KernelCombinedDBdef bamKernelDB[] =
{
    { &gBAM_TI_dmaAutoIncrementReadKernel, &gBAM_TI_dmaReadAutoIncrementKernelHelperFunc, &gBAM_TI_dmaReadAutoIncrementKernelExecFunc, "ti_dma_read_autoincrement", BAM_KERNELID_DMAREAD_AUTOINCREMENT},
    { &gBAM_TI_dmaAutoIncrementWriteKernel, &gBAM_TI_dmaWriteAutoIncrementKernelHelperFunc, &gBAM_TI_dmaWriteAutoIncrementKernelExecFunc, "ti_dma_write_autoincrement", BAM_KERNELID_DMAWRITE_AUTOINCREMENT},
    { &gBAM_TI_dmaOneShotReadKernel, &gBAM_TI_dmaReadOneShotKernelHelperFunc, &gBAM_TI_dmaReadOneShotKernelExecFunc, "ti_dma_read_oneshot", BAM_KERNELID_DMAREAD_NULL},
    { &gBAM_TI_dmaOneShotWriteKernel, &gBAM_TI_dmaWriteOneShotKernelHelperFunc, &gBAM_TI_dmaWriteOneShotKernelExecFunc, "ti_dma_write_oneshot", BAM_KERNELID_DMAWRITE_NULL},
    { &gBAM_VXLIB_absDiff_i16s_i16s_o16s_kernel, &gBAM_VXLIB_absDiff_i16s_i16s_o16s_helperFunc, &gBAM_VXLIB_absDiff_i16s_i16s_o16s_execFunc, "vxlib_absDiff_i16s_i16s_o16s", BAM_KERNELID_VXLIB_ABSDIFF_I16S_I16S_O16S },
    { &gBAM_VXLIB_absDiff_i8u_i8u_o8u_kernel, &gBAM_VXLIB_absDiff_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_absDiff_i8u_i8u_o8u_execFunc, "vxlib_absDiff_i8u_i8u_o8u", BAM_KERNELID_VXLIB_ABSDIFF_I8U_I8U_O8U },
    { &gBAM_VXLIB_add_i16s_i16s_o16s_kernel, &gBAM_VXLIB_add_i16s_i16s_o16s_helperFunc, &gBAM_VXLIB_add_i16s_i16s_o16s_execFunc, "vxlib_add_i16s_i16s_o16s", BAM_KERNELID_VXLIB_ADD_I16S_I16S_O16S },
    { &gBAM_VXLIB_add_i8u_i16s_o16s_kernel, &gBAM_VXLIB_add_i8u_i16s_o16s_helperFunc, &gBAM_VXLIB_add_i8u_i16s_o16s_execFunc, "vxlib_add_i8u_i16s_o16s", BAM_KERNELID_VXLIB_ADD_I8U_I16S_O16S },
    { &gBAM_VXLIB_add_i8u_i8u_o16s_kernel, &gBAM_VXLIB_add_i8u_i8u_o16s_helperFunc, &gBAM_VXLIB_add_i8u_i8u_o16s_execFunc, "vxlib_add_i8u_i8u_o16s", BAM_KERNELID_VXLIB_ADD_I8U_I8U_O16S },
    { &gBAM_VXLIB_add_i8u_i8u_o8u_kernel, &gBAM_VXLIB_add_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_add_i8u_i8u_o8u_execFunc, "vxlib_add_i8u_i8u_o8u", BAM_KERNELID_VXLIB_ADD_I8U_I8U_O8U },
    { &gBAM_VXLIB_addSquare_i8u_i16s_o16s_kernel, &gBAM_VXLIB_addSquare_i8u_i16s_o16s_helperFunc, &gBAM_VXLIB_addSquare_i8u_i16s_o16s_execFunc, "vxlib_addSquare_i8u_i16s_o16s", BAM_KERNELID_VXLIB_ADDSQUARE_I8U_I16S_O16S },
    { &gBAM_VXLIB_addWeight_i8u_i8u_o8u_kernel, &gBAM_VXLIB_addWeight_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_addWeight_i8u_i8u_o8u_execFunc, "vxlib_addWeight_i8u_i8u_o8u", BAM_KERNELID_VXLIB_ADDWEIGHT_I8U_I8U_O8U },
    { &gBAM_VXLIB_and_i8u_i8u_o8u_kernel, &gBAM_VXLIB_and_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_and_i8u_i8u_o8u_execFunc, "vxlib_and_i8u_i8u_o8u", BAM_KERNELID_VXLIB_AND_I8U_I8U_O8U },
    { &gBAM_VXLIB_box_3x3_i8u_o8u_kernel, &gBAM_VXLIB_box_3x3_i8u_o8u_helperFunc, &gBAM_VXLIB_box_3x3_i8u_o8u_execFunc, "vxlib_box_3x3_i8u_o8u", BAM_KERNELID_VXLIB_BOX_3X3_I8U_O8U },
    { &gBAM_VXLIB_cannyNMS_i16s_i16s_i16u_o8u_kernel, &gBAM_VXLIB_cannyNMS_i16s_i16s_i16u_o8u_helperFunc, &gBAM_VXLIB_cannyNMS_i16s_i16s_i16u_o8u_execFunc, "vxlib_cannyNMS_i16s_i16s_i16u_o8u", BAM_KERNELID_VXLIB_CANNYNMS_I16S_I16S_I16U_O8U },
    { &gBAM_VXLIB_channelCopy_1to1_i8u_o8u_kernel, &gBAM_VXLIB_channelCopy_1to1_i8u_o8u_helperFunc, &gBAM_VXLIB_channelCopy_1to1_i8u_o8u_execFunc, "vxlib_channelCopy_1to1_i8u_o8u", BAM_KERNELID_VXLIB_CHANNELCOPY_1TO1_I8U_O8U },
    { &gBAM_VXLIB_channelCombine_2to1_i8u_o8u_kernel, &gBAM_VXLIB_channelCombine_2to1_i8u_o8u_helperFunc, &gBAM_VXLIB_channelCombine_2to1_i8u_o8u_execFunc, "vxlib_channelCombine_2to1_i8u_o8u", BAM_KERNELID_VXLIB_CHANNEL_COMBINE_2TO1_I8U_O8U },
    { &gBAM_VXLIB_channelCombine_3to1_i8u_o8u_kernel, &gBAM_VXLIB_channelCombine_3to1_i8u_o8u_helperFunc, &gBAM_VXLIB_channelCombine_3to1_i8u_o8u_execFunc, "vxlib_channelCombine_3to1_i8u_o8u", BAM_KERNELID_VXLIB_CHANNEL_COMBINE_3TO1_I8U_O8U },
    { &gBAM_VXLIB_channelCombine_4to1_i8u_o8u_kernel, &gBAM_VXLIB_channelCombine_4to1_i8u_o8u_helperFunc, &gBAM_VXLIB_channelCombine_4to1_i8u_o8u_execFunc, "vxlib_channelCombine_4to1_i8u_o8u", BAM_KERNELID_VXLIB_CHANNEL_COMBINE_4TO1_I8U_O8U },
    { &gBAM_VXLIB_channelCombine_yuyv_i8u_o8u_kernel, &gBAM_VXLIB_channelCombine_yuyv_i8u_o8u_helperFunc, &gBAM_VXLIB_channelCombine_yuyv_i8u_o8u_execFunc, "vxlib_channelCombine_yuyv_i8u_o8u", BAM_KERNELID_VXLIB_CHANNEL_COMBINE_YUYV_I8U_O8U },
    { &gBAM_VXLIB_channelExtract_1of2_i8u_o8u_kernel, &gBAM_VXLIB_channelExtract_1of2_i8u_o8u_helperFunc, &gBAM_VXLIB_channelExtract_1of2_i8u_o8u_execFunc, "vxlib_channelExtract_1of2_i8u_o8u", BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF2_I8U_O8U },
    { &gBAM_VXLIB_channelExtract_1of3_i8u_o8u_kernel, &gBAM_VXLIB_channelExtract_1of3_i8u_o8u_helperFunc, &gBAM_VXLIB_channelExtract_1of3_i8u_o8u_execFunc, "vxlib_channelExtract_1of3_i8u_o8u", BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF3_I8U_O8U },
    { &gBAM_VXLIB_channelExtract_1of4_i8u_o8u_kernel, &gBAM_VXLIB_channelExtract_1of4_i8u_o8u_helperFunc, &gBAM_VXLIB_channelExtract_1of4_i8u_o8u_execFunc, "vxlib_channelExtract_1of4_i8u_o8u", BAM_KERNELID_VXLIB_CHANNELEXTRACT_1OF4_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_IYUVtoNV12_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_IYUVtoNV12_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_IYUVtoNV12_i8u_o8u_execFunc, "vxlib_colorConvert_IYUVtoNV12_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoNV12_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_IYUVtoRGB_i8u_o8u_execFunc, "vxlib_colorConvert_IYUVtoRGB_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoRGB_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_IYUVtoRGBX_i8u_o8u_execFunc, "vxlib_colorConvert_IYUVtoRGBX_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoRGBX_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_IYUVtoYUV4_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_IYUVtoYUV4_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_IYUVtoYUV4_i8u_o8u_execFunc, "vxlib_colorConvert_IYUVtoYUV4_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_IYUVtoYUV4_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_NVXXtoIYUV_i8u_o8u_execFunc, "vxlib_colorConvert_NVXXtoIYUV_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoIYUV_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_NVXXtoRGB_i8u_o8u_execFunc, "vxlib_colorConvert_NVXXtoRGB_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoRGB_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_NVXXtoRGBX_i8u_o8u_execFunc, "vxlib_colorConvert_NVXXtoRGBX_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoRGBX_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_NVXXtoYUV4_i8u_o8u_execFunc, "vxlib_colorConvert_NVXXtoYUV4_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_NVXXtoYUV4_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_RGBtoIYUV_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_RGBtoIYUV_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_RGBtoIYUV_i8u_o8u_execFunc, "vxlib_colorConvert_RGBtoIYUV_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoIYUV_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_RGBtoNV12_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_RGBtoNV12_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_RGBtoNV12_i8u_o8u_execFunc, "vxlib_colorConvert_RGBtoNV12_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoNV12_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_RGBtoRGBX_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_RGBtoRGBX_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_RGBtoRGBX_i8u_o8u_execFunc, "vxlib_colorConvert_RGBtoRGBX_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoRGBX_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_RGBtoYUV4_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_RGBtoYUV4_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_RGBtoYUV4_i8u_o8u_execFunc, "vxlib_colorConvert_RGBtoYUV4_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_RGBtoYUV4_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_RGBXtoIYUV_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_RGBXtoIYUV_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_RGBXtoIYUV_i8u_o8u_execFunc, "vxlib_colorConvert_RGBXtoIYUV_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoIYUV_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_RGBXtoNV12_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_RGBXtoNV12_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_RGBXtoNV12_i8u_o8u_execFunc, "vxlib_colorConvert_RGBXtoNV12_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoNV12_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_RGBXtoRGB_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_RGBXtoRGB_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_RGBXtoRGB_i8u_o8u_execFunc, "vxlib_colorConvert_RGBXtoRGB_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoRGB_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_RGBXtoYUV4_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_RGBXtoYUV4_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_RGBXtoYUV4_i8u_o8u_execFunc, "vxlib_colorConvert_RGBXtoYUV4_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_RGBXtoYUV4_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_YUVXtoIYUV_i8u_o8u_execFunc, "vxlib_colorConvert_YUVXtoIYUV_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoIYUV_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_YUVXtoNV12_i8u_o8u_execFunc, "vxlib_colorConvert_YUVXtoNV12_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoNV12_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_YUVXtoRGB_i8u_o8u_execFunc, "vxlib_colorConvert_YUVXtoRGB_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoRGB_I8U_O8U },
    { &gBAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u_kernel, &gBAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u_helperFunc, &gBAM_VXLIB_colorConvert_YUVXtoRGBX_i8u_o8u_execFunc, "vxlib_colorConvert_YUVXtoRGBX_i8u_o8u", BAM_KERNELID_VXLIB_COLORCONVERT_YUVXtoRGBX_I8U_O8U },
    { &gBAM_VXLIB_convertDepth_i16s_o8u_kernel, &gBAM_VXLIB_convertDepth_i16s_o8u_helperFunc, &gBAM_VXLIB_convertDepth_i16s_o8u_execFunc, "vxlib_convertDepth_i16s_o8u", BAM_KERNELID_VXLIB_CONVERTDEPTH_I16S_O8U },
    { &gBAM_VXLIB_convertDepth_i8u_o16s_kernel, &gBAM_VXLIB_convertDepth_i8u_o16s_helperFunc, &gBAM_VXLIB_convertDepth_i8u_o16s_execFunc, "vxlib_convertDepth_i8u_o16s", BAM_KERNELID_VXLIB_CONVERTDEPTH_I8U_O16S },
    { &gBAM_VXLIB_convolve_i8u_c16s_o16s_kernel, &gBAM_VXLIB_convolve_i8u_c16s_o16s_helperFunc, &gBAM_VXLIB_convolve_i8u_c16s_o16s_execFunc, "vxlib_convolve_i8u_c16s_o16s", BAM_KERNELID_VXLIB_CONVOLVE_I8U_C16S_O16S },
    { &gBAM_VXLIB_convolve_i8u_c16s_o8u_kernel, &gBAM_VXLIB_convolve_i8u_c16s_o8u_helperFunc, &gBAM_VXLIB_convolve_i8u_c16s_o8u_execFunc, "vxlib_convolve_i8u_c16s_o8u", BAM_KERNELID_VXLIB_CONVOLVE_I8U_C16S_O8U },
    { &gBAM_VXLIB_dilate_3x3_i8u_o8u_kernel, &gBAM_VXLIB_dilate_3x3_i8u_o8u_helperFunc, &gBAM_VXLIB_dilate_3x3_i8u_o8u_execFunc, "vxlib_dilate_3x3_i8u_o8u", BAM_KERNELID_VXLIB_DILATE_3X3_I8U_O8U },
    { &gBAM_VXLIB_dilate_MxN_i8u_i8u_o8u_kernel, &gBAM_VXLIB_dilate_MxN_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_dilate_MxN_i8u_i8u_o8u_execFunc, "vxlib_dilate_MxN_i8u_i8u_o8u", BAM_KERNELID_VXLIB_DILATE_MXN_I8U_I8U_O8U },
    { &gBAM_VXLIB_doubleThreshold_i16u_i8u_kernel, &gBAM_VXLIB_doubleThreshold_i16u_i8u_helperFunc, &gBAM_VXLIB_doubleThreshold_i16u_i8u_execFunc, "vxlib_doubleThreshold_i16u_i8u", BAM_KERNELID_VXLIB_DOUBLETHRESHOLD_I16S_I8U },
    { &gBAM_VXLIB_erode_3x3_i8u_o8u_kernel, &gBAM_VXLIB_erode_3x3_i8u_o8u_helperFunc, &gBAM_VXLIB_erode_3x3_i8u_o8u_execFunc, "vxlib_erode_3x3_i8u_o8u", BAM_KERNELID_VXLIB_ERODE_3X3_I8U_O8U },
    { &gBAM_VXLIB_erode_MxN_i8u_i8u_o8u_kernel, &gBAM_VXLIB_erode_MxN_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_erode_MxN_i8u_i8u_o8u_execFunc, "vxlib_erode_MxN_i8u_i8u_o8u", BAM_KERNELID_VXLIB_ERODE_MXN_I8U_I8U_O8U },
    { &gBAM_VXLIB_gaussian_3x3_i8u_o8u_kernel, &gBAM_VXLIB_gaussian_3x3_i8u_o8u_helperFunc, &gBAM_VXLIB_gaussian_3x3_i8u_o8u_execFunc, "vxlib_gaussian_3x3_i8u_o8u", BAM_KERNELID_VXLIB_GAUSSIAN_3X3_I8U_O8U },
    { &gBAM_VXLIB_halfScaleGaussian_5x5_i8u_o8u_kernel, &gBAM_VXLIB_halfScaleGaussian_5x5_i8u_o8u_helperFunc, &gBAM_VXLIB_halfScaleGaussian_5x5_i8u_o8u_execFunc, "vxlib_halfScaleGaussian_5x5_i8u_o8u", BAM_KERNELID_VXLIB_HALFSCALEGAUSSIAN_5x5_I8U_O8U },
    { &gBAM_VXLIB_harrisCornersScore_i16s_i16s_o32f_kernel, &gBAM_VXLIB_harrisCornersScore_i16s_i16s_o32f_helperFunc, &gBAM_VXLIB_harrisCornersScore_i16s_i16s_o32f_execFunc, "vxlib_harrisCornersScore_i16s_i16s_o32f", BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I16S_I16S_O32F },
    { &gBAM_VXLIB_harrisCornersScore_i32s_i32s_o32f_kernel, &gBAM_VXLIB_harrisCornersScore_i32s_i32s_o32f_helperFunc, &gBAM_VXLIB_harrisCornersScore_i32s_i32s_o32f_execFunc, "vxlib_harrisCornersScore_i32s_i32s_o32f", BAM_KERNELID_VXLIB_HARRISCORNERSSCORE_I32S_I32S_O32F },
    { &gBAM_VXLIB_histogram_i8u_o32u_kernel, &gBAM_VXLIB_histogram_i8u_o32u_helperFunc, &gBAM_VXLIB_histogram_i8u_o32u_execFunc, "vxlib_histogram_i8u_o32u", BAM_KERNELID_VXLIB_HISTOGRAM_I8U_O32U },
    { &gBAM_VXLIB_histogramSimple_i8u_o32u_kernel, &gBAM_VXLIB_histogramSimple_i8u_o32u_helperFunc, &gBAM_VXLIB_histogramSimple_i8u_o32u_execFunc, "vxlib_histogramSimple_i8u_o32u", BAM_KERNELID_VXLIB_HISTOGRAMSIMPLE_I8U_O32U },
    { &gBAM_VXLIB_integralImage_i8u_o32u_kernel, &gBAM_VXLIB_integralImage_i8u_o32u_helperFunc, &gBAM_VXLIB_integralImage_i8u_o32u_execFunc, "vxlib_integralImage_i8u_o32u", BAM_KERNELID_VXLIB_INTEGRALIMAGE_I8U_O32U },
    { &gBAM_VXLIB_magnitude_i16s_i16s_o16s_kernel, &gBAM_VXLIB_magnitude_i16s_i16s_o16s_helperFunc, &gBAM_VXLIB_magnitude_i16s_i16s_o16s_execFunc, "vxlib_magnitude_i16s_i16s_o16s", BAM_KERNELID_VXLIB_MAGNITUDE_I16S_I16S_O16S },
    { &gBAM_VXLIB_meanStdDev_i8u_o32f_kernel, &gBAM_VXLIB_meanStdDev_i8u_o32f_helperFunc, &gBAM_VXLIB_meanStdDev_i8u_o32f_execFunc, "vxlib_meanStdDev_i8u_o32f", BAM_KERNELID_VXLIB_MEAN_STDDEV_I8U_O32F },
    { &gBAM_VXLIB_median_3x3_i8u_o8u_kernel, &gBAM_VXLIB_median_3x3_i8u_o8u_helperFunc, &gBAM_VXLIB_median_3x3_i8u_o8u_execFunc, "vxlib_median_3x3_i8u_o8u", BAM_KERNELID_VXLIB_MEDIAN_3X3_I8U_O8U },
    { &gBAM_VXLIB_median_MxN_i8u_i8u_o8u_kernel, &gBAM_VXLIB_median_MxN_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_median_MxN_i8u_i8u_o8u_execFunc, "vxlib_median_MxN_i8u_i8u_o8u", BAM_KERNELID_VXLIB_MEDIAN_MXN_I8U_I8U_O8U },
    { &gBAM_VXLIB_minMaxLoc_i16s_kernel, &gBAM_VXLIB_minMaxLoc_i16s_helperFunc, &gBAM_VXLIB_minMaxLoc_i16s_execFunc, "vxlib_minMaxLoc_i16s", BAM_KERNELID_VXLIB_MINMAXLOC_I16S },
    { &gBAM_VXLIB_minMaxLoc_i8u_kernel, &gBAM_VXLIB_minMaxLoc_i8u_helperFunc, &gBAM_VXLIB_minMaxLoc_i8u_execFunc, "vxlib_minMaxLoc_i8u", BAM_KERNELID_VXLIB_MINMAXLOC_I8U },
    { &gBAM_VXLIB_multiply_i16s_i16s_o16s_kernel, &gBAM_VXLIB_multiply_i16s_i16s_o16s_helperFunc, &gBAM_VXLIB_multiply_i16s_i16s_o16s_execFunc, "vxlib_multiply_i16s_i16s_o16s", BAM_KERNELID_VXLIB_MULTIPLY_I16S_I16S_O16S },
    { &gBAM_VXLIB_multiply_i8u_i16s_o16s_kernel, &gBAM_VXLIB_multiply_i8u_i16s_o16s_helperFunc, &gBAM_VXLIB_multiply_i8u_i16s_o16s_execFunc, "vxlib_multiply_i8u_i16s_o16s", BAM_KERNELID_VXLIB_MULTIPLY_I8U_I16S_O16S },
    { &gBAM_VXLIB_multiply_i8u_i8u_o16s_kernel, &gBAM_VXLIB_multiply_i8u_i8u_o16s_helperFunc, &gBAM_VXLIB_multiply_i8u_i8u_o16s_execFunc, "vxlib_multiply_i8u_i8u_o16s", BAM_KERNELID_VXLIB_MULTIPLY_I8U_I8U_O16S },
    { &gBAM_VXLIB_multiply_i8u_i8u_o8u_kernel, &gBAM_VXLIB_multiply_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_multiply_i8u_i8u_o8u_execFunc, "vxlib_multiply_i8u_i8u_o8u", BAM_KERNELID_VXLIB_MULTIPLY_I8U_I8U_O8U },
    { &gBAM_VXLIB_normL1_i16s_i16s_o16u_kernel, &gBAM_VXLIB_normL1_i16s_i16s_o16u_helperFunc, &gBAM_VXLIB_normL1_i16s_i16s_o16u_execFunc, "vxlib_normL1_i16s_i16s_o16u", BAM_KERNELID_VXLIB_NORML1_I16S_I16S_O16U },
    { &gBAM_VXLIB_normL2_i16s_i16s_o16u_kernel, &gBAM_VXLIB_normL2_i16s_i16s_o16u_helperFunc, &gBAM_VXLIB_normL2_i16s_i16s_o16u_execFunc, "vxlib_normL2_i16s_i16s_o16u", BAM_KERNELID_VXLIB_NORML2_I16S_I16S_O16U },
    { &gBAM_VXLIB_not_i8u_o8u_kernel, &gBAM_VXLIB_not_i8u_o8u_helperFunc, &gBAM_VXLIB_not_i8u_o8u_execFunc, "vxlib_not_i8u_o8u", BAM_KERNELID_VXLIB_NOT_I8U_O8U },
    { &gBAM_VXLIB_or_i8u_i8u_o8u_kernel, &gBAM_VXLIB_or_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_or_i8u_i8u_o8u_execFunc, "vxlib_or_i8u_i8u_o8u", BAM_KERNELID_VXLIB_OR_I8U_I8U_O8U },
    { &gBAM_VXLIB_phase_i16s_i16s_o8u_kernel, &gBAM_VXLIB_phase_i16s_i16s_o8u_helperFunc, &gBAM_VXLIB_phase_i16s_i16s_o8u_execFunc, "vxlib_phase_i16s_i16s_o8u", BAM_KERNELID_VXLIB_PHASE_I16S_I16S_O8U },
    { &gBAM_VXLIB_scaleImageNearest_i8u_o8u_kernel, &gBAM_VXLIB_scaleImageNearest_i8u_o8u_helperFunc, &gBAM_VXLIB_scaleImageNearest_i8u_o8u_execFunc, "vxlib_scaleImageNearest_i8u_o8u", BAM_KERNELID_VXLIB_SCALEIMAGENEAREST_I8U_O8U },
    { &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_helperFunc, &gBAM_VXLIB_sobel_3x3_i8u_o16s_o16s_execFunc, "vxlib_sobel_3x3_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobel_5x5_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_5x5_i8u_o16s_o16s_helperFunc, &gBAM_VXLIB_sobel_5x5_i8u_o16s_o16s_execFunc, "vxlib_sobel_5x5_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_5X5_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobel_7x7_i8u_o16s_o16s_kernel, &gBAM_VXLIB_sobel_7x7_i8u_o16s_o16s_helperFunc, &gBAM_VXLIB_sobel_7x7_i8u_o16s_o16s_execFunc, "vxlib_sobel_7x7_i8u_o16s_o16s", BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O16S_O16S },
    { &gBAM_VXLIB_sobel_7x7_i8u_o32s_o32s_kernel, &gBAM_VXLIB_sobel_7x7_i8u_o32s_o32s_helperFunc, &gBAM_VXLIB_sobel_7x7_i8u_o32s_o32s_execFunc, "vxlib_sobel_7x7_i8u_o32s_o32s", BAM_KERNELID_VXLIB_SOBEL_7X7_I8U_O32S_O32S },
    { &gBAM_VXLIB_sobelX_3x3_i8u_o16s_kernel, &gBAM_VXLIB_sobelX_3x3_i8u_o16s_helperFunc, &gBAM_VXLIB_sobelX_3x3_i8u_o16s_execFunc, "vxlib_sobelX_3x3_i8u_o16s", BAM_KERNELID_VXLIB_SOBELX_3X3_I8U_O16S },
    { &gBAM_VXLIB_sobelY_3x3_i8u_o16s_kernel, &gBAM_VXLIB_sobelY_3x3_i8u_o16s_helperFunc, &gBAM_VXLIB_sobelY_3x3_i8u_o16s_execFunc, "vxlib_sobelY_3x3_i8u_o16s", BAM_KERNELID_VXLIB_SOBELY_3X3_I8U_O16S },
    { &gBAM_VXLIB_subtract_i16s_i16s_o16s_kernel, &gBAM_VXLIB_subtract_i16s_i16s_o16s_helperFunc, &gBAM_VXLIB_subtract_i16s_i16s_o16s_execFunc, "vxlib_subtract_i16s_i16s_o16s", BAM_KERNELID_VXLIB_SUBTRACT_I16S_I16S_O16S },
    { &gBAM_VXLIB_subtract_i8u_i16s_o16s_kernel, &gBAM_VXLIB_subtract_i8u_i16s_o16s_helperFunc, &gBAM_VXLIB_subtract_i8u_i16s_o16s_execFunc, "vxlib_subtract_i8u_i16s_o16s", BAM_KERNELID_VXLIB_SUBTRACT_I8U_I16S_O16S },
    { &gBAM_VXLIB_subtract_i8u_i8u_o16s_kernel, &gBAM_VXLIB_subtract_i8u_i8u_o16s_helperFunc, &gBAM_VXLIB_subtract_i8u_i8u_o16s_execFunc, "vxlib_subtract_i8u_i8u_o16s", BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O16S },
    { &gBAM_VXLIB_subtract_i8u_i8u_o8u_kernel, &gBAM_VXLIB_subtract_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_subtract_i8u_i8u_o8u_execFunc, "vxlib_subtract_i8u_i8u_o8u", BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O8U },
    { &gBAM_VXLIB_tableLookup_i16s_o16s_kernel, &gBAM_VXLIB_tableLookup_i16s_o16s_helperFunc, &gBAM_VXLIB_tableLookup_i16s_o16s_execFunc, "vxlib_tableLookup_i16s_o16s", BAM_KERNELID_VXLIB_TABLELOOKUP_I16S_O16S },
    { &gBAM_VXLIB_tableLookup_i8u_o8u_kernel, &gBAM_VXLIB_tableLookup_i8u_o8u_helperFunc, &gBAM_VXLIB_tableLookup_i8u_o8u_execFunc, "vxlib_tableLookup_i8u_o8u", BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U },
    { &gBAM_VXLIB_thresholdBinary_i8u_o8u_kernel, &gBAM_VXLIB_thresholdBinary_i8u_o8u_helperFunc, &gBAM_VXLIB_thresholdBinary_i8u_o8u_execFunc, "vxlib_thresholdBinary_i8u_o8u", BAM_KERNELID_VXLIB_THRESHOLDBINARY_I8U_O8U },
    { &gBAM_VXLIB_thresholdRange_i8u_o8u_kernel, &gBAM_VXLIB_thresholdRange_i8u_o8u_helperFunc, &gBAM_VXLIB_thresholdRange_i8u_o8u_execFunc, "vxlib_thresholdRange_i8u_o8u", BAM_KERNELID_VXLIB_THRESHOLDRANGE_I8U_O8U },
    { &gBAM_VXLIB_xor_i8u_i8u_o8u_kernel, &gBAM_VXLIB_xor_i8u_i8u_o8u_helperFunc, &gBAM_VXLIB_xor_i8u_i8u_o8u_execFunc, "vxlib_xor_i8u_i8u_o8u", BAM_KERNELID_VXLIB_XOR_I8U_I8U_O8U }
};

BAM_KernelDBdef gBAM_TI_kernelDBdef =
{
    TIVX_MAX_DSP_BAM_PLUGINS,
    bamKernelHostDB,
    bamKernelExecFuncDB
};

vx_status tivxRegisterOpenVXCoreBamPlugins(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i;
    vx_uint32 num_kernels = dimof(bamKernelDB);
    vx_uint32 num_host_kernels = dimof(bamKernelHostDB);
    vx_uint32 num_exec_kernels = dimof(bamKernelExecFuncDB);

    /* Clear database memory */
    memset((void *)&bamKernelHostDB, 0, sizeof(bamKernelHostDB));
    memset((void *)&bamKernelExecFuncDB, 0, sizeof(bamKernelExecFuncDB));
    gNumActivePlugins = 0;

    if( (num_host_kernels < num_kernels) ||
        (num_exec_kernels < num_kernels))
    {
        status = (vx_status)VX_ERROR_NO_MEMORY;
        VX_PRINT(VX_ZONE_ERROR, "BAM DB size is too small\n");
    }
    else
    {
        for (i = 0; i < num_kernels; i++)
        {
            bamKernelHostDB[i].kernelInfo = bamKernelDB[i].kernelInfo;
            bamKernelHostDB[i].kernelHelperFunc = bamKernelDB[i].kernelHelperFunc;
            bamKernelHostDB[i].name = bamKernelDB[i].name;
            bamKernelHostDB[i].kernelId = bamKernelDB[i].kernelId;

            bamKernelExecFuncDB[i].kernelInfo = bamKernelDB[i].kernelInfo;
            bamKernelExecFuncDB[i].kernelExecFunc = bamKernelDB[i].kernelExecFunc;
            bamKernelExecFuncDB[i].name = bamKernelDB[i].name;
            bamKernelExecFuncDB[i].kernelId = bamKernelDB[i].kernelId;
        }
        for (; i < TIVX_MAX_DSP_BAM_PLUGINS; i++)
        {
            bamKernelHostDB[i].kernelInfo = &dummyKernelInfo;
            bamKernelHostDB[i].kernelId = i;
            bamKernelExecFuncDB[i].kernelId = i;
        }

        gNumActivePlugins = num_kernels;
    }

    return status;
}

vx_status tivxBamRegisterPlugin(tivx_bam_plugin_def *plugin, BAM_KernelId *kernelId)
{
    vx_status status = (vx_status)VX_SUCCESS;
    BAM_KernelId local_kernel_id;

    *kernelId = (BAM_KernelId)BAM_TI_KERNELID_UNDEFINED;

    if ((NULL == plugin) ||
        (NULL == kernelId))
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "Parameter is NULL\n");
    }

    /* First, check if plugin is already in DB */
    status = tivxBamGetKernelIdFromName(plugin->name, &local_kernel_id);

    if((vx_status)VX_SUCCESS == status)
    {
        if ((BAM_KernelId)BAM_TI_KERNELID_UNDEFINED == local_kernel_id)
        {
            vx_uint32 num = gNumActivePlugins;

            if(num < TIVX_MAX_DSP_BAM_PLUGINS)
            {
                bamKernelHostDB[num].kernelInfo = (BAM_KernelInfo *)plugin->kernelInfo;
                bamKernelHostDB[num].kernelHelperFunc = (BAM_KernelHelperFuncDef *)plugin->kernelHelperFunc;
                bamKernelHostDB[num].name = (char *)plugin->name;
                bamKernelHostDB[num].kernelId = num;

                bamKernelExecFuncDB[num].kernelInfo = (BAM_KernelInfo *)plugin->kernelInfo;
                bamKernelExecFuncDB[num].kernelExecFunc = (BAM_KernelExecFuncDef *)plugin->kernelExecFunc;
                bamKernelExecFuncDB[num].name = (char *)plugin->name;
                bamKernelExecFuncDB[num].kernelId = num;

                *kernelId = num;

                gNumActivePlugins++;
            }
            else
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
                VX_PRINT(VX_ZONE_ERROR, "Not enough memory to add new plugin\n");
                VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_MAX_DSP_BAM_USER_PLUGINS in tiovx/include/TI/tivx_config.h\n");
            }
        }
        else
        {
            *kernelId = local_kernel_id;
        }
    }

    return status;
}

vx_status tivxBamGetKernelIdFromName(const char *name, BAM_KernelId *kernelId)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i;

    if ((NULL == name) ||
        (NULL == kernelId))
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "Parameter is NULL\n");
    }
    else
    {
        *kernelId = (BAM_KernelId)BAM_TI_KERNELID_UNDEFINED;

        if((vx_status)VX_SUCCESS == status)
        {
            for(i = BAM_KERNELID_EXTERNAL_START; i < gNumActivePlugins; i++)
            {
                if(strncmp(name, bamKernelHostDB[i].name, VX_MAX_KERNEL_NAME) == 0)
                {
                    *kernelId = bamKernelHostDB[i].kernelId;
                    break;
                }
            }
        }
    }

    return status;
}
