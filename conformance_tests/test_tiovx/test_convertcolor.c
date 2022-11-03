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
#include "shared_functions.h"

#include <VX/vx.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#define MAX_NODES 10

static int get_yuv_params(CT_Image img, uint8_t** ptrY, uint8_t** ptrU, uint8_t** ptrV,
                           uint32_t* strideY, uint32_t* deltaY,
                           uint32_t* strideC, uint32_t* deltaC,
                           uint32_t* shiftX, uint32_t* shiftY, int* code)
{
    int format = img->format;
    int is_yuv = 0;
    uint32_t stride = ct_stride_bytes(img);
    uint32_t height = img->height;

    *ptrY = img->data.y;
    *strideY = *strideC = stride;
    *deltaY = *deltaC = 1;
    *shiftX = *shiftY = 0;

    if( format == VX_DF_IMAGE_YUV4 )
    {
        *ptrU = *ptrY + stride*height;
        *ptrV = *ptrU + stride*height;
        *shiftX = *shiftY = 0;
        is_yuv = 1;
    }
    else if( format == VX_DF_IMAGE_IYUV )
    {
        *ptrU = *ptrY + stride*height;
        *ptrV = *ptrU + (stride*height)/4;
        *strideC = stride/2;
        *shiftX = *shiftY = 1;
        is_yuv = 1;
    }
    else if( format == VX_DF_IMAGE_NV12 || format == VX_DF_IMAGE_NV21 )
    {
        if( format == VX_DF_IMAGE_NV12 )
        {
            *ptrU = *ptrY + stride*height;
            *ptrV = *ptrU + 1;
        }
        else
        {
            *ptrV = *ptrY + stride*height;
            *ptrU = *ptrV + 1;
        }
        *deltaC = 2;
        *shiftX = *shiftY = 1;
        is_yuv = 1;
    }
    else if( format == VX_DF_IMAGE_YUYV || format == VX_DF_IMAGE_UYVY )
    {
        if( format == VX_DF_IMAGE_YUYV )
        {
            *ptrU = *ptrY + 1;
        }
        else
        {
            *ptrU = *ptrY;
            *ptrY = *ptrU + 1;
        }
        *ptrV = *ptrU + 2;
        *deltaY = 2;
        *deltaC = 4;
        *shiftX = 1;
        *shiftY = 0;
        is_yuv = 1;
    }
    *code = *shiftX == 0 ? 444 : *shiftY == 0 ? 422 : 420;
    return is_yuv;
}

static void rgb2yuv_bt709(uint8_t r, uint8_t g, uint8_t b, uint8_t* y, uint8_t* u, uint8_t* v)
{
    int yval = (int)(r*0.2126f + g*0.7152f + b*0.0722f + 0.5f);
    int uval = (int)(-r*0.1146f - g*0.3854 + b*0.5f + 128.5f);
    int vval = (int)(r*0.5f - g*0.4542f - b*0.0458f + 128.5f);
    *y = CT_CAST_U8(yval);
    *u = CT_CAST_U8(uval);
    *v = CT_CAST_U8(vval);
}

static void yuv2rgb_bt709(uint8_t y, uint8_t u, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b)
{
    int rval = (int)(y + 1.5748f*(v-128) + 0.5f);
    int gval = (int)(y - 0.1873f*(u-128) - 0.4681f*(v-128) + 0.5f);
    int bval = (int)(y + 1.8556f*(u-128) + 0.5f);
    *r = CT_CAST_U8(rval);
    *g = CT_CAST_U8(gval);
    *b = CT_CAST_U8(bval);
}

