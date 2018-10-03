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

static vx_status ownDestructMatrix(vx_reference ref);
static vx_status ownAllocMatrixBuffer(vx_reference ref);

vx_matrix VX_API_CALL vxCreateMatrix(
    vx_context context, vx_enum data_type, vx_size columns, vx_size rows)
{
    vx_matrix matrix = NULL;
    vx_size dim = 0ul;
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    if(ownIsValidContext(context) == vx_true_e)
    {
        if ((data_type == VX_TYPE_INT8) || (data_type == VX_TYPE_UINT8))
        {
            dim = sizeof(vx_uint8);
        }
        else if ((data_type == VX_TYPE_INT16) ||
                 (data_type == VX_TYPE_UINT16))
        {
            dim = sizeof(vx_uint16);
        }
        else if ((data_type == VX_TYPE_INT32) ||
                 (data_type == VX_TYPE_UINT32) ||
                 (data_type == VX_TYPE_FLOAT32))
        {
            dim = sizeof(vx_uint32);
        }
        else if ((data_type == VX_TYPE_INT64) ||
                 (data_type == VX_TYPE_UINT64) ||
                 (data_type == VX_TYPE_FLOAT64))
        {
            dim = sizeof(vx_uint64);
        }
        else
        {
            dim = 0ul;
        }

        if ((rows != 0) && (columns != 0) && (dim != 0ul))
        {
            matrix = (vx_matrix)ownCreateReference(context, VX_TYPE_MATRIX,
                VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)matrix) == VX_SUCCESS) &&
                (matrix->base.type == VX_TYPE_MATRIX))
            {
                /* assign refernce type specific callback's */
                matrix->base.destructor_callback = &ownDestructMatrix;
                matrix->base.mem_alloc_callback = &ownAllocMatrixBuffer;
                matrix->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleaseMatrix;

                obj_desc = (tivx_obj_desc_matrix_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_MATRIX, (vx_reference)matrix);
                if(obj_desc==NULL)
                {
                    vxReleaseMatrix(&matrix);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate matrix object descriptor\n");
                    matrix = (vx_matrix)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    obj_desc->data_type = data_type;
                    obj_desc->columns = columns;
                    obj_desc->rows = rows;
                    obj_desc->origin_x = columns/2;
                    obj_desc->origin_y = rows/2;
                    obj_desc->pattern = VX_PATTERN_OTHER;
                    obj_desc->mem_size = columns*rows*dim;
                    obj_desc->mem_ptr.host_ptr = (uint64_t)NULL;
                    obj_desc->mem_ptr.shared_ptr = (uint64_t)NULL;
                    obj_desc->mem_ptr.mem_heap_region = TIVX_MEM_EXTERNAL;
                    matrix->base.obj_desc = (tivx_obj_desc_t *)obj_desc;
                }
            }
        }
    }

    return (matrix);
}

