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
 * \file vx_tutorial_image_query.c Query image for attributes such as width, height and format
 *
 * In this tutorial we learn the below concepts,
 *
 * - How to create OpenVX context and OpenVX image data object
 * - How to read a BMP file and load the pixel values into the image data object
 * - How to query the image data object for attributes like width, height
 * - How to cleanup all created resources and exit the OpenVX application
 *
 * To include OpenVX interfaces include below file
 * \code
 * #include <VX/vx.h>
 * \endcode
 *
 * Follow the comments in the function vx_tutorial_image_query()
 * to understand this tutorial
 *
 * As part of this tutorial, we create few utility functions as listed below.
 * These functions will be used in subsequent tutorials to display node and graph attributes.
 *  <TABLE frame="box" rules="all" cellspacing="0" width="50%" border="1" cellpadding="3">
 *      <TR bgcolor="lightgrey">
 *          <TH> Utility function </TH>
 *          <TH> Description </TH>
 *      </TR>
 *      <TR>
 *          <TD> show_image_attributes() </TD>
 *          <TD> Displays attributes of a previously created image.
 *              <b>NOTE:</b> This function though listed in this tutorial is used in later tutorials
 *           </TD>
 *      </TR>
 *  </TABLE>
 *
 */

#include <stdio.h>
#include <VX/vx.h>
#include <bmp_rd_wr.h>
#include <utility.h>

/** \brief Input file name */
#define IN_FILE_NAME       "colors.bmp"

/**
 * \brief Tutorial Entry Point
 */
void vx_tutorial_image_query()
{
    /**
     * - Define objects that we wish to create in the OpenVX application.
     *
     * A vx_context object is defined which is used as input parameter for all subesquent
     * OpenVX object create APIs
     * \code
     */
    vx_context context;
    vx_image image;
    /** \endcode */

    printf(" vx_tutorial_image_query: Tutorial Started !!! \n");

    /**
     * - Create OpenVX context.
     *
     * This MUST be done first before any OpenVX API call.
     * The context that is returned is used as input for subsequent OpenVX APIs
     * \code
     */
    context = vxCreateContext();
    /** \endcode */

    printf(" Loading file %s ...\n", IN_FILE_NAME);

    /**
     * - Create image object.
     *
     * Follow the comments in create_image_from_file() to see
     * how a vx_image object is created and filled with RGB data from BMP file \ref IN_FILE_NAME
     * \code
     */
    image = create_image_from_file(context, IN_FILE_NAME, vx_false_e);
    /** \endcode */

    vxSetReferenceName((vx_reference)image, "MY_IMAGE");

    /**
     * - Show image attributes.
     *
     * Follow the comments in show_image_attributes() to see
     * how image attributes are queried and displayed.
     * \code
     */
    show_image_attributes(image);
    /** \endcode */

    /**
     * - Release image object.
     *
     * Since we are done with using this image object, release it
     * \code
     */
    vxReleaseImage(&image);
    /** \endcode */

    /**
     * - Release context object.
     *
     * Since we are done using OpenVX context, release it.
     * No further OpenVX API calls should be done, until a context is again created using
     * vxCreateContext()
     * \code
     */
    vxReleaseContext(&context);
    /** \endcode */

    printf(" vx_tutorial_image_query: Tutorial Done !!! \n");
    printf(" \n");
}

#define MAX_ATTRIBUTE_NAME (32u)

/**
 * \brief Show attributes of previously created image
 *
 * This function queries the vx_image image for its number of nodes, number of parameters,
 * state, performance, reference name and reference count then prints this information.
 *
 * \param image [in] Previouly created image object
 *
 */
