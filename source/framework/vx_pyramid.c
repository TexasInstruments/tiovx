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



#include <vx_internal.h>
#include <math.h>

static vx_status ownDestructPyramid(vx_reference ref);
static vx_status ownAllocPyramidBuffer(vx_reference ref);
static vx_status ownInitPyramid(vx_pyramid prmd);

static const vx_float32 gOrbScaleFactor
    [TIVX_PYRAMID_MAX_LEVELS_ORB] =
{
    1.0,
    0.8408964152537146,
    0.7071067811865476,
    0.5946035575013605,
    0.5,
    0.4204482076268573,
    0.3535533905932737,
    0.2973017787506803,
    0.25,
    0.2102241038134287,
    0.1767766952966369,
    0.1486508893753401,
    0.125,
    0.1051120519067143,
    0.08838834764831843,
    0.07432544468767006,
    0.0625
};

VX_API_ENTRY vx_status VX_API_CALL vxReleasePyramid(vx_pyramid *prmd)
{
    return (ownReleaseReferenceInt(
        (vx_reference*)prmd, VX_TYPE_PYRAMID, VX_EXTERNAL, NULL));
}

vx_pyramid VX_API_CALL vxCreatePyramid(
    vx_context context, vx_size levels, vx_float32 scale, vx_uint32 width,
    vx_uint32 height, vx_df_image format)
{
    vx_status status = VX_SUCCESS;
    vx_pyramid prmd = NULL;
    vx_uint32 i;
    tivx_obj_desc_pyramid_t *obj_desc = NULL;

    if(ownIsValidContext(context) == vx_true_e)
    {
        if (width == 0)
        {
            VX_PRINT(VX_ZONE_ERROR,"vxCreatePyramid: Width is equal to 0\n");
            status = VX_FAILURE;
        }
        if (height == 0)
        {
            VX_PRINT(VX_ZONE_ERROR,"vxCreatePyramid: Height is equal to 0\n");
            status = VX_FAILURE;
        }
        if (levels == 0)
        {
            VX_PRINT(VX_ZONE_ERROR,"vxCreatePyramid: Levels is equal to 0\n");
            status = VX_FAILURE;
        }
        if ((scale != VX_SCALE_PYRAMID_HALF) &&
            (scale != VX_SCALE_PYRAMID_ORB))
        {
            VX_PRINT(VX_ZONE_ERROR,"vxCreatePyramid: Invalid scale value\n");
            status = VX_FAILURE;
        }
        if (levels > TIVX_PYRAMID_MAX_LEVEL_OBJECTS)
        {
            VX_PRINT(VX_ZONE_ERROR,"vxCreatePyramid: Levels greater than max allowable\n");
            VX_PRINT(VX_ZONE_ERROR, "vxCreatePyramid: May need to increase the value of TIVX_PYRAMID_MAX_LEVEL_OBJECTS in tiovx/include/tivx_config.h\n");
            status = VX_FAILURE;
        }
        if ((scale == VX_SCALE_PYRAMID_ORB) &&
            (levels > TIVX_PYRAMID_MAX_LEVELS_ORB))
        {
            VX_PRINT(VX_ZONE_ERROR,"vxCreatePyramid: Orb levels are greater than max allowable\n");
            VX_PRINT(VX_ZONE_ERROR, "vxCreatePyramid: May need to increase the value of TIVX_PYRAMID_MAX_LEVELS_ORB in tiovx/include/tivx_config.h\n");
            status = VX_FAILURE;
        }

        if ((VX_SUCCESS == status) &&
            (scale == VX_SCALE_PYRAMID_ORB))
        {
            tivxLogSetResourceUsedValue("TIVX_PYRAMID_MAX_LEVELS_ORB", levels);
        }

        if (VX_SUCCESS == status)
        {
            tivxLogSetResourceUsedValue("TIVX_PYRAMID_MAX_LEVEL_OBJECTS", levels);
        }

        if (VX_SUCCESS == status)
        {
            prmd = (vx_pyramid)ownCreateReference(context, VX_TYPE_PYRAMID,
                VX_EXTERNAL, &context->base);

            if ((vxGetStatus((vx_reference)prmd) == VX_SUCCESS) &&
                (prmd->base.type == VX_TYPE_PYRAMID))
            {
                /* assign refernce type specific callback's */
                prmd->base.destructor_callback = &ownDestructPyramid;
                prmd->base.mem_alloc_callback = &ownAllocPyramidBuffer;
                prmd->base.release_callback =
                    (tivx_reference_release_callback_f)&vxReleasePyramid;

                obj_desc = (tivx_obj_desc_pyramid_t*)tivxObjDescAlloc(
                    TIVX_OBJ_DESC_PYRAMID, (vx_reference)prmd);
                if(obj_desc==NULL)
                {
                    vxReleasePyramid(&prmd);

                    vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                        "Could not allocate prmd object descriptor\n");
                    prmd = (vx_pyramid)ownGetErrorObject(
                        context, VX_ERROR_NO_RESOURCES);
                }
                else
                {
                    obj_desc->num_levels = levels;
                    obj_desc->width = width;
                    obj_desc->height = height;
                    obj_desc->scale = scale;
                    obj_desc->format = format;
                    prmd->base.obj_desc = (tivx_obj_desc_t *)obj_desc;

                    for (i = 0u; i < TIVX_PYRAMID_MAX_LEVEL_OBJECTS; i ++)
                    {
                        prmd->img[i] = NULL;
                    }

                    status = ownInitPyramid(prmd);

                    if (VX_SUCCESS != status)
                    {
                        vxReleasePyramid(&prmd);
                    }
                }
            }
        }
    }

    return (prmd);
}

