/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_lut.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>
#include <vx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
} tivxLutParams;

static tivx_target_kernel vx_lut_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelLutProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelLutCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelLutDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelLutControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelLutProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxLutParams *prms = NULL;
    vx_uint32 i;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_lut_t *lut;
    vx_uint8 *src_addr, *dst_addr;
    vx_rectangle_t rect;
    uint32_t size;

    if (num_params != TIVX_KERNEL_LUT_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0; i < TIVX_KERNEL_LUT_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LUT_IN_IMG_IDX];
        lut = (tivx_obj_desc_lut_t *)obj_desc[TIVX_KERNEL_LUT_IN_LUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_LUT_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxLutParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[2];

        src->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0U].shared_ptr, src->mem_ptr[0U].mem_type);
        dst->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0U].shared_ptr, dst->mem_ptr[0U].mem_type);
        lut->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            lut->mem_ptr.shared_ptr, lut->mem_ptr.mem_type);

        tivxMemBufferMap(src->mem_ptr[0U].target_ptr, src->mem_size[0],
            src->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(lut->mem_ptr.target_ptr, lut->mem_size,
            lut->mem_ptr.mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0U].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0U].mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        dst_addr = (uint8_t *)((uintptr_t)dst->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0]));

        img_ptrs[0] = src_addr;
        img_ptrs[1] = dst_addr;
        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

        tivxMemBufferUnmap(src->mem_ptr[0U].target_ptr, src->mem_size[0],
            src->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(lut->mem_ptr.target_ptr, lut->mem_size,
            lut->mem_ptr.mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0U].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0U].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLutCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_lut_t *lut;
    tivxLutParams *prms = NULL;
    uint32_t i;

    if (num_params != TIVX_KERNEL_LUT_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LUT_MAX_PARAMS; i ++)
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
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_LUT_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_LUT_OUT_IMG_IDX];
        lut = (tivx_obj_desc_lut_t *)obj_desc[
            TIVX_KERNEL_LUT_IN_LUT_IDX];

        lut->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            lut->mem_ptr.shared_ptr, lut->mem_ptr.mem_type);

        tivxMemBufferMap(lut->mem_ptr.target_ptr, lut->mem_size,
            lut->mem_ptr.mem_type, VX_READ_ONLY);

        prms = tivxMemAlloc(sizeof(tivxLutParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_frame_params_t frame_params;
            VXLIB_bufParams2D_t vxlib_src, vxlib_dst;

            memset(prms, 0, sizeof(tivxLutParams));

            vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
            vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
            vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;

            vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
            vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
            vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;

            if (src->format == VX_DF_IMAGE_U8)
            {
                vxlib_src.data_type = VXLIB_UINT8;
                vxlib_dst.data_type = VXLIB_UINT8;
            }
            else
            {
                vxlib_src.data_type = VXLIB_INT16;
                vxlib_dst.data_type = VXLIB_INT16;
            }

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            frame_params.buf_params[0] = &vxlib_src;
            frame_params.buf_params[1] = &vxlib_dst;

            if (src->format == VX_DF_IMAGE_U8)
            {
                BAM_VXLIB_tableLookup_i8u_o8u_params kernel_params;
                kernel_params.lut    = lut->mem_ptr.target_ptr;
                kernel_params.count  = lut->num_items;

                status = BAM_VXLIB_tableLookup_i8u_o8u_getKernelInfo( &kernel_params,
                                                                     &frame_params.kernel_info);

                if (VX_SUCCESS == status)
                {
                    status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U,
                                                           &frame_params, (void*)&kernel_params,
                                                           &prms->graph_handle);
                }
            }
            else
            {
                BAM_VXLIB_tableLookup_i16s_o16s_params kernel_params;
                kernel_params.lut    = lut->mem_ptr.target_ptr;
                kernel_params.count  = lut->num_items;
                kernel_params.offset = 32768U;

                status = BAM_VXLIB_tableLookup_i16s_o16s_getKernelInfo( &kernel_params,
                                                                     &frame_params.kernel_info);

                if (VX_SUCCESS == status)
                {
                    status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_TABLELOOKUP_I16S_O16S,
                                                           &frame_params, (void*)&kernel_params,
                                                           &prms->graph_handle);
                }
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxLutParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxLutParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelLutDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxLutParams *prms = NULL;
    uint32_t i;

    if (num_params != TIVX_KERNEL_LUT_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_LUT_MAX_PARAMS; i ++)
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
            (sizeof(tivxLutParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, sizeof(tivxLutParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelLutControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamLut(void)
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
            VX_KERNEL_TABLE_LOOKUP,
            target_name,
            tivxKernelLutProcess,
            tivxKernelLutCreate,
            tivxKernelLutDelete,
            tivxKernelLutControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamLut(void)
{
    tivxRemoveTargetKernel(vx_lut_target_kernel);
}
