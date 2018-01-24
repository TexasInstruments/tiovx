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
 * \file phase_rgb_user_kernel.c User Kernel implementation for Phase to RGB conversion function
 *
 *  This file shows a sample implementation of a user kernel function.
 *
 *  To implement a user kernel the below top level interface functions are implemented
 *  - phase_rgb_user_kernel_add() : Registers user kernel to OpenVX context
 *       - The implementation of this function has slight variation depending on the kernel implemented as
 *         as OpenVX user kernel or TIOVX target kernel
 *  - phase_rgb_user_kernel_remove() : Un-Registers user kernel from OpenVX context
 *       - The implementation of this function is same for both OpenVX user kernel and TIOVX target kernel
 *  - phase_rgb_user_kernel_node(): Using the user kernel ID, creates a OpenVX node within a graph
 *       - The implementation of this function is same for both OpenVX user kernel and TIOVX target kernel
 *
 *  When registering a user kernel, the following callback function are implemented and registered with the OpenVX context
 *  - phase_rgb_user_kernel_validate() : kernel validate function. Called during graph verify.
 *       - The implementation of this function is same for both OpenVX user kernel and TIOVX target kernel
 *  - phase_rgb_user_kernel_init() : kernel init function. Called after  phase_rgb_user_kernel_validate() during graph verify
 *       - This function is typically only implemented for user kernel and is NULL for target kernel
 *  - phase_rgb_user_kernel_run() : kernel run or execute function. Called when graph is executed.
 *       - This function is MUST be non-NULL for user kernel and MUST be NULL for target kernel
 *  - phase_rgb_user_kernel_deinit(): kernel deinit function. Called during graph release.
 *       - This function is typically only implemented for user kernel and is NULL for target kernel
 *
 *  When working with user kernel or target kernel,
 *  - phase_rgb_user_kernel_add() MUST be called after vxCreateContext() and
 *     before phase_rgb_user_kernel_node().
 *  - phase_rgb_user_kernel_remove() MUST be called before vxReleaseContext() and
 *  - phase_rgb_user_kernel_node() is called to insert the user kernel node into a OpenVX graph
 *
 *  When working with target kernel, additional the target side implementation is done in file
 *  \ref phase_rgb_target_kernel.c
 *
 *  Follow the comments for the different functions in the file to understand how a user/target kernel is implemented.
 */

#include <stdio.h>
#include <VX/vx.h>
#include <TI/tivx.h>
#include <ch03_graph/phase_rgb_user_kernel.h>

/** \brief Index of input image in parameter list */
#define PHASE_RGB_IN0_IMG_IDX   (0u)

/** \brief Index of output image in parameter list */
#define PHASE_RGB_OUT0_IMG_IDX  (1u)

/** \brief Total number of parameters for this function */
#define PHASE_RGB_MAX_PARAMS    (2u)

static vx_status VX_CALLBACK phase_rgb_user_kernel_init(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num);
static vx_status VX_CALLBACK phase_rgb_user_kernel_run(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num);
static vx_status VX_CALLBACK phase_rgb_user_kernel_deinit(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num);
static vx_status VX_CALLBACK phase_rgb_user_kernel_validate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_kernel phase_rgb_user_kernel_add_as_user_kernel(vx_context context);
static vx_kernel phase_rgb_user_kernel_add_as_target_kernel(vx_context context);

/** \brief Handle to the registered user kernel [static global] */
static vx_kernel phase_rgb_user_kernel = NULL;

/** \brief Kernel ID of the registered user kernel. Used to create a node for the kernel function [static global] */
static vx_enum phase_rgb_user_kernel_id = VX_ERROR_INVALID_PARAMETERS;


/**
 * \brief Add user/target kernel to OpenVX context
 *
 * \param context [in] OpenVX context into with the user kernel is added
 * \param add_as_target_kernel [in] 'vx_false_e', add this kernel as user kernel \n
 *                                  'vx_true_e', add this kernel as target kernel \n
 */
