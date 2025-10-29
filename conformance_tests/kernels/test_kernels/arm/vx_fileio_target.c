/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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

#include "TI/tivx.h"
#include "TI/tivx_test_kernels.h"
#include "tivx_kernel_fileio.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "utils/file_io/include/app_fileio.h"

#define FILEIO_TEST_BIG_BUFFER_SIZE     450*1024
#define FILEIO_TEST_MAX_TEST_PATH_LENGTH     512

typedef struct
{
    char envPtr[FILEIO_TEST_MAX_TEST_PATH_LENGTH];
} tivxFileioParams;


static vx_status VX_CALLBACK tivxFileioProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxFileioCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxFileioDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxFileioControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxFileioProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    return status;
}

static vx_status VX_CALLBACK tivxFileioCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    char *stringPtr = NULL;
    tivxFileioParams *prms = NULL;

    prms = tivxMemAlloc(sizeof(tivxFileioParams), TIVX_MEM_EXTERNAL);

    tivxSetTargetKernelInstanceContext(kernel, prms,
       sizeof(tivxFileioParams));

    tivx_obj_desc_user_data_object_t *path_obj_desc =
        (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_FILEIO_IN1_IDX];

    stringPtr = tivxMemShared2TargetPtr(&path_obj_desc->mem_ptr);

    tivxCheckStatus(&status, tivxMemBufferMap(stringPtr,
                     path_obj_desc->mem_size,
                     (vx_enum)VX_MEMORY_TYPE_HOST,
                     (vx_enum)VX_READ_ONLY));

    if (path_obj_desc->mem_size > FILEIO_TEST_MAX_TEST_PATH_LENGTH)
    {
        VX_PRINT(VX_ZONE_ERROR, "Path size exceeds maximum allowed length");
        status = VX_FAILURE;
    }
    else
    {
        memcpy(prms->envPtr, stringPtr, path_obj_desc->mem_size);
        prms->envPtr[path_obj_desc->mem_size-1] = '\0';
    }

    return status;
}

static vx_status VX_CALLBACK tivxFileioDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxFileioParams *prms = NULL;
    uint32_t size;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    tivxMemFree(prms, sizeof(tivxFileioParams), TIVX_MEM_EXTERNAL);

    return status;
}

