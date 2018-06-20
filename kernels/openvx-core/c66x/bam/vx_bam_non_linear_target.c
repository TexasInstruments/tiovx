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
#include <tivx_kernel_non_linear_filter.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
} tivxNonLinearFiltParams;

static tivx_target_kernel vx_lut_target_kernel = NULL;

static vx_status VX_CALLBACK tivxBamKernelNonLinearFilterProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelNonLinearFilterCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelNonLinearFilterDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelNonLinearFilterControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelNonLinearFilterProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxNonLinearFiltParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    vx_uint8 *src_addr, *dst_addr;
    uint32_t size;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_SRC_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_NON_LINEAR_FILTER_DST_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxNonLinearFiltParams) != size))
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
        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxBamKernelNonLinearFilterCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_matrix_t *mask;
    tivx_obj_desc_scalar_t *function_desc;
    tivxNonLinearFiltParams *prms = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        void *mask_target_ptr;

        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_NON_LINEAR_FILTER_SRC_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_NON_LINEAR_FILTER_DST_IDX];
        mask = (tivx_obj_desc_matrix_t *)obj_desc[
            TIVX_KERNEL_NON_LINEAR_FILTER_MASK_IDX];
        function_desc = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_NON_LINEAR_FILTER_FUNCTION_IDX];

        mask_target_ptr = tivxMemShared2TargetPtr(
            mask->mem_ptr.shared_ptr, mask->mem_ptr.mem_heap_region);

        tivxMemBufferMap(mask_target_ptr, mask->mem_size,
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        prms = tivxMemAlloc(sizeof(tivxNonLinearFiltParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_kernel_details_t kernel_details;
            VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
            VXLIB_bufParams2D_t *buf_params[2];

            memset(prms, 0, sizeof(tivxNonLinearFiltParams));

            tivxInitBufParams(src, &vxlib_src);
            tivxInitBufParams(dst, &vxlib_dst);

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src;
            buf_params[1] = &vxlib_dst;

            if (VX_NONLINEAR_FILTER_MIN == function_desc->data.enm)
            {
                BAM_VXLIB_erode_MxN_i8u_i8u_o8u_params kernel_params;

                kernel_params.mask_addr = mask_target_ptr;
                kernel_params.mask.dim_x    = mask->columns;
                kernel_params.mask.dim_y    = mask->rows;
                kernel_params.mask.stride_y = mask->columns;
                kernel_params.mask.data_type = VXLIB_UINT8;

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_erode_MxN_i8u_i8u_o8u_getKernelInfo(
                    &kernel_params, &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_ERODE_MXN_I8U_I8U_O8U,
                    buf_params, &kernel_details,
                    &prms->graph_handle);
            }
            else if (VX_NONLINEAR_FILTER_MAX == function_desc->data.enm)
            {
                BAM_VXLIB_dilate_MxN_i8u_i8u_o8u_params kernel_params;

                kernel_params.mask_addr = mask_target_ptr;
                kernel_params.mask.dim_x    = mask->columns;
                kernel_params.mask.dim_y    = mask->rows;
                kernel_params.mask.stride_y = mask->columns;

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_dilate_MxN_i8u_i8u_o8u_getKernelInfo(
                    &kernel_params, &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_DILATE_MXN_I8U_I8U_O8U,
                    buf_params, &kernel_details,
                    &prms->graph_handle);
            }
            else
            {
                BAM_VXLIB_median_MxN_i8u_i8u_o8u_params kernel_params;

                kernel_params.mask_addr = mask_target_ptr;
                kernel_params.mask.dim_x    = mask->columns;
                kernel_params.mask.dim_y    = mask->rows;
                kernel_params.mask.stride_y = mask->columns;
                kernel_params.scratch_size  = mask->columns * mask->rows *
                    2 * sizeof(int64_t);

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_median_MxN_i8u_i8u_o8u_getKernelInfo(
                    &kernel_params, &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_MEDIAN_MXN_I8U_I8U_O8U,
                    buf_params, &kernel_details,
                    &prms->graph_handle);
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxNonLinearFiltParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxNonLinearFiltParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxBamKernelNonLinearFilterDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxNonLinearFiltParams *prms = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_NON_LINEAR_FILTER_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxNonLinearFiltParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, sizeof(tivxNonLinearFiltParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxBamKernelNonLinearFilterControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamNonLinearFilter(void)
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

        vx_lut_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_NON_LINEAR_FILTER,
            target_name,
            tivxBamKernelNonLinearFilterProcess,
            tivxBamKernelNonLinearFilterCreate,
            tivxBamKernelNonLinearFilterDelete,
            tivxBamKernelNonLinearFilterControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamNonLinearFilter(void)
{
    tivxRemoveTargetKernel(vx_lut_target_kernel);
}