static void reference_colorconvert(CT_Image src, CT_Image dst)
{
    uint32_t x, y, width, height, srcstride, dststride;
    int srcformat = src->format;
    int dstformat = dst->format;
    uint8_t *srcptrY=0, *srcptrU=0, *srcptrV=0;
    uint8_t *dstptrY=0, *dstptrU=0, *dstptrV=0;
    uint32_t srcstrideY=0, srcdeltaY=1, srcstrideC=0, srcdeltaC=1;
    uint32_t dststrideY=0, dstdeltaY=1, dststrideC=0, dstdeltaC=1;
    uint32_t srcshiftX = 1, srcshiftY = 1;
    uint32_t dstshiftX = 1, dstshiftY = 1;
    int srcYUV, dstYUV;
    int srccode=0, dstcode=0;

    ASSERT(src && dst);
    ASSERT(src->width > 0 && src->height > 0 &&
           src->width == dst->width && src->height == dst->height);

    width = src->width;
    height = src->height;
    srcstride = ct_stride_bytes(src);
    dststride = ct_stride_bytes(dst);

    srcYUV = get_yuv_params(src, &srcptrY, &srcptrU, &srcptrV, &srcstrideY,
                            &srcdeltaY, &srcstrideC, &srcdeltaC,
                            &srcshiftX, &srcshiftY, &srccode);
    dstYUV = get_yuv_params(dst, &dstptrY, &dstptrU, &dstptrV, &dststrideY,
                            &dstdeltaY, &dststrideC, &dstdeltaC,
                            &dstshiftX, &dstshiftY, &dstcode);

    if( srcformat == VX_DF_IMAGE_RGB || srcformat == VX_DF_IMAGE_RGBX )
    {
        int scn = ct_channels(srcformat);
        if( dstformat == VX_DF_IMAGE_RGB || dstformat == VX_DF_IMAGE_RGBX )
        {
            int dcn = ct_channels(dstformat);

            for( y = 0; y < height; y++ )
            {
                const uint8_t* srcptr = (const uint8_t*)(src->data.y + y*srcstride);
                uint8_t* dstptr = (uint8_t*)(dst->data.y + y*dststride);
                for( x = 0; x < width; x++, srcptr += scn, dstptr += dcn )
                {
                    dstptr[0] = srcptr[0];
                    dstptr[1] = srcptr[1];
                    dstptr[2] = srcptr[2];
                    if(dcn == 4)
                        dstptr[3] = 255;
                }
            }

        }
        else if( dstYUV )
        {
            if( dstcode == 444 )
            {
                for( y = 0; y < height; y++ )
                {
                    const uint8_t* srcptr = (const uint8_t*)(src->data.y + y*srcstride);
                    for( x = 0; x < width; x++, srcptr += scn )
                    {
                        rgb2yuv_bt709(srcptr[0], srcptr[1], srcptr[2],
                                      dstptrY + dststrideY*y + dstdeltaY*x,
                                      dstptrU + dststrideC*y + dstdeltaC*x,
                                      dstptrV + dststrideC*y + dstdeltaC*x);
                    }
                }
            }
            else if( dstcode == 422 )
            {
                for( y = 0; y < height; y++ )
                {
                    const uint8_t* srcptr = (const uint8_t*)(src->data.y + y*srcstride);
                    for( x = 0; x < width; x += 2, srcptr += scn*2 )
                    {
                        uint8_t u0 = 0, v0 = 0, u1 = 0, v1 = 0;
                        rgb2yuv_bt709(srcptr[0], srcptr[1], srcptr[2],
                                      dstptrY + dststrideY*y + dstdeltaY*x, &u0, &v0);
                        rgb2yuv_bt709(srcptr[scn], srcptr[scn+1], srcptr[scn+2],
                                      dstptrY + dststrideY*y + dstdeltaY*(x+1), &u1, &v1);
                        dstptrU[dststrideC*y + dstdeltaC*(x/2)] = (uint8_t)((u0 + u1) >> 1);
                        dstptrV[dststrideC*y + dstdeltaC*(x/2)] = (uint8_t)((v0 + v1) >> 1);
                    }
                }
            }
            else if( dstcode == 420 )
            {
                for( y = 0; y < height; y += 2 )
                {
                    const uint8_t* srcptr = (const uint8_t*)(src->data.y + y*srcstride);
                    for( x = 0; x < width; x += 2, srcptr += scn*2 )
                    {
                        uint8_t u[4], v[4];
                        rgb2yuv_bt709(srcptr[0], srcptr[1], srcptr[2],
                                      dstptrY + dststrideY*y + dstdeltaY*x, &u[0], &v[0]);
                        rgb2yuv_bt709(srcptr[scn], srcptr[scn+1], srcptr[scn+2],
                                      dstptrY + dststrideY*y + dstdeltaY*(x+1), &u[1], &v[1]);
                        rgb2yuv_bt709(srcptr[srcstride+0], srcptr[srcstride+1], srcptr[srcstride+2],
                                      dstptrY + dststrideY*(y+1) + dstdeltaY*x, &u[2], &v[2]);
                        rgb2yuv_bt709(srcptr[srcstride+scn], srcptr[srcstride+scn+1], srcptr[srcstride+scn+2],
                                      dstptrY + dststrideY*(y+1) + dstdeltaY*(x+1), &u[3], &v[3]);
                        dstptrU[dststrideC*(y/2) + dstdeltaC*(x/2)] = (uint8_t)((u[0] + u[1] + u[2] + u[3]) >> 2);
                        dstptrV[dststrideC*(y/2) + dstdeltaC*(x/2)] = (uint8_t)((v[0] + v[1] + v[2] + v[3]) >> 2);
                    }
                }
            }
        }
    }
    else if( srcYUV )
    {
        if( dstformat == VX_DF_IMAGE_RGB || dstformat == VX_DF_IMAGE_RGBX )
        {
            int dcn = ct_channels(dstformat);

            for( y = 0; y < height; y++ )
            {
                uint8_t* dstptr = (uint8_t*)(dst->data.y + y*dststride);
                for( x = 0; x < width; x++, dstptr += dcn )
                {
                    int xc = x >> srcshiftX, yc = y >> srcshiftY;
                    yuv2rgb_bt709(srcptrY[srcstrideY*y + srcdeltaY*x],
                                  srcptrU[srcstrideC*yc + srcdeltaC*xc],
                                  srcptrV[srcstrideC*yc + srcdeltaC*xc],
                                  dstptr, dstptr + 1, dstptr + 2);
                    if( dcn == 4 )
                        dstptr[3] = 255;
                }
            }
        }
        else if( dstYUV )
        {
            if( srccode <= dstcode )
            {
                // if both src and dst are YUV formats and
                // the source image chroma resolution
                // is smaller then we just replicate the chroma components
                for( y = 0; y < height; y++ )
                {
                    for( x = 0; x < width; x++ )
                    {
                        int dstYC = y >> dstshiftY, dstXC = x >> dstshiftX;
                        int srcYC = y >> srcshiftY, srcXC = x >> srcshiftX;
                        dstptrY[dststrideY*y + dstdeltaY*x] = srcptrY[srcstrideY*y + srcdeltaY*x];
                        dstptrU[dststrideC*dstYC + dstdeltaC*dstXC] = srcptrU[srcstrideC*srcYC + srcdeltaC*srcXC];
                        dstptrV[dststrideC*dstYC + dstdeltaC*dstXC] = srcptrV[srcstrideC*srcYC + srcdeltaC*srcXC];
                    }
                }
            }
            else if( srccode == 422 && dstcode == 420 )
            {
                // if both src and dst are YUV formats and
                // the source image chroma resolution
                // is larger then we have to average chroma samples
                for( y = 0; y < height; y += 2 )
                {
                    for( x = 0; x < width; x++ )
                    {
                        int dstYC = y >> dstshiftY, dstXC = x >> dstshiftX;
                        int srcYC = y >> srcshiftY, srcXC = x >> srcshiftX;
                        dstptrY[dststrideY*y + dstdeltaY*x] = srcptrY[srcstrideY*y + srcdeltaY*x];
                        dstptrY[dststrideY*(y+1) + dstdeltaY*x] = srcptrY[srcstrideY*(y+1) + srcdeltaY*x];

                        dstptrU[dststrideC*dstYC + dstdeltaC*dstXC] =
                            (uint8_t)((srcptrU[srcstrideC*srcYC + srcdeltaC*srcXC] +
                                       srcptrU[srcstrideC*(srcYC+1) + srcdeltaC*srcXC]) >> 1);

                        dstptrV[dststrideC*dstYC + dstdeltaC*dstXC] =
                            (uint8_t)((srcptrV[srcstrideC*srcYC + srcdeltaC*srcXC] +
                                       srcptrV[srcstrideC*(srcYC+1) + srcdeltaC*srcXC]) >> 1);
                    }
                }
            }
        }
    }
}

