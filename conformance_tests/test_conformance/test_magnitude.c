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
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "shared_functions.h"

static void reference_mag(CT_Image dx, CT_Image dy, CT_Image mag)
{
    uint32_t x, y, width, height, dxstride, dystride, magstride;

    ASSERT(dx && dy && mag);
    ASSERT(dx->format == VX_DF_IMAGE_S16 && dy->format == VX_DF_IMAGE_S16 && mag->format == VX_DF_IMAGE_S16);
    ASSERT(dx->width > 0 && dx->height > 0 &&
           dx->width == dy->width && dx->height == dy->height &&
           dx->width == mag->width && dx->height == mag->height);
    width = dx->width;
    height = dy->height;
    dxstride = ct_stride_bytes(dx);
    dystride = ct_stride_bytes(dy);
    magstride = ct_stride_bytes(mag);

    for( y = 0; y < height; y++ )
    {
        const int16_t* dxptr = (const int16_t*)(dx->data.y + y*dxstride);
        const int16_t* dyptr = (const int16_t*)(dy->data.y + y*dystride);
        int16_t* magptr = (int16_t*)(mag->data.y + y*magstride);
        for( x = 0; x < width; x++ )
        {
            // the specification says - use double in the test implementation
            double val = sqrt((double)dxptr[x]*dxptr[x] + (double)dyptr[x]*dyptr[x]);
            int ival = (int)floor(val + 0.5);
            magptr[x] = CT_CAST_S16(ival);
        }
    }
}


TESTCASE(Magnitude, CT_VXContext, ct_setup_vx_context, 0)

typedef struct {
    const char* name;
    int mode;
    int use_sobel;
    vx_df_image format;
} format_arg;

#undef UseSobel
#undef Random
#define UseSobel 1
#define Random 0

