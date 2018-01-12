/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */



#include <vx_internal.h>

static vx_status ownDestructThreshold(vx_reference ref);
static vx_status ownAllocThresholdBuffer(vx_reference ref);


VX_API_ENTRY vx_status VX_API_CALL vxReleaseThreshold(vx_threshold *thresh)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)thresh, VX_TYPE_THRESHOLD, VX_EXTERNAL, NULL));
}

vx_threshold VX_API_CALL vxCreateThreshold(
    vx_context context, vx_enum thr_type, vx_enum data_type)
{
    vx_threshold thresh = NULL;
    tivx_obj_desc_threshold_t *obj_desc = NULL;

    if(ownIsValidContext(context) == vx_true_e)
    {
        if (((VX_THRESHOLD_TYPE_BINARY == thr_type) ||
             (VX_THRESHOLD_TYPE_RANGE == thr_type)) &&
            ((VX_TYPE_INT64 != data_type) &&
             (VX_TYPE_UINT64 != data_type)))
        {
            thresh = (vx_threshold)ownCreateReference(context, VX_TYPE_THRESHOLD,
                VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)thresh) == VX_SUCCESS) &&
                (thresh->base.type == VX_TYPE_THRESHOLD))
            {
                /* assign refernce type specific callback's */
                thresh->base.destructor_callback = &ownDestructThreshold;
                thresh->base.mem_alloc_callback = &ownAllocThresholdBuffer;
                thresh->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseThreshold;

                obj_desc = (tivx_obj_desc_threshold_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_THRESHOLD, (vx_reference)thresh);
                if(obj_desc==NULL)
                {
                    vxReleaseThreshold(&thresh);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate thresh object descriptor\n");
                    thresh = (vx_threshold)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    obj_desc->type = thr_type;
                    obj_desc->data_type = data_type;
                    thresh->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                }
            }
        }
    }

    return (thresh);
}

vx_status VX_API_CALL vxQueryThreshold(
    vx_threshold thresh, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_threshold_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&thresh->base, VX_TYPE_THRESHOLD) == vx_false_e)
        ||
        (thresh->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryThreshold: Invalid reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_threshold_t *)thresh->base.obj_desc;
        switch (attribute)
        {
            case VX_THRESHOLD_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = obj_desc->type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryThreshold: Query threshold type failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_VALUE:
                /* Value is used only for Binary thresh */
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                    (obj_desc->type == VX_THRESHOLD_TYPE_BINARY))
                {
                    *(vx_int32 *)ptr = obj_desc->value;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryThreshold: Query threshold value failed\n");
                }
                break;
            case VX_THRESHOLD_THRESHOLD_LOWER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                   (obj_desc->type == VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_int32 *)ptr = obj_desc->lower;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryThreshold: Query threshold lower failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_UPPER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                   (obj_desc->type == VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_int32 *)ptr = obj_desc->upper;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryThreshold: Query threshold upper failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_TRUE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    *(vx_int32 *)ptr = obj_desc->true_value;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryThreshold: Query threshold true value failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_FALSE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    *(vx_int32 *)ptr = obj_desc->false_value;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryThreshold: Query threshold false value failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_DATA_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = obj_desc->data_type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryThreshold: Query threshold data type failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetThresholdAttribute(
    vx_threshold thresh, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_threshold_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&thresh->base, VX_TYPE_THRESHOLD) == vx_false_e)
        ||
        (thresh->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetThresholdAttribute: Invalid reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_threshold_t *)thresh->base.obj_desc;
        switch (attribute)
        {
            case VX_THRESHOLD_THRESHOLD_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                    (obj_desc->type == VX_THRESHOLD_TYPE_BINARY))
                {
                    obj_desc->value = *(vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetThresholdAttribute: Set threshold value failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_LOWER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                    (obj_desc->type == VX_THRESHOLD_TYPE_RANGE))
                {
                    obj_desc->lower = *(vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetThresholdAttribute: Set threshold lower failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_UPPER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                    (obj_desc->type == VX_THRESHOLD_TYPE_RANGE))
                {
                    obj_desc->upper = *(vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetThresholdAttribute: Set threshold upper failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_TRUE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    obj_desc->true_value = *(vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetThresholdAttribute: Set threshold true value failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_FALSE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    obj_desc->false_value = *(vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetThresholdAttribute: Set threshold false value failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    vx_enum type = *(vx_enum *)ptr;
                    if ((VX_THRESHOLD_TYPE_BINARY == type) ||
                        (VX_THRESHOLD_TYPE_RANGE == type))
                    {
                        obj_desc->type = type;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "vxSetThresholdAttribute: Invalid threshold type\n");
                        status = VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetThresholdAttribute: Set threshold type failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxSetThresholdAttribute: Invalid attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

static vx_status ownAllocThresholdBuffer(vx_reference ref)
{
    return (VX_SUCCESS);
}

static vx_status ownDestructThreshold(vx_reference ref)
{
    if(ref->type == VX_TYPE_THRESHOLD)
    {
        if(ref->obj_desc!=NULL)
        {
            tivxObjDescFree((tivx_obj_desc_t**)&ref->obj_desc);
        }
    }
    return VX_SUCCESS;
}



