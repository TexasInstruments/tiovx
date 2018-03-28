/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <VX/vx.h>
#include <TI/tivx.h>
#include "test_engine/test.h"
#include <string.h>

#define VX_PLANE_MAX (4)

TESTCASE(tivxPackedDataFormat, CT_VXContext, ct_setup_vx_context, 0)

static uint32_t own_stride_bytes(vx_df_image format, int step)
{
    uint32_t factor = 0;

    switch (format)
    {
    case VX_DF_IMAGE_U8:
    case TIVX_DF_IMAGE_P12:
    case VX_DF_IMAGE_NV21:
    case VX_DF_IMAGE_NV12:
    case VX_DF_IMAGE_YUV4:
    case VX_DF_IMAGE_IYUV:
        factor = 1;
        break;

    case VX_DF_IMAGE_U16:
    case VX_DF_IMAGE_S16:
    case VX_DF_IMAGE_YUYV:
    case VX_DF_IMAGE_UYVY:
        factor = 2;
        break;

    case VX_DF_IMAGE_U32:
    case VX_DF_IMAGE_S32:
    case VX_DF_IMAGE_RGBX:
        factor = 4;
        break;

    case VX_DF_IMAGE_RGB:
        factor = 3;
        break;

    default:
        ASSERT_(return 0, 0);
    }

    return step*factor;
}


static int own_get_channel_step_x(vx_df_image format, vx_enum channel)
{
    switch (format)
    {
    case TIVX_DF_IMAGE_P12:
        return 0;

    case VX_DF_IMAGE_U8:
        return 1;

    case VX_DF_IMAGE_U16:
    case VX_DF_IMAGE_S16:
        return 2;

    case VX_DF_IMAGE_U32:
    case VX_DF_IMAGE_S32:
    case VX_DF_IMAGE_RGBX:
        return 4;

    case VX_DF_IMAGE_RGB:
        return 3;

    case VX_DF_IMAGE_YUYV:
    case VX_DF_IMAGE_UYVY:
        if (channel == VX_CHANNEL_Y)
            return 2;
        return 4;

    case VX_DF_IMAGE_IYUV:
    case VX_DF_IMAGE_YUV4:
        return 1;

    case VX_DF_IMAGE_NV12:
    case VX_DF_IMAGE_NV21:
        if (channel == VX_CHANNEL_Y)
            return 1;
        return 2;

    default:
        ASSERT_(return 0, 0);
    }

    return 0;
}


static int own_get_channel_step_y(vx_df_image format, vx_enum channel, int step)
{
    switch (format)
    {
    case VX_DF_IMAGE_U8:
        return step;

    case TIVX_DF_IMAGE_P12:
        return step * 1.5;

    case VX_DF_IMAGE_U16:
    case VX_DF_IMAGE_S16:
        return step * 2;

    case VX_DF_IMAGE_U32:
    case VX_DF_IMAGE_S32:
    case VX_DF_IMAGE_RGBX:
        return step * 4;

    case VX_DF_IMAGE_RGB:
        return step * 3;

    case VX_DF_IMAGE_YUYV:
    case VX_DF_IMAGE_UYVY:
        return step * 2;

    case VX_DF_IMAGE_IYUV:
        return (channel == VX_CHANNEL_Y) ? step : step / 2;

    case VX_DF_IMAGE_YUV4:
    case VX_DF_IMAGE_NV12:
    case VX_DF_IMAGE_NV21:
        return step;

    default:
        ASSERT_(return 0, 0);
    }

    return 0;
}


static int own_get_channel_subsampling_x(vx_df_image format, vx_enum channel)
{
    if (channel == VX_CHANNEL_Y)
        return 1;

    switch (format)
    {
    case VX_DF_IMAGE_IYUV:
    case VX_DF_IMAGE_NV12:
    case VX_DF_IMAGE_NV21:
    case VX_DF_IMAGE_YUYV:
    case VX_DF_IMAGE_UYVY:
        return 2;
    }

    return 1;
}


static int own_get_channel_subsampling_y(vx_df_image format, vx_enum channel)
{
    if (channel == VX_CHANNEL_Y)
        return 1;

    switch (format)
    {
    case VX_DF_IMAGE_IYUV:
    case VX_DF_IMAGE_NV12:
    case VX_DF_IMAGE_NV21:
        return 2;

    case VX_DF_IMAGE_YUYV:
    case VX_DF_IMAGE_UYVY:
        return 1;
    }

    return 1;
}


