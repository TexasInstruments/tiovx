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
/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <vx_internal.h>

static vx_status ownDestructThreshold(vx_reference ref);
static vx_status ownAllocThresholdBuffer(vx_reference ref);


VX_API_ENTRY vx_status VX_API_CALL vxReleaseThreshold(vx_threshold *thr)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)thr, VX_TYPE_THRESHOLD, VX_EXTERNAL, NULL));
}

vx_threshold VX_API_CALL vxCreateThreshold(
    vx_context context, vx_enum thr_type, vx_enum data_type)
{
    vx_threshold thr = NULL;

    if(ownIsValidContext(context) == vx_true_e)
    {
        if (((VX_THRESHOLD_TYPE_BINARY == thr_type) ||
             (VX_THRESHOLD_TYPE_RANGE == thr_type)) &&
            ((VX_TYPE_INT64 != data_type) &&
             (VX_TYPE_UINT64 != data_type)))
        {
            thr = (vx_threshold)ownCreateReference(context, VX_TYPE_THRESHOLD,
                VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)thr) == VX_SUCCESS) &&
                (thr->base.type == VX_TYPE_THRESHOLD))
            {
                /* assign refernce type specific callback's */
                thr->base.destructor_callback = ownDestructThreshold;
                thr->base.mem_alloc_callback = ownAllocThresholdBuffer;
                thr->base.release_callback =
                    (tivx_reference_release_callback_f)vxReleaseThreshold;

                thr->obj_desc = (tivx_obj_desc_threshold_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_THRESHOLD);
                if(thr->obj_desc==NULL)
                {
                    vxReleaseThreshold(&thr);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate thr object descriptor\n");
                    thr = (vx_threshold)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    thr->obj_desc->type = thr_type;
                    thr->obj_desc->data_type = data_type;
                }
            }
        }
    }

    return (thr);
}

vx_status VX_API_CALL vxQueryThreshold(
    vx_threshold thr, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&thr->base, VX_TYPE_THRESHOLD) == vx_false_e
        ||
        thr->obj_desc == NULL
        )
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_THRESHOLD_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = thr->obj_desc->type;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_VALUE:
                /* Value is used only for Binary thr */
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                    (thr->obj_desc->type == VX_THRESHOLD_TYPE_BINARY))
                {
                    *(vx_int32 *)ptr = thr->obj_desc->value;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_LOWER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                   (thr->obj_desc->type == VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_int32 *)ptr = thr->obj_desc->lower;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_UPPER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                   (thr->obj_desc->type == VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_int32 *)ptr = thr->obj_desc->upper;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_TRUE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3))
                {
                    *(vx_int32 *)ptr = thr->obj_desc->true_value;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_FALSE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3))
                {
                    *(vx_int32 *)ptr = thr->obj_desc->false_value;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_DATA_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = thr->obj_desc->data_type;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetThresholdAttribute(
    vx_threshold thr, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&thr->base, VX_TYPE_THRESHOLD) == vx_false_e
        ||
        thr->obj_desc == NULL
        )
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_THRESHOLD_THRESHOLD_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                    (thr->obj_desc->type == VX_THRESHOLD_TYPE_BINARY))
                {
                    thr->obj_desc->value = *(vx_int32 *)ptr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_LOWER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                    (thr->obj_desc->type == VX_THRESHOLD_TYPE_RANGE))
                {
                    thr->obj_desc->lower = *(vx_int32 *)ptr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_UPPER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                    (thr->obj_desc->type == VX_THRESHOLD_TYPE_RANGE))
                {
                    thr->obj_desc->upper = *(vx_int32 *)ptr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_TRUE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3))
                {
                    thr->obj_desc->true_value = *(vx_int32 *)ptr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_FALSE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3))
                {
                    thr->obj_desc->false_value = *(vx_int32 *)ptr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    vx_enum type = *(vx_enum *)ptr;
                    if ((VX_THRESHOLD_TYPE_BINARY == type) ||
                        (VX_THRESHOLD_TYPE_RANGE == type))
                    {
                        thr->obj_desc->type = type;
                    }
                    else
                    {
                        status = VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
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
    vx_threshold thr = (vx_threshold)ref;

    if(thr->base.type == VX_TYPE_THRESHOLD)
    {
        if(thr->obj_desc!=NULL)
        {
            tivxObjDescFree((tivx_obj_desc_t**)&thr->obj_desc);
        }
    }
    return VX_SUCCESS;
}



