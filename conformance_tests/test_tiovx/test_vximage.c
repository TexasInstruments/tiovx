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
#include <TI/tivx_obj_desc.h>
#include <TI/tivx_config.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include <string.h>
#include <math.h>

#include "shared_functions.h"

#define MAX_POINTS 100

TESTCASE(tivxMapImage, CT_VXContext, ct_setup_vx_context, 0)

static void printMapPerformance(uint64_t exe_time, uint32_t numPixels)
{
    printf("[ SYS ] Map time (exe_time = %"PRIu64" us, numPixels = %4d\n",
        exe_time, numPixels
        );
}


TEST(tivxMapImage, testMapImage)
{
    int w = 1024, h = 512;
    vx_df_image f = VX_DF_IMAGE_U8;
    vx_context context = context_->vx_context_;
    vx_image image;
    vx_imagepatch_addressing_t addr;
    vx_uint8 *pdata = 0;
    vx_rectangle_t rect = {0, 0, w, h};
    vx_map_id map_id;
    uint64_t exe_time;

    VX_CALL(vxDirective((vx_reference)context, VX_DIRECTIVE_ENABLE_PERFORMANCE));

    ASSERT_VX_OBJECT(image  = vxCreateImage(context, w, h, f), VX_TYPE_IMAGE);

    exe_time = tivxPlatformGetTimeInUsecs();

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxMapImagePatch(image, &rect, 0, &map_id, &addr, (void **)&pdata,
                                                VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0));

    memset(pdata, 0xFF, addr.stride_y*h);

    ASSERT_EQ_VX_STATUS(VX_SUCCESS, vxUnmapImagePatch(image, map_id));

    exe_time = tivxPlatformGetTimeInUsecs() - exe_time;

    printMapPerformance(exe_time, w*h);

    VX_CALL(vxReleaseImage(&image));
}

TESTCASE_TESTS(tivxMapImage,
        testMapImage
        )
