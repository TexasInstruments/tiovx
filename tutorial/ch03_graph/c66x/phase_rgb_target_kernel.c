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

/**
 * \file phase_rgb_target_kernel.c Target Kernel implementation for Phase to RGB conversion function
 *
 *  This file shows a sample implementation of a target kernel function.
 *
 *  To implement a target kernel the below top level interface functions are implemented
 *  - vxTutorialAddTargetKernelPhaseRgb() : Registers target kernel to TIOVX target framework
 *  - vxTutorialRemoveTargetKernelPhaseRgb() : Un-Registers target kernel from TIOVX target framework
 *
 *  When registering a target kernel, the following callback function are implemented and registered with the TIOVX framework
 *  - vxTutotrialPhaseRgb() : kernel execute/run function
 *  - vxTutotrialPhaseRgbCreate() : kernel init function
 *  - vxTutotrialPhaseRgbDelete() : kernel deinit function
 *  - vxTutotrialPhaseRgbControl(): kernel control function
 *
 *  When working with target kernel
 *  - vxTutorialAddTargetKernelPhaseRgb() MUST be called during TIOVX target framework system init
 *     - This is done by using function tivxRegisterTutorialTargetKernels() in \ref vx_tutorial_target_kernel.c
 *  - vxTutorialRemoveTargetKernelPhaseRgb() MUST be called during TIOVX target framework system deinit
 *     - This is done by using function tivxUnRegisterTutorialTargetKernels() in \ref vx_tutorial_target_kernel.c
 *
 *  When registering a target kernel a unique name MUST be used to register the
 *  kernel on target side and HOST side.
 *
 *  Follow the comments for the different functions in the file to understand how a user/target kernel is implemented.
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <TI/tivx_target_kernel.h>
#include <vx_tutorial_kernels.h>

/** \brief Index of input image in parameter list */
#define PHASE_RGB_IN0_IMG_IDX   (0u)

/** \brief Index of output image in parameter list */
#define PHASE_RGB_OUT0_IMG_IDX  (1u)

/** \brief Total number of parameters for this function */
#define PHASE_RGB_MAX_PARAMS    (2u)

/**
 * \brief Target kernel handle [static global]
 */
static tivx_target_kernel phase_rgb_target_kernel = NULL;

/**
 * \brief Target kernel run function
 *
 * \param kernel [in] target kernel handle
 * \param obj_desc [in] Parameter object descriptors
 * \param num_params [in] Number of parameter object descriptors
 * \param priv_arg [in] kernel instance priv argument
 */
