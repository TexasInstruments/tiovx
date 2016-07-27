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

static vx_status ownDestructMatrix(vx_reference ref);
static vx_status ownAllocMatrixBuffer(vx_reference ref);

vx_matrix VX_API_CALL vxCreateMatrix(
    vx_context context, vx_enum data_type, vx_size columns, vx_size rows)
{
    vx_matrix matrix = NULL;
    vx_size dim = 0ul;

    if(ownIsValidContext(context) == vx_true_e)
    {
        if ((data_type == VX_TYPE_INT8) || (data_type == VX_TYPE_UINT8))
        {
            dim = sizeof(vx_uint8);
        }
        else if ((data_type == VX_TYPE_INT32) ||
                 (data_type == VX_TYPE_UINT32) ||
                 (data_type == VX_TYPE_FLOAT32))
        {
            dim = sizeof(vx_uint32);
        }
        else
        {
            dim = 0ul;
        }

        if (rows != 0 && columns != 0 && dim != 0ul)
        {
            matrix = (vx_matrix)ownCreateReference(context, VX_TYPE_MATRIX,
                VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)matrix) == VX_SUCCESS) &&
                (matrix->base.type == VX_TYPE_MATRIX))
            {
                /* assign refernce type specific callback's */
                matrix->base.destructor_callback = ownDestructMatrix;
                matrix->base.mem_alloc_callback = ownAllocMatrixBuffer;
                matrix->base.release_callback =
                    (tivx_reference_release_callback_f)vxReleaseMatrix;

                matrix->obj_desc = (tivx_obj_desc_matrix_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_MATRIX);
                if(matrix->obj_desc==NULL)
                {
                    vxReleaseMatrix(&matrix);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate matrix object descriptor\n");
                    matrix = (vx_matrix)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    matrix->obj_desc->data_type = data_type;
                    matrix->obj_desc->columns = columns;
                    matrix->obj_desc->rows = rows;
                    matrix->obj_desc->pattern = VX_PATTERN_OTHER;
                    matrix->obj_desc->mem_size = columns*rows*dim;
                    matrix->obj_desc->mem_ptr.host_ptr = NULL;
                    matrix->obj_desc->mem_ptr.shared_ptr = NULL;
                    matrix->obj_desc->mem_ptr.mem_type = TIVX_MEM_EXTERNAL;
                }
            }
        }
    }

    return (matrix);
}

vx_matrix VX_API_CALL vxCreateMatrixFromPattern(
    vx_context context, vx_enum pattern, vx_size columns, vx_size rows)
{
    vx_status status = VX_FAILURE;
    vx_matrix matrix = NULL;
    vx_size dim = 0ul, i, j;
    vx_uint8 *pTempDataPtr;

    /* Check for errors */
    if(ownIsValidContext(context) != vx_true_e)
    {
        status = VX_FAILURE;
    }

    if ((rows == 0) || (columns == 0))
    {
        status = VX_FAILURE;
    }

    if ((VX_PATTERN_BOX != pattern) && (VX_PATTERN_CROSS != pattern) &&
             (VX_PATTERN_OTHER != pattern))
    {
        status = VX_FAILURE;
    }

    /* For Cross pattern, rows and columns must be odd */
    if ((VX_PATTERN_CROSS == pattern) && (rows%2 != 0) && (columns%2 != 0))
    {
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
            matrix->base.destructor_callback = ownDestructMatrix;
            matrix->base.mem_alloc_callback = ownAllocMatrixBuffer;
            matrix->base.release_callback =
                (tivx_reference_release_callback_f)vxReleaseMatrix;

            matrix->obj_desc = (tivx_obj_desc_matrix_t*)tivxObjDescAlloc(
                TIVX_OBJ_DESC_MATRIX);
            if(matrix->obj_desc==NULL)
            {
                vxReleaseMatrix(&matrix);

                vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                    "Could not allocate matrix object descriptor\n");
                matrix = (vx_matrix)ownGetErrorObject(
                    context, VX_ERROR_NO_RESOURCES);

                status = VX_FAILURE;
            }
            else
            {
                /* Initialize descriptor object */
                matrix->obj_desc->data_type = VX_TYPE_UINT8;
                matrix->obj_desc->columns = columns;
                matrix->obj_desc->rows = rows;
                matrix->obj_desc->pattern = pattern;
                matrix->obj_desc->mem_size = columns*rows*dim;
                matrix->obj_desc->mem_ptr.mem_type = TIVX_MEM_EXTERNAL;

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
                }
                else
                {
                    matrix->obj_desc->mem_ptr.shared_ptr =
                        tivxMemHost2SharedPtr(
                            matrix->obj_desc->mem_ptr.host_ptr,
                            TIVX_MEM_EXTERNAL);
                }
            }
        }
        else
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivxMemBufferMap(matrix->obj_desc->mem_ptr.host_ptr,
            matrix->obj_desc->mem_size, matrix->obj_desc->mem_ptr.mem_type,
            VX_WRITE_ONLY);

        pTempDataPtr = (vx_uint8 *)matrix->obj_desc->mem_ptr.host_ptr;
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
            pTempDataPtr = matrix->obj_desc->mem_ptr.host_ptr;
            pTempDataPtr = pTempDataPtr + ((rows/2) + 1)*columns;
            for (i = 0U; i < columns; i ++)
            {
                *pTempDataPtr = 255;
                pTempDataPtr ++;
            }
            pTempDataPtr = matrix->obj_desc->mem_ptr.host_ptr;
            pTempDataPtr = pTempDataPtr + ((rows/2) + 1);
            for (i = 0U; i < rows; i ++)
            {
                *pTempDataPtr = 255;
                pTempDataPtr += columns;
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

        tivxMemBufferUnmap(matrix->obj_desc->mem_ptr.host_ptr,
            matrix->obj_desc->mem_size, matrix->obj_desc->mem_ptr.mem_type,
            VX_WRITE_ONLY);
    }

    return (matrix);
}

