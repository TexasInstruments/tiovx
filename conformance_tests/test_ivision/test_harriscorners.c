#include "test_engine/test.h"
#include <VX/vx.h>
#include <VX/vxu.h>
#include <TI/tivx.h>
#include <TI/tivx_mem.h>
#include <float.h>
#include <math.h>

#include "tivx_harris_corners_test_data.h"

#define GRADIENT_SIZE       (7u)
#define BLOCK_SIZE          (7u)

#define IMG_WIDTH           (400u)
#define IMG_HEIGHT          (400u)

#define STRENGTH_THRESHOLD  (286870912)

vx_uint8 gTivxHcTestInput[TIVX_HC_TEST_INPUT_NUM_ELEM] = TIVX_HC_TEST_INPUT_CFG;

vx_int16 ReferenceOutput[TIVX_HC_NUM_REF_KEY_POINTS][2u] =
    TIVX_HC_TEST_REFERENCE_OUTPUT;

static void CheckOutput(vx_array arr)
{
    vx_status status;
    vx_size num_items;
    vx_uint32 cnt;
    vx_map_id map_id;
    vx_size stride = sizeof(vx_keypoint_t);
    void *base = NULL;
    vx_int16 x, y, num_missmatch = 0;

    status = vxQueryArray (arr, VX_ARRAY_NUMITEMS, &num_items, sizeof(num_items));
    if (VX_SUCCESS == status)
    {
        if (num_items != TIVX_HC_NUM_REF_KEY_POINTS)
        {
            printf(
                "HarrisCorners: incorrect number of detected corners!!!\n");
        }
        else
        {
            status = vxMapArrayRange(arr, 0, num_items, &map_id,
                &stride, &base, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
            if (VX_SUCCESS == status)
            {

                for (cnt = 0u; cnt < num_items; cnt ++)
                {
                    x = vxArrayItem(vx_keypoint_t, base, cnt, stride).x;
                    y = vxArrayItem(vx_keypoint_t, base, cnt, stride).y;

                    if (x != ReferenceOutput[cnt][0] ||
                        y != ReferenceOutput[cnt][1])
                    {
                        printf(
                            "HarrisCorners: Output does not match\n");
                        printf(
                            "HarrisCorners: x = %d y = %d\n",
                            ReferenceOutput[cnt][0],
                            ReferenceOutput[cnt][1]);
                        num_missmatch ++;
                    }
                }

                vxUnmapArrayRange(arr, map_id);
            }
        }
    }
    else
    {
        printf( "HarrisCorners: Cannot get array size !!!\n");
    }

    if (0 == num_missmatch)
    {
        printf( "HarrisCorners: Output matches with the reference output \n");
        printf( "HarrisCorners: Test Success !!!\n");
    }
}

void test_harriscorners()
{
    vx_context context = vxCreateContext();;

    vx_image input_image = 0;
    vx_size num_corners;
    vx_graph graph = 0;
    vx_node node = 0;
    vx_imagepatch_addressing_t addrs;
    void *image = 0;
    vx_scalar num_corners_scalar;
    vx_array corners;

    image = tivxMemAlloc(IMG_WIDTH*IMG_HEIGHT, TIVX_MEM_EXTERNAL);
    if(image)
    {
        /* Copy input image to allocated buffer */
        memcpy(image, gTivxHcTestInput, IMG_WIDTH*IMG_HEIGHT);
    }
    else
    {
        printf("HarrisCorners: Cannot allocate memory !!!\n");
        return;
    }

    addrs.dim_x = IMG_WIDTH;
    addrs.dim_y = IMG_HEIGHT;
    addrs.stride_x = 1;
    addrs.stride_y = IMG_WIDTH;
    addrs.step_x = 1;
    addrs.step_y = 1;
    ASSERT_VX_OBJECT(input_image = vxCreateImageFromHandle(context,
        VX_DF_IMAGE_U8, &addrs, &image, VX_MEMORY_TYPE_HOST), VX_TYPE_IMAGE);

    num_corners = 1023;

    ASSERT_VX_OBJECT(num_corners_scalar = vxCreateScalar(context,
        VX_TYPE_SIZE, &num_corners), VX_TYPE_SCALAR);

    ASSERT_VX_OBJECT(corners = vxCreateArray(context, VX_TYPE_KEYPOINT,
        num_corners), VX_TYPE_ARRAY);

    ASSERT_VX_OBJECT(graph = vxCreateGraph(context), VX_TYPE_GRAPH);

    ASSERT_VX_OBJECT(node = tivxHarrisCornersNode(graph,
        input_image, 1310, STRENGTH_THRESHOLD, 2, 7, 0, 7,
        corners, num_corners_scalar), VX_TYPE_NODE);

    VX_CALL(vxSetNodeTarget(node, VX_TARGET_STRING, TIVX_TARGET_EVE1));
    VX_CALL(vxVerifyGraph(graph));
    VX_CALL(vxProcessGraph(graph));

    CheckOutput(corners);

    VX_CALL(vxReleaseNode(&node));
    VX_CALL(vxReleaseGraph(&graph));
    ASSERT(node == 0);
    ASSERT(graph == 0);

    VX_CALL(vxReleaseArray(&corners));
    VX_CALL(vxReleaseImage(&input_image));
    VX_CALL(vxReleaseScalar(&num_corners_scalar));

    if (image)
    {
        tivxMemFree(image, IMG_WIDTH*IMG_HEIGHT, TIVX_MEM_EXTERNAL);
        image = NULL;
    }

    ASSERT(corners == 0);
    ASSERT(num_corners_scalar == 0);
    ASSERT(input_image == 0);
}