vx_matrix VX_API_CALL vxCreateMatrixFromPattern(
    vx_context context, vx_enum pattern, vx_size columns, vx_size rows)
{
    vx_status status = VX_SUCCESS;
    vx_matrix matrix = NULL;
    vx_size dim = 0ul, i, j;
    vx_uint8 *pTempDataPtr;
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    /* Check for errors */
    if(ownIsValidContext(context) != vx_true_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrixFromPattern: invalid context\n");
        status = VX_FAILURE;
    }

    if (rows == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrixFromPattern: rows value is equal to zero\n");
        status = VX_FAILURE;
    }

    if (columns == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrixFromPattern: columns value is equal to zero\n");
        status = VX_FAILURE;
    }

    if ((VX_PATTERN_BOX != pattern) && (VX_PATTERN_CROSS != pattern) &&
             (VX_PATTERN_OTHER != pattern) && (VX_PATTERN_DISK != pattern))
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrixFromPattern: invalid pattern value\n");
        status = VX_FAILURE;
    }

    /* For Cross pattern, rows and columns must be odd */
    if ((VX_PATTERN_CROSS == pattern) && (((rows%2) == 0) || ((columns%2) == 0)))
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrixFromPattern: cross pattern rows and columns are not odd\n");
        status = VX_FAILURE;
    }

    /* For Disk pattern, rows and columns must be equal */
    if ((VX_PATTERN_DISK == pattern) && ( rows != columns ))
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrixFromPattern: disk pattern rows and columns are not equal\n");
        status = VX_FAILURE;
    }

    if (VX_SUCCESS == status)
    {
        dim = sizeof(vx_uint8);

        matrix = (vx_matrix)ownCreateReference(context, VX_TYPE_MATRIX,
            VX_EXTERNAL, &context->base);

        if ((vxGetStatus((vx_reference)matrix) == VX_SUCCESS) &&
            (matrix->base.type == VX_TYPE_MATRIX))
        {
            /* assign refernce type specific callback's */
            matrix->base.destructor_callback = &ownDestructMatrix;
            matrix->base.mem_alloc_callback = &ownAllocMatrixBuffer;
            matrix->base.release_callback =
                (tivx_reference_release_callback_f)&vxReleaseMatrix;

            obj_desc = (tivx_obj_desc_matrix_t*)tivxObjDescAlloc(
                TIVX_OBJ_DESC_MATRIX, (vx_reference)matrix);
            if(obj_desc==NULL)
            {
                vxReleaseMatrix(&matrix);

                vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                    "Could not allocate matrix object descriptor\n");
                matrix = (vx_matrix)ownGetErrorObject(
                    context, VX_ERROR_NO_RESOURCES);
                VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrixFromPattern: Could not allocate matrix object descriptor\n");
                status = VX_FAILURE;
            }
            else
            {
                /* Initialize descriptor object */
                obj_desc->data_type = VX_TYPE_UINT8;
                obj_desc->columns = columns;
                obj_desc->rows = rows;
                obj_desc->origin_x = columns/2;
                obj_desc->origin_y = rows/2;
                obj_desc->pattern = pattern;
                obj_desc->mem_size = columns*rows*dim;
                obj_desc->mem_ptr.mem_heap_region = TIVX_MEM_EXTERNAL;
                matrix->base.obj_desc = (tivx_obj_desc_t *)obj_desc;

                obj_desc->mem_ptr.host_ptr = (uint64_t)NULL;
                obj_desc->mem_ptr.shared_ptr = (uint64_t)NULL;

                /* Allocate memory for matrix since matrix need to be
                   filled up with a pattern  */
                status = ownAllocMatrixBuffer(&matrix->base);

                if (VX_SUCCESS != status)
                {
                    /* Free up memory allocated for matrix */
                    ownDestructMatrix(&matrix->base);

                    /* Release matrix */
                    vxReleaseMatrix(&matrix);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate matrix object descriptor\n");
                    matrix = (vx_matrix)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                    VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrixFromPattern: Could not allocate matrix object descriptor\n");
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
            VX_PRINT(VX_ZONE_ERROR, "vxCreateMatrixFromPattern: Invalid matrix reference\n");
            status = VX_FAILURE;
        }
    }

    if ((VX_SUCCESS == status) && ((uint64_t)NULL != obj_desc->mem_ptr.host_ptr))
    {
        tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
            obj_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);

        pTempDataPtr = (vx_uint8 *)(uintptr_t)obj_desc->mem_ptr.host_ptr;
        if (VX_PATTERN_BOX == pattern)
        {
            for (i = 0U; i < rows; i ++)
            {
                for (j = 0U; j < columns; j ++)
                {
                    *pTempDataPtr = 255U;
                    pTempDataPtr ++;
                }
            }
        }
        else if (VX_PATTERN_CROSS == pattern)
        {
            for (i = 0U; i < rows; i ++)
            {
                for (j = 0U; j < columns; j ++)
                {
                    *pTempDataPtr = 0;
                    pTempDataPtr ++;
                }
            }
            /* Set data values in the centre row and column to 255  */
            pTempDataPtr = (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
            pTempDataPtr = pTempDataPtr + (((rows/2))*columns);
            for (i = 0U; i < columns; i ++)
            {
                *pTempDataPtr = 255;
                pTempDataPtr ++;
            }
            pTempDataPtr = (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
            pTempDataPtr = pTempDataPtr + ((columns/2));
            for (i = 0U; i < rows; i ++)
            {
                *pTempDataPtr = 255;
                pTempDataPtr += columns;
            }
        }
        else if (VX_PATTERN_DISK == pattern)
        {
            vx_uint8* mask = (vx_uint8*)(uintptr_t)obj_desc->mem_ptr.host_ptr;
            vx_uint8 ref;

            for (i = 0U; i < rows; i ++)
            {
                for (j = 0U; j < columns; j ++)
                {
                    ref = ( ( ( (i - (rows / 2.0)) + 0.5) * ( (i - (rows / 2.0) ) + 0.5) ) / ( (rows / 2.0) * (rows / 2.0) ) +
                        (((j - (columns / 2.0)) + 0.5) * ((j - (columns / 2.0)) + 0.5)) / ((columns / 2.0) * (columns / 2.0)))
                        <= 1 ? 255 : 0;

                    mask[j + (i * columns)] = ref;
                }
            }
        }
        else /* VS_PATTERN_OTHER */
        {
            for (i = 0U; i < rows; i ++)
            {
                for (j = 0U; j < columns; j ++)
                {
                    *pTempDataPtr = 0;
                    pTempDataPtr ++;
                }
            }
        }

        tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr,
            obj_desc->mem_size, VX_MEMORY_TYPE_HOST,
            VX_WRITE_ONLY);
    }

    return (matrix);
}