vx_status VX_CALLBACK vxTutotrialPhaseRgb(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;

    if ((num_params != PHASE_RGB_MAX_PARAMS)
        || (NULL == obj_desc[PHASE_RGB_IN0_IMG_IDX])
        || (NULL == obj_desc[PHASE_RGB_OUT0_IMG_IDX])
        )
    {
        status = VX_FAILURE;
    }
    else
    {
        void *src_desc_target_ptr;
        void *dst_desc_target_ptr;

        /* Get the Src and Dst descriptors */
        src_desc = (tivx_obj_desc_image_t *)obj_desc[PHASE_RGB_IN0_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[PHASE_RGB_OUT0_IMG_IDX];

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

        /* Run function for complete image, ROI is ignored in this example */
        {
            vx_uint32 x, y, width, height;
            vx_uint8 *in_data_ptr, *out_data_ptr;
            vx_uint8 *in_pixel, *out_pixel;

            in_data_ptr = src_desc_target_ptr;
            out_data_ptr = dst_desc_target_ptr;

            width = dst_desc->imagepatch_addr[0U].dim_x;
            height = dst_desc->imagepatch_addr[0U].dim_y;

            for(y=0; y<height; y++)
            {
                in_pixel = in_data_ptr;
                out_pixel = out_data_ptr;

                for(x=0; x<width; x++)
                {
                    if(in_pixel[0]<64)
                    {
                        out_pixel[0] = 0xFF;
                        out_pixel[1] = 0x00;
                        out_pixel[2] = 0x00;
                    }
                    else
                    if(in_pixel[0]<128)
                    {
                        out_pixel[0] = 0x00;
                        out_pixel[1] = 0xFF;
                        out_pixel[2] = 0x00;
                    }
                    else
                    if(in_pixel[0]<192)
                    {
                        out_pixel[0] = 0x00;
                        out_pixel[1] = 0x00;
                        out_pixel[2] = 0xFF;
                    }
                    else
                    {
                        out_pixel[0] = 0xFF;
                        out_pixel[1] = 0xFF;
                        out_pixel[2] = 0xFF;
                    }
                    in_pixel++;
                    out_pixel += 3;
                }
                in_data_ptr += src_desc->imagepatch_addr[0U].stride_y;
                out_data_ptr += dst_desc->imagepatch_addr[0U].stride_y;
            }
        }

        tivxMemBufferUnmap(src_desc_target_ptr,
            src_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc_target_ptr,
            dst_desc->mem_size[0], VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

    }

    return (status);
}

/**
 * \brief Target kernel create function
 *
 * \param kernel [in] target kernel handle
 * \param obj_desc [in] Parameter object descriptors
 * \param num_params [in] Number of parameter object descriptors
 * \param priv_arg [in] kernel instance priv argument
 */
vx_status VX_CALLBACK vxTutotrialPhaseRgbCreate(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

/**
 * \brief Target kernel delete function
 *
 * \param kernel [in] target kernel handle
 * \param obj_desc [in] Parameter object descriptors
 * \param num_params [in] Number of parameter object descriptors
 * \param priv_arg [in] kernel instance priv argument
 */
vx_status VX_CALLBACK vxTutotrialPhaseRgbDelete(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

/**
 * \brief Target kernel control function
 *
 * \param kernel [in] target kernel handle
 * \param obj_desc [in] Parameter object descriptors
 * \param num_params [in] Number of parameter object descriptors
 * \param priv_arg [in] kernel instance priv argument
 */
vx_status VX_CALLBACK vxTutotrialPhaseRgbControl(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

/**
 * \brief Add target kernel to TIOVX framework
 *
 */
void vxTutorialAddTargetKernelPhaseRgb(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    /**
     * - Get CPU ID of the running CPU
     *
     * Add kernel to target framework only if it is supported on this target
     * \code
     */
    self_cpu = tivxGetSelfCpuId();
    /** \endcode */

    if ((self_cpu == TIVX_CPU_ID_DSP1) || (self_cpu == TIVX_CPU_ID_DSP2))
    {
        /**
         * - Find target name based on currently running CPU
         *
         * \code
         */
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
        /** \endcode */

        /**
         * - Register target kernel to TIOVX framework
         *
         * "TIVX_TUTORIAL_KERNEL_PHASE_RGB_NAME" is the name of the target kernel.
         * See also \ref vx_tutorial_kernels.h
         *
         * This MUST match the name specified when registering the same kernel
         * on the HOST side.
         *
         * The registered target kernel handle is stored in a global variable.
         * This is used during vxTutorialRemoveTargetKernelPhaseRgb()
         *
         * \code
         */
        phase_rgb_target_kernel = tivxAddTargetKernelByName(
                    TIVX_TUTORIAL_KERNEL_PHASE_RGB_NAME,
                    target_name,
                    vxTutotrialPhaseRgb,
                    vxTutotrialPhaseRgbCreate,
                    vxTutotrialPhaseRgbDelete,
                    vxTutotrialPhaseRgbControl,
                    NULL);
        /** \endcode */
    }
}

/**
 * \brief Remove target kernel from TIOVX framework
 *
 */
void vxTutorialRemoveTargetKernelPhaseRgb(void)
{
    vx_status status = VX_SUCCESS;

    /**
     * - UnRegister target kernel from TIOVX framework
     *
     * The target kernel handle used is the one returned during
     * tivxAddTargetKernel()
     *
     * \code
     */
    status = tivxRemoveTargetKernel(phase_rgb_target_kernel);
    /** \endcode */

    if (VX_SUCCESS == status)
    {
        phase_rgb_target_kernel = NULL;
    }
}

