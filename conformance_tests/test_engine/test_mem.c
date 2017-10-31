/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
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

#include <stdlib.h>
#include <string.h>
#include <VX/vx.h>
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
        ptr = tivxMemAlloc(size, TIVX_MEM_EXTERNAL);

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
        tivxMemFree(ptr, size, TIVX_MEM_EXTERNAL);
    }
}

void ct_memset(void *ptr, vx_uint8 c, size_t size)
{
    if (NULL != ptr)
    {
        memset(ptr, c, size);
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
        ptr = tivxMemAlloc(new_size, TIVX_MEM_EXTERNAL);

        if (NULL != ptr)
        {
            /* First word stores the size of the memory allocated */
            *(uint32_t*)ptr = new_size;
            ptr = (void *)((uintptr_t)ptr + CT_MEM_HEADER_SIZE);
        }
    }

    return (ptr);
}
