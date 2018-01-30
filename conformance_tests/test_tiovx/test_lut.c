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




#include "test_tiovx.h"
#include <VX/vx.h>
#include <string.h>


TESTCASE(tivxLUT, CT_VXContext, ct_setup_vx_context, 0)

static vx_size lut_count(vx_enum data_type)
{
    vx_size count = 0;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        count = 256;
        break;
    case VX_TYPE_INT16:
        count = 65536;
        break;
    }

    return count;
}

static vx_size lut_size(vx_enum data_type)
{
    vx_size size = 0;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        size = 256*sizeof(vx_uint8);
        break;
    case VX_TYPE_INT16:
        size = 65536*sizeof(vx_int16);
        break;
    }

    return size;
}

static vx_lut lut_create(vx_context context, void* data, vx_enum data_type)
{
    vx_size count = lut_count(data_type);
    vx_size size = lut_size(data_type);

    vx_lut lut = vxCreateLUT(context, data_type, count);
    void* ptr = NULL;

    ASSERT_VX_OBJECT_(return 0, lut, VX_TYPE_LUT);

    vx_map_id map_id;
    VX_CALL_(return 0, vxMapLUT(lut, &map_id, &ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));
    ASSERT_(return 0, ptr);
    memcpy(ptr, data, size);
    VX_CALL_(return 0, vxUnmapLUT(lut, map_id));
    return lut;
}

static void lut_data_fill_random(void* data, vx_enum data_type)
{
    uint64_t* seed = &CT()->seed_;
    int i;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        {
            vx_uint8* data8 = (vx_uint8*)data;
            for (i = 0; i < 256; ++i)
                data8[i] = (vx_uint8)CT_RNG_NEXT_INT(*seed, 0, 256);
        }
        break;
    case VX_TYPE_INT16:
        {
            vx_int16* data16 = (vx_int16*)data;
            for (i = 0; i < 65536; ++i)
                data16[i] = (vx_int16)CT_RNG_NEXT_INT(*seed, (uint32_t)-32768, 32768);
        }
        break;
    }
}

// Generate input to cover these requirements:
// There should be a image with randomly generated pixel intensities.
static CT_Image lut_image_generate_random(const char* fileName, int width, int height, vx_enum data_type)
{
    CT_Image image = 0;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_U8, &CT()->seed_, 0, 256);
        break;
    case VX_TYPE_INT16:
        image = ct_allocate_ct_image_random(width, height, VX_DF_IMAGE_S16, &CT()->seed_, -32768, 32768);
        break;
    }
    ASSERT_(return 0, image != 0);

    return image;
}

// data_type == VX_TYPE_UINT8
static vx_uint8 lut_calculate_u8(CT_Image src, uint32_t x, uint32_t y, void* lut_data)
{
    vx_uint8* lut_data8 = (vx_uint8*)lut_data;
    vx_int32 offset = 0;
    vx_uint8 value = *CT_IMAGE_DATA_PTR_8U(src, x, y);
    vx_uint8 res = lut_data8[offset + value];
    return res;
}

// data_type == VX_TYPE_INT16
static vx_int16 lut_calculate_s16(CT_Image src, uint32_t x, uint32_t y, void* lut_data)
{
    vx_int16* lut_data16 = (vx_int16*)lut_data;
    vx_int32 offset = 65536/2;
    vx_int16 value = *CT_IMAGE_DATA_PTR_16S(src, x, y);
    vx_int16 res = lut_data16[offset + value];
    return res;
}

static CT_Image lut_create_reference_image(CT_Image src, void* lut_data, vx_enum data_type)
{
    CT_Image dst;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_U8);
        break;
    case VX_TYPE_INT16:
        CT_ASSERT_(return NULL, src->format == VX_DF_IMAGE_S16);
        break;
    }

    dst = ct_allocate_image(src->width, src->height, src->format);

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        CT_FILL_IMAGE_8U(return 0, dst,
            {
                uint8_t res = lut_calculate_u8(src, x, y, lut_data);
                *dst_data = res;
            });
        break;
    case VX_TYPE_INT16:
        CT_FILL_IMAGE_16S(return 0, dst,
            {
                int16_t res = lut_calculate_s16(src, x, y, lut_data);
                *dst_data = res;
            });
        break;
    }

    return dst;
}


