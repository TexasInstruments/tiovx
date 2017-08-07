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

#include "test.h"

#undef CT_TESTCASE
#define CT_TESTCASE(testcase) struct CT_TestCaseEntry* testcase##_register();
#include "kernel_library_tests.h"
#include "test_main.h"
#include "test_tiovx/test_main.h"

#undef CT_TESTCASE
#define CT_TESTCASE(testcase) testcase##_register,
CT_RegisterTestCaseFN g_testcase_register_fns[] = {
    #include "kernel_library_tests.h"
    #include "test_main.h"
    #include "test_tiovx/test_main.h"
    NULL
};

int CT_main(int argc, char* argv[], const char* version_str);
