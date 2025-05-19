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
    uint32_t                numEntries;
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
                                             &numEntries);

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

            meta->object.img.width  = obj_desc->width;
            meta->object.img.height = obj_desc->height;
            meta->object.img.format = obj_desc->format;
            meta->object.img.planes = obj_desc->planes;

            /* Just translate plane[0] pointer. */
            numEntries = 1;
        }
        else if (meta->type == (vx_enum)VX_TYPE_TENSOR)
        {
            tivx_obj_desc_tensor_t *obj_desc;
            int32_t                 k;

            obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;

            meta->object.tensor.number_of_dimensions = obj_desc->number_of_dimensions;
            meta->object.tensor.data_type            = obj_desc->data_type;
            meta->object.tensor.fixed_point_position = obj_desc->fixed_point_position;

            for (k = 0; k < meta->object.tensor.number_of_dimensions; k++)
            {
                meta->object.tensor.dimensions[k] = obj_desc->dimensions[k];
            }
        }
        else if (meta->type == (vx_enum)VX_TYPE_USER_DATA_OBJECT)
        {
            tivx_obj_desc_user_data_object_t *obj_desc;

            obj_desc = (tivx_obj_desc_user_data_object_t *)ref->obj_desc;

            meta->object.user_data_object.size = obj_desc->mem_size;

            memcpy(meta->object.user_data_object.type_name,
                   (const void *)obj_desc->type_name,
                   sizeof(obj_desc->type_name));
        }
        else if (meta->type == (vx_enum)VX_TYPE_ARRAY)
        {
            tivx_obj_desc_array_t *obj_desc;

            obj_desc = (tivx_obj_desc_array_t *)ref->obj_desc;

            meta->object.arr.item_type = obj_desc->item_type;
            meta->object.arr.capacity  = obj_desc->capacity;
        }
        else if (meta->type == (vx_enum)VX_TYPE_CONVOLUTION)
        {
            tivx_obj_desc_convolution_t *obj_desc;

            obj_desc = (tivx_obj_desc_convolution_t *)ref->obj_desc;

            meta->object.conv.cols = obj_desc->columns;
            meta->object.conv.rows = obj_desc->rows;

        }
        else if (meta->type == (vx_enum)VX_TYPE_MATRIX)
        {
            tivx_obj_desc_matrix_t *obj_desc;

            obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;

            meta->object.mat.type = obj_desc->data_type;
            meta->object.mat.cols = obj_desc->columns;
            meta->object.mat.rows = obj_desc->rows;
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

            tivx_obj_desc_memcpy(&meta->raw_image,
                   &obj_desc->params,
                   sizeof(tivx_raw_image_create_params_t));
        }
        else if (meta->type == (vx_enum)VX_TYPE_PYRAMID)
        {
            tivx_obj_desc_pyramid_t    *obj_desc;
            tivx_obj_desc_image_t      *img_obj_desc;
            vx_reference                img_ref;
            vx_pyramid                  pyramid;

            pyramid      = (vx_pyramid)ref;
            obj_desc     = (tivx_obj_desc_pyramid_t *)ref->obj_desc;
            img_ref      = (vx_reference)pyramid->img[0];
            img_obj_desc = (tivx_obj_desc_image_t *)img_ref->obj_desc;

            meta->object.pmd.levels = obj_desc->num_levels;
            meta->object.pmd.width  = obj_desc->width;
            meta->object.pmd.height = obj_desc->height;
            meta->object.pmd.scale  = obj_desc->scale;
            meta->object.pmd.format = obj_desc->format;
            meta->object.pmd.planes = img_obj_desc->planes;

            numEntries = meta->object.pmd.levels;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unsupported type [%d].\n", meta->type);
            vxStatus = (vx_status)VX_FAILURE;
        }

        ipcMsg->numFd = numEntries;

        for (i = 0; i < numEntries; i++)
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
                                meta->object.img.width,
                                meta->object.img.height,
                                meta->object.img.format);

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
                                 meta->object.tensor.number_of_dimensions,
                                 meta->object.tensor.dimensions,
                                 meta->object.tensor.data_type,
                                 meta->object.tensor.fixed_point_position);

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
                                         meta->object.user_data_object.type_name,
                                         meta->object.user_data_object.size,
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
                                meta->object.arr.item_type,
                                meta->object.arr.capacity);

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
                                      meta->object.conv.cols,
                                      meta->object.conv.rows);

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
                                 meta->object.mat.type,
                                 meta->object.mat.cols,
                                 meta->object.mat.rows);

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
                                  meta->object.pmd.levels,
                                  meta->object.pmd.scale,
                                  meta->object.pmd.width,
                                  meta->object.pmd.height,
                                  meta->object.pmd.format);

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
            /* Translate FD corresponding to plane[i]. */
            vxStatus = tivxMemTranslateFd((uint64_t)ipcMsg->fd[i],
                                          refDesc->handleSizes[i],
                                          &ptrs[i],
                                          &phyAddr[i]);

            if (vxStatus != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxMemTranslateFd() failed for "
                         "FD [%d]\n", i);
                break;
            }
        }
    }

    if (vxStatus == (vx_status)VX_SUCCESS)
    {
        /* Import the reference handles. */
        vxStatus =
            tivxReferenceImportHandle(*ref,
                                      (const void **)ptrs,
                                      (const uint32_t *)refDesc->handleSizes,
                                      ipcMsg->numFd);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "tivxReferenceImportHandle() failed.\n");
        }
    }

    return vxStatus;
}

vx_bool tivx_utils_compare_refs_from_ipc_xfer(tivx_utils_ref_ipc_msg_t *ipcMsg1,
                                              tivx_utils_ref_ipc_msg_t *ipcMsg2)
{
    vx_bool   ret      = (vx_bool)vx_false_e;
    vx_status vxStatus = (vx_status)VX_SUCCESS;

    if (ipcMsg1 == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'ipcMsg1' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }

    if (ipcMsg2 == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'ipcMsg2' is NULL.\n");
        vxStatus = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == vxStatus)
    {
        if (ipcMsg1->numFd != ipcMsg2->numFd)
        {
            VX_PRINT(VX_ZONE_INFO, "ipcMsg1->numFd (%d) does not match ipcMsg2->numFd (%d).\n", ipcMsg1->numFd, ipcMsg2->numFd);
            vxStatus = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == vxStatus)
    {
        uint32_t i;
        tivx_utils_ref_desc_t  *refDesc1, *refDesc2;

        refDesc1 = &ipcMsg1->refDesc;
        refDesc2 = &ipcMsg2->refDesc;

        for (i = 0; i < ipcMsg1->numFd; i++)
        {
            /* Compare FD of ipcMsg1 and ipcMsg2. */
            ret |= tivxMemCompareFd((uint64_t)ipcMsg1->fd[i], (uint64_t)ipcMsg2->fd[i], refDesc1->handleSizes[i], refDesc2->handleSizes[i]);
        }
    }

    return ret;
}
