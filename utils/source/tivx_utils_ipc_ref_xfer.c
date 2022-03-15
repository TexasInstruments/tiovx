/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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
#include <vx_internal.h>

#include <tivx_utils_ipc_ref_xfer.h>

vx_status tivx_utils_export_ref_for_ipc_xfer(const vx_reference         ref,
                                             tivx_utils_ref_ipc_msg_t  *ipcMsg)
{
    void                   *ptrs[VX_IPC_MAX_VX_PLANES];
    tivx_utils_ref_desc_t  *refDesc;
    uint32_t                numPlanes;
    vx_status               vxStatus = VX_SUCCESS;

    /* Validate the arguments. */
    if (ref == NULL)
    {
        /* More thorough checks on ref done inside tivxReferenceExportHandle()
         * below.
         */
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'ref' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }
    else if (ipcMsg == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'ipcMsg' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        refDesc = &ipcMsg->refDesc;

        /* Export the handles. */
        vxStatus = tivxReferenceExportHandle(ref,
                                             ptrs,
                                             refDesc->handleSizes,
                                             VX_IPC_MAX_VX_PLANES,
                                             &numPlanes);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
        }
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        tivx_utils_meta_format_t   *meta;
        void                       *phyAddr;
        uint64_t                    fd64;
        uint32_t                    i;

        meta       = &refDesc->meta;
        meta->type = ref->type;

        if (meta->type == (vx_enum)VX_TYPE_IMAGE)
        {
            tivx_obj_desc_image_t *obj_desc;

            obj_desc = (tivx_obj_desc_image_t *)ref->obj_desc;

            meta->img.width  = obj_desc->width;
            meta->img.height = obj_desc->height;
            meta->img.format = obj_desc->format;
            meta->img.planes = obj_desc->planes;

            /* For images, only plane[0] is allocated and plane[1]..plane[N-1]
             * pointers are offset from plane[0]. Given this, we need to
             * compute the size of the buffer allocated for the image available
             * as plane[0] handle. The size is the sum of sizes of all
             * the planes.
             */
            for (i = 1; i <  meta->img.planes; i++)
            {
                refDesc->handleSizes[0] += refDesc->handleSizes[i];
            }

            /* Just translate plane[0] pointer. */
            numPlanes = 1;
        }
        else if (meta->type == (vx_enum)VX_TYPE_TENSOR)
        {
            tivx_obj_desc_tensor_t *obj_desc;
            int32_t                 k;

            obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;

            meta->tensor.number_of_dimensions = obj_desc->number_of_dimensions;
            meta->tensor.data_type            = obj_desc->data_type;
            meta->tensor.fixed_point_position = obj_desc->fixed_point_position;

            for (k = 0; k < meta->tensor.number_of_dimensions; k++)
            {
                meta->tensor.dimensions[k] = obj_desc->dimensions[k];
            }
        }
        else if (meta->type == (vx_enum)VX_TYPE_USER_DATA_OBJECT)
        {
            tivx_obj_desc_user_data_object_t *obj_desc;

            obj_desc = (tivx_obj_desc_user_data_object_t *)ref->obj_desc;

            meta->user_data_object.size = obj_desc->mem_size;

            memcpy(meta->user_data_object.type_name,
                   (const void *)obj_desc->type_name,
                   sizeof(obj_desc->type_name));
        }
        else if (meta->type == (vx_enum)VX_TYPE_ARRAY)
        {
            tivx_obj_desc_array_t *obj_desc;

            obj_desc = (tivx_obj_desc_array_t *)ref->obj_desc;

            meta->arr.item_type = obj_desc->item_type;
            meta->arr.capacity  = obj_desc->capacity;
        }
        else if (meta->type == (vx_enum)VX_TYPE_CONVOLUTION)
        {
            tivx_obj_desc_convolution_t *obj_desc;

            obj_desc = (tivx_obj_desc_convolution_t *)ref->obj_desc;

            meta->conv.cols = obj_desc->columns;
            meta->conv.rows = obj_desc->rows;

        }
        else if (meta->type == (vx_enum)VX_TYPE_MATRIX)
        {
            tivx_obj_desc_matrix_t *obj_desc;

            obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;

            meta->mat.type = obj_desc->data_type;
            meta->mat.cols = obj_desc->columns;
            meta->mat.rows = obj_desc->rows;
        }
        else if (meta->type == (vx_enum)VX_TYPE_DISTRIBUTION)
        {
            tivx_obj_desc_distribution_t *obj_desc;

            obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;

            meta->dist.bins   = obj_desc->num_bins;
            meta->dist.offset = obj_desc->offset;
            meta->dist.range  = obj_desc->range;

        }
        else if (meta->type == (vx_enum)TIVX_TYPE_RAW_IMAGE)
        {
            tivx_obj_desc_raw_image_t *obj_desc;

            obj_desc = (tivx_obj_desc_raw_image_t *)ref->obj_desc;

            memcpy(&meta->raw_image,
                   &obj_desc->params,
                   sizeof(tivx_raw_image_create_params_t));
        }
        else if (meta->type == (vx_enum)VX_TYPE_PYRAMID)
        {
            tivx_obj_desc_pyramid_t    *obj_desc;
            tivx_obj_desc_image_t      *img_obj_desc;
            vx_reference                img_ref;
            vx_pyramid                  pyramid;
            uint32_t                   *planeSizePtr;
            uint32_t                    lHandleSizes[VX_IPC_MAX_VX_PLANES];
            uint32_t                    cnt;
            uint32_t                    j;

            pyramid      = (vx_pyramid)ref;
            obj_desc     = (tivx_obj_desc_pyramid_t *)ref->obj_desc;
            img_ref      = (vx_reference)pyramid->img[0];
            img_obj_desc = (tivx_obj_desc_image_t *)img_ref->obj_desc;

            meta->pmd.levels = obj_desc->num_levels;
            meta->pmd.width  = obj_desc->width;
            meta->pmd.height = obj_desc->height;
            meta->pmd.scale  = obj_desc->scale;
            meta->pmd.format = obj_desc->format;
            meta->pmd.planes = img_obj_desc->planes;

            /* For images, only plane[0] is allocated and plane[1]..plane[N-1]
             * pointers are offset from plane[0]. Given this, we need to
             * compute the size of the buffer allocated for the image available
             * as plane[0] handle. The size is the sum of sizes of all
             * the planes.
             *
             * At the end of the loop below,
             * - lHandleSizes[0..meta->pmd.levels-1] will have the total
             *   size of the image, across all planes, for each level.
             * - lHandleSizes[meta->pmd.levels..] will have the sizes
             *   corresponding to image planes [1..planes-1] for each level
             *   packed consecutively.
             *
             * ex:- levels = 2 and each image has 3 planes
             * lHandleSizes[0] --> total size of level 0 image across all 3 planes
             * lHandleSizes[1] --> total size of level 1 image across all 3 planes
             * lHandleSizes[2] --> size of level 0 image plane 1
             * lHandleSizes[3] --> size of level 0 image plane 2
             * lHandleSizes[4] --> size of level 1 image plane 1
             * lHandleSizes[5] --> size of level 1 image plane 2
             *
             * All the above 6 entries will be transferred to the receiver but
             * only the first two entries i.e. lHandleSizes[0,1] will be translated.
             *
             * The receiver then can use entries lHandleSizes[2..] to derive the
             * individual plane sizes and adjust the image plane pointers.
             */
            planeSizePtr = &lHandleSizes[meta->pmd.levels];
            for (i = 0, cnt = 0; i <  meta->pmd.levels; i++)
            {
                lHandleSizes[i] = refDesc->handleSizes[cnt];
                ptrs[i]         = ptrs[cnt];
                cnt++;

                for (j = 1; j < meta->pmd.planes; j++)
                {
                    uint32_t    v = refDesc->handleSizes[cnt++];

                    lHandleSizes[i] += v;
                    *planeSizePtr++ = v;
                }
            }

            memcpy(refDesc->handleSizes, lHandleSizes, sizeof(lHandleSizes));

            /* Just translate plane[0] pointer of each level. A total
             * of pyramid levels pointers would need to be translated.
             * the receiving side will need to do the reverse oepration when
             * building the images at each pyramid level.
             */
            numPlanes = meta->pmd.levels;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unsupported type [%d].\n", meta->type);
            vxStatus = (vx_status)VX_FAILURE;
        }

        ipcMsg->numFd = numPlanes;

        for (i = 0; i < numPlanes; i++)
        {
            vxStatus = tivxMemTranslateVirtAddr(ptrs[i],
                                                &fd64,
                                                &phyAddr);

            if (vxStatus != (vx_status) VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "tivxMemTranslateVirtAddr() failed for "
                         "plane [%d]\n", i);
                break;
            }

            ipcMsg->fd[i] = fd64;
        }
    }

    return vxStatus;
}

