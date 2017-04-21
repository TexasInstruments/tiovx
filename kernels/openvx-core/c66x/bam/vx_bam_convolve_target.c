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
#include <tivx_kernel_convolve.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>
#include <vx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
} tivxBamConvolveParams;

static tivx_target_kernel vx_convolve_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelConvolveProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelConvolveCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelConvolveDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelConvolveControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelConvolveProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxBamConvolveParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_convolution_t *conv;
    vx_uint8 *src_addr, *dst_addr;
    vx_rectangle_t rect;
    uint32_t size;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CONVOLVE_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CONVOLVE_IN_IMG_IDX];
        conv = (tivx_obj_desc_convolution_t *)obj_desc[TIVX_KERNEL_CONVOLVE_IN_CONVOLVE_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CONVOLVE_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxBamConvolveParams) != size))
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
        conv->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            conv->mem_ptr.shared_ptr, conv->mem_ptr.mem_type);

        tivxMemBufferMap(src->mem_ptr[0U].target_ptr, src->mem_size[0],
            src->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(conv->mem_ptr.target_ptr, conv->mem_size,
            conv->mem_ptr.mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0U].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0U].mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));
        /* TODO: Do we require to move pointer even for destination image */
        dst_addr = (uint8_t *)((uintptr_t)dst->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x + (conv->columns/2U),
            rect.start_y + (conv->rows/2U),
            &dst->imagepatch_addr[0]));

        img_ptrs[0] = src_addr;
        img_ptrs[1] = dst_addr;
        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

        tivxMemBufferUnmap(src->mem_ptr[0U].target_ptr, src->mem_size[0],
            src->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(conv->mem_ptr.target_ptr, conv->mem_size,
            conv->mem_ptr.mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0U].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0U].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelConvolveCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_convolution_t *conv;
    tivxBamConvolveParams *prms = NULL;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CONVOLVE_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CONVOLVE_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CONVOLVE_OUT_IMG_IDX];
        conv = (tivx_obj_desc_convolution_t *)obj_desc[
            TIVX_KERNEL_CONVOLVE_IN_CONVOLVE_IDX];

        conv->mem_ptr.target_ptr = tivxMemShared2TargetPtr(
            conv->mem_ptr.shared_ptr, conv->mem_ptr.mem_type);

        tivxMemBufferMap(conv->mem_ptr.target_ptr, conv->mem_size,
            conv->mem_ptr.mem_type, VX_READ_ONLY);

        prms = tivxMemAlloc(sizeof(tivxBamConvolveParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_kernel_details_t kernel_details;
            VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
            VXLIB_bufParams2D_t *buf_params[2];

            memset(prms, 0, sizeof(tivxBamConvolveParams));

            vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
            vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
            vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
            vxlib_src.data_type = VXLIB_UINT8;

            /* All filter reduces the output size, therefore reduce output
             * height, but leave output width the same (DSP optimization) */
            vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x -
                ((conv->columns/2U) * 2U);
            vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y -
                ((conv->rows/2U) * 2U);
            vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
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
            buf_params[0] = &vxlib_src;
            buf_params[1] = &vxlib_dst;

            if (vxlib_dst.data_type == VXLIB_UINT8)
            {
                BAM_VXLIB_convolve_i8u_c16s_o8u_params kernel_params;

                kernel_params.conv_mat      = conv->mem_ptr.target_ptr;
                kernel_params.conv_width    = conv->columns;
                kernel_params.conv_height   = conv->rows;
                kernel_params.conv_scale    = conv->scale;

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_convolve_i8u_c16s_o8u_getKernelInfo(
                    &kernel_params, &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_CONVOLVE_I8U_C16S_O8U,
                    buf_params, &kernel_details,
                    &prms->graph_handle);
            }
            else
            {
                BAM_VXLIB_convolve_i8u_c16s_o16s_params kernel_params;

                kernel_params.conv_mat      = conv->mem_ptr.target_ptr;
                kernel_params.conv_width    = conv->columns;
                kernel_params.conv_height   = conv->rows;
                kernel_params.conv_scale    = conv->scale;

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_convolve_i8u_c16s_o16s_getKernelInfo(
                    &kernel_params, &kernel_details.kernel_info);

                kernel_details.kernel_info.kernelExtraInfo.metaInfo =
                    BAM_METAINFO_NEIGHBORHOOD_OP(kernel_params.conv_width,
                    kernel_params.conv_height);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_CONVOLVE_I8U_C16S_O16S,
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
                sizeof(tivxBamConvolveParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxBamConvolveParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelConvolveDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxBamConvolveParams *prms = NULL;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CONVOLVE_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxBamConvolveParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, sizeof(tivxBamConvolveParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelConvolveControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamConvolve(void)
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

        vx_convolve_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_CUSTOM_CONVOLUTION,
            target_name,
            tivxKernelConvolveProcess,
            tivxKernelConvolveCreate,
            tivxKernelConvolveDelete,
            tivxKernelConvolveControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamConvolve(void)
{
    tivxRemoveTargetKernel(vx_convolve_target_kernel);
}