static void reference_sequential_colorconvert(CT_Image src, CT_Image virt, CT_Image dst)
{
    reference_colorconvert(src, virt);
    reference_colorconvert(virt, dst);
}


static int cmp_color_images(CT_Image img0, CT_Image img1, int ythresh, int cthresh)
{
    uint32_t x, y, width, height, stride0, stride1;
    int format0 = img0->format;
    int format1 = img1->format;
    uint8_t *ptrY0=0, *ptrU0=0, *ptrV0=0;
    uint8_t *ptrY1=0, *ptrU1=0, *ptrV1=0;
    uint32_t strideY0=0, deltaY0=1, strideC0=0, deltaC0=1;
    uint32_t strideY1=0, deltaY1=1, strideC1=0, deltaC1=1;
    uint32_t shiftX0 = 1, shiftY0 = 1;
    uint32_t shiftX1 = 1, shiftY1 = 1;
    int YUV0, YUV1;
    int code0=0, code1=0;

    ASSERT_(return -1, img0 && img1);
    ASSERT_(return -1, img0->width > 0 && img0->height > 0 &&
           img0->width == img1->width && img0->height == img1->height &&
           format0 == format1);

    width = img0->width;
    height = img0->height;
    stride0 = ct_stride_bytes(img0);
    stride1 = ct_stride_bytes(img1);

    YUV0 = get_yuv_params(img0, &ptrY0, &ptrU0, &ptrV0, &strideY0,
                            &deltaY0, &strideC0, &deltaC0,
                            &shiftX0, &shiftY0, &code0);
    YUV1 = get_yuv_params(img1, &ptrY1, &ptrU1, &ptrV1, &strideY1,
                          &deltaY1, &strideC1, &deltaC1,
                          &shiftX1, &shiftY1, &code1);

    if( format0 == VX_DF_IMAGE_RGB || format0 == VX_DF_IMAGE_RGBX )
    {
        int cn = ct_channels(format0);
        for( y = 0; y < height; y++ )
        {
            const uint8_t* ptr0 = (const uint8_t*)(img0->data.y + y*stride0);
            const uint8_t* ptr1 = (const uint8_t*)(img1->data.y + y*stride1);
            for( x = 0; x < width*cn; x++ )
            {
                if( abs(ptr0[x] - ptr1[x]) > ythresh )
                {
                    printf("images are very different at (%d, %d): %d vs %d\n", x, y, ptr0[x], ptr1[x]);
                    return -1;
                }
            }
        }
    }
    else
    {
        ASSERT_(return -1, YUV0 != 0 && YUV1 != 0 && code0 == code1);
        for( y = 0; y < height; y++ )
        {
            const uint8_t* tempptrY0 = (const uint8_t*)(ptrY0 + y*strideY0);
            const uint8_t* tempptrY1 = (const uint8_t*)(ptrY1 + y*strideY1);
            const uint8_t* tempptrU0_row = (const uint8_t*)(ptrU0 + (y>>shiftY0)*strideC0);
            const uint8_t* tempptrU1_row = (const uint8_t*)(ptrU1 + (y>>shiftY1)*strideC1);
            const uint8_t* tempptrV0_row = (const uint8_t*)(ptrV0 + (y>>shiftY0)*strideC0);
            const uint8_t* tempptrV1_row = (const uint8_t*)(ptrV1 + (y>>shiftY1)*strideC1);
            for( x = 0; x < width; x++, tempptrY0 += deltaY0, tempptrY1 += deltaY1 )
            {
                const uint8_t* tempptrU0 = tempptrU0_row + (x >> shiftX0)*deltaC0;
                const uint8_t* tempptrU1 = tempptrU1_row + (x >> shiftX1)*deltaC1;
                const uint8_t* tempptrV0 = tempptrV0_row + (x >> shiftX0)*deltaC0;
                const uint8_t* tempptrV1 = tempptrV1_row + (x >> shiftX1)*deltaC1;

                if( abs(tempptrY0[0] - tempptrY1[0]) > ythresh ||
                    abs(tempptrU0[0] - tempptrU1[0]) > cthresh ||
                    abs(tempptrV0[0] - tempptrV1[0]) > cthresh )
                {
                    printf("images are very different at (%d, %d): (%d, %d, %d) vs (%d, %d, %d)\n",
                           x, y, tempptrY0[0], tempptrU0[0], tempptrV0[0], tempptrY1[0], tempptrU1[0], tempptrV1[0]);
                    return -1;
                }
            }
        }
    }
    return 0;
}

