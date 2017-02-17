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

#include <TI/tivx_mem.h>

#define CT_MEM_ALLOC_ALIGN      (255U)
#define CT_MEM_HEADER_SIZE      (32U)

void *ct_alloc_mem(size_t size)
{
    void *ptr = NULL;

    if (0 != size)
    {
        size = (size + CT_MEM_HEADER_SIZE + CT_MEM_ALLOC_ALIGN) &
            ~(CT_MEM_ALLOC_ALIGN);
        ptr = tivxMemAlloc(size);

        if (NULL != ptr)
        {
            /* First word stores the size of the memory allocated */
            *(uint32_t*)ptr = size;
            ptr = (void *)((uintptr_t)ptr + CT_MEM_HEADER_SIZE);
        }
    }

    return (ptr);
}

void ct_free_mem(void *ptr)
{
    uint32_t size;

    if (NULL != ptr)
    {
        ptr = (void *)((uintptr_t)ptr - CT_MEM_HEADER_SIZE);
        size = *(uint32_t*)ptr;
        tivxMemFree(ptr, size);
    }
}

void ct_memset(void *ptr, vx_uint8 c, size_t size)
{
    if (NULL != ptr)
    {
        memset(ptr, c, size);
        tivxMemBufferUnmap(ptr, size, TIVX_MEM_EXTERNAL, VX_WRITE_ONLY);
    }
}

void *ct_calloc(size_t nmemb, size_t size)
{
    void *ptr = NULL;
    size_t new_size;

    if ((0 != size) && (0 != nmemb))
    {
        new_size = size * nmemb;
        new_size = (new_size + CT_MEM_HEADER_SIZE + CT_MEM_ALLOC_ALIGN) &
            ~(CT_MEM_ALLOC_ALIGN);
        ptr = tivxMemAlloc(new_size);

        if (NULL != ptr)
        {
            /* First word stores the size of the memory allocated */
            *(uint32_t*)ptr = new_size;
            ptr = (void *)((uint32_t)ptr + CT_MEM_HEADER_SIZE);
        }
    }

    return (ptr);
}