static void lut_check(CT_Image src, CT_Image dst, void* lut1_data, void* lut2_data, vx_enum data_type)
{
    CT_Image dst_ref = NULL, virt_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(virt_ref = lut_create_reference_image(src, lut1_data, data_type));

    ASSERT_NO_FAILURE(dst_ref = lut_create_reference_image(virt_ref, lut2_data, data_type));

    EXPECT_EQ_CTIMAGE(dst_ref, dst);
#if 0
    if (CT_HasFailure())
    {
        int i = 0;
        printf("=== LUT ===\n");
        switch (data_type)
        {
        case VX_TYPE_UINT8:
            for (i = 0; i < 256; ++i)
                printf("%3d:%3d ", i, (int)((vx_uint8*)lut_data)[i]);
            break;
        case VX_TYPE_INT16:
            for (i = 0; i < 65536; ++i)
                printf("%5d:%6d ", i, (int)((vx_int16*)lut_data)[i]);
            break;
        }
        printf("\n");
        printf("=== SRC ===\n");
        ct_dump_image_info(src);
        printf("=== DST ===\n");
        ct_dump_image_info(dst);
        printf("=== EXPECTED ===\n");
        ct_dump_image_info(dst_ref);
    }
#endif
}

typedef struct {
    const char* testName;
    CT_Image (*generator)(const char* fileName, int width, int height, vx_enum data_type);
    const char* fileName;
    void (*lut_generator)(void* data, vx_enum data_type);
    int width, height;
    vx_enum data_type;
} Arg;

#define ADD_LUT_GENERATOR(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/LutRandom", __VA_ARGS__, lut_data_fill_random))

#define ADD_TYPE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/U8", __VA_ARGS__, VX_TYPE_UINT8)), \
    CT_EXPAND(nextmacro(testArgName "/S16", __VA_ARGS__, VX_TYPE_INT16))

#define LUT_PARAMETERS                                                \
    CT_GENERATE_PARAMETERS("randomInput", ADD_LUT_GENERATOR, ADD_SIZE_18x18, ADD_TYPE, ARG, lut_image_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_LUT_GENERATOR, ADD_SIZE_644x258, ADD_TYPE, ARG, lut_image_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("randomInput", ADD_LUT_GENERATOR, ADD_SIZE_1600x1200, ADD_TYPE, ARG, lut_image_generate_random, NULL)

TEST_WITH_ARG(tivxLUT, testGraphProcessing, Arg,
    LUT_PARAMETERS
)
{
    vx_enum data_type = arg_->data_type;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0, virt;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_graph;

    CT_Image src = NULL, dst = NULL;
    void* lut1_data;
    void* lut2_data;
    vx_lut lut1, lut2;
    vx_size size;

    size = lut_size(data_type);
    lut1_data = ct_alloc_mem(size);
    ASSERT(lut1_data != 0);
    lut2_data = ct_alloc_mem(size);
    ASSERT(lut2_data != 0);

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height, data_type));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(arg_->lut_generator(lut1_data, data_type));
    ASSERT_VX_OBJECT(lut1 = lut_create(context, lut1_data, data_type), VX_TYPE_LUT);

    ASSERT_NO_FAILURE(arg_->lut_generator(lut2_data, data_type));
    ASSERT_VX_OBJECT(lut2 = lut_create(context, lut2_data, data_type), VX_TYPE_LUT);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(virt  = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT), VX_TYPE_IMAGE);

    node1 = vxTableLookupNode(graph, src_image, lut1, virt);
    ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);

    node2 = vxTableLookupNode(graph, virt, lut2, dst_image);
    ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

    vx_enum lut_type;
    vx_uint32 lut_offset;
    VX_CALL(vxQueryLUT(lut1, VX_LUT_TYPE, &lut_type, sizeof(lut_type)));
    VX_CALL(vxQueryLUT(lut1, VX_LUT_OFFSET, &lut_offset, sizeof(lut_offset)));
    if (VX_TYPE_UINT8 == lut_type)
    {
        if (lut_offset != 0)
        {
            CT_FAIL("check for LUT attribute VX_LUT_OFFSET failed\n");
        }
    }
    else if (VX_TYPE_INT16 == lut_type)
    {
        vx_size lut_count;
        VX_CALL(vxQueryLUT(lut1, VX_LUT_COUNT, &lut_count, sizeof(lut_count)));
        if (lut_offset != (lut_count/2))
        {
            CT_FAIL("check for LUT attribute VX_LUT_COUNT failed\n");
        }
    }

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
    vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
    vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(lut_check(src, dst, lut1_data, lut2_data, data_type));

    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0);
    ASSERT(node2 == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseLUT(&lut1));
    VX_CALL(vxReleaseLUT(&lut2));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(lut1 == 0);
    ASSERT(lut2 == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    printPerformance(perf_node1, arg_->width*arg_->height, "N1");

    printPerformance(perf_node2, arg_->width*arg_->height, "N2");

    printPerformance(perf_graph, arg_->width*arg_->height, "G1");

    ct_free_mem(lut2_data);
    ct_free_mem(lut1_data);
}

TESTCASE_TESTS(tivxLUT,
               testGraphProcessing)