vx_status VX_API_CALL vxQueryMatrix(
    vx_matrix matrix, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&matrix->base, VX_TYPE_MATRIX) == vx_false_e)
        ||
        (matrix->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryMatrix: Invalid matrix reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_matrix_t *)matrix->base.obj_desc;
        switch (attribute)
        {
            case VX_MATRIX_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = obj_desc->data_type;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryMatrix: Query matrix type failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_MATRIX_COLUMNS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = obj_desc->columns;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryMatrix: Query matrix columns failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_MATRIX_ROWS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = obj_desc->rows;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryMatrix: Query matrix rows failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_MATRIX_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr =
                        obj_desc->mem_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryMatrix: Query matrix size failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_MATRIX_ORIGIN:
                if (VX_CHECK_PARAM(ptr, size, vx_coordinates2d_t, 0x3))
                {
                    vx_coordinates2d_t *rect = (vx_coordinates2d_t *)ptr;

                    rect->x = obj_desc->origin_x;
                    rect->y = obj_desc->origin_y;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryMatrix: Query matrix origin failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_MATRIX_PATTERN:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = obj_desc->pattern;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "vxQueryMatrix: Query matrix pattern failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "vxQueryMatrix: Invalid matrix query attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseMatrix(vx_matrix *matrix)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)matrix, VX_TYPE_MATRIX, VX_EXTERNAL, NULL));
}

vx_status VX_API_CALL vxCopyMatrix(
    vx_matrix matrix, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 size;
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&matrix->base, VX_TYPE_MATRIX) == vx_false_e)
        ||
        (matrix->base.obj_desc == NULL)
        )
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCopyMatrix: Invalid matrix reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_matrix_t *)matrix->base.obj_desc;
        if (VX_MEMORY_TYPE_HOST != user_mem_type)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyMatrix: user mem type is not equal to VX_MEMORY_TYPE_HOST\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        /* Memory still not allocated */
        if ((VX_READ_ONLY == usage) &&
            ((uint64_t)NULL == obj_desc->mem_ptr.host_ptr))
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyMatrix: Memory is not allocated\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if (NULL == user_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCopyMatrix: User pointer is NULL\n");
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (VX_SUCCESS == status)
    {
        size = obj_desc->mem_size;

        /* Copy from matrix object to user memory */
        if (VX_READ_ONLY == usage)
        {
            tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

            memcpy(user_ptr, (void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size);

            tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        else /* Copy from user memory to matrix object */
        {
            status = ownAllocMatrixBuffer(&matrix->base);

            if (VX_SUCCESS == status)
            {
                tivxMemBufferMap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

                memcpy((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, user_ptr, size);

                tivxMemBufferUnmap((void*)(uintptr_t)obj_desc->mem_ptr.host_ptr, size,
                    VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
            }
        }
    }

    return (status);
}

static vx_status ownAllocMatrixBuffer(vx_reference ref)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_MATRIX)
    {
        obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;
        if(obj_desc != NULL)
        {
            /* memory is not allocated, so allocate it */
            if(obj_desc->mem_ptr.host_ptr == (uint64_t)NULL)
            {
                status = tivxMemBufferAlloc(
                    &obj_desc->mem_ptr, obj_desc->mem_size,
                    TIVX_MEM_EXTERNAL);

                if ((VX_SUCCESS != status) ||
                    (obj_desc->mem_ptr.host_ptr == (uint64_t)NULL))
                {
                    /* could not allocate memory */
                    VX_PRINT(VX_ZONE_ERROR, "ownAllocMatrixBuffer: Memory could not be allocated\n");
                    status = VX_ERROR_NO_MEMORY;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "ownAllocMatrixBuffer: Object descriptor is NULL\n");
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "ownAllocMatrixBuffer: Invalid matrix reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructMatrix(vx_reference ref)
{
    tivx_obj_desc_matrix_t *obj_desc = NULL;

    if(ref->type == VX_TYPE_MATRIX)
    {
        obj_desc = (tivx_obj_desc_matrix_t *)ref->obj_desc;
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
