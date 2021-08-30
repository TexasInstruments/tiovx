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
#include <tivx_kernel_laplacian_pyramid.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_laplacian_pyramid_target_kernel = NULL;

typedef struct
{
    tivx_obj_desc_image_t *img_obj_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    uint8_t *upsample_output;
    uint8_t *gauss_output;
    uint8_t *hsg_output[2];
    uint32_t buff_size;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    VXLIB_bufParams2D_t vxlib_gauss0, vxlib_gauss1;
} tivxLaplacianPyramidParams;

static vx_status VX_CALLBACK tivxKernelLplPmdProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelLplPmdCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelLplPmdDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelLplPmdProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    tivxLaplacianPyramidParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst, *low_img;
    tivx_obj_desc_pyramid_t *pmd;
    uint8_t *src_addr;
    int16_t *dst_addr;
    uint8_t *out_addr;
    uint32_t size, levels;
    vx_rectangle_t rect;

    if (num_params != TIVX_KERNEL_LAPLACIAN_PYRAMID_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LAPLACIAN_PYRAMID_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LAPLACIAN_PYRAMID_INPUT_IDX];
        pmd = (tivx_obj_desc_pyramid_t *)obj_desc[
            TIVX_KERNEL_LAPLACIAN_PYRAMID_LAPLACIAN_IDX];
        low_img = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LAPLACIAN_PYRAMID_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((vx_status)VX_SUCCESS == status)
        {
            if ((NULL != prms) &&
                (size == sizeof(tivxLaplacianPyramidParams)))
            {
                tivxGetObjDescList(pmd->obj_desc_id,
                    (tivx_obj_desc_t **)prms->img_obj_desc, pmd->num_levels);

                for (levels = 0U; levels < pmd->num_levels; levels ++)
                {
                    if (NULL == prms->img_obj_desc[levels])
                    {
                        status = (vx_status)VX_FAILURE;
                        break;
                    }
                }
            }
            else
            {
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_target_ptr;
        void *low_img_target_ptr;
        void *dst_target_ptr;

        low_img_target_ptr = tivxMemShared2TargetPtr(&low_img->mem_ptr[0]);
        tivxCheckStatus(&status, tivxMemBufferMap(low_img_target_ptr, low_img->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_LAPLACIAN_PYRAMID_INPUT_IDX];

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);

        tivxCheckStatus(&status, tivxMemBufferMap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        src_addr = (uint8_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(0, 0, &src->imagepatch_addr[0U]));

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);
        tivxInitBufParams(src, &prms->vxlib_src);

        prms->vxlib_gauss0.data_type = (uint32_t)VXLIB_UINT8;
        prms->vxlib_gauss1.data_type = (uint32_t)VXLIB_UINT8;
        prms->vxlib_dst.data_type = (uint32_t)VXLIB_INT16;

        for (levels = 0; (levels < pmd->num_levels) && ((vx_status)VX_SUCCESS == status);
                levels ++)
        {
            uint32_t buf = levels & 1U;

            dst = prms->img_obj_desc[levels];

            dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);

            tivxCheckStatus(&status, tivxMemBufferMap(dst_target_ptr, dst->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            tivxSetPointerLocation(dst, &dst_target_ptr, (uint8_t**)&dst_addr);

            /* Half scaled intermediate result */
            if(levels == (pmd->num_levels - 1u))
            {
                tivxInitBufParams(low_img, &prms->vxlib_gauss0);
                prms->vxlib_gauss0.data_type = (uint32_t)VXLIB_UINT8;

                tivxSetPointerLocation(low_img, &low_img_target_ptr, &out_addr);
            }
            else
            {
                /* Half scaled intermediate result */
                rect = dst->valid_roi;
                prms->vxlib_gauss0.dim_x = (rect.end_x - rect.start_x) / 2U;
                prms->vxlib_gauss0.dim_y = (rect.end_y - rect.start_y) / 2U;
                prms->vxlib_gauss0.stride_y = ((int32_t)rect.end_x - (int32_t)rect.start_x) / 2;

                out_addr = prms->hsg_output[buf];
            }

            /* Full scale intermediate result (half scaled upsampled) */
            tivxInitBufParams(dst, &prms->vxlib_gauss1);
            rect = dst->valid_roi;
            prms->vxlib_gauss1.stride_y = (int32_t)rect.end_x - (int32_t)rect.start_x;
            prms->vxlib_gauss1.data_type = (uint32_t)VXLIB_UINT8;

            tivxInitBufParams(dst, &prms->vxlib_dst);
            prms->vxlib_dst.data_type = (uint32_t)VXLIB_INT16;

            /* First do half scale gaussian filter with included upsampled result */
            status = (vx_status)VXLIB_halfScaleGaussian_5x5_br_i8u_o8u_o8u(
                src_addr, &prms->vxlib_src,
                out_addr, &prms->vxlib_gauss0,
                prms->upsample_output, &prms->vxlib_gauss1, 0, 0);

            if ((vx_status)VXLIB_SUCCESS == status)
            {
                /* Then do gaussian filter with * 4 multiply on upsampled result */
                status = (vx_status)VXLIB_gaussian_5x5_br_i8u_o8u(
                    prms->upsample_output, &prms->vxlib_gauss1,
                    prms->gauss_output, &prms->vxlib_gauss1, 6, 0, 0);
            }

            if ((vx_status)VXLIB_SUCCESS == status)
            {
                /* Then subtract gaussian filtered upsample from original of this level */
                status = (vx_status)VXLIB_subtract_i8u_i8u_o16s(
                    src_addr, &prms->vxlib_src,
                    prms->gauss_output, &prms->vxlib_gauss1,
                    dst_addr, &prms->vxlib_dst);
            }

            if ((vx_status)VXLIB_SUCCESS == status)
            {
                if(levels < (pmd->num_levels - 1u))
                {
                    /* Prepare for next level */
                    src_addr = prms->hsg_output[buf];

                    prms->vxlib_src.dim_x = prms->vxlib_gauss0.dim_x;
                    prms->vxlib_src.dim_y = prms->vxlib_gauss0.dim_y;
                    prms->vxlib_src.stride_y = prms->vxlib_gauss0.stride_y;

                    memset(prms->upsample_output, 0, prms->vxlib_src.dim_x*prms->vxlib_src.dim_y);
                }
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(dst_target_ptr, dst->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            if (status != (vx_status)VXLIB_SUCCESS)
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(low_img_target_ptr, low_img->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplPmdCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *img;
    tivxLaplacianPyramidParams *prms = NULL;

    if (num_params != TIVX_KERNEL_LAPLACIAN_PYRAMID_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LAPLACIAN_PYRAMID_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        img = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_LAPLACIAN_PYRAMID_INPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxLaplacianPyramidParams),
            (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxLaplacianPyramidParams));

            prms->buff_size = (img->imagepatch_addr[0].dim_x *
                img->imagepatch_addr[0].dim_y);

            prms->upsample_output = tivxMemAlloc(prms->buff_size,
                (vx_enum)TIVX_MEM_EXTERNAL);

            if (NULL == prms->upsample_output)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->gauss_output = tivxMemAlloc(prms->buff_size,
                    (vx_enum)TIVX_MEM_EXTERNAL);

                if (NULL == prms->gauss_output)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                    tivxMemFree(prms->upsample_output, prms->buff_size,
                        (vx_enum)TIVX_MEM_EXTERNAL);
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->hsg_output[0] = tivxMemAlloc(prms->buff_size / 4U,
                    (vx_enum)TIVX_MEM_EXTERNAL);

                if (NULL == prms->hsg_output[0])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                    tivxMemFree(prms->upsample_output, prms->buff_size,
                        (vx_enum)TIVX_MEM_EXTERNAL);
                    tivxMemFree(prms->gauss_output, prms->buff_size,
                        (vx_enum)TIVX_MEM_EXTERNAL);
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->hsg_output[1] = tivxMemAlloc(prms->buff_size / 16U,
                    (vx_enum)TIVX_MEM_EXTERNAL);

                if (NULL == prms->hsg_output[1])
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                    tivxMemFree(prms->upsample_output, prms->buff_size,
                        (vx_enum)TIVX_MEM_EXTERNAL);
                    tivxMemFree(prms->gauss_output, prms->buff_size,
                        (vx_enum)TIVX_MEM_EXTERNAL);
                    tivxMemFree(prms->hsg_output[0], prms->buff_size / 4U,
                        (vx_enum)TIVX_MEM_EXTERNAL);
                }
                else
                {
                    memset(prms->upsample_output, 0, prms->buff_size);
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                tivxSetTargetKernelInstanceContext(kernel, prms,
                    sizeof(tivxLaplacianPyramidParams));
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLplPmdDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxLaplacianPyramidParams *prms = NULL;

    if (num_params != TIVX_KERNEL_LAPLACIAN_PYRAMID_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LAPLACIAN_PYRAMID_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxLaplacianPyramidParams) == size))
        {

            if (NULL != prms->upsample_output)
            {
                tivxMemFree(prms->upsample_output, prms->buff_size,
                    (vx_enum)TIVX_MEM_EXTERNAL);
            }
            if (NULL != prms->gauss_output)
            {
                tivxMemFree(prms->gauss_output, prms->buff_size,
                    (vx_enum)TIVX_MEM_EXTERNAL);
            }
            if (NULL != prms->hsg_output[0])
            {
                tivxMemFree(prms->hsg_output[0], prms->buff_size / 4U,
                    (vx_enum)TIVX_MEM_EXTERNAL);
            }
            if (NULL != prms->hsg_output[1])
            {
                tivxMemFree(prms->hsg_output[1], prms->buff_size / 16U,
                    (vx_enum)TIVX_MEM_EXTERNAL);
            }

            tivxMemFree(prms, size, (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

void tivxAddTargetKernelLaplacianPyramid(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_laplacian_pyramid_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_LAPLACIAN_PYRAMID,
            target_name,
            tivxKernelLplPmdProcess,
            tivxKernelLplPmdCreate,
            tivxKernelLplPmdDelete,
            NULL,
            NULL);
    }
}


void tivxRemoveTargetKernelLaplacianPyramid(void)
{
    tivxRemoveTargetKernel(vx_laplacian_pyramid_target_kernel);
}
