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
#include <tivx_kernel_addsub.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
} tivxAddParams;

static tivx_target_kernel vx_subtract_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelSubtractProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelSubtractCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelSubtractDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelSubtractControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelSubtractProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxAddParams *prms = NULL;
    tivx_obj_desc_image_t *src0, *src1, *dst;
    uint8_t *src0_addr, *src1_addr, *dst_addr;
    uint32_t size;

    status = tivxCheckNullParams(obj_desc, num_params,
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
        void *src0_target_ptr;
        void *src1_target_ptr;
        void *dst_target_ptr;

        src0_target_ptr = tivxMemShared2TargetPtr(
            src0->mem_ptr[0].shared_ptr, src0->mem_ptr[0].mem_heap_region);

        src1_target_ptr = tivxMemShared2TargetPtr(
            src1->mem_ptr[0].shared_ptr, src1->mem_ptr[0].mem_heap_region);

        tivxSetTwoPointerLocation(src0, src1, &src0_target_ptr, &src1_target_ptr, &src0_addr, &src1_addr);

        dst_target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_heap_region);
        tivxSetPointerLocation(dst, &dst_target_ptr, &dst_addr);

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
    }
    else
    {
        status = VX_ERROR_NO_MEMORY;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSubtractCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0, *src1, *dst;
    tivx_obj_desc_scalar_t *sc_desc;
    tivxAddParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
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

            tivxInitTwoBufParams(src0, src1, &vxlib_src0, &vxlib_src1);
            tivxInitBufParams(dst, &vxlib_dst);

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src0;
            buf_params[1] = &vxlib_src1;
            buf_params[2] = &vxlib_dst;

            /* If output is in U8 format, both the input must be in
               U8 format */
            if (VXLIB_UINT8 == vxlib_dst.data_type)
            {
                BAM_VXLIB_subtract_i8u_i8u_o8u_params kernel_params;

                if (VX_CONVERT_POLICY_SATURATE == sc_desc->data.enm)
                {
                    kernel_params.overflow_policy = VXLIB_CONVERT_POLICY_SATURATE;
                }
                else
                {
                    kernel_params.overflow_policy = VXLIB_CONVERT_POLICY_WRAP;
                }

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_subtract_i8u_i8u_o8u_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O8U, buf_params,
                    &kernel_details, &prms->graph_handle);
            }
            /* Now if the both inputs are U8, output will be in S16 format */
            else if ((VXLIB_UINT8 == vxlib_src1.data_type) &&
                     (VXLIB_UINT8 == vxlib_src0.data_type))
            {
                kernel_details.compute_kernel_params = NULL;

                BAM_VXLIB_subtract_i8u_i8u_o16s_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_SUBTRACT_I8U_I8U_O16S, buf_params,
                    &kernel_details, &prms->graph_handle);
            }
            /* If both the input are in S16 format, output will be in
               S16 format */
            else if ((VXLIB_INT16 == vxlib_src1.data_type) &&
                     (VXLIB_INT16 == vxlib_src0.data_type))
            {
                BAM_VXLIB_subtract_i16s_i16s_o16s_params kernel_params;

                if (VX_CONVERT_POLICY_SATURATE == sc_desc->data.enm)
                {
                    kernel_params.overflow_policy = VXLIB_CONVERT_POLICY_SATURATE;
                }
                else
                {
                    kernel_params.overflow_policy = VXLIB_CONVERT_POLICY_WRAP;
                }

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_subtract_i16s_i16s_o16s_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_SUBTRACT_I16S_I16S_O16S, buf_params,
                    &kernel_details, &prms->graph_handle);
            }
            else
            {
                BAM_VXLIB_subtract_i8u_i16s_o16s_params kernel_params;

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
                    kernel_params.subtract_policy = 0;
                }
                else
                {
                    /* Fill in the frame level sizes of buffers here. If the port
                     * is optionally disabled, put NULL */
                    buf_params[0] = &vxlib_src1;
                    buf_params[1] = &vxlib_src0;
                    kernel_params.subtract_policy = 1;
                }

                kernel_details.compute_kernel_params = (void*)&kernel_params;

                BAM_VXLIB_subtract_i8u_i16s_o16s_getKernelInfo(NULL,
                    &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(
                    BAM_KERNELID_VXLIB_SUBTRACT_I8U_I16S_O16S, buf_params,
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

static vx_status VX_CALLBACK tivxKernelSubtractDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxAddParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
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

static vx_status VX_CALLBACK tivxKernelSubtractControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamSubtract(void)
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

        vx_subtract_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_SUBTRACT,
            target_name,
            tivxKernelSubtractProcess,
            tivxKernelSubtractCreate,
            tivxKernelSubtractDelete,
            tivxKernelSubtractControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamSubtract(void)
{
    tivxRemoveTargetKernel(vx_subtract_target_kernel);
}