static vx_status VX_CALLBACK tivxFileioControl(
       tivx_target_kernel_instance kernel, uint32_t node_cmd_id,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_scalar_t *out_desc;

    const char outpath[] =          "/output/";
    const char writefilename[] =    "rtos_fileio_write_test.txt";
    const char writefilenameBig[] = "rtos_fileio_write_test_big.txt";
    const char writefilenameErr[] = "file_doesnt_exist.txt";
    char buffer[16] = {0};
    FILE *fptr1 = NULL;
    FILE *fptr2 = NULL;
    FILE *fptr3 = NULL;
    size_t size = 0;
    char *bufferBig = NULL;
    int32_t totalSize;
    int32_t ret;
    int i;

#define FILEIO_TEST_MAX_PATH_LENGTH (FILEIO_TEST_MAX_TEST_PATH_LENGTH + sizeof(outpath) + sizeof(writefilenameBig))

    char filepath[FILEIO_TEST_MAX_PATH_LENGTH];
    char *envPtr;

    tivxFileioParams *prms = NULL;
    uint32_t prm_size;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &prm_size);

    envPtr = prms->envPtr;

    /*******************************************************************/
    /* Testing fileio functions: Write Small and big interleaved */

    if((envPtr == NULL) || (strncmp(envPtr, "", 2)==0))
    {
        VX_PRINT(VX_ZONE_ERROR, "VX_TEST_DATA_PATH env variable is empty.\n");
        status = VX_FAILURE;
    }

    if(VX_SUCCESS == status)
    {
        sprintf(buffer, "Write Test");

        snprintf(filepath, FILEIO_TEST_MAX_PATH_LENGTH, "%s%s%s",
            envPtr, outpath, writefilename);
        fptr1 = appFileIOFopen(filepath, "wb");
        if(fptr1 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFopen of %s failed\n", filepath);
            status = VX_FAILURE;
        }
    }
    if(VX_SUCCESS == status)
    {
        snprintf(filepath, FILEIO_TEST_MAX_PATH_LENGTH, "%s%s%s",
            envPtr, outpath, writefilenameBig);
        fptr2 = appFileIOFopen(filepath, "wb");
        if(fptr2 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFopen on fptr2 failed\n");
            status = VX_FAILURE;
        }

        snprintf(filepath, FILEIO_TEST_MAX_PATH_LENGTH, "%s%s%s",
            envPtr, outpath, writefilenameErr);
        fptr3 = appFileIOFopen(filepath, "rb");
        if(fptr3 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFopen on fptr3 failed\n");
        }
        else
        {
            /* Negative Test, so failure if it returns non-NULL */
            status = VX_FAILURE;
        }

        size = appFileIOFwrite(buffer, strlen(buffer), 1, fptr1);
        if (size != 1)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFwrite 1 returned wrong number\n");
            status = VX_FAILURE;
        }
        size = appFileIOFwrite(buffer, 1, strlen(buffer), fptr1);
        if (size != strlen(buffer))
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFwrite 2 returned wrong number\n");
            status = VX_FAILURE;
        }

        bufferBig = tivxMemAlloc(FILEIO_TEST_BIG_BUFFER_SIZE, TIVX_MEM_EXTERNAL);

        for (i = 0; i < FILEIO_TEST_BIG_BUFFER_SIZE; i++)
        {
            bufferBig[i] = 'a';
        }

        size = appFileIOFwrite(bufferBig, FILEIO_TEST_BIG_BUFFER_SIZE, 1, fptr2);
        if (size != 1)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFwrite 3 returned wrong number\n");
            status = VX_FAILURE;
        }
        size = appFileIOFwrite(buffer, strlen(buffer), 1, fptr1);
        if (size != 1)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFwrite 4 returned wrong number\n");
            status = VX_FAILURE;
        }
        size = appFileIOFwrite(buffer, 1, strlen(buffer), fptr1);
        if (size != strlen(buffer))
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFwrite 5 returned wrong number\n");
            status = VX_FAILURE;
        }

        size = appFileIOFwrite(bufferBig, 1, FILEIO_TEST_BIG_BUFFER_SIZE, fptr2);
        if (size != FILEIO_TEST_BIG_BUFFER_SIZE)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFwrite 6 returned wrong number\n");
            status = VX_FAILURE;
        }

        ret = appFileIOFclose(fptr1);
        if (ret != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFclose 1 failure\n");
            status = VX_FAILURE;
        }
        ret = appFileIOFclose(fptr2);
        if (ret != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFclose 1 failure\n");
            status = VX_FAILURE;
        }

        /*******************************************************************/
        /* Testing Fileio functions: Read, seek, tell */

        snprintf(filepath, FILEIO_TEST_MAX_PATH_LENGTH, "%s%s%s",
            envPtr, outpath, writefilename);
        fptr1 = appFileIOFopen(filepath, "rb");
        if(fptr1 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFopen on fptr1 failed\n");
            status = VX_FAILURE;
        }

        ret = appFileIOFseek(fptr1, 0, SEEK_END);
        if (ret != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFseek failure\n");
            status = VX_FAILURE;
        }

        totalSize = appFileIOFtell(fptr1);
        if (totalSize != (strlen(buffer)*4))
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFtell failure\n");
            status = VX_FAILURE;
        }

        ret = appFileIOFseek(fptr1, 0, SEEK_SET);
        if (ret != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFseek failure\n");
            status = VX_FAILURE;
        }

        if ( totalSize >= FILEIO_TEST_BIG_BUFFER_SIZE )
        {
            totalSize = FILEIO_TEST_BIG_BUFFER_SIZE-1;
        }
        
        size = appFileIOFread(bufferBig, totalSize, 1, fptr1);
        if (size != 1)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFread 1 returned wrong number\n");
            status = VX_FAILURE;
        }

        bufferBig[totalSize] = '\0';

        //printf("File: %s\nString: %s\n", writefilename, bufferBig);

        if(strncmp(bufferBig, "Write TestWrite TestWrite TestWrite Test", FILEIO_TEST_BIG_BUFFER_SIZE-1) != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFread did not read the right thing\n");
            status = VX_FAILURE;
        }
        tivxMemFree(bufferBig, FILEIO_TEST_BIG_BUFFER_SIZE, TIVX_MEM_EXTERNAL);

        ret = appFileIOFclose(fptr1);
        if (ret != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFclose 2 failure\n");
            status = VX_FAILURE;
        }

        /*******************************************************************/
        /* Testing Fileio functions: fprintf.*/
        snprintf(filepath, FILEIO_TEST_MAX_PATH_LENGTH, "%s%s%s",
            envPtr, outpath, writefilename);
        fptr1 = appFileIOFopen(filepath, "wb");
        if(fptr1 == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFopen on fptr1 failed\n");
            status = VX_FAILURE;
        }

        ret = appFileIOFprintf(fptr1, "%d, %s", sizeof(int32_t), writefilename);
        if (ret < 3)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFprintf failure\n");
            status = VX_FAILURE;
        }

        ret = appFileIOFclose(fptr1);
        if (ret != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appFileIOFclose 3 failure\n");
            status = VX_FAILURE;
        }

        /*******************************************************************/
        /* Testing Fileio functions: appWriteBinToFile, appReadBinFromFile.*/
        snprintf(filepath, FILEIO_TEST_MAX_PATH_LENGTH, "%s%s%s",
            envPtr, outpath, writefilename);
        ret = appWriteBinToFile(filepath, buffer, sizeof(buffer), 1);
        if (ret != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appWriteBinToFile failure\n");
            status = VX_FAILURE;
        }
        ret = appReadBinFromFile(filepath, buffer, sizeof(buffer), 1);
        if (ret != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "appReadBinFromFile failure\n");
            status = VX_FAILURE;
        }

