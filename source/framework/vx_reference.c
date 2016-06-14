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

static vx_status ownInitReference(vx_reference ref, vx_context context, vx_enum type, vx_reference scope)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    if (ref)
    {
        ref->magic = TIVX_MAGIC;
        ref->type = type;
        ref->context = context;
        ref->scope = scope;
        ref->internal_count = 0;
        ref->external_count = 0;
        ref->mem_alloc_callback = NULL;
        ref->destructor_callback = NULL;
        status = tivxMutexCreate(&ref->lock);
    }

    return status;
}


static vx_uint32 ownDecrementReference(vx_reference ref, vx_enum reftype)
{
    vx_uint32 result = UINT32_MAX;
    if (ref)
    {
        tivxMutexLock(ref->lock);
        if (reftype == VX_INTERNAL || reftype == VX_BOTH)
        {
            if (ref->internal_count == 0)
            {
                VX_PRINT(VX_ZONE_WARNING, "#### INTERNAL REF COUNT IS ALREADY ZERO!!! "VX_FMT_REF" type:%08x #####\n", ref, ref->type);
            }
            else
            {
                ref->internal_count--;
            }
        }
        if (reftype == VX_EXTERNAL || reftype == VX_BOTH)
        {
            if (ref->external_count == 0)
            {
                VX_PRINT(VX_ZONE_WARNING, "#### EXTERNAL REF COUNT IS ALREADY ZERO!!! "VX_FMT_REF" type:%08x #####\n", ref, ref->type);
            }
            else
            {
                ref->external_count--;
            }
        }
        result = ref->internal_count + ref->external_count;
        tivxMutexUnlock(ref->lock);
    }
    return result;
}