TESTCASE(tivxColorConvert, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    vx_df_image srcformat;
    vx_df_image dstformat;
    int mode;
    int ythresh;
    int cthresh;
} format_arg;

typedef struct {
    const char* name;
    vx_df_image srcformat;
    vx_df_image dstformat;
    vx_enum channel;
    int mode;
    int ythresh;
    int cthresh;
} new_format_arg;

#define CVT_CASE_(imm, from, to, ythresh, cthresh) \
    {#imm "/" #from "=>" #to, VX_DF_IMAGE_##from, VX_DF_IMAGE_##to, CT_##imm##_MODE, ythresh, cthresh}

#define CVT_CASE(from, to, ythresh, cthresh) \
    CVT_CASE_(Graph, from, to, ythresh, cthresh)

#define CVTT_CASE_(imm, from, to, channel) \
    {#imm "/" #from "=>" #to "/" #channel, VX_DF_IMAGE_##from, VX_DF_IMAGE_##to, channel, CT_##imm##_MODE}

#define CVTT_CASE(from, to, channel) \
    CVTT_CASE_(Graph, from, to, channel)

TEST_WITH_ARG(tivxColorConvert, testOnRandomAndNatural, format_arg,
              CVT_CASE(RGB, RGBX, 0, 0),
              CVT_CASE(RGB, NV12, 2, 2),
              CVT_CASE(RGB, IYUV, 1, 1),

              CVT_CASE(NV12, RGB, 1, 1),
              CVT_CASE(NV12, RGBX, 1, 1),
              CVT_CASE(NV12, IYUV, 0, 0),

              CVT_CASE(IYUV, RGB, 1, 1),
              CVT_CASE(IYUV, RGBX, 1, 1),
              CVT_CASE(IYUV, NV12, 0, 0),
              )
{
    int srcformat = arg_->srcformat;
    int dstformat = arg_->dstformat;
    int ythresh = arg_->ythresh;
    int cthresh = arg_->cthresh;
    int mode = arg_->mode;
    vx_image src=0, dst=0, virt=0;
    CT_Image src0, dst0, dst1, virt_ctimage, virt_orig;
    vx_image src_graph2=0, dst_graph2=0, virt_graph2=0;
    CT_Image dst1_graph2;
    vx_graph graph1 = 0, graph2 = 0;
    vx_node node1 = 0, node2 = 0;
    vx_node node1_graph2 = 0, node2_graph2 = 0;
    vx_perf_t perf_node1, perf_node2, perf_node1_graph2, perf_node2_graph2, perf_graph1, perf_graph2;
    vx_context context = context_->vx_context_;
    int iter, niters = 3;
    uint64_t rng;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    rng = CT()->seed_;

    for( iter = 0; iter < niters; iter++ )
    {
        int width = ct_roundf(ct_log_rng(&rng, 0, 10));
        int height = ct_roundf(ct_log_rng(&rng, 0, 10));
        vx_enum range = VX_CHANNEL_RANGE_FULL;
        vx_enum space = VX_COLOR_SPACE_BT709;

        width = CT_MAX((width+1)&-2, 2);
        height = CT_MAX((height+1)&-2, 2);

        if( !ct_check_any_size() )
        {
            width = CT_MIN((width + 7) & -8, 640);
            height = CT_MIN((height + 7) & -8, 480);
        }

        if( srcformat == VX_DF_IMAGE_RGB || srcformat == VX_DF_IMAGE_RGBX )
        {
            int scn = srcformat == VX_DF_IMAGE_RGB ? 3 : 4;
            if( iter == 0 )
            {
                ASSERT_NO_FAILURE(src0 = ct_read_image("lena.bmp", scn));
                width = src0->width;
                height = src0->height;
            }
            else if( iter == 1 )
            {
                ASSERT_NO_FAILURE(src0 = ct_read_image("colors.bmp", scn));
                width = src0->width;
                height = src0->height;
            }
            else
            {
                ASSERT_NO_FAILURE(src0 = ct_allocate_ct_image_random(width, height, srcformat, &rng, 0, 256));
            }
        }
        else
        {
            ASSERT_NO_FAILURE(src0 = ct_allocate_ct_image_random(width, height, srcformat, &rng, 0, 256));
        }
        ASSERT_NO_FAILURE(src = ct_image_to_vx_image(src0, context));
        ASSERT_VX_OBJECT(src, VX_TYPE_IMAGE);
        ASSERT_NO_FAILURE(src_graph2 = ct_image_to_vx_image(src0, context));
        ASSERT_VX_OBJECT(src_graph2, VX_TYPE_IMAGE);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(src, VX_IMAGE_SPACE, &space, sizeof(space)));

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(src_graph2, VX_IMAGE_SPACE, &space, sizeof(space)));

        ASSERT_VX_OBJECT(dst = vxCreateImage(context, width, height, srcformat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst, VX_TYPE_IMAGE);

        ASSERT_VX_OBJECT(dst_graph2 = vxCreateImage(context, width, height, srcformat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(dst_graph2, VX_TYPE_IMAGE);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(dst, VX_IMAGE_SPACE, &space, sizeof(space)));

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(dst_graph2, VX_IMAGE_SPACE, &space, sizeof(space)));

        ASSERT_VX_OBJECT(graph1 = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(graph1, VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(graph2 = vxCreateGraph(context), VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(graph2, VX_TYPE_GRAPH);
        ASSERT_VX_OBJECT(virt   = vxCreateImage(context, width, height, dstformat), VX_TYPE_IMAGE);
        ASSERT_VX_OBJECT(virt_graph2   = vxCreateVirtualImage(graph2, width, height, dstformat), VX_TYPE_IMAGE);

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(virt, VX_IMAGE_SPACE, &space, sizeof(space)));

        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(virt_graph2, VX_IMAGE_SPACE, &space, sizeof(space)));

        ASSERT_VX_OBJECT(node1 = vxColorConvertNode(graph1, src, virt), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node1, VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2 = vxColorConvertNode(graph1, virt, dst), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2, VX_TYPE_NODE);

        ASSERT_VX_OBJECT(node1_graph2 = vxColorConvertNode(graph2, src_graph2, virt_graph2), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node1_graph2, VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2_graph2 = vxColorConvertNode(graph2, virt_graph2, dst_graph2), VX_TYPE_NODE);
        ASSERT_VX_OBJECT(node2_graph2, VX_TYPE_NODE);

        VX_CALL(vxVerifyGraph(graph1));
        VX_CALL(vxProcessGraph(graph1));

        VX_CALL(vxVerifyGraph(graph2));
        VX_CALL(vxProcessGraph(graph2));

        vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
        ASSERT_EQ_INT(valid_rect, vx_false_e);

        vxGetValidRegionImage(src, &src_rect);
        vxGetValidRegionImage(dst, &dst_rect);

        ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), width);
        ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), height);

        ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), width);
        ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), height);

        vxQueryNode(node1, VX_NODE_PERFORMANCE, &perf_node1, sizeof(perf_node1));
        vxQueryNode(node2, VX_NODE_PERFORMANCE, &perf_node2, sizeof(perf_node2));
        vxQueryGraph(graph1, VX_GRAPH_PERFORMANCE, &perf_graph1, sizeof(perf_graph1));
        vxQueryNode(node1_graph2, VX_NODE_PERFORMANCE, &perf_node1_graph2, sizeof(perf_node1_graph2));
        vxQueryNode(node2_graph2, VX_NODE_PERFORMANCE, &perf_node2_graph2, sizeof(perf_node2_graph2));
        vxQueryGraph(graph2, VX_GRAPH_PERFORMANCE, &perf_graph2, sizeof(perf_graph2));

        dst1 = ct_image_from_vx_image(dst);
        dst1_graph2 = ct_image_from_vx_image(dst_graph2);

        virt_ctimage = ct_allocate_image(width, height, dstformat);
        ASSERT_NO_FAILURE(dst0 = ct_allocate_image(width, height, srcformat));
        virt_orig = ct_image_from_vx_image(virt);
        reference_colorconvert(src0, virt_ctimage);
        reference_colorconvert(virt_orig, dst0);

        ASSERT(cmp_color_images(virt_orig, virt_ctimage, ythresh, cthresh) >= 0);
        ASSERT(cmp_color_images(dst0, dst1, ythresh, cthresh) >= 0);
        ASSERT(cmp_color_images(dst1_graph2, dst1, ythresh, cthresh) >= 0);

        VX_CALL(vxReleaseImage(&src));
        VX_CALL(vxReleaseImage(&src_graph2));
        VX_CALL(vxReleaseImage(&virt));
        VX_CALL(vxReleaseImage(&virt_graph2));
        VX_CALL(vxReleaseImage(&dst));
        VX_CALL(vxReleaseImage(&dst_graph2));

        VX_CALL(vxReleaseNode(&node1));
        VX_CALL(vxReleaseNode(&node2));
        VX_CALL(vxReleaseNode(&node1_graph2));
        VX_CALL(vxReleaseNode(&node2_graph2));
        VX_CALL(vxReleaseGraph(&graph1));
        VX_CALL(vxReleaseGraph(&graph2));

        ASSERT(node1 == 0 && node2 == 0 && graph1 == 0);
        ASSERT(node1_graph2 == 0 && node2_graph2 == 0 && graph2 == 0);
        CT_CollectGarbage(CT_GC_IMAGE);

        printPerformance(perf_node1, width*height, "N1");
        printPerformance(perf_node2, width*height, "N2");
        printPerformance(perf_graph1, width*height, "G1");
        printPerformance(perf_node1_graph2, width*height, "N1");
        printPerformance(perf_node2_graph2, width*height, "N2");
        printPerformance(perf_graph2, width*height, "G2");
    }
}


