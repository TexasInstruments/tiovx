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

#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <string.h>


TESTCASE(LUT, CT_VXContext, ct_setup_vx_context, 0)

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

static void lut_data_fill_identity(void* data, vx_enum data_type)
{
    int i;

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        {
            vx_int32 offset = 0;
            vx_uint8* data8 = (vx_uint8*)data;
            for (i = 0; i < 256; ++i)
                data8[i] = i - offset;
        }
        break;
    case VX_TYPE_INT16:
        {
            vx_int32 offset = 65536/2;
            vx_int16* data16 = (vx_int16*)data;
            for (i = 0; i < 65536; ++i)
                data16[i] = i - offset;
        }
        break;
    }
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

TEST(LUT, testNodeCreation)
{
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_uint8 lut_data[256];
    vx_lut lut = 0;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_enum data_type = VX_TYPE_UINT8;

    src_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8);
    ASSERT_VX_OBJECT(src_image, VX_TYPE_IMAGE);

    dst_image = vxCreateImage(context, 128, 128, VX_DF_IMAGE_U8);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, data_type));
    ASSERT_VX_OBJECT(lut = lut_create(context, lut_data, data_type), VX_TYPE_LUT);

    {
        vx_enum lut_type;
        vx_size lut_count, lut_size;
        VX_CALL(vxQueryLUT(lut, VX_LUT_TYPE, &lut_type, sizeof(lut_type)));
        if (VX_TYPE_UINT8 != lut_type)
        {
            CT_FAIL("check for LUT attribute VX_LUT_TYPE failed\n");
        }
        VX_CALL(vxQueryLUT(lut, VX_LUT_COUNT, &lut_count, sizeof(lut_count)));
        if (256 != lut_count)
        {
            CT_FAIL("check for LUT attribute VX_LUT_COUNT failed\n");
        }
        VX_CALL(vxQueryLUT(lut, VX_LUT_SIZE, &lut_size, sizeof(lut_size)));
        if (256 > lut_size)
        {
            CT_FAIL("check for LUT attribute VX_LUT_SIZE failed\n");
        }
    }

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxTableLookupNode(graph, src_image, lut, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    VX_CALL(vxReleaseLUT(&lut));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(node == 0);
    ASSERT(graph == 0);
    ASSERT(lut == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);
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

static CT_Image lut_image_read(const char* fileName, int width, int height, vx_enum data_type)
{
    CT_Image image8 = NULL;
    ASSERT_(return 0, width == 0 && height == 0);
    image8 = ct_read_image(fileName, 1);
    ASSERT_(return 0, image8);
    ASSERT_(return 0, image8->format == VX_DF_IMAGE_U8);

    switch (data_type)
    {
    case VX_TYPE_UINT8:
        return image8;
    case VX_TYPE_INT16:
        {
            vx_int32 offset = 65536/2;
            CT_Image image16 = ct_allocate_image(image8->width, image8->height, VX_DF_IMAGE_S16);
            if (image16)
            {
                CT_FILL_IMAGE_16S(return 0, image16,
                    {
                        vx_uint8 value8 = *CT_IMAGE_DATA_PTR_8U(image8, x, y);
                        vx_uint16 value16 = ((vx_uint16)value8 << 8) | value8;
                        vx_int16 res = (vx_int16)((vx_int32)value16 - offset);
                        *dst_data = res;
                    });
            }
            return image16;
        }
    }

    return NULL;
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


static void lut_check(CT_Image src, CT_Image dst, void* lut_data, vx_enum data_type)
{
    CT_Image dst_ref = NULL;

    ASSERT(src && dst);

    ASSERT_NO_FAILURE(dst_ref = lut_create_reference_image(src, lut_data, data_type));

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
    CT_EXPAND(nextmacro(testArgName "/LutIdentity", __VA_ARGS__, lut_data_fill_identity)), \
    CT_EXPAND(nextmacro(testArgName "/LutRandom", __VA_ARGS__, lut_data_fill_random))

#define ADD_TYPE(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/U8", __VA_ARGS__, VX_TYPE_UINT8)), \
    CT_EXPAND(nextmacro(testArgName "/S16", __VA_ARGS__, VX_TYPE_INT16))

#define LUT_PARAMETERS                                                \
    CT_GENERATE_PARAMETERS("randomInput", ADD_LUT_GENERATOR, ADD_SIZE_SMALL_SET, ADD_TYPE, ARG, lut_image_generate_random, NULL), \
    CT_GENERATE_PARAMETERS("lena", ADD_LUT_GENERATOR, ADD_SIZE_NONE, ADD_TYPE, ARG, lut_image_read, "lena.bmp")

TEST_WITH_ARG(LUT, testGraphProcessing, Arg,
    LUT_PARAMETERS
)
{
    vx_enum data_type = arg_->data_type;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;
    vx_graph graph = 0;
    vx_node node = 0;

    CT_Image src = NULL, dst = NULL;
    void* lut_data;
    vx_lut lut;
    vx_size size;

    size = lut_size(data_type);
    lut_data = ct_alloc_mem(size);
    ASSERT(lut_data != 0);

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height, data_type));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(arg_->lut_generator(lut_data, data_type));
    ASSERT_VX_OBJECT(lut = lut_create(context, lut_data, data_type), VX_TYPE_LUT);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    graph = vxCreateGraph(context);
    ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);

    node = vxTableLookupNode(graph, src_image, lut, dst_image);
    ASSERT_VX_OBJECT(node, VX_TYPE_NODE);

    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(lut_check(src, dst, lut_data, data_type));

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseLUT(&lut));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(lut == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    ct_free_mem(lut_data);
}

