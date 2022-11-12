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

/* Constant internal to this file. */
#define TIOVX_REF_MAX_NUM_MEM_ELEM  (TIVX_PYRAMID_MAX_LEVEL_OBJECTS)

typedef struct _vx_enum_type_size {
    vx_enum item_type;
    vx_size item_size;
} vx_enum_type_size_t;

static vx_enum_type_size_t g_reference_enum_type_sizes[] = {
    {(vx_enum)VX_TYPE_INVALID,       0},
    /* Scalar Types */
    {(vx_enum)VX_TYPE_CHAR,          sizeof(vx_char)},
    {(vx_enum)VX_TYPE_INT8,          sizeof(vx_int8)},
    {(vx_enum)VX_TYPE_INT16,         sizeof(vx_int16)},
    {(vx_enum)VX_TYPE_INT32,         sizeof(vx_int32)},
    {(vx_enum)VX_TYPE_INT64,         sizeof(vx_int64)},
    {(vx_enum)VX_TYPE_UINT8,         sizeof(vx_uint8)},
    {(vx_enum)VX_TYPE_UINT16,        sizeof(vx_uint16)},
    {(vx_enum)VX_TYPE_UINT32,        sizeof(vx_uint32)},
    {(vx_enum)VX_TYPE_UINT64,        sizeof(vx_uint64)},
    {(vx_enum)VX_TYPE_FLOAT32,       sizeof(vx_float32)},
    {(vx_enum)VX_TYPE_FLOAT64,       sizeof(vx_float64)},
    {(vx_enum)VX_TYPE_ENUM,          sizeof(vx_enum)},
    {(vx_enum)VX_TYPE_BOOL,          sizeof(vx_bool)},
    {(vx_enum)VX_TYPE_SIZE,          sizeof(vx_size)},
    {(vx_enum)VX_TYPE_DF_IMAGE,      sizeof(vx_df_image)},
    /* Structures */
    {(vx_enum)VX_TYPE_RECTANGLE,     sizeof(vx_rectangle_t)},
    {(vx_enum)VX_TYPE_COORDINATES2D, sizeof(vx_coordinates2d_t)},
    {(vx_enum)VX_TYPE_COORDINATES3D, sizeof(vx_coordinates3d_t)},
    {(vx_enum)VX_TYPE_KEYPOINT,      sizeof(vx_keypoint_t)},
    /* Pseudo Objects */
    {(vx_enum)VX_TYPE_META_FORMAT,   sizeof(tivx_meta_format_t)},
    /* Framework Objects */
    {(vx_enum)VX_TYPE_REFERENCE,     sizeof(tivx_reference_t)},
    {(vx_enum)VX_TYPE_CONTEXT,       sizeof(tivx_context_t)},
    {(vx_enum)VX_TYPE_GRAPH,         sizeof(tivx_graph_t)},
    {(vx_enum)VX_TYPE_NODE,          sizeof(tivx_node_t)},
    {(vx_enum)VX_TYPE_PARAMETER,     sizeof(tivx_parameter_t)},
    {(vx_enum)VX_TYPE_KERNEL,        sizeof(tivx_kernel_t)},
    {TIVX_TYPE_SUPER_NODE,  sizeof(tivx_super_node_t)},
    /* data objects */
    {(vx_enum)VX_TYPE_ARRAY,         sizeof(tivx_array_t)},
    {VX_TYPE_USER_DATA_OBJECT, sizeof(tivx_user_data_object_t)},
    {TIVX_TYPE_RAW_IMAGE, sizeof(tivx_raw_image_t)},
    {(vx_enum)VX_TYPE_CONVOLUTION,   sizeof(tivx_convolution_t)},
    {(vx_enum)VX_TYPE_DELAY,         sizeof(tivx_delay_t)},
    {(vx_enum)VX_TYPE_DISTRIBUTION,  sizeof(tivx_distribution_t)},
    {(vx_enum)VX_TYPE_IMAGE,         sizeof(tivx_image_t)},
    {(vx_enum)VX_TYPE_TENSOR,        sizeof(tivx_tensor_t)},
    {(vx_enum)VX_TYPE_LUT,           sizeof(tivx_lut_t)},
    {(vx_enum)VX_TYPE_MATRIX,        sizeof(tivx_matrix_t)},
    {(vx_enum)VX_TYPE_PYRAMID,       sizeof(tivx_pyramid_t)},
    {(vx_enum)VX_TYPE_REMAP,         sizeof(tivx_remap_t)},
    {(vx_enum)VX_TYPE_SCALAR,        sizeof(tivx_scalar_t)},
    {(vx_enum)VX_TYPE_THRESHOLD,     sizeof(tivx_threshold_t)},
    {(vx_enum)TIVX_TYPE_DATA_REF_Q,  sizeof(tivx_data_ref_queue_t)},
};

