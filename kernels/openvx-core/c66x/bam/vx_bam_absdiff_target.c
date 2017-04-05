/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_kernel_absdiff.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>
#include <vx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
} tivxAbsDiffParams;

static tivx_target_kernel vx_absdiff_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelAbsDiffProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelAbsDiffCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelAbsDiffDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelAbsDiffControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelAbsDiffProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxAbsDiffParams *prms = NULL;
    tivx_obj_desc_image_t *src0, *src1, *dst;
    uint8_t *src0_addr, *src1_addr, *dst_addr;
    vx_rectangle_t rect;
    uint32_t size;

    if (num_params != TIVX_KERNEL_ABSDIFF_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        /* Check for NULL */
        if ((NULL == obj_desc[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX])))
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    if (VX_SUCCESS == status)
    {
        src0 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX];
        src1 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxAbsDiffParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[3];

        src0->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src0->mem_ptr[0].shared_ptr, src0->mem_ptr[0].mem_type);

        tivxMemBufferMap(src0->mem_ptr[0].target_ptr, src0->mem_size[0],
            src0->mem_ptr[0].mem_type, VX_READ_ONLY);

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src0->valid_roi;

        src0_addr = (uint8_t *)((uintptr_t)src0->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src0->imagepatch_addr[0U]));

        src1->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src1->mem_ptr[0].shared_ptr, src1->mem_ptr[0].mem_type);

        tivxMemBufferMap(src1->mem_ptr[0].target_ptr, src1->mem_size[0],
            src1->mem_ptr[0].mem_type, VX_READ_ONLY);

        src1_addr = (uint8_t *)((uintptr_t)src1->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src1->imagepatch_addr[0U]));

        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);

        tivxMemBufferMap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_READ_ONLY);

        dst_addr = (uint8_t *)((uintptr_t)dst->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0U]));

        img_ptrs[0] = src0_addr;
        img_ptrs[1] = src1_addr;
        img_ptrs[2] = dst_addr;
        tivxBamUpdatePointers(prms->graph_handle, 2U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        tivxMemBufferUnmap(src1->mem_ptr[0].target_ptr, src1->mem_size[0],
            src1->mem_ptr[0].mem_type, VX_READ_ONLY);

        tivxMemBufferUnmap(src0->mem_ptr[0].target_ptr, src0->mem_size[0],
            src0->mem_ptr[0].mem_type, VX_READ_ONLY);

    }
    else
    {
        status = VX_ERROR_NO_MEMORY;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelAbsDiffCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{

    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0, *src1, *dst;
    tivxAbsDiffParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    if (num_params != TIVX_KERNEL_ABSDIFF_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        if ((NULL == obj_desc[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX])))
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    if (VX_SUCCESS == status)
    {
        src0 = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX];
        src1 = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX];

        prms = tivxMemAlloc(sizeof(tivxAbsDiffParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_kernel_details_t kernel_details;
            VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_dst;
            VXLIB_bufParams2D_t *buf_params[3];

            memset(prms, 0, sizeof(tivxAbsDiffParams));

            vxlib_src0.dim_x = src0->imagepatch_addr[0U].dim_x;
            vxlib_src0.dim_y = src0->imagepatch_addr[0U].dim_y;
            vxlib_src0.stride_y = src0->imagepatch_addr[0U].stride_y;
            if (VX_DF_IMAGE_U8 == src0->format)
            {
                vxlib_src0.data_type = VXLIB_UINT8;
            }
            else
            {
                vxlib_src0.data_type = VXLIB_INT16;
            }

            vxlib_src1.dim_x = src1->imagepatch_addr[0U].dim_x;
            vxlib_src1.dim_y = src1->imagepatch_addr[0U].dim_y;
            vxlib_src1.stride_y = src1->imagepatch_addr[0U].stride_y;
            if (VX_DF_IMAGE_U8 == src1->format)
            {
                vxlib_src1.data_type = VXLIB_UINT8;
            }
            else
            {
                vxlib_src1.data_type = VXLIB_INT16;
            }

            vxlib_dst.dim_x = dst->imagepatch_addr[0U].dim_x;
            vxlib_dst.dim_y = dst->imagepatch_addr[0U].dim_y;
            vxlib_dst.stride_y = dst->imagepatch_addr[0U].stride_y;
            if (VX_DF_IMAGE_U8 == dst->format)
            {
                vxlib_dst.data_type = VXLIB_UINT8;
            }
            else
            {
                vxlib_dst.data_type = VXLIB_INT16;
            }

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src0;
            buf_params[1] = &vxlib_src1;
            buf_params[2] = &vxlib_dst;

            kernel_details.compute_kernel_params = NULL;

            if (VXLIB_UINT8 == vxlib_dst.data_type)
            {
                BAM_VXLIB_absDiff_i8u_i8u_o8u_getKernelInfo( NULL,
                                                             &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_ABSDIFF_I8U_I8U_O8U,
                                                       buf_params, &kernel_details,
                                                       &prms->graph_handle);
            }
            else
            {
                BAM_VXLIB_absDiff_i16s_i16s_o16s_getKernelInfo( NULL,
                                                                &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_ABSDIFF_I16S_I16S_O16S,
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
                sizeof(tivxAbsDiffParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxAbsDiffParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelAbsDiffDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxAbsDiffParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    if (num_params != TIVX_KERNEL_ABSDIFF_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        if ((NULL == obj_desc[TIVX_KERNEL_ABSDIFF_IN0_IMG_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_ABSDIFF_IN1_IMG_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_ABSDIFF_OUT_IMG_IDX])))
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxAbsDiffParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, sizeof(tivxAbsDiffParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelAbsDiffControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamAbsDiff(void)
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

        vx_absdiff_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_ABSDIFF,
            target_name,
            tivxKernelAbsDiffProcess,
            tivxKernelAbsDiffCreate,
            tivxKernelAbsDiffDelete,
            tivxKernelAbsDiffControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamAbsDiff(void)
{
    tivxRemoveTargetKernel(vx_absdiff_target_kernel);
}
