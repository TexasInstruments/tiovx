/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <tivx_utils.h>

extern char *tivxPlatformGetEnv(char *env_var);

const char *tivx_utils_get_test_file_dir()
{
    const char *env;

#if defined(FREERTOS) || defined(SAFERTOS) || defined(THREADX)
    env = tivxPlatformGetEnv("VX_TEST_DATA_PATH");
#else
    env = getenv("VX_TEST_DATA_PATH");
#endif

    if (env == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Please define the environment variable VX_TEST_DATA_PATH.\n");
    }

    return env;

}

vx_status tivx_utils_expand_file_path(const char *inFilePath, char *outFilePath)
{
    char       *start;
    char       *end;
    int32_t     len;
    vx_status  status;

    status = (vx_status)VX_SUCCESS;
    start  = strstr(inFilePath, "${");
    end    = strstr(inFilePath, "}");

    if ((start == NULL) || (end == NULL))
    {
        len = strlen(inFilePath);

        if (len > (TIOVX_UTILS_MAXPATHLENGTH-1))
        {
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            /* The path does not contain a reference to an
             * environment variable.
             */
            strcpy(outFilePath, inFilePath);
        }
    }
    else
    {
        char   *b = NULL;

        len = (int32_t)(end - start)-2;

        if (len >= (TIOVX_UTILS_MAXPATHLENGTH/2))
        {
            status = (vx_status)VX_FAILURE;
        }

        if (status == (vx_status)VX_SUCCESS)
        {
            strncpy(outFilePath, &start[2], len);
            outFilePath[len] = '\0';

#if defined(FREERTOS) || defined(SAFERTOS) || defined(THREADX)
            b = tivxPlatformGetEnv(outFilePath);
#else
            b = getenv(outFilePath);
#endif

            if (!b)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "Please define the environment variable %s.\n", b);

                status = (vx_status)VX_FAILURE;
            }
            else
            {
                snprintf(outFilePath,
                         TIOVX_UTILS_MAXPATHLENGTH-1,
                         "%s%s", b, &end[1]);
            }
        }
    }

    return status;
}

