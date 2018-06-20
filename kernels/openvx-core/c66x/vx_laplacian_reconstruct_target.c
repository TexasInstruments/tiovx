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
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_laplacian_reconstruct.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_laplacian_reconstruct_target_kernel = NULL;

typedef struct
{
    tivx_obj_desc_image_t *img_obj_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    uint8_t *upsample_output;
    uint8_t *gauss_output;
    int16_t *add_output;
    uint32_t buff_size;

    VXLIB_bufParams2D_t vxlib_src, vxlib_laplac, vxlib_scratch;
    VXLIB_bufParams2D_t vxlib_add;
    VXLIB_bufParams2D_t vxlib_dst;
} tivxLaplacianReconstructParams;

static vx_status VX_CALLBACK tivxKernelLplRcstrctProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    int32_t level;
    tivxLaplacianReconstructParams *prms = NULL;
    tivx_obj_desc_image_t *low_img, *out_img, *pyd_level;
    tivx_obj_desc_pyramid_t *pmd;
    int16_t *laplac_addr;
    uint8_t *src_addr, *dst_addr;
    uint32_t size;
    vx_rectangle_t rect;

    if (num_params != TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }
    if (VX_SUCCESS == status)
    {
        low_img = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LPL_RCNSTR_IN_IMG_IDX];
        pmd = (tivx_obj_desc_pyramid_t *)obj_desc[
            TIVX_KERNEL_LPL_RCNSTR_IN_PMD_IDX];
        out_img = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LPL_RCNSTR_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            if ((NULL != prms) &&
                (size == sizeof(tivxLaplacianReconstructParams)))
            {
                tivxGetObjDescList(pmd->obj_desc_id,
                    (tivx_obj_desc_t **)prms->img_obj_desc, pmd->num_levels);

                for (i = 0U; i < pmd->num_levels; i ++)
                {
                    if (NULL == prms->img_obj_desc[i])
                    {
                        status = VX_FAILURE;
                        break;
                    }
                }
            }
            else
            {
                status = VX_FAILURE;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        void *low_img_target_ptr;
        void *out_img_target_ptr;
        void *pyd_level_target_ptr;

        low_img_target_ptr = tivxMemShared2TargetPtr(
            low_img->mem_ptr[0].shared_ptr, low_img->mem_ptr[0].mem_heap_region);
        out_img_target_ptr = tivxMemShared2TargetPtr(
            out_img->mem_ptr[0].shared_ptr, out_img->mem_ptr[0].mem_heap_region);

        tivxMemBufferMap(low_img_target_ptr, low_img->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferMap(out_img_target_ptr, out_img->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        /* Input is 8-bit image ... need to convert to 16 */
        tivxInitBufParams(low_img, &prms->vxlib_scratch);

        prms->vxlib_src.dim_x = prms->vxlib_scratch.dim_x;
        prms->vxlib_src.dim_y = prms->vxlib_scratch.dim_y;
        prms->vxlib_src.stride_y = prms->vxlib_scratch.dim_x*2u;
        prms->vxlib_src.data_type = VXLIB_INT16;

        src_addr = (uint8_t *)prms->add_output;
        tivxSetPointerLocation(out_img, &out_img_target_ptr, &dst_addr);

        status = VXLIB_convertDepth_i8u_o16s(
            (uint8_t *)low_img_target_ptr, &prms->vxlib_scratch,
            (int16_t *)src_addr, &prms->vxlib_src, 0);

        /* Reinterpret 16-bit version of low_img as an 8 bit image, where every other byte is 0 */
        prms->vxlib_src.dim_x = prms->vxlib_scratch.dim_x*2u;
        prms->vxlib_src.data_type = VXLIB_UINT8;

        for (level = pmd->num_levels-1; (level >= 0) && (VX_SUCCESS == status);
                level --)
        {
            pyd_level = prms->img_obj_desc[level];

            pyd_level_target_ptr = tivxMemShared2TargetPtr(
                pyd_level->mem_ptr[0].shared_ptr,
                pyd_level->mem_ptr[0].mem_heap_region);
            tivxMemBufferMap(pyd_level_target_ptr,
                pyd_level->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);

            tivxSetPointerLocation(pyd_level, &pyd_level_target_ptr, (uint8_t**)&laplac_addr);

            rect = pyd_level->valid_roi;

            prms->vxlib_scratch.dim_x = rect.end_x - rect.start_x;
            prms->vxlib_scratch.dim_y = (rect.end_y - rect.start_y)/2u;
            prms->vxlib_scratch.stride_y = (rect.end_x - rect.start_x)*2;
            prms->vxlib_scratch.data_type = VXLIB_UINT8;

            prms->vxlib_laplac.dim_x = rect.end_x - rect.start_x;
            prms->vxlib_laplac.dim_y = rect.end_y - rect.start_y;
            prms->vxlib_laplac.stride_y = pyd_level->imagepatch_addr[0].stride_y;
            prms->vxlib_laplac.data_type = VXLIB_INT16;

            prms->vxlib_add.dim_x = rect.end_x - rect.start_x;;
            prms->vxlib_add.dim_y = rect.end_y - rect.start_y;
            prms->vxlib_add.stride_y = (rect.end_x - rect.start_x)*2;
            prms->vxlib_add.data_type = VXLIB_INT16;

            /* First upsample previous stage result */
            status = VXLIB_channelCopy_1to1_i8u_o8u(src_addr, &prms->vxlib_src,
                prms->upsample_output, &prms->vxlib_scratch);

            prms->vxlib_scratch.dim_y = rect.end_y - rect.start_y;
            prms->vxlib_scratch.stride_y = rect.end_x - rect.start_x;

            if (VXLIB_SUCCESS == status)
            {
                /* Then do gaussian filter with * 4 multiply on upsampled result */
                status = VXLIB_gaussian_5x5_br_i8u_o8u(
                    prms->upsample_output, &prms->vxlib_scratch,
                    prms->gauss_output, &prms->vxlib_scratch, 6u, 0, 0);
            }

            if (VXLIB_SUCCESS == status)
            {
                /* Then add gaussian filtered upsample to laplacian of this level */
                status = VXLIB_add_i8u_i16s_o16s(
                    prms->gauss_output, &prms->vxlib_scratch,
                    laplac_addr, &prms->vxlib_laplac,
                    prms->add_output, &prms->vxlib_add, VXLIB_CONVERT_POLICY_SATURATE);
            }

            if (VXLIB_SUCCESS == status)
            {
                if(level == 0)
                {
                    tivxInitBufParams(out_img, &prms->vxlib_dst);

                    status = VXLIB_convertDepth_i16s_o8u(
                        prms->add_output, &prms->vxlib_add,
                        dst_addr, &prms->vxlib_dst, 0, VXLIB_CONVERT_POLICY_SATURATE);
                }
                else
                {
                    /* Prepare for next level */
                    prms->vxlib_src.dim_x = prms->vxlib_add.dim_x*2u;
                    prms->vxlib_src.dim_y = prms->vxlib_add.dim_y;
                    prms->vxlib_src.stride_y = prms->vxlib_add.stride_y;

                    memset(prms->upsample_output, 0, prms->vxlib_src.dim_x*prms->vxlib_src.dim_y*2u);
                }
            }

            tivxMemBufferUnmap(pyd_level_target_ptr,
                pyd_level->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_READ_ONLY);

            if (status != VXLIB_SUCCESS)
            {
                status = VX_FAILURE;
                break;
            }
        }

        tivxMemBufferUnmap(low_img_target_ptr, low_img->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(out_img_target_ptr, out_img->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplRcstrctCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_pyramid_t *pmd;
    tivxLaplacianReconstructParams *prms = NULL;

    if (num_params != TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        pmd = (tivx_obj_desc_pyramid_t *)obj_desc[
            TIVX_KERNEL_LPL_RCNSTR_IN_PMD_IDX];

        prms = tivxMemAlloc(sizeof(tivxLaplacianReconstructParams),
            TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxLaplacianReconstructParams));

            prms->buff_size = (pmd->width * pmd->height * 2u);

            prms->add_output = tivxMemAlloc(prms->buff_size,
                TIVX_MEM_EXTERNAL);

            if (NULL == prms->add_output)
            {
                status = VX_ERROR_NO_MEMORY;
            }

            if (VX_SUCCESS == status)
            {
                prms->upsample_output = tivxMemAlloc(prms->buff_size/2u,
                    TIVX_MEM_EXTERNAL);

                if (NULL == prms->upsample_output)
                {
                    status = VX_ERROR_NO_MEMORY;
                    tivxMemFree(prms->add_output, prms->buff_size,
                        TIVX_MEM_EXTERNAL);
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->gauss_output = tivxMemAlloc(prms->buff_size/2u,
                    TIVX_MEM_EXTERNAL);

                if (NULL == prms->gauss_output)
                {
                    status = VX_ERROR_NO_MEMORY;
                    tivxMemFree(prms->add_output, prms->buff_size,
                        TIVX_MEM_EXTERNAL);
                    tivxMemFree(prms->upsample_output, prms->buff_size/2u,
                        TIVX_MEM_EXTERNAL);
                }
                else
                {
                    memset(prms->upsample_output, 0, prms->buff_size/2u);
                }
            }

            if (VX_SUCCESS == status)
            {
                tivxSetTargetKernelInstanceContext(kernel, prms,
                    sizeof(tivxLaplacianReconstructParams));
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplRcstrctDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxLaplacianReconstructParams *prms = NULL;

    if (num_params != TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LPL_RCNSTR_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxLaplacianReconstructParams) == size))
        {
            if (NULL != prms->add_output)
            {
                tivxMemFree(prms->add_output, prms->buff_size,
                    TIVX_MEM_EXTERNAL);
            }
            if (NULL != prms->upsample_output)
            {
                tivxMemFree(prms->upsample_output, prms->buff_size/2u,
                    TIVX_MEM_EXTERNAL);
            }
            if (NULL != prms->gauss_output)
            {
                tivxMemFree(prms->gauss_output, prms->buff_size/2u,
                    TIVX_MEM_EXTERNAL);
            }
            tivxMemFree(prms, size, TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplRcstrctControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelLaplacianReconstruct(void)
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

        vx_laplacian_reconstruct_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_LAPLACIAN_RECONSTRUCT,
            target_name,
            tivxKernelLplRcstrctProcess,
            tivxKernelLplRcstrctCreate,
            tivxKernelLplRcstrctDelete,
            tivxKernelLplRcstrctControl,
            NULL);
    }
}


void tivxRemoveTargetKernelLaplacianReconstruct(void)
{
    tivxRemoveTargetKernel(vx_laplacian_reconstruct_target_kernel);
}