#if !defined(LINUX) && !defined(QNX)
        /*******************************************************************/
        /* Testing Fileio functions: Negative Tests for invalid fptr.*/
        fptr1 = (FILE*)0;
        fptr2 = (FILE*)APP_FILEIO_MAX_NUM_FILES+1;
        ret = appFileIOFclose(fptr1);
        if (ret != -1)
        {
            status = VX_FAILURE;
        }
        ret = appFileIOFclose(fptr2);
        if (ret != -1)
        {
            status = VX_FAILURE;
        }

        ret = appFileIOFread(buffer, sizeof(buffer), 1, fptr1);
        if (ret != 0)
        {
            status = VX_FAILURE;
        }
        ret = appFileIOFread(buffer, sizeof(buffer), 1, fptr2);
        if (ret != 0)
        {
            status = VX_FAILURE;
        }

        ret = appFileIOFwrite(buffer, sizeof(buffer), 1, fptr1);
        if (ret != 0)
        {
            status = VX_FAILURE;
        }
        ret = appFileIOFwrite(buffer, sizeof(buffer), 1, fptr2);
        if (ret != 0)
        {
            status = VX_FAILURE;
        }

        ret = appFileIOFseek(fptr1, 0, SEEK_END);
        if (ret != -1)
        {
            status = VX_FAILURE;
        }
        ret = appFileIOFseek(fptr2, 0, SEEK_END);
        if (ret != -1)
        {
            status = VX_FAILURE;
        }

        ret = appFileIOFtell(fptr1);
        if (ret != -1)
        {
            status = VX_FAILURE;
        }
        ret = appFileIOFtell(fptr2);
        if (ret != -1)
        {
            status = VX_FAILURE;
        }

        ret = appFileIOFprintf(fptr1, "abc");
        if (ret != -1)
        {
            status = VX_FAILURE;
        }
        ret = appFileIOFprintf(fptr2, "abc");
        if (ret != -1)
        {
            status = VX_FAILURE;
        }
#endif
    }

    out_desc = (tivx_obj_desc_scalar_t *)obj_desc[0U];

    /* Send status back */
    out_desc->data.u08 = (uint8_t)status;

    return status;
}

void tivxAddTargetKernelFileio(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMcu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMpu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameC7x(target_name)))
    {
        tivxAddTargetKernelByName(
                            TIVX_KERNEL_FILEIO_NAME,
                            target_name,
                            tivxFileioProcess,
                            tivxFileioCreate,
                            tivxFileioDelete,
                            tivxFileioControl,
                            NULL);
    }
}

void tivxRemoveTargetKernelFileio(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMcu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameMpu(target_name)) ||
        ((vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameC7x(target_name)))
    {
        tivxRemoveTargetKernelByName(TIVX_KERNEL_FILEIO_NAME,
                            target_name);
    }
}
