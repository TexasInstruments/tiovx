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

static vx_status ownDestructScalar(vx_reference ref)
{
    vx_scalar scalar = (vx_scalar)ref;

    if(scalar->base.type == VX_TYPE_SCALAR)
    {
        if(scalar->obj_desc!=NULL)
        {
            tivxObjDescFree((tivx_obj_desc_t**)&scalar->obj_desc);
        }
    }
    return VX_SUCCESS;
}

static vx_status ownScalarToHostMem(vx_scalar scalar, void* user_ptr)
{
    vx_status status = VX_SUCCESS;

    if (VX_SUCCESS != ownReferenceLock(&scalar->base))
    {
        status = VX_ERROR_NO_RESOURCES;
    }
    else
    {
        switch (scalar->obj_desc->data_type)
        {
            case VX_TYPE_CHAR:     *(vx_char*)user_ptr = scalar->obj_desc->data.chr; break;
            case VX_TYPE_INT8:     *(vx_int8*)user_ptr = scalar->obj_desc->data.s08; break;
            case VX_TYPE_UINT8:    *(vx_uint8*)user_ptr = scalar->obj_desc->data.u08; break;
            case VX_TYPE_INT16:    *(vx_int16*)user_ptr = scalar->obj_desc->data.s16; break;
            case VX_TYPE_UINT16:   *(vx_uint16*)user_ptr = scalar->obj_desc->data.u16; break;
            case VX_TYPE_INT32:    *(vx_int32*)user_ptr = scalar->obj_desc->data.s32; break;
            case VX_TYPE_UINT32:   *(vx_uint32*)user_ptr = scalar->obj_desc->data.u32; break;
            case VX_TYPE_INT64:    *(vx_int64*)user_ptr = scalar->obj_desc->data.s64; break;
            case VX_TYPE_UINT64:   *(vx_uint64*)user_ptr = scalar->obj_desc->data.u64; break;
        #if OVX_SUPPORT_HALF_FLOAT
            case VX_TYPE_FLOAT16:  *(vx_float16*)ptr = scalar->obj_desc->data.f16; break;
        #endif
            case VX_TYPE_FLOAT32:  *(vx_float32*)user_ptr = scalar->obj_desc->data.f32; break;
            case VX_TYPE_FLOAT64:  *(vx_float64*)user_ptr = scalar->obj_desc->data.f64; break;
            case VX_TYPE_DF_IMAGE: *(vx_df_image*)user_ptr = scalar->obj_desc->data.fcc; break;
            case VX_TYPE_ENUM:     *(vx_enum*)user_ptr = scalar->obj_desc->data.enm; break;
            case VX_TYPE_SIZE:     *(vx_size*)user_ptr = scalar->obj_desc->data.size; break;
            case VX_TYPE_BOOL:     *(vx_bool*)user_ptr = scalar->obj_desc->data.boolean; break;

            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
        if (VX_SUCCESS != ownReferenceUnlock(&scalar->base))
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }

    return status;
}

static vx_status ownHostMemToScalar(vx_scalar scalar, void* user_ptr)
{
    vx_status status = VX_SUCCESS;

    if (VX_SUCCESS != ownReferenceLock(&scalar->base))
    {
        status = VX_ERROR_NO_RESOURCES;
    }
    else
    {
        switch (scalar->obj_desc->data_type)
        {
            case VX_TYPE_CHAR:     scalar->obj_desc->data.chr = *(vx_char*)user_ptr; break;
            case VX_TYPE_INT8:     scalar->obj_desc->data.s08 = *(vx_int8*)user_ptr; break;
            case VX_TYPE_UINT8:    scalar->obj_desc->data.u08 = *(vx_uint8*)user_ptr; break;
            case VX_TYPE_INT16:    scalar->obj_desc->data.s16 = *(vx_int16*)user_ptr; break;
            case VX_TYPE_UINT16:   scalar->obj_desc->data.u16 = *(vx_uint16*)user_ptr; break;
            case VX_TYPE_INT32:    scalar->obj_desc->data.s32 = *(vx_int32*)user_ptr; break;
            case VX_TYPE_UINT32:   scalar->obj_desc->data.u32 = *(vx_uint32*)user_ptr; break;
            case VX_TYPE_INT64:    scalar->obj_desc->data.s64 = *(vx_int64*)user_ptr; break;
            case VX_TYPE_UINT64:   scalar->obj_desc->data.u64 = *(vx_uint64*)user_ptr; break;
        #if OVX_SUPPORT_HALF_FLOAT
            case VX_TYPE_FLOAT16:  scalar->obj_desc->data.f16 = *(vx_float16*)user_ptr; break;
        #endif
            case VX_TYPE_FLOAT32:  scalar->obj_desc->data.f32 = *(vx_float32*)user_ptr; break;
            case VX_TYPE_FLOAT64:  scalar->obj_desc->data.f64 = *(vx_float64*)user_ptr; break;
            case VX_TYPE_DF_IMAGE: scalar->obj_desc->data.fcc = *(vx_df_image*)user_ptr; break;
            case VX_TYPE_ENUM:     scalar->obj_desc->data.enm = *(vx_enum*)user_ptr; break;
            case VX_TYPE_SIZE:     scalar->obj_desc->data.size = *(vx_size*)user_ptr; break;
            case VX_TYPE_BOOL:     scalar->obj_desc->data.boolean = *(vx_bool*)user_ptr; break;

            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
        if (VX_SUCCESS != ownReferenceUnlock(&scalar->base))
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }

    return status;
} /* own_host_mem_to_scalar() */

void ownPrintScalar(vx_scalar scalar)
{
    ownPrintReference(&scalar->base);
    switch (scalar->obj_desc->data_type)
    {
        case VX_TYPE_CHAR:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %c\n", scalar, scalar->obj_desc->data.chr);
            break;
        case VX_TYPE_INT8:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %d\n", scalar, scalar->obj_desc->data.s08);
            break;
        case VX_TYPE_UINT8:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %u\n", scalar, scalar->obj_desc->data.u08);
            break;
        case VX_TYPE_INT16:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %hd\n", scalar, scalar->obj_desc->data.s16);
            break;
        case VX_TYPE_UINT16:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %hu\n", scalar, scalar->obj_desc->data.u16);
            break;
        case VX_TYPE_INT32:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %d\n", scalar, scalar->obj_desc->data.s32);
            break;
        case VX_TYPE_UINT32:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %u\n", scalar, scalar->obj_desc->data.u32);
            break;
        case VX_TYPE_INT64:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %ld\n", scalar, scalar->obj_desc->data.s64);
            break;
        case VX_TYPE_UINT64:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %lu\n", scalar, scalar->obj_desc->data.u64);
            break;
        case VX_TYPE_FLOAT32:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %lf\n", scalar, scalar->obj_desc->data.f32);
            break;
        case VX_TYPE_FLOAT64:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %lf\n", scalar, scalar->obj_desc->data.f64);
            break;
        case VX_TYPE_DF_IMAGE:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %08x\n", scalar, scalar->obj_desc->data.fcc);
            break;
        case VX_TYPE_ENUM:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %d\n", scalar, scalar->obj_desc->data.enm);
            break;
        case VX_TYPE_SIZE:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %zu\n", scalar, scalar->obj_desc->data.size);
            break;
        case VX_TYPE_BOOL:
            VX_PRINT(VX_ZONE_SCALAR, "scalar "VX_FMT_REF" = %s\n", scalar, (scalar->obj_desc->data.boolean == vx_true_e?"TRUE":"FALSE"));
            break;
        default:
            VX_PRINT(VX_ZONE_ERROR, "Case %08x is not covered!\n", scalar->obj_desc->data_type);
            break;
    }

    return;
} /* vxPrintScalar() */


VX_API_ENTRY vx_scalar VX_API_CALL vxCreateScalar(vx_context context, vx_enum data_type, const void* ptr)
{
    vx_scalar scalar = NULL;

    if (ownIsValidContext(context) == vx_true_e)
    {
        if (!TIVX_TYPE_IS_SCALAR(data_type))
        {
            vxAddLogEntry(&context->base, VX_ERROR_INVALID_TYPE, "Invalid type to scalar\n");
            scalar = (vx_scalar)ownGetErrorObject(context, VX_ERROR_INVALID_TYPE);
        }
        else
        {
            scalar = (vx_scalar)ownCreateReference(context, VX_TYPE_SCALAR, VX_EXTERNAL, &context->base);
            if (vxGetStatus((vx_reference)scalar) == VX_SUCCESS && scalar->base.type == VX_TYPE_SCALAR)
            {
                /* assign refernce type specific callback's */
                scalar->base.destructor_callback = ownDestructScalar;
                scalar->base.release_callback = (tivx_reference_release_callback_f)vxReleaseScalar;

                scalar->obj_desc = (tivx_obj_desc_scalar_t*)tivxObjDescAlloc(TIVX_OBJ_DESC_SCALAR);
                if(scalar->obj_desc==NULL)
                {
                    vxReleaseScalar(&scalar);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES, "Could not allocate scalar object descriptor\n");
                    scalar = (vx_scalar)ownGetErrorObject(context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    scalar->obj_desc->data_type = data_type;
                    vxCopyScalar(scalar, (void*)ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
                }
            }
        }
    }
    return (vx_scalar)scalar;
} /* vxCreateScalar() */

VX_API_ENTRY vx_status VX_API_CALL vxReleaseScalar(vx_scalar *s)
{
    return ownReleaseReferenceInt((vx_reference *)s, VX_TYPE_SCALAR, VX_EXTERNAL, NULL);
} /* vxReleaseScalar() */

VX_API_ENTRY vx_status VX_API_CALL vxQueryScalar(vx_scalar scalar, vx_enum attribute, void* ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    vx_scalar pscalar = (vx_scalar)scalar;

    if (ownIsValidSpecificReference(&pscalar->base,VX_TYPE_SCALAR) == vx_false_e)
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_SCALAR_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum*)ptr = pscalar->obj_desc->data_type;
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
} /* vxQueryScalar() */

VX_API_ENTRY vx_status VX_API_CALL vxCopyScalar(vx_scalar scalar, void* user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;

    if (vx_false_e == ownIsValidSpecificReference(&scalar->base, VX_TYPE_SCALAR))
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (NULL == user_ptr || VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            switch (usage)
            {
                case VX_READ_ONLY:
                    status = ownScalarToHostMem(scalar, user_ptr);
                    break;
                case VX_WRITE_ONLY:
                    status = ownHostMemToScalar(scalar, user_ptr);
                    break;
                default:
                    status = VX_ERROR_INVALID_PARAMETERS;
                    break;
            }
        }
    }
    return status;
} /* vxCopyScalar() */