vx_status phase_rgb_user_kernel_add(vx_context context, vx_bool add_as_target_kernel)
{
    vx_kernel kernel = NULL;
    vx_status status;
    uint32_t index;

    /**
     * - Depending on 'add_as_target_kernel' invoke user kernel or target kernel specific registration logic
     *
     * \ref phase_rgb_user_kernel_id is set as part of this function invocation.
     * \code
     */
    if(add_as_target_kernel==vx_false_e)
    {
        kernel = phase_rgb_user_kernel_add_as_user_kernel(context);
    }
    if(add_as_target_kernel==vx_true_e)
    {
        kernel = phase_rgb_user_kernel_add_as_target_kernel(context);
    }
    /** \endcode */

    /**
     * - Checking is kernel was added successfully to OpenVX context
     * \code
     */
    status = vxGetStatus((vx_reference)kernel);
    /** \endcode */
    if ( status == VX_SUCCESS)
    {
        /**
         * - Now define parameters for the kernel
         *
         *   When specifying the parameters, the below attributes of each parameter are specified,
         *   - parameter index in the function parameter list
         *   - the parameter direction: VX_INPUT or VX_OUTPUT
         *   - parameter data object type
         *   - paramater state: VX_PARAMETER_STATE_REQUIRED or VX_PARAMETER_STATE_OPTIONAL
         * \code
         */
        index = 0;

        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_INPUT,
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        if ( status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                index,
                VX_OUTPUT,
                VX_TYPE_IMAGE,
                VX_PARAMETER_STATE_REQUIRED
                );
            index++;
        }
        /** \endcode */
        /**
         * - After all parameters are defined, now the kernel is finalized, i.e it is ready for use.
         * \code
         */
        if ( status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        /** \endcode */
        if( status != VX_SUCCESS)
        {
            printf(" phase_rgb_user_kernel_add: ERROR: vxAddParameterToKernel, vxFinalizeKernel failed (%d)!!!\n", status);
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
        printf(" phase_rgb_user_kernel_add: ERROR: vxAddUserKernel failed (%d)!!!\n", status);
    }

    if(status==VX_SUCCESS)
    {
        /**
         * - Set kernel handle to the global user kernel handle
         *
         * This global handle is used later to release the kernel when done with it
         * \code
         */
        phase_rgb_user_kernel = kernel;
        /** \endcode */
        printf(" phase_rgb_user_kernel_add: SUCCESS !!!\n");
    }

    return status;
}

/**
 * \brief Add kernel as user kernel
 *
 * \param context [in] OpenVX context to which the kernel will be added
 *
 * \return OpenVX kernel object handle
 */
static vx_kernel phase_rgb_user_kernel_add_as_user_kernel(vx_context context)
{
    vx_kernel kernel = NULL;
    vx_status status;

    /**
     * - Dynamically allocate a kernel ID and store it in  the global 'phase_rgb_user_kernel_id'
     *
     * This is used later to create the node with this kernel function
     * \code
     */
    status = vxAllocateUserKernelId(context, &phase_rgb_user_kernel_id);
    /** \endcode */
    if(status!=VX_SUCCESS)
    {
        printf(" phase_rgb_user_kernel_add_as_user_kernel: ERROR: vxAllocateUserKernelId failed (%d)!!!\n", status);
    }
    if(status==VX_SUCCESS)
    {
        /**
         * - Register kernel to OpenVX context
         *
         * A kernel can be identified with its kernel ID (allocated above). \n
         * A kernel can also be identified with its unique kernel name string.
         * "vx_tutorial_graph.phase_rgb" or TIVX_TUTORIAL_KERNEL_PHASE_RGB_NAME defined in
         * file vx_tutorial_kernels.h in this case.
         *
         * When calling vxAddUserKernel(), additional callbacks are registered
         * including the mandatory phase_rgb_user_kernel_run() function.
         *
         * \code
         */
        kernel = vxAddUserKernel(
                    context,
                    TIVX_TUTORIAL_KERNEL_PHASE_RGB_NAME,
                    phase_rgb_user_kernel_id,
                    phase_rgb_user_kernel_run,
                    2, /* number of parameters objects for this user function */
                    phase_rgb_user_kernel_validate,
                    phase_rgb_user_kernel_init,
                    phase_rgb_user_kernel_deinit);
        /** \endcode */
    }

    return kernel;
}

/**
 * \brief Add kernel as target kernel
 *
 * \param context [in] OpenVX context to which the kernel will be added
 *
 * \return OpenVX kernel object handle
 */
static vx_kernel phase_rgb_user_kernel_add_as_target_kernel(vx_context context)
{
    vx_kernel kernel = NULL;
    vx_status status;

    /**
     * - Dynamically allocate a kernel ID and store it in  the global 'phase_rgb_user_kernel_id'
     *
     * This is used later to create the node with this kernel function
     * \code
     */
    status = vxAllocateUserKernelId(context, &phase_rgb_user_kernel_id);
    /** \endcode */
    if(status!=VX_SUCCESS)
    {
        printf(" phase_rgb_user_kernel_add_as_target_kernel: ERROR: vxAllocateUserKernelId failed (%d)!!!\n", status);
    }
    if(status==VX_SUCCESS)
    {
        /**
         * - Register kernel to OpenVX context
         *
         * A kernel can be identified with its kernel ID (allocated above). \n
         * A kernel can also be identified with its unique kernel name string.
         * "vx_tutorial_graph.phase_rgb" or TIVX_TUTORIAL_KERNEL_PHASE_RGB_NAME defined in
         * file vx_tutorial_kernels.h in this case.
         *
         * When calling vxAddUserKernel(), additional callbacks are registered.
         * For target kernel, the 'run' callback is set to NULL.
         * Typically 'init' and 'deinit' callbacks are also set to NULL.
         * 'validate' callback is typically set
         *
         * \code
         */
        kernel = vxAddUserKernel(
                    context,
                    TIVX_TUTORIAL_KERNEL_PHASE_RGB_NAME,
                    phase_rgb_user_kernel_id,
                    NULL,
                    2, /* number of parameters objects for this user function */
                    phase_rgb_user_kernel_validate,
                    NULL,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
        /** \endcode */

        if ( status == VX_SUCCESS)
        {
            /**
             * - Add supported target's on which this target kernel can be run
             *
             * \code
             */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP2);
            /** \endcode */
        }
    }
    return kernel;
}

/**
 * \brief Remove user/target kernel from context
 *
 * \param context [in] OpenVX context from which the kernel will be removed
 */
vx_status phase_rgb_user_kernel_remove(vx_context context)
{
    vx_status status;

    /**
     * - Remove user kernel from context and set the global 'phase_rgb_user_kernel' to NULL
     *
     * \code
     */
    status = vxRemoveKernel(phase_rgb_user_kernel);
    phase_rgb_user_kernel = NULL;
    /** \endcode */

    if(status!=VX_SUCCESS)
    {
        printf(" phase_rgb_user_kernel_remove: Unable to remove kernel (%d)!!!\n", status);
    }

    if(status==VX_SUCCESS)
    {
        printf(" phase_rgb_user_kernel_remove: SUCCESS !!!\n");
    }

    return status;
}

/**
 *  \brief User/target kernel validate function
 *
 *  This function gets called during vxGraphVerify.
 *  The node which will runs the kernel, parameter references
 *  are passed as input to this function.
 *  This function checks the parameters to make sure all attributes of the parameter are as expected.
 *  ex, data format checks, image width, height relations between input and output.
 *
 *  \param node [in] OpenVX node which will execute the kernel
 *  \param parameters [in] Parameters references for this kernel function
 *  \param num [in] Number of parameter references
 *  \param metas [in/out] Meta references update with attribute values
 *
 *  \return VX_SUCCESS if validate is successful, else appropiate error code
 */
static vx_status VX_CALLBACK phase_rgb_user_kernel_validate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_image img[PHASE_RGB_MAX_PARAMS];
    vx_uint32 w[PHASE_RGB_MAX_PARAMS];
    vx_uint32 h[PHASE_RGB_MAX_PARAMS];
    vx_uint32 out_w, out_h, i;
    vx_df_image fmt[PHASE_RGB_MAX_PARAMS], out_fmt;

    if (num != PHASE_RGB_MAX_PARAMS)
    {
        printf(" phase_rgb_user_kernel_validate: ERROR: Number of parameters dont match !!!\n");
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    for (i = 0U; i < PHASE_RGB_MAX_PARAMS; i ++)
    {
        img[i] = (vx_image)parameters[i];

        /* Check for NULL */
        if (NULL == img[i])
        {
            printf(" phase_rgb_user_kernel_validate: ERROR: Parameter %d is NULL !!!\n", i);
            status = VX_ERROR_NO_MEMORY;
            break;
        }
    }
    if (VX_SUCCESS == status)
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[PHASE_RGB_IN0_IMG_IDX],
            VX_IMAGE_FORMAT, &fmt[PHASE_RGB_IN0_IMG_IDX],
            sizeof(vx_df_image));
        status |= vxQueryImage(img[PHASE_RGB_IN0_IMG_IDX],
            VX_IMAGE_WIDTH, &w[PHASE_RGB_IN0_IMG_IDX],
            sizeof(vx_uint32));
        status |= vxQueryImage(img[PHASE_RGB_IN0_IMG_IDX],
            VX_IMAGE_HEIGHT, &h[PHASE_RGB_IN0_IMG_IDX],
            sizeof(vx_uint32));

        if(status!=VX_SUCCESS)
        {
            printf(" phase_rgb_user_kernel_validate: ERROR: Unable to query input image !!!\n");
        }
    }
    if ((VX_SUCCESS == status) &&
        (vx_false_e == tivxIsReferenceVirtual(
            parameters[PHASE_RGB_OUT0_IMG_IDX])))
    {
        /* Get the image width/heigh and format */
        status = vxQueryImage(img[PHASE_RGB_OUT0_IMG_IDX],
            VX_IMAGE_FORMAT, &fmt[PHASE_RGB_OUT0_IMG_IDX],
            sizeof(vx_df_image));
        status |= vxQueryImage(img[PHASE_RGB_OUT0_IMG_IDX],
            VX_IMAGE_WIDTH, &w[PHASE_RGB_OUT0_IMG_IDX],
            sizeof(vx_uint32));
        status |= vxQueryImage(img[PHASE_RGB_OUT0_IMG_IDX],
            VX_IMAGE_HEIGHT, &h[PHASE_RGB_OUT0_IMG_IDX],
            sizeof(vx_uint32));

        if(status!=VX_SUCCESS)
        {
            printf(" phase_rgb_user_kernel_validate: ERROR: Unable to query output image !!!\n");
        }
    }

    if (VX_SUCCESS == status)
    {
        /* Check for validity of data format */
        if (VX_DF_IMAGE_U8 != fmt[PHASE_RGB_IN0_IMG_IDX] &&
            VX_DF_IMAGE_RGB != fmt[PHASE_RGB_OUT0_IMG_IDX]
            )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            printf(" phase_rgb_user_kernel_validate: ERROR: Input/Output image format not correct !!!\n");
        }

        /* Check for frame sizes */
        if (vx_false_e == tivxIsReferenceVirtual(
            parameters[PHASE_RGB_OUT0_IMG_IDX]))
        {
            if ((w[PHASE_RGB_IN0_IMG_IDX] !=
                 w[PHASE_RGB_OUT0_IMG_IDX]) ||
                (h[PHASE_RGB_IN0_IMG_IDX] !=
                 h[PHASE_RGB_OUT0_IMG_IDX]))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                printf(" phase_rgb_user_kernel_validate: ERROR: Input/Output image wxh do not match !!!\n");
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        out_fmt = VX_DF_IMAGE_RGB;
        out_w = w[PHASE_RGB_OUT0_IMG_IDX];
        out_h = h[PHASE_RGB_OUT0_IMG_IDX];

        vxSetMetaFormatAttribute(metas[PHASE_RGB_OUT0_IMG_IDX], VX_IMAGE_FORMAT, &out_fmt,
            sizeof(out_fmt));
        vxSetMetaFormatAttribute(metas[PHASE_RGB_OUT0_IMG_IDX], VX_IMAGE_WIDTH, &out_w,
            sizeof(out_w));
        vxSetMetaFormatAttribute(metas[PHASE_RGB_OUT0_IMG_IDX], VX_IMAGE_HEIGHT, &out_h,
            sizeof(out_h));
    }

    if(status==VX_SUCCESS)
    {
        printf(" phase_rgb_user_kernel_validate: SUCCESS !!!\n");
    }

    return status;
}