static unsigned int own_image_bits_per_pixel(vx_df_image format, unsigned int p)
{
    switch (format)
    {
    case VX_DF_IMAGE_U8:
    case TIVX_DF_IMAGE_P12:
        return 8 * 1;

    case VX_DF_IMAGE_U16:
    case VX_DF_IMAGE_S16:
    case VX_DF_IMAGE_UYVY:
    case VX_DF_IMAGE_YUYV:
        return 8 * 2;

    case VX_DF_IMAGE_U32:
    case VX_DF_IMAGE_S32:
    case VX_DF_IMAGE_RGBX:
        return 8 * 4;

    case VX_DF_IMAGE_RGB:
    case VX_DF_IMAGE_YUV4:
        return 8 * 3;

    case VX_DF_IMAGE_IYUV:
        return 8 * 3 / 2;

    case VX_DF_IMAGE_NV12:
    case VX_DF_IMAGE_NV21:
        if (p == 0)
            return 8 * 1;
        else
            return 8 * 2;

    default:
        CT_RecordFailure();
        return 0;
    };
}

static size_t own_plane_size(uint32_t width, uint32_t height, unsigned int p, vx_df_image format)
{
    return (size_t)(width * height * own_image_bits_per_pixel(format, p) / 8);
}

/*
// Allocates image plane pointers from user controlled memory according to format, width, height params
// and initialize with some value
*/
static void own_allocate_image_ptrs(
    vx_df_image format, int width, int height,
    vx_uint32* nplanes, void* ptrs[], vx_imagepatch_addressing_t addr[],
    vx_pixel_value_t* val)
{
    unsigned int p;
    int channel[VX_PLANE_MAX] = { 0, 0, 0, 0 };

    switch (format)
    {
    case VX_DF_IMAGE_U8:
    case TIVX_DF_IMAGE_P12:
    case VX_DF_IMAGE_U16:
    case VX_DF_IMAGE_S16:
    case VX_DF_IMAGE_U32:
    case VX_DF_IMAGE_S32:
        channel[0] = VX_CHANNEL_0;
        break;

    case VX_DF_IMAGE_RGB:
    case VX_DF_IMAGE_RGBX:
        channel[0] = VX_CHANNEL_R;
        channel[1] = VX_CHANNEL_G;
        channel[2] = VX_CHANNEL_B;
        channel[3] = VX_CHANNEL_A;
        break;

    case VX_DF_IMAGE_UYVY:
    case VX_DF_IMAGE_YUYV:
    case VX_DF_IMAGE_NV12:
    case VX_DF_IMAGE_NV21:
    case VX_DF_IMAGE_YUV4:
    case VX_DF_IMAGE_IYUV:
        channel[0] = VX_CHANNEL_Y;
        channel[1] = VX_CHANNEL_U;
        channel[2] = VX_CHANNEL_V;
        break;

    default:
        ASSERT(0);
    }

    if (format != TIVX_DF_IMAGE_P12)
        ASSERT_NO_FAILURE(*nplanes = ct_get_num_planes(format));
    else
        *nplanes = 1;

    for (p = 0; p < *nplanes; p++)
    {
        size_t plane_size = 0;

        vx_uint32 subsampling_x = own_get_channel_subsampling_x(format, channel[p]);
        vx_uint32 subsampling_y = own_get_channel_subsampling_y(format, channel[p]);

        addr[p].dim_x    = width  / subsampling_x;
        addr[p].dim_y    = height / subsampling_y;
        addr[p].stride_x = own_get_channel_step_x(format, channel[p]);
        addr[p].stride_y = own_get_channel_step_y(format, channel[p], width);

        plane_size = addr[p].stride_y * addr[p].dim_y;

        if (plane_size != 0)
        {
            ptrs[p] = ct_alloc_mem(plane_size);
            /* init memory */
            ct_memset(ptrs[p], val->reserved[p], plane_size);
        }
    }

    return;
}

static void mem_free(void**ptr)
{
    ct_free_mem(*ptr);
    *ptr = 0;
}

TEST(tivxPackedDataFormat, testCreateImage)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[384];
    vx_df_image format = 0;
    int i;

    for (i = 0; i < 384; i++)
    {
        ASSERT_VX_OBJECT(src_image[i] = vxCreateImage(context, 16, 16, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
    }

    for (i = 0; i < 384; i++)
    {
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(src_image[i], VX_IMAGE_FORMAT,  &format,  sizeof(format)));
        ASSERT_EQ_INT(TIVX_DF_IMAGE_P12, format);
    }

    for (i = 0; i < 384; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
    }
}