TEST_WITH_ARG(tivxColorConvert, testColConvertSupernode, format_arg,
              CVT_CASE(RGB, RGBX, 0, 0),
              CVT_CASE(RGB, NV12, 3, 3),
              CVT_CASE(RGB, IYUV, 3, 3),

              CVT_CASE(NV12, RGB, 2, 2),
              CVT_CASE(NV12, RGBX, 2, 2),
              CVT_CASE(NV12, IYUV, 0, 0),

              CVT_CASE(IYUV, RGB, 2, 2),
              CVT_CASE(IYUV, RGBX, 2, 2),
              CVT_CASE(IYUV, NV12, 0, 0),
              )
{
    int node_count = 2;
    int srcformat = arg_->srcformat;
    int dstformat = arg_->dstformat;
    int ythresh = arg_->ythresh;
    int cthresh = arg_->cthresh;
    int mode = arg_->mode;
    vx_image src=0, dst=0, virt=0;
    CT_Image ref_src, ref_dst, vxdst, ref_virt;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_context context = context_->vx_context_;
    uint64_t rng;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    rng = CT()->seed_;

    int width = ct_roundf(ct_log_rng(&rng, 0, 10));
    int height = ct_roundf(ct_log_rng(&rng, 0, 10));
    vx_enum range = VX_CHANNEL_RANGE_FULL;
    vx_enum space = VX_COLOR_SPACE_BT709;

    width = CT_MAX((width+1)&-2, 2);
    height = CT_MAX((height+1)&-2, 2);

    if( !ct_check_any_size() )
    {
        width = CT_MIN((width + 7) & -8, 640);
        height = CT_MIN((height + 7) & -8, 480);
    }

    width = 640;
    height = 480;

    if( srcformat == VX_DF_IMAGE_RGB || srcformat == VX_DF_IMAGE_RGBX )
    {
        int scn = srcformat == VX_DF_IMAGE_RGB ? 3 : 4;

        ASSERT_NO_FAILURE(ref_src = ct_read_image("colors.bmp", scn));
        width = ref_src->width;
        height = ref_src->height;
    }
    else
    {
        ASSERT_NO_FAILURE(ref_src = ct_allocate_ct_image_random(width, height, srcformat, &rng, 0, 256));
    }

    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(ref_src, context));
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, width, height, srcformat), VX_TYPE_IMAGE);
    ASSERT_VX_OBJECT(virt   = vxCreateImage(context, width, height, dstformat), VX_TYPE_IMAGE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(src, VX_IMAGE_SPACE, &space, sizeof(space)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(dst, VX_IMAGE_SPACE, &space, sizeof(space)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(virt, VX_IMAGE_SPACE, &space, sizeof(space)));
    
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxColorConvertNode(graph, src, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxColorConvertNode(graph, virt, dst), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src, &src_rect);
    vxGetValidRegionImage(dst, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), width);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), height);

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    vxdst = ct_image_from_vx_image(dst);

    ASSERT_NO_FAILURE(ref_virt = ct_allocate_image(width, height, dstformat));
    ASSERT_NO_FAILURE(ref_dst = ct_allocate_image(width, height, srcformat));
    reference_sequential_colorconvert(ref_src, ref_virt, ref_dst);

    ASSERT(cmp_color_images(ref_dst, vxdst, ythresh, cthresh) >= 0);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0 && node2 == 0 && super_node == 0 && graph == 0);
    CT_CollectGarbage(CT_GC_IMAGE);

    printPerformance(perf_super_node, width * height, "SN");
    printPerformance(perf_graph, width*height, "G");
}

