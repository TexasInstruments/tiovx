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

#ifndef __VX_CT_TEST_BMP_H__
#define __VX_CT_TEST_BMP_H__

#include <VX/vx.h>
#include "test_image.h"

CT_Image ct_read_bmp(const unsigned char* data, int datasize, int dcn);
int ct_write_bmp(const char* filename, CT_Image img);

#endif