vx_image VX_API_CALL vxGetPyramidLevel(vx_pyramid prmd, vx_uint32 index)
{
    vx_image img = NULL;

    if ((ownIsValidSpecificReference(&prmd->base, VX_TYPE_PYRAMID) ==
            vx_true_e) && (prmd->base.obj_desc != NULL) &&
        (index < ((tivx_obj_desc_pyramid_t *)prmd->base.obj_desc)->
            num_levels))
    {
        img = prmd->img[index];

        /* Should increment the reference count,
           To release this image, app should explicitely call ReleaseImage */
        ownIncrementReference(&img->base, VX_EXTERNAL);
    }
    else
    {
        img = (vx_image)ownGetErrorObject(prmd->base.context,
            VX_ERROR_INVALID_PARAMETERS);
    }

    return (img);
}

vx_pyramid VX_API_CALL vxCreateVirtualPyramid(
    vx_graph graph, vx_size levels, vx_float32 scale, vx_uint32 width,
    vx_uint32 height, vx_df_image format)
{
    vx_pyramid prmd = NULL;
    vx_context context;
    vx_uint32 i;
    tivx_obj_desc_pyramid_t *obj_desc = NULL;

    /* levels can not be 0 even in virtual prmd */
    if ((ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) ==
            vx_true_e) &&
        (levels > 0) && (levels <= TIVX_PYRAMID_MAX_LEVEL_OBJECTS) &&
        ( ((scale == VX_SCALE_PYRAMID_ORB) && (levels <= TIVX_PYRAMID_MAX_LEVELS_ORB)) ||
           (scale == VX_SCALE_PYRAMID_HALF) ) )
    {
        context = graph->base.context;

        /* frame size and format can be unspecified in virtual prmd */

        prmd = (vx_pyramid)ownCreateReference(context, VX_TYPE_PYRAMID,
            VX_EXTERNAL, &context->base);

        if ((vxGetStatus((vx_reference)prmd) == VX_SUCCESS) &&
            (prmd->base.type == VX_TYPE_PYRAMID))
        {
            /* assign refernce type specific callback's */
            prmd->base.destructor_callback = &ownDestructPyramid;
            prmd->base.mem_alloc_callback = &ownAllocPyramidBuffer;
            prmd->base.release_callback =
                (tivx_reference_release_callback_f)&vxReleasePyramid;

            obj_desc = (tivx_obj_desc_pyramid_t*)tivxObjDescAlloc(
                TIVX_OBJ_DESC_PYRAMID, (vx_reference)prmd);
            if(obj_desc==NULL)
            {
                vxReleasePyramid(&prmd);

                vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES,
                    "Could not allocate prmd object descriptor\n");
                prmd = (vx_pyramid)ownGetErrorObject(
                    context, VX_ERROR_NO_RESOURCES);
            }
            else
            {
                obj_desc->num_levels = levels;
                obj_desc->width = width;
                obj_desc->height = height;
                obj_desc->scale = scale;
                obj_desc->format = format;

                prmd->base.is_virtual = vx_true_e;
                ownReferenceSetScope(&prmd->base, &graph->base);

                prmd->base.obj_desc = (tivx_obj_desc_t *)obj_desc;

                for (i = 0u; i < TIVX_PYRAMID_MAX_LEVEL_OBJECTS; i ++)
                {
                    prmd->img[i] = NULL;
                }

                if (scale == VX_SCALE_PYRAMID_ORB)
                {
                    tivxLogSetResourceUsedValue("TIVX_PYRAMID_MAX_LEVELS_ORB", levels);
                }

                tivxLogSetResourceUsedValue("TIVX_PYRAMID_MAX_LEVEL_OBJECTS", levels);
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_WARNING, "vxCreateVirtualPyramid: May need to increase the value of TIVX_PYRAMID_MAX_LEVELS_ORB or TIVX_PYRAMID_MAX_LEVEL_OBJECTS in tiovx/include/tivx_config.h\n");
    }

    return (prmd);
}

