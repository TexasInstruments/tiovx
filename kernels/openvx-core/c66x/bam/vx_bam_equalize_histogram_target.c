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



#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_equalize_histogram.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

#define SCRATCH_BUFFER_SIZE         (1024)

typedef struct
{
    tivx_bam_graph_handle hist_graph_handle;
    tivx_bam_graph_handle lut_graph_handle;

    uint32_t *scratch;
    uint32_t  scratch_size;

    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
} tivxEqHistParams;

static tivx_target_kernel vx_eq_hist_target_kernel = NULL;

static vx_status VX_CALLBACK tivxBamKernelEqHistProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelEqHistCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelEqHistDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelEqHistControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelEqHistProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxEqHistParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    vx_uint8 *src_addr, *dst_addr;
    uint32_t size;
    BAM_VXLIB_histogramSimple_i8u_o32u_params hist_params;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_EQUALIZE_HISTOGRAM_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_EQUALIZE_HISTOGRAM_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxEqHistParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[2];
        void *src_target_ptr;
        void *dst_target_ptr;

        src_target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0U].shared_ptr, src->mem_ptr[0U].mem_heap_region);
        dst_target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0U].shared_ptr, dst->mem_ptr[0U].mem_heap_region);

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);
        tivxSetPointerLocation(dst, &dst_target_ptr, &dst_addr);

        img_ptrs[0] = src_addr;
        img_ptrs[1] = dst_addr;

        /* Update the pointers in histogram and Lut kernels */
        tivxBamUpdatePointers(prms->hist_graph_handle, 1U, 0U, img_ptrs);
        tivxBamUpdatePointers(prms->lut_graph_handle, 1U, 1U, img_ptrs);

        hist_params.dist = prms->scratch;
        hist_params.minValue = 0xffffffffu;
        tivxBamControlNode(prms->hist_graph_handle, 0,
                           VXLIB_HISTOGRAMSIMPLE_I8U_O32U_CMD_SET_PARAMS,
                           &hist_params);
        status  = tivxBamProcessGraph(prms->hist_graph_handle);
        tivxBamControlNode(prms->hist_graph_handle, 0,
                           VXLIB_HISTOGRAMSIMPLE_I8U_O32U_CMD_GET_PARAMS,
                           &hist_params);

        status = VXLIB_histogramCdfLut_i32u_o8u(prms->scratch, NULL,
            (uint8_t*)prms->scratch,
            prms->vxlib_src.dim_x * prms->vxlib_src.dim_y,
            hist_params.minValue);

        status  = tivxBamProcessGraph(prms->lut_graph_handle);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxBamKernelEqHistCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivxEqHistParams *prms = NULL;
    tivx_bam_kernel_details_t kernel_details;
    BAM_VXLIB_tableLookup_i8u_o8u_params lut_kernel_params;
    BAM_VXLIB_histogramSimple_i8u_o32u_params hist_kernel_params;
    VXLIB_bufParams2D_t *buf_params[2];
    uint32_t lut_kern_create = 0, hist_kern_create = 0;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_EQUALIZE_HISTOGRAM_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_EQUALIZE_HISTOGRAM_OUT_IMG_IDX];

        prms = tivxMemAlloc(sizeof(tivxEqHistParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxEqHistParams));

            prms->scratch = tivxMemAlloc(SCRATCH_BUFFER_SIZE *
                sizeof(uint32_t), TIVX_MEM_EXTERNAL);

            if (NULL == prms->scratch)
            {
                status = VX_ERROR_NO_MEMORY;
                tivxMemFree(prms, sizeof(tivxEqHistParams), TIVX_MEM_EXTERNAL);
                prms = NULL;
            }
            else
            {
                memset(prms->scratch, 0, SCRATCH_BUFFER_SIZE *
                    sizeof(uint32_t));
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxInitBufParams(src, &prms->vxlib_src);
            tivxInitBufParams(dst, &prms->vxlib_dst);

            buf_params[0] = &prms->vxlib_src;
            buf_params[1] = &prms->vxlib_dst;
        }

        if (VX_SUCCESS == status)
        {
            lut_kernel_params.lut    = (uint8_t *)prms->scratch;
            lut_kernel_params.count  = 256U;

            kernel_details.compute_kernel_params = (void*)&lut_kernel_params;

            BAM_VXLIB_tableLookup_i8u_o8u_getKernelInfo(
                &lut_kernel_params, &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(
                BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U,
                buf_params, &kernel_details, &prms->lut_graph_handle);

            if (VX_SUCCESS == status)
            {
                lut_kern_create = 1;
            }
        }
        if (VX_SUCCESS == status)
        {
            hist_kernel_params.dist    = prms->scratch;
            hist_kernel_params.minValue  = 0xffffffffu;

            kernel_details.compute_kernel_params = (void*)&hist_kernel_params;

            BAM_VXLIB_histogramSimple_i8u_o32u_getKernelInfo(
                &hist_kernel_params, &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_HISTOGRAMSIMPLE_I8U_O32U,
                buf_params, &kernel_details,
                &prms->hist_graph_handle);
            if (VX_SUCCESS == status)
            {
                hist_kern_create = 1;
            }
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxEqHistParams));
        }
        else
        {
            if (lut_kern_create)
            {
                tivxBamDestroyHandle(prms->lut_graph_handle);
            }
            if (hist_kern_create)
            {
                tivxBamDestroyHandle(prms->hist_graph_handle);
            }
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxEqHistParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxBamKernelEqHistDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxEqHistParams *prms = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxEqHistParams) == size))
        {
            tivxBamDestroyHandle(prms->hist_graph_handle);
            tivxBamDestroyHandle(prms->lut_graph_handle);
            tivxMemFree(prms->scratch, SCRATCH_BUFFER_SIZE *
                    sizeof(uint32_t), TIVX_MEM_EXTERNAL);
            tivxMemFree(prms, sizeof(tivxEqHistParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxBamKernelEqHistControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamEqHist(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_DSP1) || (self_cpu == TIVX_CPU_ID_DSP2))
    {
        if (self_cpu == TIVX_CPU_ID_DSP1)
        {
            strncpy(target_name, TIVX_TARGET_DSP1,
                TIVX_TARGET_MAX_NAME);
        }
        else
        {
            strncpy(target_name, TIVX_TARGET_DSP2,
                TIVX_TARGET_MAX_NAME);
        }

        vx_eq_hist_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_EQUALIZE_HISTOGRAM,
            target_name,
            tivxBamKernelEqHistProcess,
            tivxBamKernelEqHistCreate,
            tivxBamKernelEqHistDelete,
            tivxBamKernelEqHistControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamEqHist(void)
{
    tivxRemoveTargetKernel(vx_eq_hist_target_kernel);
}