TEST(tivxPackedDataFormat, testCreateVirtualImage)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[384];
    int i;
    vx_graph graph = 0;
    vx_df_image format = 0;

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    for (i = 0; i < 384; i++)
    {
        ASSERT_VX_OBJECT(src_image[i] = vxCreateVirtualImage(graph, 16, 16, TIVX_DF_IMAGE_P12), VX_TYPE_IMAGE);
    }

    for (i = 0; i < 384; i++)
    {
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(src_image[i], VX_IMAGE_FORMAT,  &format,  sizeof(format)));
        ASSERT_EQ_INT(TIVX_DF_IMAGE_P12, format);
    }

    for (i = 0; i < 384; i++)
    {
        VX_CALL(vxReleaseImage(&src_image[i]));
    }

    VX_CALL(vxReleaseGraph(&graph));
}

TEST(tivxPackedDataFormat, testCreateImageFromHandleAndROI)
{
    vx_context context = context_->vx_context_;
    vx_image   src_image[384];
    int i;
    vx_df_image format = 0;

    vx_image image1 = 0;
    vx_image roi1 = 0;
    vx_imagepatch_addressing_t addr1[VX_PLANE_MAX] =
    {
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT
    };

    vx_uint32 nplanes1 = 0;
    vx_pixel_value_t val1;
    void* mem1_ptrs[VX_PLANE_MAX] = { 0, 0, 0, 0 };

    val1.reserved[0] = 0x11;
    val1.reserved[1] = 0x22;
    val1.reserved[2] = 0x33;
    val1.reserved[3] = 0x44;

    own_allocate_image_ptrs(TIVX_DF_IMAGE_P12, 24, 24, &nplanes1, mem1_ptrs, addr1, &val1);

    EXPECT_VX_OBJECT(image1 = vxCreateImageFromHandle(context, TIVX_DF_IMAGE_P12, addr1, mem1_ptrs, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);


    vx_rectangle_t roi1_rect =
    {
        12,
        12,
        24,
        24
    };

    ASSERT_VX_OBJECT(roi1 = vxCreateImageFromROI(image1, &roi1_rect), VX_TYPE_IMAGE);

    VX_CALL(vxReleaseImage(&roi1));
    VX_CALL(vxReleaseImage(&image1));
}

TEST(tivxPackedDataFormat, testCreateUniformImage)
{
    vx_context context = context_->vx_context_;
    vx_image   image   = 0;
    vx_size memsz;
    vx_df_image format = 0;
    int i;
    vx_uint8* buffer;
    vx_uint8* buffer0;

    vx_int16* buffer_16;
    vx_int16* buffer0_16;

    vx_rectangle_t rect             = { 0, 0, 240, 120 };
    vx_imagepatch_addressing_t addr = { 240, 120, 0, 240 };

    vx_rectangle_t rect_16             = { 0, 0, 240, 120 };
    vx_imagepatch_addressing_t addr_16 = { 240, 120, 2, 240 };

    vx_pixel_value_t vals;

    vals.reserved[0] = 0x11;
    vals.reserved[1] = 0x22;
    vals.reserved[2] = 0x33;
    vals.reserved[3] = 0x44;

    ASSERT_VX_OBJECT(image = vxCreateUniformImage(context, 240, 120, TIVX_DF_IMAGE_P12, &vals), VX_TYPE_IMAGE);

    memsz = vxComputeImagePatchSize(image, &rect, 0);

    ASSERT(buffer = ct_alloc_mem(memsz));
    CT_RegisterForGarbageCollection(buffer, mem_free, CT_GC_OBJECT);
    buffer0 = buffer;

    ASSERT(buffer_16 = ct_alloc_mem(480*120));
    CT_RegisterForGarbageCollection(buffer_16, mem_free, CT_GC_OBJECT);
    buffer0_16 = buffer_16;

    // copy image data to our buffer
    VX_CALL(vxCopyImagePatch(image, &rect, 0, &addr, buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    ASSERT_EQ_PTR(buffer0, buffer);

    /* Normal P12 test case with stride_x = 0 */
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxQueryImage(image, VX_IMAGE_FORMAT,  &format,  sizeof(format)));
    ASSERT_EQ_INT(TIVX_DF_IMAGE_P12, format);

    /* Negative test for stride_x = 1 */
    addr.stride_x = 1;
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxCopyImagePatch(image, &rect, 0, &addr, buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    /* Test for stride_x = 2 */
    VX_CALL(vxCopyImagePatch(image, &rect_16, 0, &addr_16, buffer_16, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    ASSERT_EQ_PTR(buffer0_16, buffer_16);

    VX_CALL(vxReleaseImage(&image));
    ASSERT(image == 0);
}

static uint32_t ct_image_removeref(CT_Image img)
{
    if (img->refcount_)
        return --*img->refcount_;
    else
        return (uint32_t)(-1);
}

static void ct_release_image(CT_Image* img)
{
    if (!img || !*img) return;

#ifdef DEBUG_CT_IMAGE
    printf("RELEASING: "); ct_print_image(*img);
#endif

    if ((*img)->data.u32 && (*img)->refcount_) // if refcount_ is NULL then data is not ours
    {
        if (ct_image_removeref(*img) == 0)
        {
            ct_free_mem((*img)->data_begin_);
        }
        (*img)->data.u32 = 0;
        (*img)->data_begin_ = 0;
        (*img)->refcount_ = 0;
    }
    ct_free_mem(*img);
    *img = 0;
}

static size_t ct_image_data_size(uint32_t width, uint32_t height, vx_df_image format)
{
    return (size_t)width * height * ct_image_bits_per_pixel(format) / 8;
}

static CT_Image ct_allocate_image_hdr_impl(uint32_t width, uint32_t height, uint32_t step, vx_df_image format, int allocate_data)
{
    CT_Image image;

    CT_ASSERT_(return 0, step >= width);

    image = (CT_Image)ct_alloc_mem(sizeof(*image));
    CT_ASSERT_(return 0, image); // out of memory

    image->data.u32    = 0;
    image->data_begin_ = 0;
    image->refcount_   = 0;
    image->width  = width;
    image->height = height;
    image->stride = step;
    image->format = format;
    image->roi.x = 0;
    image->roi.y = 0;
    image->roi.width = width;
    image->roi.height = height;

    CT_RegisterForGarbageCollection(image, (CT_ObjectDestructor)ct_release_image, CT_GC_IMAGE);

    if (allocate_data)
    {
        uint8_t* bytes;
        size_t memory_size = ct_image_data_size(width, height, format) + sizeof(uint32_t) * 2;

        bytes = (uint8_t*)ct_alloc_mem(memory_size);
        CT_ASSERT_(return 0, bytes); // out of memory
        ct_memset(bytes, 153, memory_size); // fill with some "magic" value
        image->data_begin_ = bytes;
        image->refcount_ = (uint32_t*)( (((size_t)bytes) + sizeof(uint32_t) - 1) & ~((size_t)(sizeof(uint32_t) - 1)) ); // align ptr to 4 bytes
        *image->refcount_ = 1;
        image->data.u32 = image->refcount_ + 1;
    }

    return image;
}

static CT_Image own_generate_rand_image(int width, int height, vx_df_image format)
{
    CT_Image image;
    uint32_t x, y;
    uint64_t rng;

    rng = CT()->seed_;

    image = ct_allocate_image_hdr_impl(width, height, width, format, 1);

    uint8_t* ptr = image->data.y;
    for( y = 0; y < height; y++)
    {
        for( x = 0; x < width; x+=2 )
        {
            int val = CT_RNG_NEXT_INT(rng, 0, 4096);
            uint8_t value_b0 = (uint8_t)(val & 0xFF);
            uint8_t value_b1 = (uint8_t)(val>>8u) | (uint8_t)((val & 0x0F)<<4u);
            uint8_t value_b2 = (uint8_t)(val>>4u);
            *ptr = value_b0;
            ptr++;
            *ptr = value_b1;
            ptr++;
            *ptr = value_b2;
            ptr++;
        }
    }

    return image;
} /* own_generate_rand_image() */

TEST(tivxPackedDataFormat, testMapImage)
{
    vx_uint8* p1;
    vx_uint8* p2;
    vx_uint32 i;
    vx_int32  j;
    vx_uint32 nplanes;
    vx_context context = context_->vx_context_;
    vx_image image1 = 0;
    vx_image image2 = 0;
    vx_rectangle_t rect = { 0, 0, 0, 0 };
    vx_imagepatch_addressing_t addr1;
    vx_imagepatch_addressing_t addr2;

    vx_map_id map_id1;
    vx_map_id map_id2;

    CT_Image src = NULL;
    CT_Image tst = NULL;

    void* ptrs1 = { 0 };
    void* ptrs2 = { 0 };

    image1 = vxCreateImage(context, 240, 120, TIVX_DF_IMAGE_P12);
    ASSERT_VX_OBJECT(image1, VX_TYPE_IMAGE);

    image2 = vxCreateImage(context, 240, 120, TIVX_DF_IMAGE_P12);
    ASSERT_VX_OBJECT(image2, VX_TYPE_IMAGE);

    rect.start_x = 0;
    rect.start_y = 0;
    rect.end_x = 240;
    rect.end_y = 120;

    VX_CALL(vxMapImagePatch(image1, &rect, 0, &map_id1, &addr1, &ptrs1, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
    VX_CALL(vxMapImagePatch(image2, &rect, 0, &map_id2, &addr2, &ptrs2, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0));

    /* use linear addressing function */
    for (i = 0; i < addr1.dim_x*addr1.dim_y; i += addr1.step_x) // will need to modify based on this
    {
        p1 = vxFormatImagePatchAddress1d(ptrs1, i, &addr1);
        p2 = vxFormatImagePatchAddress1d(ptrs2, i, &addr2);
        /*for (j = 0; j < addr1.stride_x; j++)
            p2[j] = p1[j];*/
    }
    VX_CALL(vxUnmapImagePatch(image1, map_id1));
    VX_CALL(vxUnmapImagePatch(image2, map_id2));

    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));

    ASSERT(image1 == 0);
    ASSERT(image2 == 0);
}

vx_uint32 own_plane_subsampling_x(vx_df_image format, vx_uint32 plane)
{
    int subsampling_x = 0;

    switch (format)
    {
    case VX_DF_IMAGE_IYUV:
    case VX_DF_IMAGE_NV12:
    case VX_DF_IMAGE_NV21:
    case VX_DF_IMAGE_YUYV:
    case VX_DF_IMAGE_UYVY:
        subsampling_x = (0 == plane) ? 1 : 2;
        break;

    default:
        subsampling_x = 1;
        break;
    }

    return subsampling_x;
}

static
vx_uint32 own_plane_subsampling_y(vx_df_image format, vx_uint32 plane)
{
    int subsampling_y = 0;

    switch (format)
    {
    case VX_DF_IMAGE_IYUV:
    case VX_DF_IMAGE_NV12:
    case VX_DF_IMAGE_NV21:
        subsampling_y = (0 == plane) ? 1 : 2;
        break;

    default:
        subsampling_y = 1;
        break;
    }

    return subsampling_y;
}

TEST(tivxPackedDataFormat, testFormatImagePatchAddress2D)
{
    vx_uint32 n;
    vx_context context = context_->vx_context_;
    vx_image image1 = 0;
    vx_image image2 = 0;
    vx_imagepatch_addressing_t addr1[VX_PLANE_MAX] =
    {
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT
    };
    vx_imagepatch_addressing_t addr2[VX_PLANE_MAX] =
    {
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT
    };

    vx_uint32 nplanes1 = 0;
    vx_uint32 nplanes2 = 0;
    vx_pixel_value_t val1;
    vx_pixel_value_t val2;
    vx_pixel_value_t val3;
    void* mem1_ptrs[VX_PLANE_MAX] = { 0, 0, 0, 0 };
    void* mem2_ptrs[VX_PLANE_MAX] = { 0, 0, 0, 0 };
    void* prev_ptrs[VX_PLANE_MAX] = { 0, 0, 0, 0 };
    CT_Image tst1 = 0;
    CT_Image tst2 = 0;

    val1.reserved[0] = 0x11;
    val1.reserved[1] = 0x22;
    val1.reserved[2] = 0x33;
    val1.reserved[3] = 0x44;

    val2.reserved[0] = 0x99;
    val2.reserved[1] = 0x88;
    val2.reserved[2] = 0x77;
    val2.reserved[3] = 0x66;

    val3.reserved[0] = 0xaa;
    val3.reserved[1] = 0xbb;
    val3.reserved[2] = 0xcc;
    val3.reserved[3] = 0xdd;

    own_allocate_image_ptrs(TIVX_DF_IMAGE_P12, 240, 120, &nplanes1, mem1_ptrs, addr1, &val1);
    own_allocate_image_ptrs(TIVX_DF_IMAGE_P12, 240, 120, &nplanes2, mem2_ptrs, addr2, &val2);
    EXPECT_EQ_INT(nplanes1, nplanes2);

    EXPECT_VX_OBJECT(image1 = vxCreateImageFromHandle(context, TIVX_DF_IMAGE_P12, addr1, mem1_ptrs, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);
    EXPECT_VX_OBJECT(image2 = vxCreateImageFromHandle(context, TIVX_DF_IMAGE_P12, addr2, mem2_ptrs, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);

    vx_image roi1 = 0;
    vx_image roi2 = 0;
    vx_uint32 roi1_width;
    vx_uint32 roi1_height;

    vx_rectangle_t roi1_rect =
    {
        240 / 2,
        120 / 2,
        240,
        120
    };

    vx_rectangle_t roi2_rect;

    /* first level subimage */
    ASSERT_VX_OBJECT(roi1 = vxCreateImageFromROI(image1, &roi1_rect), VX_TYPE_IMAGE);

    VX_CALL(vxQueryImage(roi1, VX_IMAGE_WIDTH, &roi1_width, sizeof(vx_uint32)));
    VX_CALL(vxQueryImage(roi1, VX_IMAGE_HEIGHT, &roi1_height, sizeof(vx_uint32)));

    roi2_rect.start_x = roi1_width / 2;
    roi2_rect.start_y = roi1_height / 2;
    roi2_rect.end_x   = roi1_width;
    roi2_rect.end_y   = roi1_height;

    /* second level subimage */
    ASSERT_VX_OBJECT(roi2 = vxCreateImageFromROI(roi1, &roi2_rect), VX_TYPE_IMAGE);

    /* try to get back ROI pointers */
    /* Negative test */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxSwapImageHandle(roi2, NULL, prev_ptrs, nplanes1));

    /* try to replace and get back ROI pointers */
    /* Negative test */
    ASSERT_NE_VX_STATUS(VX_SUCCESS, vxSwapImageHandle(roi2, mem2_ptrs, prev_ptrs, nplanes1));

    /* check the content of roi2 image equal image1 */
    for (n = 0; n < nplanes1; n++)
    {
        unsigned int i;
        unsigned int j;
        vx_rectangle_t rect_roi2 = { 0, 0, 0, 0 };

        vx_imagepatch_addressing_t addr = VX_IMAGEPATCH_ADDR_INIT;

        vx_map_id map_id;

        void* plane_ptr = 0;

        VX_CALL(vxGetValidRegionImage(roi2, &rect_roi2));
        VX_CALL(vxMapImagePatch(roi2, &rect_roi2, n, &map_id, &addr, &plane_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
        for (i = 0; i < addr.dim_y; i += addr.step_y)
        {
            unsigned char* p = vxFormatImagePatchAddress2d(plane_ptr, 0, i, &addr);
            for (j = 0; j < addr.dim_x; j += 2)
            {
                if (p[0] != val1.reserved[n])
                    CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val1, p[0]);
                p++;
               if (p[0] != val1.reserved[n])
                    CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val1, p[0]);
                p++;
               if (p[0] != val1.reserved[n])
                    CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val1, p[0]);
                p++;
            }
        }
        VX_CALL(vxUnmapImagePatch(roi2, map_id));
    }

    /* replace image pointers */
    VX_CALL(vxSwapImageHandle(image1, mem2_ptrs, NULL, nplanes1));

    /* check the content of roi2 image equal image2 */
    for (n = 0; n < nplanes1; n++)
    {
        unsigned int i;
        unsigned int j;
        vx_rectangle_t rect_roi2 = { 0, 0, 0, 0 };

        vx_imagepatch_addressing_t addr = VX_IMAGEPATCH_ADDR_INIT;

        vx_map_id map_id;

        void* plane_ptr = 0;

        VX_CALL(vxGetValidRegionImage(roi2, &rect_roi2));
        VX_CALL(vxMapImagePatch(roi2, &rect_roi2, n, &map_id, &addr, &plane_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
        for (i = 0; i < addr.dim_y; i += addr.step_y)
        {
            unsigned char* p = vxFormatImagePatchAddress2d(plane_ptr, 0, i, &addr);
            for (j = 0; j < addr.dim_x; j += 2)
            {
                if (p[0] != val2.reserved[n])
                   CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val2, p[0]);
                p++;
                if (p[0] != val2.reserved[n])
                   CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val2, p[0]);
                p++;
                if (p[0] != val2.reserved[n])
                   CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val2, p[0]);
                p++;
            }
        }
        VX_CALL(vxUnmapImagePatch(roi2, map_id));
    }

    /* modify the content of roi2 */
    for (n = 0; n < nplanes1; n++)
    {
        unsigned int i;
        unsigned int j;
        vx_rectangle_t rect_roi2 = { 0, 0, 0, 0 };

        vx_imagepatch_addressing_t addr = VX_IMAGEPATCH_ADDR_INIT;

        vx_map_id map_id;

        void* plane_ptr = 0;

        VX_CALL(vxGetValidRegionImage(roi2, &rect_roi2));
        VX_CALL(vxMapImagePatch(roi2, &rect_roi2, n, &map_id, &addr, &plane_ptr, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));
        for (i = 0; i < addr.dim_y; i += addr.step_y)
        {
            unsigned char* p = vxFormatImagePatchAddress2d(plane_ptr, 0, i, &addr);
            for (j = 0; j < addr.dim_x; j += 2)
            {
                *p = val3.reserved[n];
                p++;
                *p = val3.reserved[n];
                p++;
                *p = val3.reserved[n];
                p++;
            }
        }
        VX_CALL(vxUnmapImagePatch(roi2, map_id));
    }

    /* reclaim image ptrs */
    VX_CALL(vxSwapImageHandle(image1, NULL, prev_ptrs, nplanes1));

    /* check that the reclaimed host memory contains the correct data */
    for (n = 0; n < nplanes2; n++)
    {
        vx_uint8* plane_ptr = prev_ptrs[n];
        vx_uint32 i;
        vx_uint32 j;
        vx_uint32 subsampling_x = own_plane_subsampling_x(TIVX_DF_IMAGE_P12, n);
        vx_uint32 subsampling_y = own_plane_subsampling_y(TIVX_DF_IMAGE_P12, n);
        vx_uint32 start_x = (roi1_rect.start_x + roi2_rect.start_x) / subsampling_x;
        vx_uint32 start_y = (roi1_rect.start_y + roi2_rect.start_y) / subsampling_y;
        vx_uint32 end_x   = (vx_uint32)(240  / subsampling_x);
        vx_uint32 end_y   = (vx_uint32)(120 / subsampling_y);

        for (i = 0; i < addr2[n].dim_y; i++)
        {
            unsigned int k = i * addr2[n].stride_y;

            for (j = 0; j < addr2[n].dim_x; j+=2)
            {
                unsigned char p = plane_ptr[k];

                if (i >= start_y && i <= end_y - 1 &&
                    j >= start_x && j <= end_x - 1)
                {
                    if (p != val3.reserved[n])
                        CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val3, p);
                }
                else
                {
                    if (p != val2.reserved[n])
                        CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val2, p);
                }
                k++;
                p = plane_ptr[k];

                if (i >= start_y && i <= end_y - 1 &&
                    j >= start_x && j <= end_x - 1)
                {
                    if (p != val3.reserved[n])
                        CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val3, p);
                }
                else
                {
                    if (p != val2.reserved[n])
                        CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val2, p);
                }
                k++;
                p = plane_ptr[k];

                if (i >= start_y && i <= end_y - 1 &&
                    j >= start_x && j <= end_x - 1)
                {
                    if (p != val3.reserved[n])
                        CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val3, p);
                }
                else
                {
                    if (p != val2.reserved[n])
                        CT_FAIL("ROI content mismath at [x=%d, y=%d]: expected %d, actual %d", j, i, val2, p);
                }
                k++;
            }
        }
    }

    /* check that pointers are reclaimed in ROI */
    for (n = 0; n < nplanes1; n++)
    {
        if (prev_ptrs[n] != NULL)
        {
            vx_rectangle_t rect = { 0, 0, 0, 0 };

            vx_imagepatch_addressing_t addr = VX_IMAGEPATCH_ADDR_INIT;

            vx_map_id map_id;

            void* plane_ptr = 0;

            vx_uint8* ptr = (vx_uint8*)prev_ptrs[n];

            EXPECT_EQ_PTR(mem2_ptrs[n], ptr);

            ct_free_mem(ptr);
            prev_ptrs[n] = NULL;
            mem2_ptrs[n] = NULL;

            VX_CALL(vxGetValidRegionImage(roi2, &rect));

            EXPECT_EQ_INT(VX_ERROR_NO_MEMORY, vxMapImagePatch(roi2, &rect, n, &map_id, &addr, &plane_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
        }
    }

    VX_CALL(vxReleaseImage(&roi1));
    VX_CALL(vxReleaseImage(&roi2));
    ASSERT(roi1 == 0);
    ASSERT(roi2 == 0);
//end of if

    for (n = 0; n < VX_PLANE_MAX; n++)
    {
        if (mem1_ptrs[n] != NULL)
        {
            ct_free_mem(mem1_ptrs[n]);
            mem1_ptrs[n] = NULL;
        }
    }

    VX_CALL(vxReleaseImage(&image1));
    VX_CALL(vxReleaseImage(&image2));

    ASSERT(image1 == 0);
    ASSERT(image2 == 0);
}

/*
//  Fill image patch info according to user memory layout
*/
static void own_image_patch_from_ct_image(CT_Image ref, vx_imagepatch_addressing_t* ref_addr, void** ref_ptrs, vx_df_image format)
{
    switch (format)
    {
    case TIVX_DF_IMAGE_P12:
    {
        ref_addr[0].dim_x   = ref->width;
        ref_addr[0].dim_y   = ref->height;
        ref_addr[0].stride_x = 0;
        ref_addr[0].stride_y = ref->stride * 1.5;

        ref_ptrs[0] = ref->data.s16;
    }
    break;

    default:
        FAIL("unexpected image format: (%.4s)", format);
        break;
    } /* switch format */

    return;
} /* own_image_patch_from_ct_image() */

vx_uint32 own_elem_size(vx_df_image format, vx_uint32 plane)
{
    int channel_step_x = 0;

    switch (format)
    {
    case VX_DF_IMAGE_U8:
    case TIVX_DF_IMAGE_P12:
        channel_step_x = 1;
        break;

    case VX_DF_IMAGE_U16:
    case VX_DF_IMAGE_S16:
        channel_step_x = 2;
        break;

    default:
        channel_step_x = 0;
    }

    return channel_step_x;
}

static void own_check_image_patch_plane_vx_layout(CT_Image ctimg, vx_imagepatch_addressing_t* vx_addr, void* p_vx_base, vx_uint32 plane, vx_df_image format)
{
    vx_uint32 x;
    vx_uint32 y;
    double ct_elem_size = 1.5;
    double ct_stride_size = ct_elem_size*ctimg->stride;
    void*     p_ct_base = ct_image_get_plane_base(ctimg, plane);

    for (y = 0; y < vx_addr->dim_y; y += vx_addr->step_y)
    {
        vx_uint8* ref_ptr = (vx_uint8*)((vx_uint8*)p_ct_base + y * (vx_uint32)ct_stride_size);
        vx_uint8* tst_ptr = (vx_uint8*)vxFormatImagePatchAddress2d(p_vx_base, 0, y, vx_addr);
        for (x = 0; x < vx_addr->dim_x; x += 2)
        {
            ASSERT_EQ_INT(ref_ptr[0], tst_ptr[0]);
            ref_ptr++;
            tst_ptr++;
            ASSERT_EQ_INT(ref_ptr[0], tst_ptr[0]);
            ref_ptr++;
            tst_ptr++;
            ASSERT_EQ_INT(ref_ptr[0], tst_ptr[0]);
            ref_ptr++;
            tst_ptr++;
        } /* for tst_addr.dim_x */
    } /* for tst_addr.dim_y */

    return;
} /* own_check_image_patch_plane_vx_layout() */

TEST(tivxPackedDataFormat, testStrideX)
{
    vx_context context = context_->vx_context_;

    vx_uint32 plane;
    vx_size num_planes = 0;
    vx_image image = 0;
    CT_Image ref = 0;
    vx_imagepatch_addressing_t ref_addr[VX_PLANE_MAX] =
    {
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT,
        VX_IMAGEPATCH_ADDR_INIT
    };
    void* ref_ptrs[VX_PLANE_MAX] = { 0, 0, 0, 0 };

    ASSERT_NO_FAILURE(ref = own_generate_rand_image(240, 120, TIVX_DF_IMAGE_P12));

    own_image_patch_from_ct_image(ref, ref_addr, ref_ptrs, TIVX_DF_IMAGE_P12);

    ASSERT_VX_OBJECT(image = vxCreateImageFromHandle(context, TIVX_DF_IMAGE_P12, ref_addr, ref_ptrs, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);

    VX_CALL(vxQueryImage(image, VX_IMAGE_PLANES, &num_planes, sizeof(num_planes)));

    /* Testing stride_x = 0 */
    /* Normal P12 test case */
    for (plane = 0; plane < (vx_uint32)num_planes; plane++)
    {
        vx_rectangle_t             rect     = { 0, 0, 240, 120 };
        vx_imagepatch_addressing_t tst_addr = VX_IMAGEPATCH_ADDR_INIT;
        vx_map_id map_id;
        vx_uint32 flags = VX_NOGAP_X;
        void* ptr = 0;

        VX_CALL(vxMapImagePatch(image, &rect, plane, &map_id, &tst_addr, &ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, flags));

        /* check if image patch plane equal to reference data */
        own_check_image_patch_plane_vx_layout(ref, &tst_addr, ptr, plane, TIVX_DF_IMAGE_P12);

        VX_CALL(vxUnmapImagePatch(image, map_id));
    } /* for num_planes */

    VX_CALL(vxReleaseImage(&image));
    ASSERT(image == 0);
}

TESTCASE_TESTS(tivxPackedDataFormat,
               testCreateImage,
               testCreateVirtualImage,
               testCreateImageFromHandleAndROI,
               testCreateUniformImage,
               testMapImage,
               testFormatImagePatchAddress2D,
               testStrideX)