TEST_WITH_ARG(LUT, testImmediateProcessing, Arg,
    LUT_PARAMETERS
)
{
    vx_enum data_type = arg_->data_type;
    vx_context context = context_->vx_context_;
    vx_image src_image = 0, dst_image = 0;

    CT_Image src = NULL, dst = NULL;
    void* lut_data;
    vx_lut lut;
    vx_size size;

    size = lut_size(data_type);
    lut_data = ct_alloc_mem(size);
    ASSERT(lut_data != 0);

    ASSERT_NO_FAILURE(src = arg_->generator(arg_->fileName, arg_->width, arg_->height, data_type));

    ASSERT_VX_OBJECT(src_image = ct_image_to_vx_image(src, context), VX_TYPE_IMAGE);

    ASSERT_NO_FAILURE(arg_->lut_generator(lut_data, data_type));
    ASSERT_VX_OBJECT(lut = lut_create(context, lut_data, data_type), VX_TYPE_LUT);

    dst_image = ct_create_similar_image(src_image);
    ASSERT_VX_OBJECT(dst_image, VX_TYPE_IMAGE);

    VX_CALL(vxuTableLookup(context, src_image, lut, dst_image));

    ASSERT_NO_FAILURE(dst = ct_image_from_vx_image(dst_image));

    ASSERT_NO_FAILURE(lut_check(src, dst, lut_data, data_type));

    VX_CALL(vxReleaseLUT(&lut));
    VX_CALL(vxReleaseImage(&dst_image));
    VX_CALL(vxReleaseImage(&src_image));

    ASSERT(lut == 0);
    ASSERT(dst_image == 0);
    ASSERT(src_image == 0);

    ct_free_mem(lut_data);
}

typedef struct {
    const char* name;
    vx_enum type;
} data_type_arg;