/**
 *  \brief User/target kernel init function
 *
 *  This function gets called during vxGraphVerify after kernel validate function.
 *  Typically any resources the kernel needs during its execution should be allocated during
 *  kernel init.
 *
 *  \param node [in] OpenVX node which will execute the kernel
 *  \param parameters [in] Parameters references for this kernel function
 *  \param num [in] Number of parameter references
 *
 *  \return VX_SUCCESS if validate is successful, else appropiate error code
 */
static vx_status VX_CALLBACK phase_rgb_user_kernel_init(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num)
{
    printf(" phase_rgb_user_kernel_init: SUCCESS !!!\n");
    return VX_SUCCESS;
}

/**
 *  \brief User/target kernel execute or run function
 *
 *  This function gets called during graph execution.
 *  This where the actual kernel function which reads the input data and writes the output data
 *  is implemented.
 *
 *  \param node [in] OpenVX node which will execute the kernel
 *  \param parameters [in] Parameters references for this kernel function
 *  \param num [in] Number of parameter references
 *
 *  \return VX_SUCCESS if validate is successful, else appropiate error code
 */
static vx_status VX_CALLBACK phase_rgb_user_kernel_run(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num)
{
    vx_image in_image = (vx_image)parameters[PHASE_RGB_IN0_IMG_IDX];
    vx_image out_image = (vx_image)parameters[PHASE_RGB_OUT0_IMG_IDX];
    vx_rectangle_t rect = { 0 };
    vx_map_id in_map_id = (vx_map_id)VX_ERROR_INVALID_PARAMETERS, out_map_id = (vx_map_id)VX_ERROR_INVALID_PARAMETERS;
    vx_imagepatch_addressing_t in_image_addr, out_image_addr;
    vx_uint32 out_w=0, out_h=0;
    vx_status status=0;
    void *in_data_ptr, *out_data_ptr;

    status |= vxQueryImage(out_image,
        VX_IMAGE_WIDTH, &out_w,
        sizeof(vx_uint32));
    status |= vxQueryImage(out_image,
        VX_IMAGE_HEIGHT, &out_h,
        sizeof(vx_uint32));

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x   = out_w;
    rect.end_y   = out_h;

    if(status==VX_SUCCESS)
    {
        status = vxMapImagePatch(in_image,
                &rect,
                0,
                &in_map_id,
                &in_image_addr,
                (void**)&in_data_ptr,
                VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);
    }

    if(status==VX_SUCCESS)
    {
        status = vxMapImagePatch(out_image,
                &rect,
                0,
                &out_map_id,
                &out_image_addr,
                (void**)&out_data_ptr,
                VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X);
    }
    if(status==VX_SUCCESS)
    {
        vx_uint32 x, y;

        for(y=0; y<out_image_addr.dim_y; y++)
        {
            for(x=0; x<out_image_addr.dim_x; x++)
            {
                vx_uint8 *in_pixel = (vx_uint8 *)vxFormatImagePatchAddress2d(in_data_ptr, x, y, &in_image_addr);
                vx_uint8 *out_pixel = (vx_uint8 *)vxFormatImagePatchAddress2d(out_data_ptr, x, y, &out_image_addr);

                if(*in_pixel<64)
                {
                    out_pixel[0] = 0xFF;
                    out_pixel[1] = 0x00;
                    out_pixel[2] = 0x00;
                }
                else
                if(*in_pixel<128)
                {
                    out_pixel[0] = 0x00;
                    out_pixel[1] = 0xFF;
                    out_pixel[2] = 0x00;
                }
                else
                if(*in_pixel<192)
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
            }
        }

    }

    if(in_map_id!=(vx_map_id)VX_ERROR_INVALID_PARAMETERS)
    {
        vxUnmapImagePatch(in_image, in_map_id);
    }
    if(out_map_id!=(vx_map_id)VX_ERROR_INVALID_PARAMETERS)
    {
        vxUnmapImagePatch(out_image, out_map_id);
    }

    if(status!=VX_SUCCESS)
    {
        printf(" phase_rgb_user_kernel_run: ERROR: Run failed (%d)!!!\n", status);
    }
    return status;
}