TEST_WITH_ARG(tivxColorConvert, testColConvertSupernodeSingleNode, format_arg,
              CVT_CASE(RGB, YUV4, 1, 1),
              CVT_CASE(RGB, RGBX, 0, 0),
              CVT_CASE(RGB, NV12, 2, 2),
              CVT_CASE(RGB, IYUV, 1, 1),

              CVT_CASE(RGBX, RGB, 0, 0),
              CVT_CASE(RGBX, NV12, 1, 1),
              CVT_CASE(RGBX, IYUV, 1, 1),
              CVT_CASE(RGBX, YUV4, 1, 1),

              CVT_CASE(NV12, YUV4, 1, 1),
              CVT_CASE(NV12, RGB, 1, 1),
              CVT_CASE(NV12, RGBX, 1, 1),
              CVT_CASE(NV12, IYUV, 0, 0),

              CVT_CASE(NV21, RGB, 1, 1),
              CVT_CASE(NV21, RGBX, 1, 1),
              CVT_CASE(NV21, IYUV, 0, 0),
              CVT_CASE(NV21, YUV4, 0, 0),

              CVT_CASE(IYUV, YUV4, 1, 1),
              CVT_CASE(IYUV, RGB, 1, 1),
              CVT_CASE(IYUV, RGBX, 1, 1),
              CVT_CASE(IYUV, NV12, 0, 0),

              CVT_CASE(UYVY, RGB, 1, 1),
              CVT_CASE(UYVY, RGBX, 1, 1),
              CVT_CASE(UYVY, NV12, 0, 0),
              CVT_CASE(UYVY, IYUV, 0, 0),

              CVT_CASE(YUYV, RGB, 1, 1),
              CVT_CASE(YUYV, RGBX, 1, 1),
              CVT_CASE(YUYV, NV12, 0, 0),
              CVT_CASE(YUYV, IYUV, 0, 0),
              )
{
    int node_count = 1;
    int srcformat = arg_->srcformat;
    int dstformat = arg_->dstformat;
    int ythresh = arg_->ythresh;
    int cthresh = arg_->cthresh;
    vx_image src=0, dst=0;
    CT_Image ref_src, ref_dst, vxdst;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_context context = context_->vx_context_;
    uint64_t rng;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    rng = CT()->seed_;

    int width = ct_roundf(ct_log_rng(&rng, 0, 10));
    int height = ct_roundf(ct_log_rng(&rng, 0, 10));
    vx_enum range = VX_CHANNEL_RANGE_FULL;
    vx_enum space = VX_COLOR_SPACE_BT709;

    width = CT_MAX((width+1)&-2, 2);
    height = CT_MAX((height+1)&-2, 2);

    if( !ct_check_any_size() )
    {
        width = CT_MIN((width + 7) & -8, 640);
        height = CT_MIN((height + 7) & -8, 480);
    }

    width = 640;
    height = 480;

    if( srcformat == VX_DF_IMAGE_RGB || srcformat == VX_DF_IMAGE_RGBX )
    {
        int scn = srcformat == VX_DF_IMAGE_RGB ? 3 : 4;

        ASSERT_NO_FAILURE(ref_src = ct_read_image("colors.bmp", scn));
        width = ref_src->width;
        height = ref_src->height;
    }
    else
    {
        ASSERT_NO_FAILURE(ref_src = ct_allocate_ct_image_random(width, height, srcformat, &rng, 0, 256));
    }

    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(ref_src, context));
    ASSERT_VX_OBJECT(dst = vxCreateImage(context, width, height, dstformat), VX_TYPE_IMAGE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(src, VX_IMAGE_SPACE, &space, sizeof(space)));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(dst, VX_IMAGE_SPACE, &space, sizeof(space)));
    
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node = vxColorConvertNode(graph, src, dst), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(node_list[0] = node); 
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src, &src_rect);
    vxGetValidRegionImage(dst, &dst_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), height);

    ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), width);
    ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), height);

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    vxdst = ct_image_from_vx_image(dst);

    ASSERT_NO_FAILURE(ref_dst = ct_allocate_image(width, height, dstformat));
    reference_colorconvert(ref_src, ref_dst);

    ASSERT(cmp_color_images(ref_dst, vxdst, ythresh, cthresh) >= 0);

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&dst));

    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node == 0 && super_node == 0 && graph == 0);
    CT_CollectGarbage(CT_GC_IMAGE);

    printPerformance(perf_super_node, width * height, "SN");
    printPerformance(perf_graph, width*height, "G");
}

