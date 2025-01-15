/*
 * Copyright (c) 2024 The Khronos Group Inc.
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
#ifndef _ENUMSTRING_H
#define _ENUMSTRING_H
#ifdef  __cplusplus
extern "C" {
#endif
/* Function for turning an enum into a string. (A massive case statement we add to as required) */
#include <VX/vx.h>
#include "TI/tivx.h"

const char * enumToString(vx_enum enum_c);

#ifdef  __cplusplus
}
#endif

#endif
