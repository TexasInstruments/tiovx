/*

 * Copyright (c) 2015-2017 The Khronos Group Inc.
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
/*
 * Copyright (c) 2022 Texas Instruments Incorporated
 */

#include "test_engine/test.h"
#include <TI/tivx_debug.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include "TI/tivx.h"
#include <VX/vx_types.h>
#include <utils/mem/include/app_mem.h>

#if defined(QNX)
#include <SharedMemoryAllocatorUsr.h>
#endif

#define INVALID_ARG -1

TESTCASE(tivxDmaHeap, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxDmaHeap, testappMemTranslateDmaBufFd)
{
    uint32_t dmaBufFd = INVALID_ARG;
    uint32_t size = 0;
    uint64_t *virtAddr = NULL;
    uint64_t *phyAddr = NULL;

    ASSERT((vx_status)VX_FAILURE == tivxMemTranslateFd(dmaBufFd, size, (void **)&virtAddr, (void **)&phyAddr));
}

TEST(tivxDmaHeap, testappMemFree)
{
   uint32_t block = 0;
   void *virPtr = NULL;
   uint32_t size = 0;

   ASSERT((vx_status)VX_FAILURE == appMemFree(block, virPtr, size));
}

#ifndef PC
TEST(tivxDmaHeap, testappMemStats)
{
    app_mem_stats_t stats;
    vx_enum cpu_id;

    cpu_id = tivxGetSelfCpuId();
    ASSERT((uint32_t)0 == appMemStats(APP_MEM_HEAP_DDR, &stats));
    ASSERT((uint32_t)0 == appMemStats(-1, &stats));
    ASSERT((uint32_t)0 == appMemStats(1,NULL));
}

TEST(tivxDmaHeap, testappMemCacheWbInv)
{
    void *ptr = NULL;
    uint32_t size = 0;

    appMemCacheWbInv(ptr, size);
}

TEST(tivxDmaHeap, testappMemCacheInv)
{
    void *ptr = NULL;
    uint32_t size = 0;

    appMemCacheInv(ptr, size);
}

TEST(tivxDmaHeap, testappMemCacheWb)
{
    void *ptr = NULL;
    uint32_t size = 0;

    appMemCacheWb(ptr, size);
}
#endif

TEST(tivxDmaHeap, testappMemAddTupleToList)
{
    uint32_t dmaBufFd = 0;
    uint32_t size = 1;
    uint64_t *virtAddr = NULL;
    uint64_t *phyAddr = NULL;

    ASSERT((vx_status)VX_FAILURE == tivxMemTranslateFd(dmaBufFd, size, (void **)&virtAddr, (void **)&phyAddr));
}

TEST(tivxDmaHeap, testappMemShared2PhysPtr)
{
    uint64_t shared_ptr = 0;
    uint32_t heap_id = 0;

    ASSERT(shared_ptr == appMemShared2PhysPtr(shared_ptr, heap_id));
}

TEST(tivxDmaHeap, testappMemShared2TargetPtr)
{
    uint64_t shared_ptr = 0;

    ASSERT(shared_ptr == appMemShared2TargetPtr(shared_ptr));
}

#if defined(QNX)
TEST(tivxDmaHeap, testAppMemAlloc)
{
    void *ptr = NULL;
    uint32_t size = 1024;
    uint32_t region = 0;

    ptr = appMemAlloc(region, size, 0);

    ASSERT(ptr != NULL);

    VX_CALL(appMemFree(region, ptr, size));
}

TEST(tivxDmaHeap, testAppMemTranslateQnx)
{
    shm_buf buf;
    uint32_t size = 1024;
    uint32_t align = 1024;
    uint32_t region = 0;
    void     *phyAddr;
    void     *virtAddr;

    SHM_alloc_aligned((int)size, align, &buf);

    ASSERT(buf.vir_addr != 0);

    VX_CALL(tivxMemTranslateFd(buf.phy_addr, size, &virtAddr, &phyAddr));

    VX_CALL(appMemFree(region, (void *)buf.vir_addr, size));
}

#endif

TESTCASE_TESTS(
    tivxDmaHeap,
    testappMemTranslateDmaBufFd,
    testappMemFree,
#ifndef PC
    testappMemStats,
    testappMemCacheWbInv,
    testappMemCacheInv,
    testappMemCacheWb,
#endif
#if defined(QNX)
    testAppMemAlloc,
    testAppMemTranslateQnx,
#endif
    testappMemAddTupleToList,
    testappMemShared2PhysPtr,
    testappMemShared2TargetPtr
    )