vx_status tivx_utils_import_ref_from_ipc_xfer(vx_context                context,
                                              tivx_utils_ref_ipc_msg_t *ipcMsg,
                                              vx_reference              *ref)
{
    void                           *ptrs[VX_IPC_MAX_VX_PLANES] = {0};
    void                           *phyAddr[VX_IPC_MAX_VX_PLANES];
    tivx_utils_meta_format_t const *meta;
    tivx_utils_ref_desc_t          *refDesc;
    uint32_t                        i;
    vx_status                       vxStatus = VX_SUCCESS;

    refDesc = (tivx_utils_ref_desc_t *)&ipcMsg->refDesc;

    /* Validate the arguments. */
    if (context == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'context' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }
    else if (ref == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'ref' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }
    else if (ipcMsg == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'ipcMsg' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }

    /* Check if we have been provided a valid reference to work with. If not,
     * create one and then proceed.
     */
    if ((vxStatus == (vx_status)VX_SUCCESS) && (*ref == NULL))
    {
        vx_reference    lRef = NULL;

        meta = &refDesc->meta;

        if (meta->type == (vx_enum)VX_TYPE_IMAGE)
        {
            vx_image    obj;

            obj = vxCreateImage(context,
                                meta->img.width,
                                meta->img.height,
                                meta->img.format);

            if (obj == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "vxCreateImage() failed.\n");
                vxStatus = VX_FAILURE;
            }

            lRef = (vx_reference)obj;
        }
        else if (meta->type == (vx_enum)VX_TYPE_TENSOR)
        {
            vx_tensor   obj;

            obj = vxCreateTensor(context,
                                 meta->tensor.number_of_dimensions,
                                 meta->tensor.dimensions,
                                 meta->tensor.data_type,
                                 meta->tensor.fixed_point_position);

            if (obj == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "vxCreateTensor() failed.\n");
                vxStatus = VX_FAILURE;
            }

            lRef = (vx_reference)obj;
        }
        else if (meta->type == (vx_enum)VX_TYPE_USER_DATA_OBJECT)
        {
            vx_user_data_object obj;

            obj = vxCreateUserDataObject(context,
                                         meta->user_data_object.type_name,
                                         meta->user_data_object.size,
                                         NULL);

            if (obj == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "vxCreateUserDataObject() failed.\n");
                vxStatus = VX_FAILURE;
            }

            lRef = (vx_reference)obj;
        }
        else if (meta->type == (vx_enum)VX_TYPE_ARRAY)
        {
            vx_array    obj;

            obj = vxCreateArray(context,
                                meta->arr.item_type,
                                meta->arr.capacity);

            if (obj == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "vxCreateArray() failed.\n");
                vxStatus = VX_FAILURE;
            }

            lRef = (vx_reference)obj;
        }
        else if (meta->type == (vx_enum)VX_TYPE_CONVOLUTION)
        {
            vx_convolution  obj;

            obj = vxCreateConvolution(context,
                                      meta->conv.cols,
                                      meta->conv.rows);

            if (obj == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "vxCreateConvolution() failed.\n");
                vxStatus = VX_FAILURE;
            }

            lRef = (vx_reference)obj;
        }
        else if (meta->type == (vx_enum)VX_TYPE_MATRIX)
        {
            vx_matrix   obj;

            obj = vxCreateMatrix(context,
                                 meta->mat.type,
                                 meta->mat.cols,
                                 meta->mat.rows);

            if (obj == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrix() failed.\n");
                vxStatus = VX_FAILURE;
            }

            lRef = (vx_reference)obj;
        }
        else if (meta->type == (vx_enum)VX_TYPE_DISTRIBUTION)
        {
            vx_distribution obj;

            obj = vxCreateDistribution(context,
                                       meta->dist.bins,
                                       meta->dist.offset,
                                       meta->dist.range);

            if (obj == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "vxCreateDistribution() failed.\n");
                vxStatus = VX_FAILURE;
            }

            lRef = (vx_reference)obj;
        }
        else if (meta->type == (vx_enum)TIVX_TYPE_RAW_IMAGE)
        {
            tivx_raw_image_create_params_t *p;
            tivx_raw_image                  obj;

            p = (tivx_raw_image_create_params_t *)&meta->raw_image;

            obj = tivxCreateRawImage(context, p);

            if (obj == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxCreateRawImage() failed.\n");
                vxStatus = VX_FAILURE;
            }

            lRef = (vx_reference)obj;
        }
        else if (meta->type == (vx_enum)VX_TYPE_PYRAMID)
        {
            vx_pyramid  obj;

            obj = vxCreatePyramid(context,
                                  meta->pmd.levels,
                                  meta->pmd.scale,
                                  meta->pmd.width,
                                  meta->pmd.height,
                                  meta->pmd.format);

            if (obj == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "vxCreatePyramid() failed.\n");
                vxStatus = VX_FAILURE;
            }

            lRef = (vx_reference)obj;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unsupported type [%d].\n", meta->type);
            vxStatus = (vx_status)VX_FAILURE;
        }

        if (vxStatus == (vx_status)VX_SUCCESS)
        {
            *ref = lRef;
        }
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        for (i = 0; i < ipcMsg->numFd; i++)
        {
            /* Translate FD correcponding to plane[i]. */
            vxStatus = tivxMemTranslateFd((uint64_t)ipcMsg->fd[i],
                                          refDesc->handleSizes[i],
                                          &ptrs[i],
                                          &phyAddr[i]);

            if (vxStatus != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxMemTranslateFd() failed for "
                         "plane [%d]\n", i);
                break;
            }
        }
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        meta = &refDesc->meta;

        /* Treat image and pyramid objects separately. Additional processing
         * is necessary if the image has more than one plane. If the image has
         * a single plane then the information after the translation is in the
         * order expected by the import handle API.
         *
         * Similarly, if the images in the pyramid have just one single plane,
         * then the translated handles are  in the correct order needed by the
         * import handler API.
         */
        if ((meta->type == (vx_enum)VX_TYPE_IMAGE) &&
            (meta->img.planes > 1))
        {
            /* Adjust the size of plane[0] that was manipulated before
             * packing the information.
             */
            for (i = 1; i <  meta->img.planes; i++)
            {
                refDesc->handleSizes[0] -= refDesc->handleSizes[i];
            }

            /* Updated the plane pointers, plane[1]..plane[numPlanes-1]. Each
             * one is offset from the previous plane by the size fo the
             * previous plane.
             */
            for (i = 1; i < meta->img.planes; i++)
            {
                ptrs[i] = (uint8_t*)ptrs[i-1] + refDesc->handleSizes[i-1];
            }

            ipcMsg->numFd = meta->img.planes;
        }
        else if ((meta->type == (vx_enum)VX_TYPE_PYRAMID) &&
                 (meta->pmd.planes > 1))
        {
            uint32_t   *planeSizePtr;
            void       *lPtrs[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
            uint32_t    lHandleSizes[VX_IPC_MAX_VX_PLANES] = {0};
            uint32_t    cnt;
            uint32_t    j;

            planeSizePtr = &refDesc->handleSizes[meta->pmd.levels];
            for (i = 0, cnt = 0; i < meta->pmd.levels; i++)
            {
                uint32_t   *p = &lHandleSizes[cnt++];

                *p = refDesc->handleSizes[i];

                for (j = 1; j < meta->pmd.planes; j++)
                {
                    uint32_t    v = *planeSizePtr++;

                    *p                 -= v;
                    lHandleSizes[cnt++] = v;
                }

                /* Make a copy of the translated base address for each image. */
                lPtrs[i] = ptrs[i];
            }

            /* The 'ptrs' variable will have 'meta->pmd.levels'
             * number of entries populated.
             *
             * We need to compute other offsets from these values and
             * the lHandleSizes[] information.
             */
            for (i = 0, cnt = 0; i < meta->pmd.levels; i++)
            {
                /* Get the base address fromt the translated information. */
                ptrs[cnt++] = lPtrs[i];

                /* Generate the addresses for the rest of the planes. */
                for (j = 1; j < meta->pmd.planes; j++)
                {
                    ptrs[cnt] = (uint8_t*)ptrs[cnt-1] + lHandleSizes[cnt-1];;
                    cnt++;
                }
            }

            memcpy(refDesc->handleSizes, lHandleSizes, sizeof(lHandleSizes));

            ipcMsg->numFd = meta->pmd.levels * meta->pmd.planes;
        }

        /* Import the reference handles. */
        vxStatus =
            tivxReferenceImportHandle(*ref,
                                      (const void **)ptrs,
                                      (const uint32_t *)refDesc->handleSizes,
                                      ipcMsg->numFd);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "tivxReferenceImportHandle() failed.");
        }
    }

    return vxStatus;
}

