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

static vx_status ownDestructConvolution(vx_reference ref);
static vx_status ownAllocConvolutionBuffer(vx_reference ref);
static vx_bool vxIsPowerOfTwo(vx_uint32 a);
static int8_t isodd(size_t a);

vx_convolution VX_API_CALL vxCreateConvolution(
    vx_context context, vx_size columns, vx_size rows)
{
    vx_convolution cnvl = NULL;
    tivx_obj_desc_convolution_t *obj_desc = NULL;

    if(ownIsValidContext(context) == vx_true_e)
    {
        if ((isodd(columns) == 1) && (columns >= 3) &&
            (columns < VX_CONTEXT_CONVOLUTION_MAX_DIMENSION) &&
            (isodd(rows) == 1) && (rows >= 3) &&
            (rows < VX_CONTEXT_CONVOLUTION_MAX_DIMENSION))
        {
            cnvl = (vx_convolution)ownCreateReference(context,
                VX_TYPE_CONVOLUTION, VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)cnvl) == VX_SUCCESS) &&
                (cnvl->base.type == VX_TYPE_CONVOLUTION))
            {
                /* assign refernce type specific callback's */
                cnvl->base.destructor_callback = &ownDestructConvolution;
                cnvl->base.mem_alloc_callback = &ownAllocConvolutionBuffer;
                cnvl->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseConvolution;

                obj_desc = (tivx_obj_desc_convolution_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_CONVOLUTION, (vx_reference)cnvl);
                if(obj_desc==NULL)
                {
                    vxReleaseConvolution(&cnvl);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate cnvl object descriptor\n");
                    cnvl = (vx_convolution)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    obj_desc->columns = columns;
                    obj_desc->rows = rows;
                    obj_desc->scale = 1;
                    obj_desc->mem_size = columns*rows*sizeof(vx_int16);
                    obj_desc->mem_ptr.host_ptr = (uint64_t)NULL;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)NULL;
                    obj_desc->mem_ptr.mem_heap_region = TIVX_MEM_EXTERNAL;
                    cnvl->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                }
            }
        }
    }

    return (cnvl);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseConvolution(vx_convolution *cnvl)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)cnvl, VX_TYPE_CONVOLUTION, VX_EXTERNAL, NULL));
}