#define MAG_TEST_CASE(imm, sob, tp) \
    {#imm "/" #sob "/" #tp, CT_##imm##_MODE, sob, VX_DF_IMAGE_##tp}

TEST_WITH_ARG(Magnitude, testOnRandom, format_arg,
              MAG_TEST_CASE(Immediate, UseSobel, S16),
              MAG_TEST_CASE(Immediate, Random, S16),
              MAG_TEST_CASE(Graph, UseSobel, S16),
              MAG_TEST_CASE(Graph, Random, S16),
              )
{
    int dxformat = arg_->format;
    int mode = arg_->mode;
    int use_sobel = arg_->use_sobel;
    int srcformat = dxformat == VX_DF_IMAGE_S16 ? VX_DF_IMAGE_U8 : -1;
    int magformat = dxformat == VX_DF_IMAGE_S16 ? VX_DF_IMAGE_S16 : -1;
    vx_image presrc=0, src=0, dx=0, dy=0, mag=0;
    CT_Image src0, dx0, dy0, mag0, mag1;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_context context = context_->vx_context_;
    int iter, niters = 100;
    uint64_t rng;
    int srcmin = 0, srcmax = 256;
    int dxmin = -32768, dxmax = 32768;
    vx_border_t border;

    ASSERT( srcformat != -1 && magformat != -1 );
    rng = CT()->seed_;
    border.mode = //VX_BORDER_UNDEFINED;
                  VX_BORDER_REPLICATE;

    ASSERT_EQ_VX_STATUS(VX_SUCCESS,
                        vxSetContextAttribute(context, VX_CONTEXT_IMMEDIATE_BORDER,
                                              &border, sizeof(border)));

    for( iter = 0; iter < niters; iter++ )
    {
        int width = ct_roundf(ct_log_rng(&rng, 0, 10));
        int height = ct_roundf(ct_log_rng(&rng, 0, 10));

        width = CT_MAX(width, 1);
        height = CT_MAX(height, 1);

        if( !ct_check_any_size() )
        {
            width = CT_MIN((width + 7) & -8, 640);
            height = CT_MIN((height + 7) & -8, 480);
        }

        ct_update_progress(iter, niters);

        if( use_sobel )
        {
            CT_Image box_img;

            width = CT_MAX(width, 3);
            height = CT_MAX(height, 3);

            ASSERT_NO_FAILURE(src0 = ct_allocate_ct_image_random(width, height, srcformat, &rng, srcmin, srcmax));
            ASSERT_EQ_INT(VX_DF_IMAGE_S16, dxformat);
            ASSERT_NO_FAILURE(box_img = box3x3_create_reference_image(src0, border));
            ASSERT_NO_FAILURE(sobel3x3_create_reference_image(box_img, border, &dx0, &dy0));

            if( border.mode == VX_BORDER_UNDEFINED )
            {
                width -= 2;
                height -= 2;
                ASSERT_NO_FAILURE(ct_adjust_roi(dx0, 1, 1, 1, 1));
                ASSERT_NO_FAILURE(ct_adjust_roi(dy0, 1, 1, 1, 1));
            }

            if (NULL != dx)
                VX_CALL(vxReleaseImage(&dx));

            if (NULL != dy)
                VX_CALL(vxReleaseImage(&dy));
        }
        else
        {
            int k, maxk = CT_RNG_NEXT_INT(rng, 0, 20);
            ASSERT_NO_FAILURE(dx0 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));
            ASSERT_NO_FAILURE(dy0 = ct_allocate_ct_image_random(width, height, dxformat, &rng, dxmin, dxmax));

            // add some extreme points to the generated images
            for( k = 0; k < maxk; k++ )
            {
                int x = CT_RNG_NEXT_INT(rng, 0, width);
                int y = CT_RNG_NEXT_INT(rng, 0, height);
                int dxval = CT_RNG_NEXT_BOOL(rng) ? dxmin : dxmax;
                int dyval = CT_RNG_NEXT_BOOL(rng) ? dxmin : dxmax;
                dx0->data.s16[dx0->stride*y + x] = (short)dxval;
                dy0->data.s16[dy0->stride*y + x] = (short)dyval;
            }
            presrc = src = 0;
        }

        dx = ct_image_to_vx_image(dx0, context);
        ASSERT_VX_OBJECT(dx, VX_TYPE_IMAGE);
        dy = ct_image_to_vx_image(dy0, context);
        ASSERT_VX_OBJECT(dy, VX_TYPE_IMAGE);

        ASSERT_NO_FAILURE(mag0 = ct_allocate_image(width, height, magformat));
        ASSERT_NO_FAILURE(reference_mag(dx0, dy0, mag0));
        mag = vxCreateImage(context, width, height, magformat);
        ASSERT_VX_OBJECT(mag, VX_TYPE_IMAGE);

        if( mode == CT_Immediate_MODE )
        {
            ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxuMagnitude(context, dx, dy, mag));
        }
        else
        {
            graph = vxCreateGraph(context);
            ASSERT_VX_OBJECT(graph, VX_TYPE_GRAPH);
            node = vxMagnitudeNode(graph, dx, dy, mag);
            ASSERT_VX_OBJECT(node, VX_TYPE_NODE);
            VX_CALL(vxVerifyGraph(graph));
            VX_CALL(vxProcessGraph(graph));
        }
        mag1 = ct_image_from_vx_image(mag);

        ASSERT_CTIMAGE_NEAR(mag0, mag1, 1);
        if(presrc)
            VX_CALL(vxReleaseImage(&presrc));
        if(src)
            VX_CALL(vxReleaseImage(&src));
        VX_CALL(vxReleaseImage(&dx));
        VX_CALL(vxReleaseImage(&dy));
        VX_CALL(vxReleaseImage(&mag));
        if(node)
            VX_CALL(vxReleaseNode(&node));
        if(graph)
            VX_CALL(vxReleaseGraph(&graph));
        ASSERT(node == 0 && graph == 0);
        CT_CollectGarbage(CT_GC_IMAGE);
    }
}

TESTCASE_TESTS(Magnitude, testOnRandom)