vx_size ownSizeOfEnumType(vx_enum item_type)
{
    vx_uint32 i = 0;
    vx_size size = 0U;

    for (i = 0; i < dimof(g_reference_enum_type_sizes); i++) {
        if (item_type == g_reference_enum_type_sizes[i].item_type) {
            size = g_reference_enum_type_sizes[i].item_size;
            break;
        }
    }

    return (size);
}

vx_bool ownIsValidReference(vx_reference ref)
{
    vx_bool ret = (vx_bool)vx_false_e;
    if (ref != NULL)
    {
        if ( (ref->magic == TIVX_MAGIC) &&
             (ownIsValidType(ref->type) == (vx_bool)vx_true_e) &&
             (( (ref->type != (vx_enum)VX_TYPE_CONTEXT) && (ownIsValidContext(ref->context) == (vx_bool)vx_true_e) ) ||
              ( (ref->type == (vx_enum)VX_TYPE_CONTEXT) && (ref->context == NULL) )) )
        {
            ret = (vx_bool)vx_true_e;
        }
        else if (ref->magic == TIVX_BAD_MAGIC)
        {
            VX_PRINT(VX_ZONE_INFO, "Reference has already been released and garbage collected!\n");
        }
        else if (ref->type != (vx_enum)VX_TYPE_CONTEXT)
        {
            VX_PRINT(VX_ZONE_INFO, "Not a valid reference!\n");
        }
        else
        {
            /* do nothing as ret is already initialized */
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_INFO, "Reference was NULL\n");
    }
    return ret;
}

vx_status ownInitReference(vx_reference ref, vx_context context, vx_enum type, vx_reference scope)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if (ref != NULL)
    {
        ref->magic = TIVX_MAGIC;
        ref->type = type;
        ref->context = context;
        ref->internal_count = 0;
        ref->external_count = 0;
        ref->mem_alloc_callback = NULL;
        ref->destructor_callback = NULL;
        ref->delay = NULL;
        ref->delay_slot_index = 0;
        ref->is_virtual = (vx_bool)vx_false_e;
        ref->obj_desc = NULL;
        ref->lock = NULL;
        ref->is_accessible = (vx_bool)vx_false_e;
        ref->is_array_element = (vx_bool)vx_false_e;

        ownReferenceSetScope(ref, scope);

        status = (vx_status)VX_SUCCESS;

        if((ref->type==(vx_enum)VX_TYPE_CONTEXT) || (ref->type==(vx_enum)VX_TYPE_GRAPH))
        {
            /* create referencec only for context and graph
             * for others use the context lock
             */
            status = tivxMutexCreate(&ref->lock);
            if (status != 0)
            {
                VX_PRINT(VX_ZONE_ERROR, "Cannot create Semaphore\n");
            }
        }
    }

    return status;
}


vx_uint32 ownDecrementReference(vx_reference ref, vx_enum reftype)
{
    vx_uint32 result = (uint32_t)UINT32_MAX;
    if (ref != NULL)
    {
        ownReferenceLock(ref);
        if ((reftype == (vx_enum)VX_INTERNAL) || (reftype == (vx_enum)VX_BOTH))
        {
            if (ref->internal_count == 0U)
            {
                VX_PRINT(VX_ZONE_WARNING, "#### INTERNAL REF COUNT IS ALREADY ZERO!!! "VX_FMT_REF" type:%08x #####\n", ref, ref->type);
            }
            else
            {
                ref->internal_count--;
            }
        }
        if ((reftype == (vx_enum)VX_EXTERNAL) || (reftype == (vx_enum)VX_BOTH))
        {
            if (ref->external_count == 0U)
            {
                VX_PRINT(VX_ZONE_WARNING, "#### EXTERNAL REF COUNT IS ALREADY ZERO!!! "VX_FMT_REF" type:%08x #####\n", ref, ref->type);
            }
            else
            {
                ref->external_count--;
            }
        }
        result = ref->internal_count + ref->external_count;
        ownReferenceUnlock(ref);
    }
    return result;
}


vx_uint32 ownTotalReferenceCount(vx_reference ref)
{
    vx_uint32 count = 0;
    if (ref != NULL)
    {
        ownReferenceLock(ref);
        count = ref->external_count + ref->internal_count;
        ownReferenceUnlock(ref);
    }
    return count;
}

vx_uint32 ownIncrementReference(vx_reference ref, vx_enum reftype)
{
    vx_uint32 count = 0u;
    if (ref != NULL)
    {
        ownReferenceLock(ref);
        if ((reftype == (vx_enum)VX_EXTERNAL) || (reftype == (vx_enum)VX_BOTH))
        {
            ref->external_count++;
        }
        if ((reftype == (vx_enum)VX_INTERNAL) || (reftype == (vx_enum)VX_BOTH))
        {
            ref->internal_count++;
        }
        count = ref->internal_count + ref->external_count;
        ownReferenceUnlock(ref);
    }
    return count;
}