TEST_WITH_ARG(LUT, test_vxCopyLUT, data_type_arg,
              ARG_ENUM(VX_TYPE_UINT8),
              ARG_ENUM(VX_TYPE_INT16))
{
    vx_context context = context_->vx_context_;
    vx_enum data_type = arg_->type;
    vx_lut lut;
    vx_size size = lut_size(data_type);
    vx_size i;

    void* identity_data = ct_alloc_mem(size);
    ASSERT(identity_data != 0);
    ASSERT_NO_FAILURE(lut_data_fill_identity(identity_data, data_type));
    ASSERT_VX_OBJECT(lut = lut_create(context, identity_data, data_type), VX_TYPE_LUT);
    /* read only mode */
    void* user_data = ct_alloc_mem(size);
    VX_CALL(vxCopyLUT(lut, user_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    /* Check */
    for (i = 0; i < size; i++)
    {
        ASSERT(((vx_uint8*)user_data)[i] == ((vx_uint8*)identity_data)[i]);
    }
    /* write only mode */
    void* random_data = ct_alloc_mem(size);
    ASSERT_NO_FAILURE(lut_data_fill_random(random_data, data_type));
    VX_CALL(vxCopyLUT(lut, random_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    /* Check */
    vx_map_id map_id;
    void* lut_data = NULL;
    VX_CALL(vxMapLUT(lut, &map_id, &lut_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for (i = 0; i < size; i++)
    {
        ASSERT(((vx_uint8*)lut_data)[i] == ((vx_uint8*)random_data)[i]);
    }
    VX_CALL(vxUnmapLUT(lut, map_id));

    VX_CALL(vxReleaseLUT(&lut));
    ASSERT(lut == 0);

    ct_free_mem(identity_data);
    ct_free_mem(user_data);
    ct_free_mem(random_data);
}

TEST_WITH_ARG(LUT, test_vxMapLUTWrite, data_type_arg,
              ARG_ENUM(VX_TYPE_UINT8),
              ARG_ENUM(VX_TYPE_INT16))
{
    vx_context context = context_->vx_context_;
    vx_enum data_type = arg_->type;
    vx_lut lut;
    vx_size size = lut_size(data_type);
    void* identity_data = ct_alloc_mem(size);
    vx_size i;

    ASSERT(identity_data != 0);
    ASSERT_NO_FAILURE(lut_data_fill_identity(identity_data, data_type));
    ASSERT_VX_OBJECT(lut = lut_create(context, identity_data, data_type), VX_TYPE_LUT);
    /* Read and write mode, read */
    vx_map_id map_id;
    void* lut_data = NULL;
    VX_CALL(vxMapLUT(lut, &map_id, &lut_data, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));
    /* Check */
    for (i = 0; i < size; i++)
    {
        ASSERT(((vx_uint8*)lut_data)[i] == ((vx_uint8*)identity_data)[i]);
    }
    /* Read and write mode, write */
    void* random_data = ct_alloc_mem(size);
    ASSERT_NO_FAILURE(lut_data_fill_random(random_data, data_type));
    memcpy(lut_data, random_data, size);
    VX_CALL(vxUnmapLUT(lut, map_id));
    /* Check */
    lut_data = NULL;
    VX_CALL(vxMapLUT(lut, &map_id, &lut_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for (i = 0; i < size; i++)
    {
        ASSERT(((vx_uint8*)lut_data)[i] == ((vx_uint8*)random_data)[i]);
    }
    VX_CALL(vxUnmapLUT(lut, map_id));

    /* Write only mode */
    VX_CALL(vxMapLUT(lut, &map_id, &lut_data, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));
    ASSERT_NO_FAILURE(lut_data_fill_identity(lut_data, data_type));
    VX_CALL(vxUnmapLUT(lut, map_id));
    /* Check */
    lut_data = NULL;
    VX_CALL(vxMapLUT(lut, &map_id, &lut_data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    for ( i = 0; i < size; i++)
    {
        ASSERT(((vx_uint8*)lut_data)[i] == ((vx_uint8*)identity_data)[i]);
    }
    VX_CALL(vxUnmapLUT(lut, map_id));

    VX_CALL(vxReleaseLUT(&lut));
    ASSERT(lut == 0);

    ct_free_mem(identity_data);
    ct_free_mem(random_data);
}

TESTCASE_TESTS(LUT,
               testNodeCreation,
               testGraphProcessing,
               testImmediateProcessing,
               test_vxCopyLUT,
               test_vxMapLUTWrite)
