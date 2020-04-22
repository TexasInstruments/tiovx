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

    if ((NULL != ref) &&
        (ownIsValidSpecificReference(ref, type) == (vx_bool)vx_true_e))
    {
        if (ownDecrementReference(ref, reftype) == 0U)
        {
            tivx_reference_callback_f destructor = special_destructor;

            if (ownRemoveReferenceFromContext(ref->context, ref) == (vx_bool)vx_false_e)
            {
                VX_PRINT(VX_ZONE_ERROR,"ownReleaseReferenceInt: Invalid reference\n");
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
                    destructor(ref);
                }

                if(ref->lock != NULL)
                {
                    tivxMutexDelete(&ref->lock);
                }
                ref->magic = TIVX_BAD_MAGIC; /* make sure no existing copies of refs can use ref again */

                tivxObjectFree(ref);
            }
        }
        *pref = NULL;
    } else {
        VX_PRINT(VX_ZONE_ERROR,"ownReleaseReferenceInt: Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

vx_reference ownCreateReference(vx_context context, vx_enum type, vx_enum reftype, vx_reference scope)
{
    vx_reference ref = (vx_reference)tivxObjectAlloc(type);
    vx_status status = (vx_status)VX_SUCCESS;

    if (ref != NULL)
    {
        status = ownInitReference(ref, context, type, scope);
        if(status==(vx_status)VX_SUCCESS)
        {
            ownIncrementReference(ref, reftype);
            if (ownAddReferenceToContext(context, ref) == (vx_bool)vx_false_e)
            {
                VX_PRINT(VX_ZONE_ERROR,"ownCreateReference: Add reference to context failed\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
        }

        if(status!=(vx_status)VX_SUCCESS)
        {
            tivxObjectFree(ref);
            vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES, "Failed to add to resources table\n");
            VX_PRINT(VX_ZONE_ERROR,"ownCreateReference: Failed to add to resources table\n");
            ref = (vx_reference)ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
        }
    }
    else
    {
        vxAddLogEntry(&context->base, (vx_status)VX_ERROR_NO_RESOURCES, "Failed to allocate reference object\n");
        VX_PRINT(VX_ZONE_ERROR,"ownCreateReference: Failed to allocate reference object\n");
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
        VX_PRINT(VX_ZONE_ERROR,"ownReferenceAllocMem: Invalid reference\n");
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

vx_reference ownReferenceGetHandleFromObjDescId(uint16_t obj_desc_id)
{
    tivx_obj_desc_t *obj_desc = tivxObjDescGet(obj_desc_id);
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

        VX_PRINT(VX_ZONE_ERROR,"tivxSetReferenceAttribute: Invalid reference\n");
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
                    VX_PRINT(VX_ZONE_ERROR, "tivxSetReferenceAttribute: error setting reference timestamp\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"tivxSetReferenceAttribute: Invalid attribute\n");
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
        VX_PRINT(VX_ZONE_ERROR,"vxQueryReference: Invalid reference\n");
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
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryReference: Query reference count failed\n");
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
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryReference: Query reference type failed\n");
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
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryReference: Query reference name failed\n");
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
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryReference: Query reference timestamp failed\n");
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
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryReference: Query reference invalid flag failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"vxQueryReference: Invalid attribute\n");
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
        VX_PRINT(VX_ZONE_ERROR,"vxSetReferenceName: Invalid reference\n");
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
        VX_PRINT(VX_ZONE_ERROR,"vxRetainReference: Invalid reference\n");
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
        VX_PRINT(VX_ZONE_ERROR,"vxGetStatus: Reference is NULL\n");
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
        VX_PRINT(VX_ZONE_ERROR,"vxHint: Invalid reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"vxHint: Hint not supported\n");
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
    tivx_obj_desc_t *obj_desc =
        (tivx_obj_desc_t *)child_ref->obj_desc;

    if ( (ownIsValidReference(child_ref) == (vx_bool)vx_true_e) &&
         (obj_desc != NULL) )
    {
        if ((vx_bool)vx_true_e == child_ref->is_array_element)
        {
            ref = ownReferenceGetHandleFromObjDescId(obj_desc->scope_obj_desc_id);
        }
    }

    return ref;
}