void show_image_attributes(vx_image image)
{
    vx_uint32 width=0, height=0, ref_count=0;
    vx_df_image df=0;
    vx_size num_planes=0, size=0;
    vx_enum color_space=0, channel_range=0, memory_type=0;
    vx_char *ref_name=NULL;
    char df_name[MAX_ATTRIBUTE_NAME];
    char color_space_name[MAX_ATTRIBUTE_NAME];
    char channel_range_name[MAX_ATTRIBUTE_NAME];
    char memory_type_name[MAX_ATTRIBUTE_NAME];
    char ref_name_invalid[MAX_ATTRIBUTE_NAME];

    /** - Query image attributes.
     *
     *  Queries image for width, height, format, planes, size, space, range,
     *  range and memory type.
     *
     * \code
     */
    vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(image, VX_IMAGE_FORMAT, &df, sizeof(vx_df_image));
    vxQueryImage(image, VX_IMAGE_PLANES, &num_planes, sizeof(vx_size));
    vxQueryImage(image, VX_IMAGE_SIZE, &size, sizeof(vx_size));
    vxQueryImage(image, VX_IMAGE_SPACE, &color_space, sizeof(vx_enum));
    vxQueryImage(image, VX_IMAGE_RANGE, &channel_range, sizeof(vx_enum));
    vxQueryImage(image, VX_IMAGE_MEMORY_TYPE, &memory_type, sizeof(vx_enum));
    /** \endcode */

    vxQueryReference((vx_reference)image, VX_REFERENCE_NAME, &ref_name, sizeof(vx_char*));
    vxQueryReference((vx_reference)image, VX_REFERENCE_COUNT, &ref_count, sizeof(vx_uint32));

    switch(df)
    {
        case VX_DF_IMAGE_VIRT:
            strncpy(df_name, "VX_DF_IMAGE_VIRT", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_RGB:
            strncpy(df_name, "VX_DF_IMAGE_RGB", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_RGBX:
            strncpy(df_name, "VX_DF_IMAGE_RGBX", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_NV12:
            strncpy(df_name, "VX_DF_IMAGE_NV12", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_NV21:
            strncpy(df_name, "VX_DF_IMAGE_NV21", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_UYVY:
            strncpy(df_name, "VX_DF_IMAGE_UYVY", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_YUYV:
            strncpy(df_name, "VX_DF_IMAGE_YUYV", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_IYUV:
            strncpy(df_name, "VX_DF_IMAGE_IYUV", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_YUV4:
            strncpy(df_name, "VX_DF_IMAGE_YUV4", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_U8:
            strncpy(df_name, "VX_DF_IMAGE_U8", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_U16:
            strncpy(df_name, "VX_DF_IMAGE_U16", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_S16:
            strncpy(df_name, "VX_DF_IMAGE_S16", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_U32:
            strncpy(df_name, "VX_DF_IMAGE_U32", MAX_ATTRIBUTE_NAME);
            break;
        case VX_DF_IMAGE_S32:
            strncpy(df_name, "VX_DF_IMAGE_S32", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(df_name, "VX_DF_IMAGE_UNKNOWN", MAX_ATTRIBUTE_NAME);
            break;
    }

    switch(color_space)
    {
        case VX_COLOR_SPACE_NONE:
            strncpy(color_space_name, "VX_COLOR_SPACE_NONE", MAX_ATTRIBUTE_NAME);
            break;
        case VX_COLOR_SPACE_BT601_525:
            strncpy(color_space_name, "VX_COLOR_SPACE_BT601_525", MAX_ATTRIBUTE_NAME);
            break;
        case VX_COLOR_SPACE_BT601_625:
            strncpy(color_space_name, "VX_COLOR_SPACE_BT601_625", MAX_ATTRIBUTE_NAME);
            break;
        case VX_COLOR_SPACE_BT709:
            strncpy(color_space_name, "VX_COLOR_SPACE_BT709", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(color_space_name, "VX_COLOR_SPACE_UNKNOWN", MAX_ATTRIBUTE_NAME);
            break;
    }

    switch(channel_range)
    {
        case VX_CHANNEL_RANGE_FULL:
            strncpy(channel_range_name, "VX_CHANNEL_RANGE_FULL", MAX_ATTRIBUTE_NAME);
            break;
        case VX_CHANNEL_RANGE_RESTRICTED:
            strncpy(channel_range_name, "VX_CHANNEL_RANGE_RESTRICTED", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(channel_range_name, "VX_CHANNEL_RANGE_UNKNOWN", MAX_ATTRIBUTE_NAME);
            break;
    }

    switch(memory_type)
    {
        case VX_MEMORY_TYPE_NONE:
            strncpy(memory_type_name, "VX_MEMORY_TYPE_NONE", MAX_ATTRIBUTE_NAME);
            break;
        case VX_MEMORY_TYPE_HOST:
            strncpy(memory_type_name, "VX_MEMORY_TYPE_HOST", MAX_ATTRIBUTE_NAME);
            break;
        default:
            strncpy(memory_type_name, "VX_MEMORY_TYPE_UNKNOWN", MAX_ATTRIBUTE_NAME);
            break;
    }

    if(ref_name==NULL)
    {
        strncpy(ref_name_invalid, "INVALID_REF_NAME", MAX_ATTRIBUTE_NAME);
        ref_name = &ref_name_invalid[0];
    }

    printf(" VX_TYPE_IMAGE: %s, %d x %d, %d plane(s), %d B, %s %s %s %s, %d refs\n",
        ref_name,
        width,
        height,
        (uint32_t)num_planes,
        (uint32_t)size,
        df_name,
        color_space_name,
        channel_range_name,
        memory_type_name,
        ref_count
        );
}