vx_status ownReleaseReferenceInt(vx_reference *pref,
                        vx_enum type,
                        vx_enum reftype,
                        tivx_reference_callback_f special_destructor)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference ref;

    if (pref != NULL)
    {
        ref = *pref;
    }
    else
    {
        ref = NULL;
    }

    if (ownIsValidSpecificReference(ref, type) == (vx_bool)vx_true_e)
    {
        if (ownDecrementReference(ref, reftype) == 0U)
        {
            tivx_reference_callback_f destructor = special_destructor;

            if (ownRemoveReferenceFromContext(ref->context, ref) == (vx_bool)vx_false_e)
            {
                VX_PRINT(VX_ZONE_ERROR,"Invalid reference\n");
                status = (vx_status)VX_ERROR_INVALID_REFERENCE;
            }
            else
            {
                /* find the destructor method */
                if (destructor==NULL)
                {
                    destructor = ref->destructor_callback;
                }

                /* if there is a destructor, call it. */
                if (destructor != NULL)
                {
                    status = destructor(ref);

                    if (status != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"destructor() returned error.\n");
                    }
                }

                if(ref->lock != NULL)
                {
                    tivxMutexDelete(&ref->lock);
                }
                ref->magic = TIVX_BAD_MAGIC; /* make sure no existing copies of refs can use ref again */

                ownObjectFree(ref);
            }
        }
        *pref = NULL;
    } else {
        VX_PRINT(VX_ZONE_ERROR,"Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

vx_reference ownCreateReference(vx_context context, vx_enum type, vx_enum reftype, vx_reference scope)
{
    vx_reference ref = (vx_reference)ownObjectAlloc(type);
    vx_status status = (vx_status)VX_SUCCESS;

    if (ref != NULL)
    {
        status = ownInitReference(ref, context, type, scope);
        if(status==(vx_status)VX_SUCCESS)
        {
            ownIncrementReference(ref, reftype);
            if (ownAddReferenceToContext(context, ref) == (vx_bool)vx_false_e)
            {
                VX_PRINT(VX_ZONE_ERROR, "Add reference to context failed\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
        }

        if(status!=(vx_status)VX_SUCCESS)
        {
            ownObjectFree(ref);
            vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES, "Failed to add to resources table\n");
            VX_PRINT(VX_ZONE_ERROR, "Failed to add to resources table\n");
            ref = (vx_reference)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
        }
    }
    else
    {
        vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES, "Failed to allocate reference object\n");
        VX_PRINT(VX_ZONE_ERROR, "Failed to allocate reference object\n");
        ref = (vx_reference)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
    }
    return ref;
}



vx_bool ownIsValidSpecificReference(vx_reference ref, vx_enum type)
{
    vx_bool ret = (vx_bool)vx_false_e;
    if (ref != NULL)
    {
        if ((ref->magic == TIVX_MAGIC) &&
            (ref->type == type) &&
            (ownIsValidContext(ref->context) == (vx_bool)vx_true_e))
        {
            ret = (vx_bool)vx_true_e;
        }
    }
    return ret;
}


void ownPrintReference(vx_reference ref)
{
    if (ref != NULL)
    {
        VX_PRINT(VX_ZONE_REFERENCE, "vx_reference: magic:%08x type:%08x count:[%u,%u]\n", ref, ref->magic, ref->type, ref->external_count, ref->internal_count);
    }
}




vx_bool ownIsValidType(vx_enum type)
{
    vx_bool ret = (vx_bool)vx_false_e;
    if (type <= (vx_enum)VX_TYPE_INVALID)
    {
        ret = (vx_bool)vx_false_e;
    }
    else if (TIVX_TYPE_IS_SCALAR(type)) /* some scalar */
    {
        ret = (vx_bool)vx_true_e;
    }
    else if (TIVX_TYPE_IS_STRUCT(type)) /* some struct */
    {
        ret = (vx_bool)vx_true_e;
    }
    else if (TIVX_TYPE_IS_OBJECT(type)) /* some object */
    {
        ret = (vx_bool)vx_true_e;
    }
    else if (TIVX_TYPE_IS_TI_OBJECT(type)) /* some object */
    {
        ret = (vx_bool)vx_true_e;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Type 0x%08x is invalid!\n");
    }
    return ret; /* otherwise, not a valid type */
}

vx_status ownReferenceLock(vx_reference ref)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if(ref != NULL)
    {
        if(ref->lock != NULL)
        {
            status = tivxMutexLock(ref->lock);
        }
        else
        {
            if(ref->context != NULL)
            {
                status = tivxMutexLock(ref->context->base.lock);
            }
        }
    }

    return status;
}

vx_status ownReferenceUnlock(vx_reference ref)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if(ref != NULL)
    {
        if(ref->lock != NULL)
        {
            status = tivxMutexUnlock(ref->lock);
        }
        else
        {
            if(ref->context != NULL)
            {
                status = tivxMutexUnlock(ref->context->base.lock);
            }
        }
    }

    return status;
}

