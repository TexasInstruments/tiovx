/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include <vx_internal.h>

static vx_status ownDestructThreshold(vx_reference ref);
static vx_status ownAllocThresholdBuffer(vx_reference ref);


VX_API_ENTRY vx_status VX_API_CALL vxReleaseThreshold(vx_threshold *thresh)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)thresh, (vx_enum)VX_TYPE_THRESHOLD, (vx_enum)VX_EXTERNAL, NULL));
}

vx_threshold VX_API_CALL vxCreateThreshold(
    vx_context context, vx_enum thr_type, vx_enum data_type)
{
    vx_threshold thresh = NULL;
    tivx_obj_desc_threshold_t *obj_desc = NULL;

    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if ((((vx_enum)VX_THRESHOLD_TYPE_BINARY == thr_type) ||
             ((vx_enum)VX_THRESHOLD_TYPE_RANGE == thr_type)) &&
            (((vx_enum)VX_TYPE_INT64 != data_type) &&
             ((vx_enum)VX_TYPE_UINT64 != data_type)))
        {
            thresh = (vx_threshold)ownCreateReference(context, (vx_enum)VX_TYPE_THRESHOLD,
                (vx_enum)VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)thresh) == (vx_status)VX_SUCCESS) &&
                (thresh->base.type == (vx_enum)VX_TYPE_THRESHOLD))
            {
                /* assign refernce type specific callback's */
                thresh->base.destructor_callback = &ownDestructThreshold;
                thresh->base.mem_alloc_callback = &ownAllocThresholdBuffer;
                thresh->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseThreshold;

                obj_desc = (tivx_obj_desc_threshold_t*)ownObjDescAlloc(
                    (vx_enum)TIVX_OBJ_DESC_THRESHOLD, (vx_reference)thresh);
                if(obj_desc==NULL)
                {
                    vxReleaseThreshold(&thresh);

                    vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES,
                        "Could not allocate thresh object descriptor\n");
                    thresh = (vx_threshold)ownGetErrorObject(
                        context, (vx_status)VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "Could not allocate thresh object descriptor\n");
                    VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                    VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in source/platform/psdk_j7/common/soc/tivx_platform_psdk_<soc>.h\n");
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
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_threshold_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference((vx_reference)thresh, (vx_enum)VX_TYPE_THRESHOLD) == (vx_bool)vx_false_e)
        ||
        (thresh->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_threshold_t *)thresh->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)VX_THRESHOLD_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = obj_desc->type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query threshold type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_THRESHOLD_THRESHOLD_VALUE:
                /* Value is used only for Binary thresh */
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                    (obj_desc->type == (vx_enum)VX_THRESHOLD_TYPE_BINARY))
                {
                    *(vx_int32 *)ptr = obj_desc->value;
                }
                else
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "Query threshold value failed\n");
                }
                break;
            case (vx_enum)VX_THRESHOLD_THRESHOLD_LOWER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                   (obj_desc->type == (vx_enum)VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_int32 *)ptr = obj_desc->lower;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query threshold lower failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_THRESHOLD_THRESHOLD_UPPER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                   (obj_desc->type == (vx_enum)VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_int32 *)ptr = obj_desc->upper;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query threshold upper failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_THRESHOLD_TRUE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    *(vx_int32 *)ptr = obj_desc->true_value;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query threshold true value failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_THRESHOLD_FALSE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    *(vx_int32 *)ptr = obj_desc->false_value;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query threshold false value failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_THRESHOLD_DATA_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = obj_desc->data_type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query threshold data type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
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
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_threshold_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference((vx_reference)thresh, (vx_enum)VX_TYPE_THRESHOLD) == (vx_bool)vx_false_e)
        ||
        (thresh->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_threshold_t *)thresh->base.obj_desc;
        switch (attribute)
        {
            case (vx_enum)VX_THRESHOLD_THRESHOLD_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                    (obj_desc->type == (vx_enum)VX_THRESHOLD_TYPE_BINARY))
                {
                    obj_desc->value = *(const vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set threshold value failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_THRESHOLD_THRESHOLD_LOWER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                    (obj_desc->type == (vx_enum)VX_THRESHOLD_TYPE_RANGE))
                {
                    obj_desc->lower = *(const vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set threshold lower failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_THRESHOLD_THRESHOLD_UPPER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U) &&
                    (obj_desc->type == (vx_enum)VX_THRESHOLD_TYPE_RANGE))
                {
                    obj_desc->upper = *(const vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set threshold upper failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_THRESHOLD_TRUE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    obj_desc->true_value = *(const vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set threshold true value failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_THRESHOLD_FALSE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3U))
                {
                    obj_desc->false_value = *(const vx_int32 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set threshold false value failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_THRESHOLD_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    vx_enum type = *(const vx_enum *)ptr;
                    if (((vx_enum)VX_THRESHOLD_TYPE_BINARY == type) ||
                        ((vx_enum)VX_THRESHOLD_TYPE_RANGE == type))
                    {
                        obj_desc->type = type;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Invalid threshold type\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set threshold type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

static vx_status ownAllocThresholdBuffer(vx_reference ref)
{
    return ((vx_status)VX_SUCCESS);
}

static vx_status ownDestructThreshold(vx_reference ref)
{
    if(ref->type == (vx_enum)VX_TYPE_THRESHOLD)
    {
        if(ref->obj_desc!=NULL)
        {
            ownObjDescFree((tivx_obj_desc_t**)&ref->obj_desc);
        }
    }
    return (vx_status)VX_SUCCESS;
}