static vx_bool ownIsValidReference(vx_reference ref)
{
    vx_bool ret = vx_false_e;
    if (ref != NULL)
    {
        if ( (ref->magic == TIVX_MAGIC) &&
             (ownIsValidType(ref->type) == vx_true_e) &&
             (( (ref->type != VX_TYPE_CONTEXT) && (ownIsValidContext(ref->context) == vx_true_e) ) ||
              ( (ref->type == VX_TYPE_CONTEXT) && (ref->context == NULL) )) )
        {
            ret = vx_true_e;
        }
        else if (ref->magic == TIVX_BAD_MAGIC)
        {
            VX_PRINT(VX_ZONE_ERROR, "Reference has already been released and garbage collected!\n");
        }
        else if (ref->type != VX_TYPE_CONTEXT)
        {
            VX_PRINT(VX_ZONE_ERROR, "Not a valid reference!\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference was NULL\n");
    }
    return ret;
}

vx_uint32 ownTotalReferenceCount(vx_reference ref)
{
    vx_uint32 count = 0;
    if (ref)
    {
        tivxMutexLock(ref->lock);
        count = ref->external_count + ref->internal_count;
        tivxMutexUnlock(ref->lock);
    }
    return count;
}

vx_uint32 ownIncrementReference(vx_reference ref, vx_enum reftype)
{
    vx_uint32 count = 0u;
    if (ref)
    {
        tivxMutexLock(ref->lock);
        if (reftype == VX_EXTERNAL || reftype == VX_BOTH)
            ref->external_count++;
        if (reftype == VX_INTERNAL || reftype == VX_BOTH)
            ref->internal_count++;
        count = ref->internal_count + ref->external_count;
        tivxMutexUnlock(ref->lock);
    }
    return count;
}

vx_status ownReleaseReferenceInt(vx_reference *pref,
                        vx_enum type,
                        vx_enum reftype,
                        tivx_reference_callback_f special_destructor)
{
    vx_status status = VX_SUCCESS;
    vx_reference ref = (pref ? *pref : NULL);
    if (ref && ownIsValidSpecificReference(ref, type) == vx_true_e)
    {
        if (ownDecrementReference(ref, reftype) == 0)
        {
            tivx_reference_callback_f destructor = special_destructor;

            if (ownRemoveReferenceFromContext(ref->context, ref) == vx_false_e)
            {
                status = VX_ERROR_INVALID_REFERENCE;
            }
            else
            {
                /* find the destructor method */
                if (destructor==NULL)
                {
                    destructor = ref->destructor_callback;
                }

                /* if there is a destructor, call it. */
                if (destructor)
                {
                    destructor(ref);
                }

                tivxMutexDelete(&ref->lock);
                ref->magic = TIVX_BAD_MAGIC; /* make sure no existing copies of refs can use ref again */

                ownReferenceFree(ref);
            }
        }
        *pref = NULL;
    } else {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

vx_reference ownCreateReference(vx_context context, vx_enum type, vx_enum reftype, vx_reference scope)
{
    vx_reference ref = (vx_reference)ownReferenceAlloc(type);
    vx_status status = VX_SUCCESS;

    if (ref)
    {
        status = ownInitReference(ref, context, type, scope);
        if(status==VX_SUCCESS)
        {
            ownIncrementReference(ref, reftype);
            if (ownAddReferenceToContext(context, ref) == vx_false_e)
            {
                status = VX_ERROR_NO_RESOURCES;
            }
        }

        if(status!=VX_SUCCESS)
        {
            ownReferenceFree(ref);
            vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES, "Failed to add to resources table\n");
            ref = (vx_reference)ownGetErrorObject(context, VX_ERROR_NO_RESOURCES);
        }
    }
    else
    {
        vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES, "Failed to allocate reference object\n");
        ref = (vx_reference)ownGetErrorObject(context, VX_ERROR_NO_RESOURCES);
    }
    return ref;
}



vx_bool ownIsValidSpecificReference(vx_reference ref, vx_enum type)
{
    vx_bool ret = vx_false_e;
    if (ref != NULL)
    {
        if ((ref->magic == TIVX_MAGIC) &&
            (ref->type == type) &&
            (ownIsValidContext(ref->context) == vx_true_e))
        {
            ret = vx_true_e;
        }
        else if (ref->type != VX_TYPE_CONTEXT)
        {
            VX_PRINT(VX_ZONE_ERROR, "Not a valid reference!\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_WARNING, "Reference was NULL\n");
    }
    return ret;
}


void ownPrintReference(vx_reference ref)
{
    if (ref)
    {
        VX_PRINT(VX_ZONE_REFERENCE, "vx_reference: magic:%08x type:%08x count:[%u,%u]\n", ref, ref->magic, ref->type, ref->external_count, ref->internal_count);
    }
}




vx_bool ownIsValidType(vx_enum type)
{
    vx_bool ret = vx_false_e;
    if (type <= VX_TYPE_INVALID)
    {
        ret = vx_false_e;
    }
    else if (TIVX_TYPE_IS_SCALAR(type)) /* some scalar */
    {
        ret = vx_true_e;
    }
    else if (TIVX_TYPE_IS_STRUCT(type)) /* some struct */
    {
        ret = vx_true_e;
    }
    else if (TIVX_TYPE_IS_OBJECT(type)) /* some object */
    {
        ret = vx_true_e;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Type 0x%08x is invalid!\n");
    }
    return ret; /* otherwise, not a valid type */
}

vx_status ownReferenceLock(vx_reference ref)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    if(ref)
    {
        status = tivxMutexLock(ref->lock);
    }

    return status;
}

vx_status ownReferenceUnlock(vx_reference ref)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    if(ref)
    {
        status = tivxMutexLock(ref->lock);
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryReference(vx_reference ref, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    /* if it is not a reference and not a context */
    if ((ownIsValidReference(ref) == vx_false_e) &&
        (ownIsValidContext((vx_context)ref) == vx_false_e)) {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_REF_ATTRIBUTE_COUNT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = ref->external_count;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_REF_ATTRIBUTE_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = ref->type;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_REF_ATTRIBUTE_NAME:
                if (VX_CHECK_PARAM(ptr, size, vx_char*, 0x3))
                {
                    *(vx_char**)ptr = &ref->name[0];
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

VX_API_ENTRY vx_status VX_API_CALL vxSetReferenceName(vx_reference ref, const vx_char *name)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (ownIsValidReference(ref))
    {
        snprintf(ref->name, VX_MAX_REFERENCE_NAME, name);
        status = VX_SUCCESS;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseReference(vx_reference* ref_ptr)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    vx_reference ref = (ref_ptr ? *ref_ptr : NULL);
    if (ownIsValidReference(ref) == vx_true_e)
    {
        if(ref->release_callback)
        {
            status = ref->release_callback(ref_ptr);
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRetainReference(vx_reference ref)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidReference(ref) == vx_true_e)
    {
        ownIncrementReference(ref, VX_EXTERNAL);
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}
