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
#include <stdlib.h>
#include <signal.h>
#include <TI/tivx.h>

#ifndef SOC_J6
#include <utils/app_init/include/app_init.h>
#endif

int vx_conformance_test_main(int argc, char* argv[]);
void TestModuleRegister();
void TestModuleUnRegister();

static void intSigHandler(int sig)
{
    exit(1);
}

int main(int argc, char* argv[])
{
    int status = 0;
    /* Register the signal handler. */
    signal(SIGINT, intSigHandler);

#ifndef SOC_J6
    appInit();
#else
    tivxInit();
#endif

    TestModuleRegister();
    status = vx_conformance_test_main(argc, argv);
    TestModuleUnRegister();

#ifndef SOC_J6
    appDeInit();
#else
    tivxDeInit();
#endif

    return status;
}
