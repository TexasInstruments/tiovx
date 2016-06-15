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

static const vx_char implementation[VX_MAX_IMPLEMENTATION_NAME] = "tiovx";

static const vx_char extensions[] = " ";

static vx_context single_context = NULL;

static tivx_mutex context_lock = NULL;

static tivx_context_t single_context_obj;

vx_bool ownAddReferenceToContext(vx_context context, vx_reference ref)
{
    uint32_t ref_idx;
    vx_bool is_success = vx_false_e;

    if (ownIsValidContext(context)==vx_true_e)
    {
        ownReferenceLock(&context->base);

        for(ref_idx=0; ref_idx < dimof(context->reftable); ref_idx++)
        {
            if(context->reftable[ref_idx]==NULL)
            {
                context->reftable[ref_idx] = ref;
                context->num_references++;
                is_success = vx_true_e;
                break;
            }
        }

        ownReferenceUnlock(&context->base);
    }
    return is_success;
}

vx_bool ownRemoveReferenceFromContext(vx_context context, vx_reference ref)
{
    uint32_t ref_idx;
    vx_bool is_success = vx_false_e;

    if (ownIsValidContext(context)==vx_true_e)
    {
        ownReferenceLock(&context->base);

        for(ref_idx=0; ref_idx < dimof(context->reftable); ref_idx++)
        {
            if(context->reftable[ref_idx]==ref)
            {
                context->reftable[ref_idx] = NULL;
                context->num_references--;
                is_success = vx_true_e;
                break;
            }
        }

        ownReferenceUnlock(&context->base);
    }
    return is_success;
}

vx_bool ownIsValidContext(vx_context context)
{
    vx_bool ret = vx_false_e;
    if ((context != NULL) &&
        (context->base.magic == TIVX_MAGIC) &&
        (context->base.type == VX_TYPE_CONTEXT) &&
        (context->base.context == NULL))
    {
        ret = vx_true_e; /* this is the top level context */
    }
    return ret;
}

static vx_bool ownIsValidBorderMode(vx_enum mode)
{
    vx_bool ret = vx_true_e;
    switch (mode)
    {
        case VX_BORDER_UNDEFINED:
        case VX_BORDER_CONSTANT:
        case VX_BORDER_REPLICATE:
            break;
        default:
            ret = vx_false_e;
            break;
    }
    return ret;
}

