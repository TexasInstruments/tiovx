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
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <VX/vx.h>
#include "test_tiovx_engine/test_list_helper.h"

#ifdef HAVE_VERSION_INC
#include "openvx_cts_version.inc"
#else
#define VERSION_STR "v1.1"
#endif

void TestModuleRegister();
void tivx_set_debug_zone(vx_enum zone);

int vx_tiovx_test_main(int argc, char* argv[])
{
    TestModuleRegister();
    tivx_set_debug_zone(0);
    tivx_set_debug_zone(1);
#if 0
    tivx_set_debug_zone(2);
    tivx_set_debug_zone(3);
    tivx_set_debug_zone(4);
    tivx_set_debug_zone(5);
    tivx_set_debug_zone(6);
    tivx_set_debug_zone(7);
    tivx_set_debug_zone(8);
    tivx_set_debug_zone(9);
    tivx_set_debug_zone(10);
    tivx_set_debug_zone(11);
    tivx_set_debug_zone(12);
    tivx_set_debug_zone(13);
    tivx_set_debug_zone(14);
    tivx_set_debug_zone(15);
    tivx_set_debug_zone(16);
    tivx_set_debug_zone(17);
#endif
    return tiovx_main(argc, argv, VERSION_STR);
}