/**
 *  \brief User/target kernel de-init function
 *
 *  This function gets called during vxReleaseGraph.
 *  Typically any resources allcoated during kernel init are released during deinit.
 *
 *  \param node [in] OpenVX node which will execute the kernel
 *  \param parameters [in] Parameters references for this kernel function
 *  \param num [in] Number of parameter references
 *
 *  \return VX_SUCCESS if validate is successful, else appropiate error code
 */
static vx_status VX_CALLBACK phase_rgb_user_kernel_deinit(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num)
{
    printf(" phase_rgb_user_kernel_deinit: SUCCESS !!!\n");
    return VX_SUCCESS;
}

/**
 *  \brief User/target kernel node create function
 *
 *  Given a graph reference, this function creates a OpenVX node and inserts it into the graph.
 *  The list of parameter references is also provided as input.
 *  Exact data type are used instead of base class references to allow some level
 *  of compile time type checking.
 *  In this example, there is one input image and one output image that are passed as parameters.
 *
 *  \param graph [in] OpenVX graph
 *  \param in [in] Input image reference
 *  \param out [in] Output image reference
 *
 *  \return OpenVX node that is created and inserted into the graph
 */
vx_node phase_rgb_user_kernel_node(vx_graph graph, vx_image in, vx_image out)
{
    vx_node node;
    /**
     * - Put parameters into a array of references
     * \code
     */
    vx_reference refs[] = {(vx_reference)in, (vx_reference)out};
    /** \endcode */

    /**
     * - Use TIOVX API to make a node using the graph, kernel ID, and parameter reference array as input
     * \code
     */
    node = tivxCreateNodeByKernelEnum(graph,
                phase_rgb_user_kernel_id,
                refs, sizeof(refs)/sizeof(refs[0])
                );
    vxSetReferenceName((vx_reference)node, "PHASE_RGB");
    /** \endcode */

    return node;
}