VX_API_ENTRY vx_context VX_API_CALL vxCreateContext()
{
    vx_context context = NULL;
    vx_status status = VX_SUCCESS;
    uint32_t idx;

    if (context_lock == NULL)
    {
        status = tivxMutexCreate(&context_lock);
    }

    if(status==VX_SUCCESS)
    {
        tivxMutexLock(context_lock);
        if (single_context == NULL)
        {
            context = &single_context_obj;

            if (context)
            {
                memset(context, 0, sizeof(tivx_context_t));

                context->imm_border.mode = VX_BORDER_UNDEFINED;
                context->imm_border_policy = VX_BORDER_POLICY_DEFAULT_TO_UNDEFINED;
                context->next_dynamic_user_kernel_id = 0;
                context->next_dynamic_user_library_id = 1;
                context->perf_enabled = vx_false_e;
                context->imm_target_enum = VX_TARGET_ANY;
                memset(context->imm_target_string, 0, sizeof(context->imm_target_string));
                context->num_references = 0;
                for(idx=0; idx<dimof(context->reftable); idx++)
                {
                    context->reftable[idx] = NULL;
                }
                for(idx=0; idx<dimof(context->user_structs); idx++)
                {
                    context->user_structs[idx].type = VX_TYPE_INVALID;
                }
                context->num_unique_kernels = 0;
                context->num_modules = 0;
                context->log_reentrant = vx_true_e;
                context->log_enabled = vx_false_e;

                ownInitReference(&context->base, NULL, VX_TYPE_CONTEXT, NULL);
                ownIncrementReference(&context->base, VX_EXTERNAL);
                ownCreateConstErrors(context);

                single_context = context;
            }
        }
        else
        {
            context = single_context;
            ownIncrementReference(&context->base, VX_EXTERNAL);
        }
        tivxMutexUnlock(context_lock);
    }
    return context;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseContext(vx_context *c)
{
    vx_status status = VX_SUCCESS;
    vx_context context = (c?*c:0);
    vx_uint32 r;

    if (c)
    {
        *c = 0;
    }
    tivxMutexLock(context_lock);
    if (ownIsValidContext(context) == vx_true_e)
    {
        if (ownDecrementReference(&context->base, VX_EXTERNAL) == 0)
        {
            /* Deregister any log callbacks if there is any registered */
            vxRegisterLogCallback(context, NULL, vx_false_e);

            /*! \internal Garbage Collect All References */
            /* Details:
             *   1. This loop will warn of references which have not been released by the user.
             *   2. It will close all internally opened error references.
             *   3. It will close the external references, which in turn will internally
             *      close any internally dependent references that they reference, assuming the
             *      reference counting has been done properly in the framework.
             *   4. This garbage collection must be done before the targets are released since some of
             *      these external references may have internal references to target kernels.
             */
            for (r = 0; r < dimof(context->reftable); r++)
            {
                vx_reference ref = context->reftable[r];

                /* Warnings should only come when users have not released all external references */
                if (ref && ref->external_count > 0) {
                    VX_PRINT(VX_ZONE_WARNING,"Stale reference "VX_FMT_REF" of type %08x at external count %u, internal count %u\n",
                             ref, ref->type, ref->external_count, ref->internal_count);
                }

                /* These were internally opened during creation, so should internally close ERRORs */
                if(ref && ref->type == VX_TYPE_ERROR) {
                    ownReleaseReferenceInt(&ref, ref->type, VX_INTERNAL, NULL);
                }

                /* Warning above so user can fix release external objects, but close here anyway */
                while (ref && ref->external_count > 1) {
                    ownDecrementReference(ref, VX_EXTERNAL);
                }
                if (ref && ref->external_count > 0) {
                    ownReleaseReferenceInt(&ref, ref->type, VX_EXTERNAL, NULL);
                }
            }

            /* By now, all external and internal references should be removed */
            for (r = 0; r < dimof(context->reftable); r++)
            {
                if(context->reftable[r])
                    VX_PRINT(VX_ZONE_ERROR,"Reference %d not removed\n", r);
            }

            /*! \internal wipe away the context memory first */
            /* Normally destroy sem is part of release reference, but can't for context */
            tivxMutexDelete(&context->base.lock);
            memset(context, 0, sizeof(tivx_context_t));

            single_context = NULL;
        }
        else
        {
            VX_PRINT(VX_ZONE_WARNING, "Context still has %u holders\n", ownTotalReferenceCount(&context->base));
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    tivxMutexUnlock(context_lock);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryContext(vx_context context, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (ownIsValidContext(context) == vx_false_e)
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_CONTEXT_VENDOR_ID:
                if (VX_CHECK_PARAM(ptr, size, vx_uint16, 0x1))
                {
                    *(vx_uint16 *)ptr = VX_ID_TI;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_VERSION:
                if (VX_CHECK_PARAM(ptr, size, vx_uint16, 0x1))
                {
                    *(vx_uint16 *)ptr = (vx_uint16)VX_VERSION;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_MODULES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = context->num_modules;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_REFERENCES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = context->num_references;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_IMPLEMENTATION:
                if (size <= VX_MAX_IMPLEMENTATION_NAME && ptr)
                {
                    strncpy(ptr, implementation, VX_MAX_IMPLEMENTATION_NAME);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_EXTENSIONS_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = sizeof(extensions);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_EXTENSIONS:
                if (size <= sizeof(extensions) && ptr)
                {
                    strncpy(ptr, extensions, sizeof(extensions));
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_CONVOLUTION_MAX_DIMENSION:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = TIVX_CONTEXT_MAX_CONVOLUTION_DIM;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_NONLINEAR_MAX_DIMENSION:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = TIVX_CONTEXT_MAX_NONLINEAR_DIM;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_OPTICAL_FLOW_MAX_WINDOW_DIMENSION:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = TIVX_CONTEXT_OPTICALFLOWPYRLK_MAX_DIM;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_IMMEDIATE_BORDER:
                if (VX_CHECK_PARAM(ptr, size, vx_border_t, 0x3))
                {
                    *(vx_border_t *)ptr = context->imm_border;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_IMMEDIATE_BORDER_POLICY:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = context->imm_border_policy;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_UNIQUE_KERNELS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = context->num_unique_kernels;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_UNIQUE_KERNEL_TABLE:
                if ((size == (context->num_unique_kernels * sizeof(vx_kernel_info_t))) &&
                    (ptr != NULL))
                {
                    /* TODO */
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

VX_API_ENTRY vx_status VX_API_CALL vxSetContextAttribute(vx_context context, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (ownIsValidContext(context) == vx_false_e)
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute) {
            case VX_CONTEXT_IMMEDIATE_BORDER:
                if (VX_CHECK_PARAM(ptr, size, vx_border_t, 0x3))
                {
                    vx_border_t *config = (vx_border_t *)ptr;
                    if (ownIsValidBorderMode(config->mode) == vx_false_e)
                    {
                        status = VX_ERROR_INVALID_VALUE;
                    }
                    else
                    {
                        context->imm_border = *config;
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


VX_API_ENTRY vx_status VX_API_CALL vxDirective(vx_reference reference, vx_enum directive) {
    vx_status status = VX_SUCCESS;
    vx_context context;
    vx_enum ref_type;

    status = vxQueryReference(reference, VX_REF_ATTRIBUTE_TYPE, &ref_type, sizeof(ref_type));
    if (status == VX_SUCCESS)
    {
        if (ref_type == VX_TYPE_CONTEXT)
        {
            context = (vx_context)reference;
        }
        else
        {
            context = reference->context;
        }
        if (ownIsValidContext(context) == vx_false_e)
        {
            status = VX_ERROR_INVALID_REFERENCE;
        }
        else
        {
            switch (directive)
            {
                case VX_DIRECTIVE_DISABLE_LOGGING:
                    context->log_enabled = vx_false_e;
                    break;
                case VX_DIRECTIVE_ENABLE_LOGGING:
                    context->log_enabled = vx_true_e;
                    break;
                case VX_DIRECTIVE_DISABLE_PERFORMANCE:
                    if (ref_type == VX_TYPE_CONTEXT)
                    {
                        context->perf_enabled = vx_false_e;
                    }
                    else
                    {
                        status = VX_ERROR_NOT_SUPPORTED;
                    }
                    break;
                case VX_DIRECTIVE_ENABLE_PERFORMANCE:
                    if (ref_type == VX_TYPE_CONTEXT)
                    {
                        context->perf_enabled = vx_true_e;
                    }
                    else
                    {
                        status = VX_ERROR_NOT_SUPPORTED;
                    }
                    break;
                default:
                    status = VX_ERROR_NOT_SUPPORTED;
                    break;
            }
        }
    }
    return status;
}

VX_API_ENTRY vx_enum VX_API_CALL vxRegisterUserStruct(vx_context context, vx_size size)
{
    vx_enum type = VX_TYPE_INVALID;
    vx_uint32 i = 0;

    if ((ownIsValidContext(context) == vx_true_e) &&
        (size != 0))
    {
        for (i = 0; i < TIVX_CONTEXT_MAX_USER_STRUCTS; ++i)
        {
            if (context->user_structs[i].type == VX_TYPE_INVALID)
            {
                context->user_structs[i].type = VX_TYPE_USER_STRUCT_START + i;
                context->user_structs[i].size = size;
                type = context->user_structs[i].type;
                break;
            }
        }
    }
    return type;
}

VX_API_ENTRY vx_status VX_API_CALL vxAllocateUserKernelId(vx_context context, vx_enum * pKernelEnumId)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if ((ownIsValidContext(context) == vx_true_e) && pKernelEnumId)
    {
        status = VX_ERROR_NO_RESOURCES;
        if(context->next_dynamic_user_kernel_id <= VX_KERNEL_MASK)
        {
            *pKernelEnumId = VX_KERNEL_BASE(VX_ID_USER,0) + context->next_dynamic_user_kernel_id++;
            status = VX_SUCCESS;
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAllocateUserKernelLibraryId(vx_context context, vx_enum * pLibraryId)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if ((ownIsValidContext(context) == vx_true_e) && pLibraryId)
    {
        status = VX_ERROR_NO_RESOURCES;
        if(context->next_dynamic_user_library_id <= VX_LIBRARY(VX_LIBRARY_MASK))
        {
            *pLibraryId = context->next_dynamic_user_library_id++;
            status = VX_SUCCESS;
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetImmediateModeTarget(vx_context context, vx_enum target_enum, const char* target_string)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (ownIsValidContext(context) == vx_true_e)
    {
        switch (target_enum)
        {
            case VX_TARGET_ANY:
                context->imm_target_enum = VX_TARGET_ANY;
                memset(context->imm_target_string, 0, sizeof(context->imm_target_string));
                status = VX_SUCCESS;
                break;

            case VX_TARGET_STRING:
                if (target_string != NULL)
                {
                    context->imm_target_enum = VX_TARGET_STRING;
                    strncpy(context->imm_target_string, target_string, sizeof(context->imm_target_string));
                    context->imm_target_string[sizeof(context->imm_target_string) - 1] = '\0';
                    status = VX_SUCCESS;
                }
                else /* target was not found */
                {
                    status = VX_ERROR_NOT_SUPPORTED;
                }
                break;

            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    return status;
}