TEST_WITH_ARG(tivxColorConvert, testColConvertChannelExtractSupernode, new_format_arg,
              CVTT_CASE(RGB, RGBX, VX_CHANNEL_A),
              CVTT_CASE(RGB, RGBX, VX_CHANNEL_R),
              CVTT_CASE(RGB, NV12, VX_CHANNEL_Y),
              CVTT_CASE(RGB, NV12, VX_CHANNEL_U),
              CVTT_CASE(RGB, NV12, VX_CHANNEL_V),
              CVTT_CASE(RGB, IYUV, VX_CHANNEL_Y),
              CVTT_CASE(RGB, IYUV, VX_CHANNEL_U),
              CVTT_CASE(RGB, YUV4, VX_CHANNEL_Y),
              CVTT_CASE(RGB, YUV4, VX_CHANNEL_V),

              CVTT_CASE(RGBX, RGB, VX_CHANNEL_R),
              CVTT_CASE(RGBX, RGB, VX_CHANNEL_B),
              CVTT_CASE(RGBX, NV12, VX_CHANNEL_Y),
              CVTT_CASE(RGBX, NV12, VX_CHANNEL_U),
              CVTT_CASE(RGBX, NV12, VX_CHANNEL_V),
              CVTT_CASE(RGBX, IYUV, VX_CHANNEL_Y),
              CVTT_CASE(RGBX, IYUV, VX_CHANNEL_U),
              CVTT_CASE(RGBX, YUV4, VX_CHANNEL_Y),
              CVTT_CASE(RGBX, YUV4, VX_CHANNEL_V),

              CVTT_CASE(NV12, RGB, VX_CHANNEL_G),
              CVTT_CASE(NV12, RGB, VX_CHANNEL_B),
              CVTT_CASE(NV12, RGBX, VX_CHANNEL_R),
              CVTT_CASE(NV12, RGBX, VX_CHANNEL_A),
              CVTT_CASE(NV12, IYUV, VX_CHANNEL_Y),
              CVTT_CASE(NV12, IYUV, VX_CHANNEL_V),
              CVTT_CASE(NV12, YUV4, VX_CHANNEL_Y),
              CVTT_CASE(NV12, YUV4, VX_CHANNEL_U),

              CVTT_CASE(IYUV, RGB, VX_CHANNEL_R),
              CVTT_CASE(IYUV, RGB, VX_CHANNEL_G),
              CVTT_CASE(IYUV, RGBX, VX_CHANNEL_B),
              CVTT_CASE(IYUV, RGBX, VX_CHANNEL_A),
              CVTT_CASE(IYUV, NV12, VX_CHANNEL_Y),
              CVTT_CASE(IYUV, NV12, VX_CHANNEL_U),
              CVTT_CASE(IYUV, NV12, VX_CHANNEL_V),
              CVTT_CASE(IYUV, YUV4, VX_CHANNEL_Y),
              CVTT_CASE(IYUV, YUV4, VX_CHANNEL_V),
              )
{
    int node_count = 2;
    int srcformat = arg_->srcformat;
    int virtformat = arg_->dstformat;
    int mode = arg_->mode;
    vx_image src=0, dst=0, virt=0;
    CT_Image ref_src, ref_dst, vxdst, vxvirt, ref_virt;
    vx_graph graph = 0;
    vx_node node1 = 0, node2 = 0;
    vx_perf_t perf_super_node, perf_graph;
    tivx_super_node super_node = 0;
    vx_node node_list[MAX_NODES];
    vx_context context = context_->vx_context_;
    uint64_t rng;
    vx_rectangle_t src_rect, dst_rect;
    vx_bool valid_rect;

    rng = CT()->seed_;

    int width = ct_roundf(ct_log_rng(&rng, 0, 10));
    int height = ct_roundf(ct_log_rng(&rng, 0, 10));
    vx_enum range = VX_CHANNEL_RANGE_FULL;
    vx_enum space = VX_COLOR_SPACE_BT709;

    width = CT_MAX((width+1)&-2, 2);
    height = CT_MAX((height+1)&-2, 2);

    if( !ct_check_any_size() )
    {
        width = CT_MIN((width + 7) & -8, 640);
        height = CT_MIN((height + 7) & -8, 480);
    }
    
    width = 640;
    height = 480;

    if( srcformat == VX_DF_IMAGE_RGB || srcformat == VX_DF_IMAGE_RGBX )
    {
        int scn = srcformat == VX_DF_IMAGE_RGB ? 3 : 4;

        ASSERT_NO_FAILURE(ref_src = ct_read_image("colors.bmp", scn));
        width = ref_src->width;
        height = ref_src->height;
    }
    else
    {
        ASSERT_NO_FAILURE(ref_src = ct_allocate_ct_image_random(width, height, srcformat, &rng, 0, 256));
    }
    ASSERT_NO_FAILURE(ref_virt = ct_allocate_image(width, height, virtformat));
    reference_colorconvert(ref_src, ref_virt);
    ASSERT_NO_FAILURE(ref_dst = channel_extract_create_reference_image(ref_virt, arg_->channel));

    ASSERT_NO_FAILURE(src = ct_image_to_vx_image(ref_src, context));
    if (NULL != ref_dst)
    {
        ASSERT_VX_OBJECT(dst = vxCreateImage(context, ref_dst->width, ref_dst->height, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    }
    ASSERT_VX_OBJECT(virt   = vxCreateImage(context, width, height, virtformat), VX_TYPE_IMAGE);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(src, VX_IMAGE_SPACE, &space, sizeof(space)));
    if (NULL != dst)
    {
        ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(dst, VX_IMAGE_SPACE, &space, sizeof(space)));
    }
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxSetImageAttribute(virt, VX_IMAGE_SPACE, &space, sizeof(space)));
    
    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);
    ASSERT_VX_OBJECT(node1 = vxColorConvertNode(graph, src, virt), VX_TYPE_NODE);
    ASSERT_VX_OBJECT(node2 = vxChannelExtractNode(graph, virt, arg_->channel, dst), VX_TYPE_NODE);

    ASSERT_NO_FAILURE(node_list[0] = node1); 
    ASSERT_NO_FAILURE(node_list[1] = node2);
    ASSERT_VX_OBJECT(super_node = tivxCreateSuperNode(graph, node_list, node_count), (enum vx_type_e)TIVX_TYPE_SUPER_NODE);
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxGetStatus((vx_reference)super_node));

    VX_CALL(vxVerifyGraph(graph));
    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxProcessGraph(graph));

    vxQueryNode(node1, VX_NODE_VALID_RECT_RESET, &valid_rect, sizeof(valid_rect));
    ASSERT_EQ_INT(valid_rect, vx_false_e);

    vxGetValidRegionImage(src, &src_rect);

    ASSERT_EQ_INT((src_rect.end_x - src_rect.start_x), width);
    ASSERT_EQ_INT((src_rect.end_y - src_rect.start_y), height);

    if ( (NULL != ref_dst) && (NULL != dst))
    {
        vxGetValidRegionImage(dst, &dst_rect);
        ASSERT_EQ_INT((dst_rect.end_x - dst_rect.start_x), ref_dst->width);
        ASSERT_EQ_INT((dst_rect.end_y - dst_rect.start_y), ref_dst->height);
    }

    VX_CALL(tivxQuerySuperNode(super_node, TIVX_SUPER_NODE_PERFORMANCE, &perf_super_node, sizeof(perf_super_node)));
    VX_CALL(vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(perf_graph)));

    /*vxvirt = ct_image_from_vx_image(virt);*/
    vxdst = ct_image_from_vx_image(dst);

    /*ASSERT(EXPECT_CTIMAGE_NEAR(vxvirt, ref_virt, 1));*/
    ASSERT(EXPECT_CTIMAGE_NEAR(ref_dst, vxdst, 1));

    VX_CALL(vxReleaseImage(&src));
    VX_CALL(vxReleaseImage(&virt));
    VX_CALL(vxReleaseImage(&dst));
    VX_CALL(tivxReleaseSuperNode(&super_node));
    VX_CALL(vxReleaseNode(&node1));
    VX_CALL(vxReleaseNode(&node2));
    VX_CALL(vxReleaseGraph(&graph));

    ASSERT(node1 == 0 && node2 == 0 && super_node == 0 && graph == 0);
    CT_CollectGarbage(CT_GC_IMAGE);

    printPerformance(perf_super_node, width * height, "SN");
    printPerformance(perf_graph, width*height, "G");
}

#ifdef BUILD_BAM
#define testColConvertSupernodeSingleNode testColConvertSupernodeSingleNode
#define testColConvertSupernode testColConvertSupernode
#define testColConvertChannelExtractSupernode testColConvertChannelExtractSupernode
#else
#define testColConvertSupernodeSingleNode DISABLED_testColConvertSupernodeSingleNode
#define testColConvertSupernode DISABLED_testColConvertSupernode
#define testColConvertChannelExtractSupernode DISABLED_testColConvertChannelExtractSupernode
#endif

TESTCASE_TESTS(tivxColorConvert, 
               testOnRandomAndNatural,
               testColConvertSupernodeSingleNode,
               testColConvertSupernode,
               testColConvertChannelExtractSupernode)
