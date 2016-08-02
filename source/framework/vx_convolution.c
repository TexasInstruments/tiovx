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

static vx_status ownDestructConvolution(vx_reference ref);
static vx_status ownAllocConvolutionBuffer(vx_reference ref);
static vx_bool vxIsPowerOfTwo(vx_uint32 a);
static int isodd(size_t a);

vx_convolution VX_API_CALL vxCreateConvolution(
    vx_context context, vx_size columns, vx_size rows)
{
    vx_convolution cnvl = NULL;

    if(ownIsValidContext(context) == vx_true_e)
    {
        if ((isodd(columns)) && (columns >= 3) &&
            (columns < VX_CONTEXT_CONVOLUTION_MAX_DIMENSION) &&
            (isodd(rows)) && (rows >= 3) &&
            (rows < VX_CONTEXT_CONVOLUTION_MAX_DIMENSION))
        {
            cnvl = (vx_convolution)ownCreateReference(context,
                VX_TYPE_CONVOLUTION, VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)cnvl) == VX_SUCCESS) &&
                (cnvl->base.type == VX_TYPE_CONVOLUTION))
            {
                /* assign refernce type specific callback's */
                cnvl->base.destructor_callback = ownDestructConvolution;
                cnvl->base.mem_alloc_callback = ownAllocConvolutionBuffer;
                cnvl->base.release_callback =
                    (tivx_reference_release_callback_f)vxReleaseConvolution;

                cnvl->obj_desc = (tivx_obj_desc_convolution_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_CONVOLUTION);
                if(cnvl->obj_desc==NULL)
                {
                    vxReleaseConvolution(&cnvl);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate cnvl object descriptor\n");
                    cnvl = (vx_convolution)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    cnvl->obj_desc->columns = columns;
                    cnvl->obj_desc->rows = rows;
                    cnvl->obj_desc->scale = 1;
                    cnvl->obj_desc->mem_size = columns*rows*sizeof(vx_int16);
                    cnvl->obj_desc->mem_ptr.host_ptr = NULL;
                    cnvl->obj_desc->mem_ptr.shared_ptr = NULL;
                    cnvl->obj_desc->mem_ptr.mem_type = TIVX_MEM_EXTERNAL;
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

    if (ownIsValidSpecificReference(&cnvl->base, VX_TYPE_CONVOLUTION) == vx_false_e
        || (cnvl->obj_desc == NULL))
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_CONVOLUTION_SCALE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = cnvl->obj_desc->scale;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONVOLUTION_COLUMNS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = cnvl->obj_desc->columns;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONVOLUTION_ROWS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = cnvl->obj_desc->rows;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONVOLUTION_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr =
                        cnvl->obj_desc->mem_size;
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

VX_API_ENTRY vx_status VX_API_CALL vxSetConvolutionAttribute(
    vx_convolution cnvl, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&cnvl->base, VX_TYPE_CONVOLUTION) == vx_false_e
        || (cnvl->obj_desc == NULL))
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_CONVOLUTION_SCALE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    vx_uint32 scale = *(vx_uint32 *)ptr;
                    if (vxIsPowerOfTwo(scale) == vx_true_e)
                    {
                        cnvl->obj_desc->scale = scale;
                    }
                    else
                    {
                        status = VX_ERROR_INVALID_VALUE;
                    }
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                status = VX_ERROR_INVALID_PARAMETERS;
                break;
        }
        if (status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set attribute on convolution! (%d)\n", status);
        }
    }
    return status;
}

vx_status VX_API_CALL vxCopyConvolution(
    vx_convolution cnvl, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 size;

    if ((ownIsValidSpecificReference(&cnvl->base, VX_TYPE_CONVOLUTION) ==
            vx_false_e) || (cnvl->obj_desc == NULL))
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if ((VX_READ_ONLY == usage) &&
            (NULL == cnvl->obj_desc->mem_ptr.host_ptr))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == user_ptr)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        size = cnvl->obj_desc->mem_size;

        /* Copy from cnvl object to user memory */
        if (VX_READ_ONLY == usage)
        {
            tivxMemBufferMap(cnvl->obj_desc->mem_ptr.host_ptr, size,
                cnvl->obj_desc->mem_ptr.mem_type, VX_READ_ONLY);

            memcpy(user_ptr, cnvl->obj_desc->mem_ptr.host_ptr, size);

            tivxMemBufferUnmap(cnvl->obj_desc->mem_ptr.host_ptr, size,
                cnvl->obj_desc->mem_ptr.mem_type, VX_READ_ONLY);
        }
        else /* Copy from user memory to cnvl object */
        {
            status = ownAllocConvolutionBuffer(&cnvl->base);

            if (VX_SUCCESS == status)
            {
                tivxMemBufferMap(cnvl->obj_desc->mem_ptr.host_ptr, size,
                    cnvl->obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);

                memcpy(cnvl->obj_desc->mem_ptr.host_ptr, user_ptr, size);

                tivxMemBufferUnmap(cnvl->obj_desc->mem_ptr.host_ptr, size,
                    cnvl->obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);
            }
        }
    }

    return (status);
}


static vx_status ownAllocConvolutionBuffer(vx_reference ref)
{
    vx_status status = VX_SUCCESS;
    vx_convolution cnvl = (vx_convolution)ref;

    if(cnvl->base.type == VX_TYPE_CONVOLUTION)
    {
        if(cnvl->obj_desc != NULL)
        {
            /* memory is not allocated, so allocate it */
            if(cnvl->obj_desc->mem_ptr.host_ptr == NULL)
            {
                tivxMemBufferAlloc(
                    &cnvl->obj_desc->mem_ptr, cnvl->obj_desc->mem_size,
                    TIVX_MEM_EXTERNAL);

                if(cnvl->obj_desc->mem_ptr.host_ptr==NULL)
                {
                    /* could not allocate memory */
                    status = VX_ERROR_NO_MEMORY ;
                }
            }
        }
        else
        {
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructConvolution(vx_reference ref)
{
    vx_convolution cnvl = (vx_convolution)ref;

    if(cnvl->base.type == VX_TYPE_CONVOLUTION)
    {
        if(cnvl->obj_desc!=NULL)
        {
            if(cnvl->obj_desc->mem_ptr.host_ptr!=NULL)
            {
                tivxMemBufferFree(
                    &cnvl->obj_desc->mem_ptr, cnvl->obj_desc->mem_size);
            }

            tivxObjDescFree((tivx_obj_desc_t**)&cnvl->obj_desc);
        }
    }
    return VX_SUCCESS;
}

static vx_bool vxIsPowerOfTwo(vx_uint32 a)
{
    if (a == 0)
        return vx_false_e;
    else if ((a & ((a) - 1)) == 0)
        return vx_true_e;
    else
        return vx_false_e;
}

static int isodd(size_t a)
{
    return (int)(a & 1);
}