vx_status VX_API_CALL vxQueryConvolution(
    vx_convolution cnvl, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_convolution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(&cnvl->base, VX_TYPE_CONVOLUTION))
    {
        obj_desc = (tivx_obj_desc_convolution_t *)cnvl->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryConvolution: Invalid Object Descriptor! \n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    if (VX_SUCCESS == status)
    {
        status = ownAllocConvolutionBuffer(&cnvl->base);

        if (VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "vxQueryConvolution: Could not allocate memory! \n");
        }
    }

    if (VX_SUCCESS == status)
    {
        switch (attribute)
        {
            case VX_CONVOLUTION_SCALE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->scale;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryConvolution: scale query failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONVOLUTION_COLUMNS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->columns;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryConvolution: columns query failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONVOLUTION_ROWS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->rows;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryConvolution: rows query failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONVOLUTION_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr =
                        obj_desc->mem_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryConvolution: size query failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxQueryConvolution: invalid attribute \n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetConvolutionAttribute(
    vx_convolution cnvl, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_convolution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(&cnvl->base, VX_TYPE_CONVOLUTION))
    {
        obj_desc = (tivx_obj_desc_convolution_t *)cnvl->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetConvolutionAttribute: Invalid Object Descriptor! \n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    if (VX_SUCCESS == status)
    {
        status = ownAllocConvolutionBuffer(&cnvl->base);
    }

    if (VX_SUCCESS == status)
    {
        switch (attribute)
        {
            case VX_CONVOLUTION_SCALE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    vx_uint32 scale = *(vx_uint32 *)ptr;
                    if (vxIsPowerOfTwo(scale) == vx_true_e)
                    {
                        obj_desc->scale = scale;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "vxSetConvolutionAttribute: scale value is not a power of two \n");
                        status = VX_ERROR_INVALID_VALUE;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxSetConvolutionAttribute: set convolution scale failed \n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxSetConvolutionAttribute: invalid attribute \n");
                status = VX_ERROR_INVALID_PARAMETERS;
                break;
        }
    }
    return status;
}

vx_status VX_API_CALL vxCopyConvolutionCoefficients(
    vx_convolution cnvl, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 size;
    tivx_obj_desc_convolution_t *obj_desc = NULL;

    if (ownIsValidSpecificReference(&cnvl->base, VX_TYPE_CONVOLUTION))
    {
        obj_desc = (tivx_obj_desc_convolution_t *)cnvl->base.obj_desc;
    }

    if (obj_desc == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCopyConvolutionCoefficients: Invalid Object Descriptor! \n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyConvolutionCoefficients: user_mem_type is not VX_MEMORY_TYPE_HOST\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if ((VX_READ_ONLY == usage) &&
            ((uint64_t)NULL == obj_desc->mem_ptr.host_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyConvolutionCoefficients: Memory still not allocated\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == user_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyConvolutionCoefficients: user pointer is NULL\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        size = obj_desc->mem_size;

        /* Copy from cnvl object to user memory */
        if (VX_READ_ONLY == usage)
        {
            tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            memcpy(user_ptr, (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size);

            tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        else /* Copy from user memory to cnvl object */
        {
            status = ownAllocConvolutionBuffer(&cnvl->base);

            if (VX_SUCCESS == status)
            {
                tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

                memcpy((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, user_ptr, size);

                tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "vxCopyConvolutionCoefficients: Convolution allocation failed\n");
            }
        }
    }

    return (status);
}


static vx_status ownAllocConvolutionBuffer(vx_reference ref)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_convolution_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_CONVOLUTION)
    {
        obj_desc = (tivx_obj_desc_convolution_t *)ref->obj_desc;

        if(obj_desc != NULL)
        {
            /* memory is not allocated, so allocate it */
            if(obj_desc->mem_ptr.host_ptr == (uint64_t)NULL)
            {
                tivxMemBufferAlloc(
                    &obj_desc->mem_ptr, obj_desc->mem_size,
                    TIVX_MEM_EXTERNAL);

                if(obj_desc->mem_ptr.host_ptr==(uint64_t)NULL)
                {
                    /* could not allocate memory */
                    status = VX_ERROR_NO_MEMORY;
                    VX_PRINT(VX_ZONE_ERROR, "ownAllocConvolutionBuffer: could not allocate memory\n");
                }
                else
                {
                    obj_desc->mem_ptr.shared_ptr =
                        tivxMemHost2SharedPtr(
                            obj_desc->mem_ptr.host_ptr,
                            TIVX_MEM_EXTERNAL);
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "ownAllocConvolutionBuffer: convolution object descriptor is null\n");
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownAllocConvolutionBuffer: reference type is not a convolution\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructConvolution(vx_reference ref)
{
    tivx_obj_desc_convolution_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_CONVOLUTION)
    {
        obj_desc = (tivx_obj_desc_convolution_t *)ref->obj_desc;

        if(obj_desc!=NULL)
        {
            if(obj_desc->mem_ptr.host_ptr!=(uint64_t)NULL)
            {
                tivxMemBufferFree(
                    &obj_desc->mem_ptr, obj_desc->mem_size);
            }

            tivxObjDescFree((tivx_obj_desc_t**)&obj_desc);
        }
    }
    return VX_SUCCESS;
}

static vx_bool vxIsPowerOfTwo(vx_uint32 a)
{
    vx_bool isPowTwo;
    if (a == 0)
    {
        isPowTwo =vx_false_e;
    }
    else if ((a & ((a) - 1)) == 0)
    {
        isPowTwo = vx_true_e;
    }
    else
    {
        isPowTwo = vx_false_e;
    }

    return (isPowTwo);
}

static int8_t isodd(size_t a)
{
    return (int8_t)(a & 1U);
}