vx_status ownInitVirtualPyramid(
    vx_pyramid prmd, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_status status = VX_FAILURE;
    tivx_obj_desc_pyramid_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&prmd->base, VX_TYPE_PYRAMID) == vx_true_e)
        &&
        (prmd->base.obj_desc != NULL))
    {
        obj_desc = (tivx_obj_desc_pyramid_t *)prmd->base.obj_desc;

        if ((width > 0) &&
            (height > 0) &&
            (prmd->base.is_virtual == vx_true_e))
        {
            obj_desc->width = width;
            obj_desc->height = height;
            obj_desc->format = format;

            status = ownInitPyramid(prmd);
        }
    }

    return (status);
}

vx_status VX_API_CALL vxQueryPyramid(
    vx_pyramid prmd, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_pyramid_t *obj_desc = NULL;

    if ((ownIsValidSpecificReference(&prmd->base, VX_TYPE_PYRAMID) == vx_false_e)
            || (prmd->base.obj_desc == NULL))
    {
        VX_PRINT(VX_ZONE_ERROR,"vxQueryPyramid: Invalid reference\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        obj_desc = (tivx_obj_desc_pyramid_t *)prmd->base.obj_desc;
        switch (attribute)
        {
            case VX_PYRAMID_LEVELS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = obj_desc->num_levels;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryPyramid: Query pyramid levels failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PYRAMID_SCALE:
                if (VX_CHECK_PARAM(ptr, size, vx_float32, 0x3U))
                {
                    *(vx_float32 *)ptr = obj_desc->scale;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryPyramid: Query pyramid scale failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PYRAMID_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->width;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryPyramid: Query pyramid width failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PYRAMID_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = obj_desc->height;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryPyramid: Query pyramid height failed\n");
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PYRAMID_FORMAT:
                if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3U))
                {
                    *(vx_df_image *)ptr = obj_desc->format;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR,"vxQueryPyramid: Query pyramid format failed\n");
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"vxQueryPyramid: Invalid attribute\n");
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status;
}

static vx_status ownAllocPyramidBuffer(vx_reference ref)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i=0;
    vx_pyramid prmd = (vx_pyramid)ref;
    tivx_obj_desc_pyramid_t *obj_desc = NULL;
    vx_image img;

    if(prmd->base.type == VX_TYPE_PYRAMID)
    {
        if(prmd->base.obj_desc != NULL)
        {
            obj_desc = (tivx_obj_desc_pyramid_t *)prmd->base.obj_desc;
            for (i = 0u; i < obj_desc->num_levels; i++)
            {
                img = prmd->img[i];

                if ((NULL != img) && (NULL != img->base.mem_alloc_callback))
                {
                    status = img->base.mem_alloc_callback((vx_reference)img);

                    if (VX_SUCCESS != status)
                    {
                        break;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"ownAllocPyramidBuffer: Image level %d is NULL\n", i);
                    status = VX_ERROR_INVALID_VALUE;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"ownAllocPyramidBuffer: Pyramid base object descriptor is NULL\n");
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"ownAllocPyramidBuffer: Reference type is not pyramid\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

static vx_status ownDestructPyramid(vx_reference ref)
{
    vx_pyramid prmd = (vx_pyramid)ref;
    vx_uint32 i = 0;
    tivx_obj_desc_pyramid_t *obj_desc = NULL;

    obj_desc = (tivx_obj_desc_pyramid_t *)prmd->base.obj_desc;

    for (i = 0; i < obj_desc->num_levels; i++)
    {
        if ((NULL != prmd->img[i]) &&
            (vxGetStatus((vx_reference)prmd->img[i]) == VX_SUCCESS))
        {
            /* increment the internal counter on the image, not the
               external one */
            ownDecrementReference((vx_reference)prmd->img[i], VX_INTERNAL);

            ownReleaseReferenceInt((vx_reference *)&prmd->img[i],
                VX_TYPE_IMAGE, VX_EXTERNAL, NULL);
        }
    }

    if(prmd->base.type == VX_TYPE_PYRAMID)
    {
        if(prmd->base.obj_desc!=NULL)
        {
            tivxObjDescFree((tivx_obj_desc_t**)&prmd->base.obj_desc);
        }
    }
    return VX_SUCCESS;
}

static vx_status ownInitPyramid(vx_pyramid prmd)
{
    vx_status status = VX_SUCCESS;
    vx_image img;
    vx_uint32 i, w, h, j;
    vx_float32 t1, scale;
    tivx_obj_desc_pyramid_t *obj_desc = NULL;

    obj_desc = (tivx_obj_desc_pyramid_t *)prmd->base.obj_desc;

    w = obj_desc->width;
    h = obj_desc->height;
    t1 = scale = obj_desc->scale;

    for (i = 0; i < obj_desc->num_levels; i++)
    {
        img = vxCreateImage(prmd->base.context, w, h,
            obj_desc->format);

        if (vxGetStatus((vx_reference)img) == VX_SUCCESS)
        {
            prmd->img[i] = img;
            obj_desc->obj_desc_id[i] =
                img->base.obj_desc->obj_desc_id;

            /* increment the internal counter on the image, not the
               external one */
            ownIncrementReference((vx_reference)img, VX_INTERNAL);

            /* remember that the scope of the image is the prmd */
            ownReferenceSetScope(&img->base, &prmd->base);

            if (VX_SCALE_PYRAMID_ORB == scale)
            {
                w = (vx_uint32)ceilf((vx_float32)obj_desc->width *
                    gOrbScaleFactor[i+1U]);
                h = (vx_uint32)ceilf((vx_float32)obj_desc->height *
                    gOrbScaleFactor[i+1U]);
            }
            else
            {
                w = (vx_uint32)ceilf((vx_float32)obj_desc->width * t1);
                h = (vx_uint32)ceilf((vx_float32)obj_desc->height * t1);
                t1 = t1 * scale;
            }
        }
        else
        {
            status = VX_FAILURE;
            vxAddLogEntry(&prmd->base.context->base, VX_ERROR_NO_RESOURCES,
               "Could not allocate image object descriptor\n");
            VX_PRINT(VX_ZONE_ERROR,"Could not allocate image object descriptor\n");
            break;
        }
    }

    if (VX_SUCCESS != status)
    {
        for (j = 0; j < i; j ++)
        {
            if (NULL != prmd->img[j])
            {
                /* increment the internal counter on the image, not the
                   external one */
                ownDecrementReference((vx_reference)prmd->img[j], VX_INTERNAL);

                ownReleaseReferenceInt((vx_reference *)&prmd->img[j],
                    VX_TYPE_IMAGE, VX_EXTERNAL, NULL);
            }
        }
    }

    return (status);
}

