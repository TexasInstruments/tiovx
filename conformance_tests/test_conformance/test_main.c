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

#include <VX/vx.h>
#include "test_engine/test_list_helper.h"
#include <TI/tivx_debug.h>

#ifdef HAVE_VERSION_INC
#include "openvx_cts_version.inc"
#else
#define VERSION_STR "unknown"
#endif

int vx_conformance_test_main(int argc, char* argv[])
{
    /* tivx_clr_debug_zone(VX_ZONE_ERROR); */
    return CT_main(argc, argv, VERSION_STR);
}