vx_status VX_API_CALL vxQueryMatrix(
    vx_matrix matrix, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    if (ownIsValidSpecificReference(&matrix->base, VX_TYPE_MATRIX) == vx_false_e
        &&
        matrix->obj_desc != NULL
        )
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_MATRIX_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = matrix->obj_desc->data_type;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_MATRIX_COLUMNS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = matrix->obj_desc->columns;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_MATRIX_ROWS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = matrix->obj_desc->rows;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_MATRIX_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr =
                        matrix->obj_desc->mem_size;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_MATRIX_PATTERN:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    /* TODO: Check */
                    *(vx_enum *)ptr = VX_PATTERN_BOX;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            /* case VX_MATRIX_ORIGIN ?? */
            default:
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

    if (ownIsValidSpecificReference(&matrix->base, VX_TYPE_MATRIX) == vx_false_e
        &&
        matrix->obj_desc != NULL
        )
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
            (NULL == matrix->obj_desc->mem_ptr.host_ptr))
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
        size = matrix->obj_desc->mem_size;

        /* Copy from matrix object to user memory */
        if (VX_READ_ONLY == usage)
        {
            tivxMemBufferMap(matrix->obj_desc->mem_ptr.host_ptr, size,
                matrix->obj_desc->mem_ptr.mem_type, VX_READ_ONLY);

            memcpy(user_ptr, matrix->obj_desc->mem_ptr.host_ptr, size);

            tivxMemBufferUnmap(matrix->obj_desc->mem_ptr.host_ptr, size,
                matrix->obj_desc->mem_ptr.mem_type, VX_READ_ONLY);
        }
        else /* Copy from user memory to matrix object */
        {
            status = ownAllocMatrixBuffer(&matrix->base);

            if (VX_SUCCESS == status)
            {
                tivxMemBufferMap(matrix->obj_desc->mem_ptr.host_ptr, size,
                    matrix->obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);

                memcpy(matrix->obj_desc->mem_ptr.host_ptr, user_ptr, size);

                tivxMemBufferUnmap(matrix->obj_desc->mem_ptr.host_ptr, size,
                    matrix->obj_desc->mem_ptr.mem_type, VX_WRITE_ONLY);
            }
        }
    }

    return (status);
}

static vx_status ownAllocMatrixBuffer(vx_reference ref)
{
    vx_status status = VX_SUCCESS;
    vx_matrix matrix = (vx_matrix)ref;

    if(matrix->base.type == VX_TYPE_MATRIX)
    {
        if(matrix->obj_desc != NULL)
        {
            /* memory is not allocated, so allocate it */
            if(matrix->obj_desc->mem_ptr.host_ptr == NULL)
            {
                tivxMemBufferAlloc(
                    &matrix->obj_desc->mem_ptr, matrix->obj_desc->mem_size,
                    TIVX_MEM_EXTERNAL);

                if(matrix->obj_desc->mem_ptr.host_ptr==NULL)
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

static vx_status ownDestructMatrix(vx_reference ref)
{
    vx_matrix matrix = (vx_matrix)ref;

    if(matrix->base.type == VX_TYPE_MATRIX)
    {
        if(matrix->obj_desc!=NULL)
        {
            if(matrix->obj_desc->mem_ptr.host_ptr!=NULL)
            {
                tivxMemBufferFree(
                    &matrix->obj_desc->mem_ptr, matrix->obj_desc->mem_size);
            }

            tivxObjDescFree((tivx_obj_desc_t**)&matrix->obj_desc);
        }
    }
    return VX_SUCCESS;
}
