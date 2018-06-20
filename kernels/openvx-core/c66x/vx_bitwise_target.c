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
#include <tivx_kernel_bitwise.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

typedef VXLIB_STATUS (*VxLib_Bitwise_Fxn)(
    const uint8_t *src0, const VXLIB_bufParams2D_t *src0_prms,
    const uint8_t *src1, const VXLIB_bufParams2D_t *src1_prms,
    uint8_t *dst, const VXLIB_bufParams2D_t *dst_prms);

typedef struct
{
    /*! \brief ID of the kernel */
    vx_enum                 kernel_id;

    /*! \brief Pointer to the kernel Registered */
    tivx_target_kernel      target_kernel;

    /*! \brief Pointer to filter function */
    VxLib_Bitwise_Fxn       vxlib_process;
} tivxBitwiseKernelInfo;

tivxBitwiseKernelInfo gTivxBitwiseKernelInfo[] =
{
    {
        VX_KERNEL_OR,
        NULL,
        VXLIB_or_i8u_i8u_o8u
    },
    {
        VX_KERNEL_XOR,
        NULL,
        VXLIB_xor_i8u_i8u_o8u
    },
    {
        VX_KERNEL_AND,
        NULL,
        VXLIB_and_i8u_i8u_o8u
    },
    {
        VX_KERNEL_NOT,
        NULL,
        NULL
    }
};

vx_status VX_CALLBACK tivxKernelBitwiseProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src0_desc, *src1_desc, *dst_desc;
    uint8_t *src0_addr, *src1_addr, *dst_addr;
    VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_dst;
    tivxBitwiseKernelInfo *kern_info;

    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_BITWISE_MAX_PARAMS);

    if ((VX_FAILURE == status) || (NULL == priv_arg))
    {
        status = VX_FAILURE;
    }
    else
    {
        void *src0_desc_target_ptr;
        void *src1_desc_target_ptr;
        void *dst_desc_target_ptr;

        kern_info = (tivxBitwiseKernelInfo *)priv_arg;

        /* Get the Src and Dst descriptors */
        src0_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_IN0_IMG_IDX];
        src1_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_IN1_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_OUT_IMG_IDX];

        /* Get the target pointer from the shared pointer for all
           buffers */
        src0_desc_target_ptr = tivxMemShared2TargetPtr(
            src0_desc->mem_ptr[0].shared_ptr, src0_desc->mem_ptr[0].mem_heap_region);
        src1_desc_target_ptr = tivxMemShared2TargetPtr(
            src1_desc->mem_ptr[0].shared_ptr, src1_desc->mem_ptr[0].mem_heap_region);
        dst_desc_target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_heap_region);

        /* Map all three buffers, which invalidates the cache */
        tivxMemBufferMap(src0_desc_target_ptr,
            src0_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(src1_desc_target_ptr,
            src1_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc_target_ptr,
            dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        tivxSetTwoPointerLocation(src0_desc, src1_desc, &src0_desc_target_ptr, &src1_desc_target_ptr, &src0_addr, &src1_addr);
        tivxSetPointerLocation(dst_desc, &dst_desc_target_ptr, &dst_addr);

        tivxInitTwoBufParams(src0_desc, src1_desc, &vxlib_src0, &vxlib_src1);
        tivxInitBufParams(dst_desc, &vxlib_dst);

        if (NULL != kern_info->vxlib_process)
        {
            status = kern_info->vxlib_process(
                src0_addr, &vxlib_src0, src1_addr, &vxlib_src1,
                dst_addr, &vxlib_dst);

            if (VXLIB_SUCCESS != status)
            {
                status = VX_FAILURE;
            }
        }
        else
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src0_desc_target_ptr,
            src0_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(src1_desc_target_ptr,
            src1_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc_target_ptr,
            dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);
    }

    return (status);
}

vx_status VX_CALLBACK tivxKernelBitwiseNotProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    uint8_t *src_addr, *dst_addr;

    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_BITWISE_NOT_MAX_PARAMS);

    if ((VX_FAILURE == status) || (NULL == priv_arg))
    {
        status = VX_FAILURE;
    }
    else
    {
        void *src_desc_target_ptr;
        void *dst_desc_target_ptr;

        /* Get the Src and Dst descriptors */
        src_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_NOT_IN_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_BITWISE_NOT_OUT_IMG_IDX];

        /* Get the target pointer from the shared pointer for all
           buffers */
        src_desc_target_ptr = tivxMemShared2TargetPtr(
            src_desc->mem_ptr[0].shared_ptr, src_desc->mem_ptr[0].mem_heap_region);
        dst_desc_target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_heap_region);

        /* Map all buffers, which invalidates the cache */
        tivxMemBufferMap(src_desc_target_ptr,
            src_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc_target_ptr,
            dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        tivxInitBufParams(src_desc, &vxlib_src);
        tivxInitBufParams(dst_desc, &vxlib_dst);

        tivxSetPointerLocation(src_desc, &src_desc_target_ptr, &src_addr);
        tivxSetPointerLocation(dst_desc, &dst_desc_target_ptr, &dst_addr);

        status = VXLIB_not_i8u_o8u(src_addr, &vxlib_src, dst_addr, &vxlib_dst);

        if (VXLIB_SUCCESS == status)
        {
            tivxMemBufferUnmap(dst_desc_target_ptr,
                dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
                VX_WRITE_ONLY);
        }
        else
        {
            status = VX_FAILURE;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelBitwiseCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelBitwiseDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelBitwiseControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBitwise(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;
    vx_uint32 i;

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

        for (i = 0; i < (sizeof(gTivxBitwiseKernelInfo)/
                sizeof(tivxBitwiseKernelInfo)); i ++)
        {
            if (VX_KERNEL_NOT == gTivxBitwiseKernelInfo[i].kernel_id)
            {
                gTivxBitwiseKernelInfo[i].target_kernel = tivxAddTargetKernel(
                    gTivxBitwiseKernelInfo[i].kernel_id,
                    target_name,
                    tivxKernelBitwiseNotProcess,
                    tivxKernelBitwiseCreate,
                    tivxKernelBitwiseDelete,
                    tivxKernelBitwiseControl,
                    &gTivxBitwiseKernelInfo[i]);
            }
            else
            {
                gTivxBitwiseKernelInfo[i].target_kernel = tivxAddTargetKernel(
                    gTivxBitwiseKernelInfo[i].kernel_id,
                    target_name,
                    tivxKernelBitwiseProcess,
                    tivxKernelBitwiseCreate,
                    tivxKernelBitwiseDelete,
                    tivxKernelBitwiseControl,
                    &gTivxBitwiseKernelInfo[i]);
            }

            if (NULL == gTivxBitwiseKernelInfo[i].target_kernel)
            {
                break;
            }
        }
    }
}


void tivxRemoveTargetKernelBitwise(void)
{
    vx_status status;
    vx_uint32 i;

    for (i = 0; i < (sizeof(gTivxBitwiseKernelInfo)/
            sizeof(tivxBitwiseKernelInfo)); i ++)
    {
        if (gTivxBitwiseKernelInfo[i].target_kernel)
        {
            status = tivxRemoveTargetKernel(gTivxBitwiseKernelInfo[i].target_kernel);

            if (VX_SUCCESS == status)
            {
                gTivxBitwiseKernelInfo[i].target_kernel = NULL;
            }
        }
    }
}