void ownInitReferenceForDelay(vx_reference ref, vx_delay d, vx_int32 slot_index) {
    ref->delay=d;
    ref->delay_slot_index=slot_index;
}

vx_status ownReferenceAllocMem(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidReference(ref) == (vx_bool)vx_true_e)
    {
        if(ref->mem_alloc_callback != NULL)
        {
            status = ref->mem_alloc_callback(ref);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

void ownReferenceSetScope(vx_reference ref, vx_reference scope)
{
    if(ref != NULL)
    {
        ref->scope = scope;
        if(NULL != ref->obj_desc)
        {
            ref->obj_desc->scope_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
            if((NULL != scope) && (NULL != scope->obj_desc))
            {
                if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                        ||
                    (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
                    )
                {
                    /* set scope_obj_desc_id in obj_desc only for composite objects like pyramid and object array */
                    ref->obj_desc->scope_obj_desc_id = scope->obj_desc->obj_desc_id;
                }
            }
        }
    }
}

uint64_t ownReferenceGetHostRefFromObjDescId(uint16_t obj_desc_id)
{
    tivx_obj_desc_t *obj_desc = ownObjDescGet(obj_desc_id);
    uint64_t ref = 0;

    if(obj_desc!=NULL)
    {
        ref = (uint64_t)obj_desc->host_ref;
    }

    return ref;
}

vx_reference ownReferenceGetHandleFromObjDescId(uint16_t obj_desc_id)
{
    tivx_obj_desc_t *obj_desc = ownObjDescGet(obj_desc_id);
    vx_reference ref = NULL;

    if(obj_desc!=NULL)
    {
        ref = (vx_reference)(uintptr_t)obj_desc->host_ref;
    }

    return ref;
}

VX_API_ENTRY vx_status VX_API_CALL tivxSetReferenceAttribute(vx_reference ref, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((ownIsValidReference(ref) == (vx_bool)vx_false_e) &&
        (ownIsValidContext((vx_context)ref) == (vx_bool)vx_false_e))
    {

        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case (vx_enum)TIVX_REFERENCE_TIMESTAMP:
                if ((VX_CHECK_PARAM(ptr, size, vx_uint64, 0x3U) && (NULL != ref->obj_desc)))
                {
                    ref->obj_desc->timestamp = (vx_uint64)*(const vx_uint64 *)ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "error setting reference timestamp\n");
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

VX_API_ENTRY vx_status VX_API_CALL vxQueryReference(vx_reference ref, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* if it is not a reference and not a context */
    if ((ownIsValidReference(ref) == (vx_bool)vx_false_e) &&
        (ownIsValidContext((vx_context)ref) == (vx_bool)vx_false_e)) {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case (vx_enum)VX_REFERENCE_COUNT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = ref->external_count;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference count failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_REFERENCE_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = ref->type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference type failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_REFERENCE_NAME:
                if (VX_CHECK_PARAM(ptr, size, vx_char*, 0x3U))
                {
                    *(vx_char**)ptr = &ref->name[0];
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference name failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_REFERENCE_TIMESTAMP:
                if ((VX_CHECK_PARAM(ptr, size, vx_uint64, 0x3U) && (NULL != ref->obj_desc)))
                {
                    *(vx_uint64 *)ptr = ref->obj_desc->timestamp;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference timestamp failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_REFERENCE_INVALID:
                if (VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U))
                {
                    vx_bool is_ref_invalid;

                    if (tivxFlagIsBitSet(ref->obj_desc->flags, TIVX_REF_FLAG_IS_INVALID) != 0U)
                    {
                        is_ref_invalid = (vx_bool)vx_true_e;
                    }
                    else
                    {
                        is_ref_invalid = (vx_bool)vx_false_e;
                    }
                    *(vx_bool*)ptr = is_ref_invalid;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Query reference invalid flag failed\n");
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

VX_API_ENTRY vx_status VX_API_CALL vxSetReferenceName(vx_reference ref, const vx_char *name)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    if (ownIsValidReference(ref) != 0)
    {
        snprintf(ref->name, VX_MAX_REFERENCE_NAME, name);
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseReference(vx_reference* ref_ptr)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    vx_reference ref;

    if (ref_ptr != NULL)
    {
        ref = *ref_ptr;
    }
    else
    {
        ref = NULL;
    }

    if (ownIsValidReference(ref) == (vx_bool)vx_true_e)
    {
        if(ref->release_callback != NULL)
        {
            status = ref->release_callback(ref_ptr);
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRetainReference(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidReference(ref) == (vx_bool)vx_true_e)
    {
        ownIncrementReference(ref, (vx_enum)VX_EXTERNAL);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetStatus(vx_reference ref)
{
    vx_status status = (vx_status)VX_FAILURE;

    if (ref == NULL)
    {
        /*! \internal probably ran out of handles or memory */
        VX_PRINT(VX_ZONE_ERROR, "Reference is NULL\n");
        status = (vx_status)VX_ERROR_NO_RESOURCES;
    }
    else if (ownIsValidReference(ref) == (vx_bool)vx_true_e)
    {
        if (ref->type == (vx_enum)VX_TYPE_ERROR)
        {
            tivx_error_t *error = (tivx_error_t *)ref;
            status = error->status;
        }
        else
        {
            status = (vx_status)VX_SUCCESS;
        }
    }
    else if (ownIsValidContext((vx_context)ref) == (vx_bool)vx_true_e)
    {
        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        /* do nothing */
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxHint(vx_reference reference, vx_enum hint, const void* data, vx_size data_size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* reference param should be a valid OpenVX reference*/
    if ((ownIsValidContext((vx_context)reference) == (vx_bool)vx_false_e) && (ownIsValidReference(reference) == (vx_bool)vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Hint not supported\n");
        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
    }

    return status;
}

VX_API_ENTRY vx_context VX_API_CALL vxGetContext(vx_reference reference)
{
    vx_context context = NULL;
    if (ownIsValidReference(reference) == (vx_bool)vx_true_e)
    {
        context = reference->context;
    }
    else if (ownIsValidContext((vx_context)reference) == (vx_bool)vx_true_e)
    {
        context = (vx_context)reference;
    }
    else
    {
        /* Do nothing as context is already initialized with null */
    }
    return context;
}

VX_API_ENTRY vx_bool VX_API_CALL tivxIsReferenceMetaFormatEqual(vx_reference ref1, vx_reference ref2)
{
    vx_meta_format  mf1;
    vx_meta_format  mf2;
    vx_status       status;
    vx_bool         boolStatus;

    mf1        = NULL;
    mf2        = NULL;
    status     = (vx_status)VX_SUCCESS;
    boolStatus = (vx_bool)vx_false_e;

    if ((ref1 == NULL) ||
        (ownIsValidReference(ref1) == (vx_bool)vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference1 is Invalid.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if ((ref2 == NULL) ||
             (ownIsValidReference(ref2) == (vx_bool)vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference2 is Invalid.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (ref1->type != ref2->type)
    {
        VX_PRINT(VX_ZONE_ERROR, "The type of the references do not match.\n");
        status = (vx_status)VX_FAILURE;
    }

    /* Create a meta format object. */
    if (status == (vx_status)VX_SUCCESS)
    {
        mf1 = ownCreateMetaFormat(ref1->context);

        if (mf1 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to create meta format object.\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    /* Create a meta format object. */
    if (status == (vx_status)VX_SUCCESS)
    {
        mf2 = ownCreateMetaFormat(ref2->context);

        if (mf2 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to create meta format object.\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    /* Set the ref1 in mf1. */
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxSetMetaFormatFromReference(mf1, ref1);

        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "vxSetMetaFormatFromReference(ref1) failed.\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    /* Set the ref2 in mf2. */
    if (status == (vx_status)VX_SUCCESS)
    {
        status = vxSetMetaFormatFromReference(mf2, ref2);

        if (status != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "vxSetMetaFormatFromReference(ref2) failed.\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        boolStatus = ownIsMetaFormatEqual(mf1, mf2, mf1->type);
    }

    /* Release the meta objects. */
    if (mf1 != NULL)
    {
        ownReleaseMetaFormat(&mf1);
    }

    if (mf2 != NULL)
    {
        ownReleaseMetaFormat(&mf2);
    }

    return boolStatus;
}

vx_bool tivxIsReferenceVirtual(vx_reference ref)
{
    vx_bool ret = (vx_bool)vx_false_e;

    if (NULL != ref)
    {
        ret = ref->is_virtual;
    }

    return (ret);
}

vx_reference tivxGetReferenceParent(vx_reference child_ref)
{
    vx_reference ref = NULL;

    if (ownIsValidReference(child_ref) == (vx_bool)vx_true_e)
    {
        tivx_obj_desc_t *obj_desc =
            (tivx_obj_desc_t *)child_ref->obj_desc;

        if (obj_desc != NULL)
        {
            if ((vx_bool)vx_true_e == child_ref->is_array_element)
            {
                ref = ownReferenceGetHandleFromObjDescId(obj_desc->scope_obj_desc_id);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Provided child_ref->obj_desc is NULL.\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Provided child_ref is invalid.\n");
    }

    return ref;
}

vx_status tivxReferenceImportHandle(vx_reference ref, const void *addr[], const uint32_t size[], uint32_t num_entries)
{
    tivx_shared_mem_ptr_t  *mem_ptr;
    volatile uint32_t      *mem_size;
    uint64_t                shared_ptr[TIOVX_REF_MAX_NUM_MEM_ELEM] = {0};
    uint32_t                numMemElem;
    uint32_t                numNulls;
    vx_status               status;
    vx_uint32               num_levels = 0;
    tivx_shared_mem_ptr_t  *mem_ptr_arr[TIOVX_REF_MAX_NUM_MEM_ELEM];
    volatile uint32_t      *mem_size_arr[TIOVX_REF_MAX_NUM_MEM_ELEM];
    volatile uint32_t       num_planes[TIOVX_REF_MAX_NUM_MEM_ELEM];
    uint32_t                i;
    uint32_t                j;
    uint32_t                k;

    status = (vx_status)VX_SUCCESS;

    /* Validate the arguments. */
    if ((ref == NULL) || (ownIsValidReference(ref) == (vx_bool)vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference is Invalid.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (num_entries == 0)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "The parameter 'num_entries' must be non-zero.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (addr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'addr' is NULL.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (size == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'size' is NULL.\n");
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        numMemElem = 1;
        mem_ptr    = NULL;
        mem_size   = NULL;

        if (ref->type == (vx_enum)VX_TYPE_IMAGE)
        {
            tivx_obj_desc_image_t *obj_desc;

            obj_desc = (tivx_obj_desc_image_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                numMemElem = obj_desc->planes;
                mem_ptr    = obj_desc->mem_ptr;
                mem_size   = obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_TENSOR)
        {
            tivx_obj_desc_tensor_t *obj_desc;

            obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_USER_DATA_OBJECT)
        {
            tivx_obj_desc_user_data_object_t *obj_desc;

            obj_desc = (tivx_obj_desc_user_data_object_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_ARRAY)
        {
            tivx_obj_desc_array_t *obj_desc;

            obj_desc = (tivx_obj_desc_array_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_CONVOLUTION)
        {
            tivx_obj_desc_convolution_t *obj_desc;

            obj_desc = (tivx_obj_desc_convolution_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_MATRIX)
        {
            tivx_obj_desc_matrix_t *obj_desc;

            obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_DISTRIBUTION)
        {
            tivx_obj_desc_distribution_t *obj_desc;

            obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)TIVX_TYPE_RAW_IMAGE)
        {
            tivx_obj_desc_raw_image_t *obj_desc;

            obj_desc = (tivx_obj_desc_raw_image_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                numMemElem = obj_desc->params.num_exposures;
                mem_ptr    = obj_desc->mem_ptr;
                mem_size   = obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_PYRAMID)
        {
            tivx_obj_desc_pyramid_t    *obj_desc;
            tivx_obj_desc_image_t      *img_obj_desc;
            vx_pyramid                  pyramid;
            vx_reference                img_ref;

            obj_desc = (tivx_obj_desc_pyramid_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                num_levels = obj_desc->num_levels;
                pyramid    = (vx_pyramid)ref;
                numMemElem = 0;

                for (i = 0; i < num_levels; i++)
                {
                    img_ref      = (vx_reference)pyramid->img[i];
                    img_obj_desc = (tivx_obj_desc_image_t *)img_ref->obj_desc;

                    if (img_obj_desc == NULL)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "'img_obj_desc' is NULL.\n");
                        status = (vx_status)VX_FAILURE;
                        break;
                    }

                    numMemElem      += img_obj_desc->planes;
                    mem_ptr_arr[i]   = img_obj_desc->mem_ptr;
                    mem_size_arr[i]  = img_obj_desc->mem_size;
                    num_planes[i]    = img_obj_desc->planes;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unsupported type [%d].\n", ref->type);
            status = (vx_status)VX_FAILURE;
        }

        if ((status == (vx_status)VX_SUCCESS) &&
            (numMemElem > num_entries))
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "num_entries [%d] less than num handles expected [%d].\n",
                     num_entries, numMemElem);

            status = (vx_status)VX_FAILURE;
        }

        /* Validate the addr entries. */
        numNulls = 0;

        if (status == (vx_status)VX_SUCCESS)
        {
            for (i = 0; i < numMemElem; i++)
            {
                if (addr[i] != NULL)
                {
                    /* Check if the memory allocated can be translated. */
                    shared_ptr[i] =
                        tivxMemHost2SharedPtr((uint64_t)(uintptr_t)addr[i],
                                              (vx_enum)TIVX_MEM_EXTERNAL);

                    if (shared_ptr[i] == 0)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "addr[%d] is INVALID.\n", i);
                        status = (vx_status)VX_FAILURE;
                        break;
                    }
                }
                else
                {
                    numNulls++;
                }
            }

            if ((status == (vx_status)VX_SUCCESS) &&
                (numNulls != 0) &&
                (numNulls != numMemElem))
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "addr[] has a mix of NULL and non-NULL entries.\n");
                status = (vx_status)VX_FAILURE;
            }
        }

        /* Validate the sizes of the handles. Do this only if we are not
         * importing NULLs.
         */
        if ((status == (vx_status)VX_SUCCESS) && (numNulls == 0))
        {
            if (ref->type == (vx_enum)VX_TYPE_PYRAMID)
            {
                k = 0;
                for (i = 0; i < num_levels; i++)
                {
                    mem_size = mem_size_arr[i];

                    for (j = 0; j < num_planes[i]; j++)
                    {
                        if (mem_size[j] != size[k])
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                     "[Entry %d] Memory size mis-match: Expecting [%d] "
                                     "but given [%d]\n",
                                     k, mem_size[j], size[k]);

                            status = (vx_status)VX_FAILURE;
                        }

                        k++;
                    }
                }
            }
            else
            {
                for (i = 0; i < numMemElem; i++)
                {
                    if (mem_size[i] != size[i])
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                                 "[Entry %d] Memory size mis-match: Expecting [%d] "
                                 "but given [%d]\n",
                                 i, mem_size[i], size[i]);

                        status = (vx_status)VX_FAILURE;
                    }
                }
            }
        }

        if (status == (vx_status)VX_SUCCESS)
        {
            /* Issue a warning if the number of handles passed is more than
             * what is needed.
             */
            if (numMemElem < num_entries)
            {
                VX_PRINT(VX_ZONE_WARNING,
                         "The value 'num_entries' exceeds the number of "
                         "handles needed.\n");
            }

            /* Update the object. */
            if (ref->type == (vx_enum)VX_TYPE_PYRAMID)
            {
                k = 0;
                for (i = 0; i < num_levels; i++)
                {
                    mem_ptr  = mem_ptr_arr[i];
                    mem_size = mem_size_arr[i];

                    for (j = 0; j < num_planes[i]; j++)
                    {
                        mem_ptr[j].mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;

                        if (mem_ptr[j].host_ptr != (uint64_t)(uintptr_t)NULL)
                        {
                            VX_PRINT(VX_ZONE_INFO,
                                     "Non-NULL handle detected. Overwriting.\n");
                        }

                        mem_ptr[j].host_ptr   = (uint64_t)(uintptr_t)addr[k];
                        mem_ptr[j].shared_ptr = shared_ptr[k];

                        if (mem_ptr[j].host_ptr != (uint64_t)(uintptr_t)NULL)
                        {
                            void *hostPtr = (void*)(uintptr_t)mem_ptr[j].host_ptr;

                            /* Perform a cache write back. */
                            status = tivxMemBufferUnmap(hostPtr,
                                                        mem_size[j],
                                                        (vx_enum)TIVX_MEM_EXTERNAL,
                                                        (vx_enum)VX_WRITE_ONLY);

                            if (status != (vx_status)VX_SUCCESS)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferUnmap() failed.\n");
                            }
                        }

                        k++;

                    } /* for (j = 0; j < num_planes[i]; j++) */

                } /* for (i = 0; i < num_levels; i++) */
            }
            else
            {
                for (i = 0; i < numMemElem; i++)
                {
                    mem_ptr[i].mem_heap_region = (vx_enum)TIVX_MEM_EXTERNAL;

                    if (mem_ptr[i].host_ptr != (uint64_t)(uintptr_t)NULL)
                    {
                        VX_PRINT(VX_ZONE_INFO,
                                 "Non-NULL handle detected. Overwriting.\n");
                    }

                    mem_ptr[i].host_ptr   = (uint64_t)(uintptr_t)addr[i];
                    mem_ptr[i].shared_ptr = shared_ptr[i];

                    if (mem_ptr[i].host_ptr != (uint64_t)(uintptr_t)NULL)
                    {
                        void *hostPtr = (void*)(uintptr_t)mem_ptr[i].host_ptr;

                        /* Perform a cache write back. */
                        status = tivxMemBufferUnmap(hostPtr,
                                                    mem_size[i],
                                                    (vx_enum)TIVX_MEM_EXTERNAL,
                                                    (vx_enum)VX_WRITE_ONLY);

                        if (status != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "tivxMemBufferUnmap() failed.\n");
                        }
                    }
                }
            }
        }

        if((status == (vx_status)VX_SUCCESS) &&
           (ref->type == (vx_enum)TIVX_TYPE_RAW_IMAGE))
        {
            status = ownDeriveRawImageBufferPointers(ref);
        }
    }

    return status;
}

vx_status tivxReferenceExportHandle(const vx_reference ref, void *addr[], uint32_t size[], uint32_t max_entries, uint32_t *num_entries)
{
    tivx_shared_mem_ptr_t  *mem_ptr;
    volatile uint32_t      *mem_size = NULL;
    uint32_t                numMemElem;
    uint32_t                i,j;
    vx_status               status;

    status = (vx_status)VX_SUCCESS;

    /* Validate the arguments. */
    if ((ref == NULL) || (ownIsValidReference(ref) == (vx_bool)vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference is Invalid.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (addr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'addr' is NULL.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (size == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'size' is NULL.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (max_entries == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'max_entries' is 0.\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (num_entries == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "The parameter 'num_entries' is NULL.\n");
        status = (vx_status)VX_FAILURE;
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        numMemElem = 1;
        mem_ptr    = NULL;

        if (ref->type == (vx_enum)VX_TYPE_IMAGE)
        {
            tivx_obj_desc_image_t *obj_desc;

            obj_desc = (tivx_obj_desc_image_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else if (obj_desc->planes > max_entries)
            {
                /* Having more handles than needed is OK but not the
                 * other way.
                 */
                VX_PRINT(VX_ZONE_ERROR,
                         "max_entries [%d] less than num planes [%d].\n",
                         max_entries, obj_desc->planes);
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                numMemElem = obj_desc->planes;
                mem_ptr    = obj_desc->mem_ptr;
                mem_size   = obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_TENSOR)
        {
            tivx_obj_desc_tensor_t *obj_desc;

            obj_desc = (tivx_obj_desc_tensor_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_USER_DATA_OBJECT)
        {
            tivx_obj_desc_user_data_object_t *obj_desc;

            obj_desc = (tivx_obj_desc_user_data_object_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_ARRAY)
        {
            tivx_obj_desc_array_t *obj_desc;

            obj_desc = (tivx_obj_desc_array_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_CONVOLUTION)
        {
            tivx_obj_desc_convolution_t *obj_desc;

            obj_desc = (tivx_obj_desc_convolution_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_MATRIX)
        {
            tivx_obj_desc_matrix_t *obj_desc;

            obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_DISTRIBUTION)
        {
            tivx_obj_desc_distribution_t *obj_desc;

            obj_desc = (tivx_obj_desc_distribution_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                mem_ptr  = &obj_desc->mem_ptr;
                mem_size = &obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)TIVX_TYPE_RAW_IMAGE)
        {
            tivx_obj_desc_raw_image_t *obj_desc;

            obj_desc = (tivx_obj_desc_raw_image_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                numMemElem = obj_desc->params.num_exposures;
                mem_ptr    = obj_desc->mem_ptr;
                mem_size   = obj_desc->mem_size;
            }
        }
        else if (ref->type == (vx_enum)VX_TYPE_PYRAMID)
        {
            tivx_obj_desc_pyramid_t    *obj_desc;
            tivx_obj_desc_image_t      *img_obj_desc;
            vx_uint32                   num_levels;
            vx_pyramid                  pyramid;
            vx_reference                img_ref;
            vx_uint32                   num_planes;
            int32_t                     cnt;

            obj_desc = (tivx_obj_desc_pyramid_t *)ref->obj_desc;

            if (obj_desc == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR, "'obj_desc' is NULL.\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                num_levels = obj_desc->num_levels;
                pyramid    = (vx_pyramid)ref;
                num_planes = 0;
                cnt        = 0;
                numMemElem = 0;

                for (i = 0; i < num_levels; i++)
                {
                    img_ref      = (vx_reference)pyramid->img[i];
                    img_obj_desc = (tivx_obj_desc_image_t *)img_ref->obj_desc;

                    if (img_obj_desc == NULL)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "'img_obj_desc' is NULL.\n");
                        status = (vx_status)VX_FAILURE;
                        break;
                    }

                    num_planes += img_obj_desc->planes;

                    if (num_planes > max_entries)
                    {
                        /* Having more handles than needed is OK but not the
                         * other way.
                         */
                        VX_PRINT(VX_ZONE_ERROR,
                                 "max_entries [%d] less than num "
                                 "planes [%d].\n",
                                  max_entries, num_planes);
                        status = (vx_status)VX_FAILURE;
                        break;
                    }

                    numMemElem += img_obj_desc->planes;

                    for (j = 0; j < img_obj_desc->planes; j++)
                    {
                        addr[cnt] = (void *)
                            (uintptr_t)img_obj_desc->mem_ptr[j].host_ptr;
                        size[cnt] = img_obj_desc->mem_size[j];
                        cnt++;
                    }

                } /* for (i = 0; i < num_levels; i++) */

                /* Update the entry count. */
                *num_entries = numMemElem;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unsupported type [%d].\n", ref->type);
            status = (vx_status)VX_FAILURE;
        }

        if ((status == (vx_status)VX_SUCCESS) &&
            (ref->type != (vx_enum)VX_TYPE_PYRAMID))
        {
            for (i = 0; i < numMemElem; i++)
            {
                addr[i] = (void *)(uintptr_t)mem_ptr[i].host_ptr;
                size[i] = mem_size[i];
            }

            /* Update the entry count. */
            *num_entries = numMemElem;

        } /* if (status == (vx_status)VX_SUCCESS) */

    } /* if (status == (vx_status)VX_SUCCESS) */

    return status;
}

