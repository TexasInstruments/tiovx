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
#include <tivx_kernel_addsub.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>
#include <vx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
} tivxAddParams;

static tivx_target_kernel vx_add_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelAddProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelAddCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelAddDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelAddControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelAddProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxAddParams *prms = NULL;
    tivx_obj_desc_image_t *src0, *src1, *dst;
    uint8_t *src0_addr, *src1_addr, *dst_addr;
    vx_rectangle_t rect;
    uint32_t size;

    status = ownCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_ADDSUB_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src0 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_ADDSUB_IN0_IMG_IDX];
        src1 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_ADDSUB_IN1_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_ADDSUB_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxAddParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[3];

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src0->valid_roi;

        src0->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src0->mem_ptr[0].shared_ptr, src0->mem_ptr[0].mem_type);
        tivxMemBufferMap(src0->mem_ptr[0].target_ptr, src0->mem_size[0],
            src0->mem_ptr[0].mem_type, VX_READ_ONLY);
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

        if ((VX_DF_IMAGE_S16 == src0->format) &&
            (VX_DF_IMAGE_U8 == src1->format) &&
            (VX_DF_IMAGE_S16 == dst->format))
        {
            img_ptrs[0] = src1_addr;
            img_ptrs[1] = src0_addr;
        }
        else
        {
            img_ptrs[0] = src0_addr;
            img_ptrs[1] = src1_addr;
        }

        img_ptrs[2] = dst_addr;
        tivxBamUpdatePointers(prms->graph_handle, 2U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
        tivxMemBufferUnmap(src0->mem_ptr[0].target_ptr, src0->mem_size[0],
            src0->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(src1->mem_ptr[0].target_ptr, src1->mem_size[0],
            src1->mem_ptr[0].mem_type, VX_READ_ONLY);
    }
    else
    {
        status = VX_ERROR_NO_MEMORY;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelAddCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0, *src1, *dst;
    tivx_obj_desc_scalar_t *sc_desc;
    tivxAddParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = ownCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_ADDSUB_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src0 = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ADDSUB_IN0_IMG_IDX];
        src1 = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ADDSUB_IN1_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_ADDSUB_OUT_IMG_IDX];
        sc_desc = (tivx_obj_desc_scalar_t *)
            obj_desc[TIVX_KERNEL_ADDSUB_IN_SCALAR_IDX];

        prms = tivxMemAlloc(sizeof(tivxAddParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_kernel_details_t kernel_details;
            VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_dst;
            VXLIB_bufParams2D_t *buf_params[3];

            memset(prms, 0, sizeof(tivxAddParams));

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

            /* If output is in U8 format, both the input must be in
               U8 format */
            if (VXLIB_UINT8 == vxlib_dst.data_type)
            {
                BAM_VXLIB_add_i8u_i8u_o8u_params kernel_params;

                if (VX_CONVERT_POLICY_SATURATE == sc_desc->data.enm)
                {
                    kernel_params.overflow_policy = VXLIB_CONVERT_POLICY_SATURATE;
                }
                else
                {
                    kernel_params.overflow_policy = VXLIB_CONVERT_POLICY_WRAP;
                }

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_add_i8u_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_ADD_I8U_I8U_O8U, buf_params,
                    &kernel_details, &prms->graph_handle);
            }
            /* Now if the both inputs are U8, output will be in S16 format */
            else if ((VXLIB_UINT8 == vxlib_src1.data_type) &&
                     (VXLIB_UINT8 == vxlib_src0.data_type))
            {
                kernel_details.compute_kernel_params = NULL;

                BAM_VXLIB_add_i8u_i8u_o16s_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_ADD_I8U_I8U_O16S, buf_params,
                    &kernel_details, &prms->graph_handle);
            }
            /* If both the input are in S16 format, output will be in
               S16 format */
            else if ((VXLIB_INT16 == vxlib_src1.data_type) &&
                     (VXLIB_INT16 == vxlib_src0.data_type))
            {
                BAM_VXLIB_add_i16s_i16s_o16s_params kernel_params;

                if (VX_CONVERT_POLICY_SATURATE == sc_desc->data.enm)
                {
                    kernel_params.overflow_policy = VXLIB_CONVERT_POLICY_SATURATE;
                }
                else
                {
                    kernel_params.overflow_policy = VXLIB_CONVERT_POLICY_WRAP;
                }

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_add_i16s_i16s_o16s_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_ADD_I16S_I16S_O16S, buf_params,
                    &kernel_details, &prms->graph_handle);
            }
            else
            {
                BAM_VXLIB_add_i8u_i16s_o16s_params kernel_params;

                if (VX_CONVERT_POLICY_SATURATE == sc_desc->data.enm)
                {
                    kernel_params.overflow_policy = VXLIB_CONVERT_POLICY_SATURATE;
                }
                else
                {
                    kernel_params.overflow_policy = VXLIB_CONVERT_POLICY_WRAP;
                }

                if (VXLIB_UINT8 == vxlib_src0.data_type)
                {
                    /* Fill in the frame level sizes of buffers here. If the port
                     * is optionally disabled, put NULL */
                    buf_params[0] = &vxlib_src0;
                    buf_params[1] = &vxlib_src1;
                }
                else
                {
                    /* Fill in the frame level sizes of buffers here. If the port
                     * is optionally disabled, put NULL */
                    buf_params[0] = &vxlib_src1;
                    buf_params[1] = &vxlib_src0;
                }
                kernel_details.compute_kernel_params = (void*)&kernel_params;
                BAM_VXLIB_add_i8u_i16s_o16s_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_ADD_I8U_I16S_O16S, buf_params,
                    &kernel_details, &prms->graph_handle);
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxAddParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxAddParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelAddDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxAddParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = ownCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_ADDSUB_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxAddParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, sizeof(tivxAddParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelAddControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamAdd(void)
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

        vx_add_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_ADD,
            target_name,
            tivxKernelAddProcess,
            tivxKernelAddCreate,
            tivxKernelAddDelete,
            tivxKernelAddControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamAdd(void)
{
    tivxRemoveTargetKernel(vx_add_target_kernel);
}
