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

/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

VX_API_ENTRY vx_node VX_API_CALL tivxHarrisCornersNode(vx_graph graph,
                            vx_image  input,
                            vx_uint32 scaling_factor,
                            vx_int32  nms_threshold,
                            vx_uint8  q_shift,
                            vx_uint8  win_size,
                            vx_uint8  score_method,
                            vx_uint8  suppression_method,
                            vx_array  corners,
                            vx_scalar num_corners)
{
    vx_scalar sc_fact = vxCreateScalar(vxGetContext((vx_reference)graph),
        VX_TYPE_UINT32, &scaling_factor);
    vx_scalar sc_thr = vxCreateScalar(vxGetContext((vx_reference)graph),
        VX_TYPE_INT32, &nms_threshold);
    vx_scalar sc_q_shift = vxCreateScalar(vxGetContext((vx_reference)graph),
        VX_TYPE_UINT8, &q_shift);
    vx_scalar sc_win = vxCreateScalar(vxGetContext((vx_reference)graph),
        VX_TYPE_UINT8, &win_size);
    vx_scalar sc_score = vxCreateScalar(vxGetContext((vx_reference)graph),
        VX_TYPE_UINT8, &score_method);
    vx_scalar sc_suppr = vxCreateScalar(vxGetContext((vx_reference)graph),
        VX_TYPE_UINT8, &suppression_method);

    vx_reference params[] = {
            (vx_reference)input,
            (vx_reference)sc_fact,
            (vx_reference)sc_thr,
            (vx_reference)sc_q_shift,
            (vx_reference)sc_win,
            (vx_reference)sc_score,
            (vx_reference)sc_suppr,
            (vx_reference)corners,
            (vx_reference)num_corners,
    };
    vx_node node = tivxCreateNodeByStructure(graph,
                                           TIVX_KERNEL_IVISION_HARRIS_CORNERS,
                                           params,
                                           dimof(params));
    vxReleaseScalar(&sc_fact);
    vxReleaseScalar(&sc_thr);
    vxReleaseScalar(&sc_q_shift);
    vxReleaseScalar(&sc_win);
    vxReleaseScalar(&sc_score);
    vxReleaseScalar(&sc_suppr);
    return node;
}
