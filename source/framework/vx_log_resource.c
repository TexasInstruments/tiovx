/*
*
* Copyright (c) 2018 Texas Instruments Incorporated
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

#include <vx_internal.h>

static int32_t findMacroSize(const char *resource_name);
static int32_t getNumDigits(int32_t value);
static char *test_file_path(void);

static tivx_mutex g_tivx_log_resource_lock;

static tivx_resource_stats_t g_tivx_resource_stats_table[] = {
    {
        TIVX_KERNEL_MAX_PARAMS, 0, 0, "TIVX_KERNEL_MAX_PARAMS"
    },
    {
        TIVX_NODE_MAX_OUT_NODES, 0, 0, "TIVX_NODE_MAX_OUT_NODES"
    },
    {
        TIVX_NODE_MAX_IN_NODES, 0, 0, "TIVX_NODE_MAX_IN_NODES"
    },
    {
        TIVX_PYRAMID_MAX_LEVEL_OBJECTS, 0, 0, "TIVX_PYRAMID_MAX_LEVEL_OBJECTS"
    },
    {
        TIVX_OBJECT_ARRAY_MAX_ITEMS, 0, 0, "TIVX_OBJECT_ARRAY_MAX_ITEMS"
    },
    {
        TIVX_GRAPH_MAX_NODES, 0, 0, "TIVX_GRAPH_MAX_NODES"
    },
    {
        TIVX_GRAPH_MAX_SUPER_NODES, 0, 0, "TIVX_GRAPH_MAX_SUPER_NODES"
    },
    {
        TIVX_GRAPH_MAX_PIPELINE_DEPTH, 0, 0, "TIVX_GRAPH_MAX_PIPELINE_DEPTH"
    },
    {
        TIVX_NODE_MAX_REPLICATE, 0, 0, "TIVX_NODE_MAX_REPLICATE"
    },
    {
        TIVX_META_FORMAT_MAX_OBJECTS, 0, 0, "TIVX_META_FORMAT_MAX_OBJECTS"
    },
    {
        TIVX_CONTEXT_MAX_OBJECTS, 0, 0, "TIVX_CONTEXT_MAX_OBJECTS"
    },
    {
        TIVX_GRAPH_MAX_OBJECTS, 0, 0, "TIVX_GRAPH_MAX_OBJECTS"
    },
    {
        TIVX_SUPER_NODE_MAX_OBJECTS, 0, 0, "TIVX_SUPER_NODE_MAX_OBJECTS"
    },
    {
        TIVX_NODE_MAX_OBJECTS, 0, 0, "TIVX_NODE_MAX_OBJECTS"
    },
    {
        TIVX_KERNEL_MAX_OBJECTS, 0, 0, "TIVX_KERNEL_MAX_OBJECTS"
    },
    {
        TIVX_ARRAY_MAX_OBJECTS, 0, 0, "TIVX_ARRAY_MAX_OBJECTS"
    },
    {
        TIVX_USER_DATA_OBJECT_MAX_OBJECTS, 0, 0, "TIVX_USER_DATA_OBJECT_MAX_OBJECTS"
    },
    {
        TIVX_RAW_IMAGE_MAX_OBJECTS, 0, 0, "TIVX_RAW_IMAGE_MAX_OBJECTS"
    },
    {
        TIVX_CONVOLUTION_MAX_OBJECTS, 0, 0, "TIVX_CONVOLUTION_MAX_OBJECTS"
    },
    {
        TIVX_DELAY_MAX_OBJECTS, 0, 0, "TIVX_DELAY_MAX_OBJECTS"
    },
    {
        TIVX_DISTRIBUTION_MAX_OBJECTS, 0, 0, "TIVX_DISTRIBUTION_MAX_OBJECTS"
    },
    {
        TIVX_IMAGE_MAX_OBJECTS, 0, 0, "TIVX_IMAGE_MAX_OBJECTS"
    },
    {
        TIVX_TENSOR_MAX_OBJECTS, 0, 0, "TIVX_TENSOR_MAX_OBJECTS"
    },
    {
        TIVX_LUT_MAX_OBJECTS, 0, 0, "TIVX_LUT_MAX_OBJECTS"
    },
    {
        TIVX_MATRIX_MAX_OBJECTS, 0, 0, "TIVX_MATRIX_MAX_OBJECTS"
    },
    {
        TIVX_PYRAMID_MAX_OBJECTS, 0, 0, "TIVX_PYRAMID_MAX_OBJECTS"
    },
    {
        TIVX_REMAP_MAX_OBJECTS, 0, 0, "TIVX_REMAP_MAX_OBJECTS"
    },
    {
        TIVX_SCALAR_MAX_OBJECTS, 0, 0, "TIVX_SCALAR_MAX_OBJECTS"
    },
    {
        TIVX_THRESHOLD_MAX_OBJECTS, 0, 0, "TIVX_THRESHOLD_MAX_OBJECTS"
    },
    {
        TIVX_ERROR_MAX_OBJECTS, 0, 0, "TIVX_ERROR_MAX_OBJECTS"
    },
    {
        TIVX_OBJ_ARRAY_MAX_OBJECTS, 0, 0, "TIVX_OBJ_ARRAY_MAX_OBJECTS"
    },
    {
        TIVX_PARAMETER_MAX_OBJECTS, 0, 0, "TIVX_PARAMETER_MAX_OBJECTS"
    },
    {
        TIVX_DATA_REF_Q_MAX_OBJECTS, 0, 0, "TIVX_DATA_REF_Q_MAX_OBJECTS"
    },
    {
        TIVX_TARGET_MAX_TARGETS_IN_CPU, 0, 0, "TIVX_TARGET_MAX_TARGETS_IN_CPU"
    },
    {
        TIVX_TARGET_MAX_JOB_QUEUE_DEPTH, TIVX_TARGET_MAX_JOB_QUEUE_DEPTH, TIVX_TARGET_MAX_JOB_QUEUE_DEPTH, "TIVX_TARGET_MAX_JOB_QUEUE_DEPTH"
    },
    {
        TIVX_CONTEXT_MAX_REFERENCES, 0, 0, "TIVX_CONTEXT_MAX_REFERENCES"
    },
    {
        TIVX_CONTEXT_MAX_KERNELS, 0, 0, "TIVX_CONTEXT_MAX_KERNELS"
    },
    {
        TIVX_CONTEXT_MAX_USER_STRUCTS, 0, 0, "TIVX_CONTEXT_MAX_USER_STRUCTS"
    },
    {
        TIVX_MAX_TARGETS_PER_KERNEL, 0, 0, "TIVX_MAX_TARGETS_PER_KERNEL"
    },
    {
        TIVX_MODULE_MAX, 0, 0, "TIVX_MODULE_MAX"
    },
    {
        TIVX_TARGET_KERNEL_MAX, 0, 0, "TIVX_TARGET_KERNEL_MAX"
    },
    {
        TIVX_PYRAMID_MAX_LEVELS_ORB, 0, 0, "TIVX_PYRAMID_MAX_LEVELS_ORB"
    },
    {
        TIVX_TARGET_KERNEL_INSTANCE_MAX, 0, 0, "TIVX_TARGET_KERNEL_INSTANCE_MAX"
    },
    {
        TIVX_ARRAY_MAX_MAPS, 0, 0, "TIVX_ARRAY_MAX_MAPS"
    },
    {
        TIVX_USER_DATA_OBJECT_MAX_MAPS, 0, 0, "TIVX_USER_DATA_OBJECT_MAX_MAPS"
    },
    {
        TIVX_RAW_IMAGE_MAX_MAPS, 0, 0, "TIVX_RAW_IMAGE_MAX_MAPS"
    },
    {
        TIVX_GRAPH_MAX_HEAD_NODES, 0, 0, "TIVX_GRAPH_MAX_HEAD_NODES"
    },
    {
        TIVX_GRAPH_MAX_LEAF_NODES, 0, 0, "TIVX_GRAPH_MAX_LEAF_NODES"
    },
    {
        TIVX_GRAPH_MAX_PARAMS, 0, 0, "TIVX_GRAPH_MAX_PARAMS"
    },
    {
        TIVX_GRAPH_MAX_DATA_REF_QUEUE, 0, 0, "TIVX_GRAPH_MAX_DATA_REF_QUEUE"
    },
    {
        TIVX_GRAPH_MAX_DELAYS, 0, 0, "TIVX_GRAPH_MAX_DELAYS"
    },
    {
        TIVX_GRAPH_MAX_DATA_REF, 0, 0, "TIVX_GRAPH_MAX_DATA_REF"
    },
    {
        TIVX_IMAGE_MAX_SUBIMAGES, 0, 0, "TIVX_IMAGE_MAX_SUBIMAGES"
    },
    {
        TIVX_RAW_IMAGE_MAX_SUBIMAGES, 0, 0, "TIVX_RAW_IMAGE_MAX_SUBIMAGES"
    },
    {
        TIVX_IMAGE_MAX_MAPS, 0, 0, "TIVX_IMAGE_MAX_MAPS"
    },
    {
        TIVX_TENSOR_MAX_MAPS, 0, 0, "TIVX_TENSOR_MAX_MAPS"
    },
    {
        TIVX_DELAY_MAX_OBJECT, 0, 0, "TIVX_DELAY_MAX_OBJECT"
    },
    {
        TIVX_DELAY_MAX_PRM_OBJECT, 0, 0, "TIVX_DELAY_MAX_PRM_OBJECT"
    },
    {
        TIVX_EVENT_QUEUE_MAX_SIZE, 0, 0, "TIVX_EVENT_QUEUE_MAX_SIZE"
    },
    {
        TIVX_MAX_DSP_BAM_USER_PLUGINS, 0, 0, "TIVX_MAX_DSP_BAM_USER_PLUGINS"
    },
    {
        TIVX_SUPER_NODE_MAX_NODES, 0, 0, "TIVX_SUPER_NODE_MAX_NODES"
    },
    {
        TIVX_SUPER_NODE_MAX_EDGES, 0, 0, "TIVX_SUPER_NODE_MAX_EDGES"
    },
    {
        TIVX_DEFAULT_TILE_WIDTH, TIVX_DEFAULT_TILE_WIDTH, TIVX_DEFAULT_TILE_WIDTH, "TIVX_DEFAULT_TILE_WIDTH"
    },
    {
        TIVX_DEFAULT_TILE_HEIGHT, TIVX_DEFAULT_TILE_HEIGHT, TIVX_DEFAULT_TILE_HEIGHT, "TIVX_DEFAULT_TILE_HEIGHT"
    }
};

#define TIVX_RESOURCE_STATS_TABLE_SIZE (sizeof(g_tivx_resource_stats_table)/sizeof(g_tivx_resource_stats_table[0]))

void ownLogResourceInit(void)
{
#ifdef TIVX_RESOURCE_LOG_ENABLE
    tivxMutexCreate(&g_tivx_log_resource_lock);
#endif
}

void ownLogResourceDeInit(void)
{
#ifdef TIVX_RESOURCE_LOG_ENABLE
    tivxMutexDelete(&g_tivx_log_resource_lock);
#endif
}

void ownLogResourceAlloc(const char *resource_name, uint16_t num_allocs)
{
#ifdef TIVX_RESOURCE_LOG_ENABLE
    int32_t i;
    tivxMutexLock(g_tivx_log_resource_lock);
    for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
    {
        if ( strncmp(g_tivx_resource_stats_table[i].name, resource_name, TIVX_RESOURCE_NAME_MAX) == 0 )
        {
            g_tivx_resource_stats_table[i].cur_used_value += num_allocs;
            if (g_tivx_resource_stats_table[i].cur_used_value > g_tivx_resource_stats_table[i].max_used_value)
            {
                g_tivx_resource_stats_table[i].max_used_value = g_tivx_resource_stats_table[i].cur_used_value;
            }
            break;
        }
    }
    tivxMutexUnlock(g_tivx_log_resource_lock);
#endif
}

void ownLogResourceFree(const char *resource_name, uint16_t num_frees)
{
#ifdef TIVX_RESOURCE_LOG_ENABLE
    int32_t i;
    tivxMutexLock(g_tivx_log_resource_lock);
    for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
    {
        if ( strncmp(g_tivx_resource_stats_table[i].name, resource_name, TIVX_RESOURCE_NAME_MAX) == 0 )
        {
            g_tivx_resource_stats_table[i].cur_used_value -= num_frees;
            break;
        }
    }
    tivxMutexUnlock(g_tivx_log_resource_lock);
#endif
}

void ownLogSetResourceUsedValue(const char *resource_name, uint16_t value)
{
#ifdef TIVX_RESOURCE_LOG_ENABLE
    int32_t i;
    tivxMutexLock(g_tivx_log_resource_lock);
    for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
    {
        if ( strncmp(g_tivx_resource_stats_table[i].name, resource_name, TIVX_RESOURCE_NAME_MAX) == 0 )
        {
            if (value > g_tivx_resource_stats_table[i].max_used_value)
            {
                g_tivx_resource_stats_table[i].max_used_value = value;
            }
            break;
        }
    }
    tivxMutexUnlock(g_tivx_log_resource_lock);
#endif
}

vx_status tivxQueryResourceStats(const char *resource_name, tivx_resource_stats_t *stat)
{
    vx_status status = (vx_status)VX_FAILURE;
#ifdef TIVX_RESOURCE_LOG_ENABLE
    int32_t i;
    tivxMutexLock(g_tivx_log_resource_lock);

    for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
    {
        if ( strncmp(g_tivx_resource_stats_table[i].name, resource_name, TIVX_RESOURCE_NAME_MAX) == 0 )
        {
            *stat = g_tivx_resource_stats_table[i];
            status = (vx_status)VX_SUCCESS;
            break;
        }
    }
    tivxMutexUnlock(g_tivx_log_resource_lock);
#endif
    return status;
}

static int32_t findMacroSize(const char *resource_name)
{
    int32_t i, size = (int32_t)TIVX_RESOURCE_NAME_MAX;

    for (i = 0; i < (int32_t)TIVX_RESOURCE_NAME_MAX; i++)
    {
        if (resource_name[i] == '\0')
        {
            size = i;
            break;
        }
    }

    return size;
}

static int32_t getNumDigits(int32_t value)
{
    int32_t numDigits;

    if (value < 10)
    {
        numDigits = 1;
    }
    else if (value < 100)
    {
        numDigits = 2;
    }
    else
    {
        numDigits = 3;
    }

    return numDigits;
}

void tivxPrintAllResourceStats(void)
{
#ifdef TIVX_RESOURCE_LOG_ENABLE
    int32_t i, j;
    tivx_resource_stats_t stat;
    printf("\n\nMAX VALUE NAME:                         MAX VALUE:   VALUE BEING USED:\n");
    printf("----------------------------------------------------------------------\n");
    tivxMutexLock(g_tivx_log_resource_lock);

    for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
    {
        int32_t name_length, numDigits;
        stat = g_tivx_resource_stats_table[i];
        name_length = findMacroSize(stat.name);
        printf("%s ", stat.name);
        for (j = 0; j < ((int32_t)TIVX_RESOURCE_NAME_MAX - name_length); j++)
        {
            printf(" ");
        }
        printf("|");
        numDigits = getNumDigits((int32_t)stat.max_value);
        for (j = 0; j < (6 - numDigits); j++)
        {
            printf(" ");
        }
        printf("%d", stat.max_value);
        for (j = 0; j < 4; j++)
        {
            printf(" ");
        }
        printf("|");
        numDigits = getNumDigits((int32_t)stat.max_used_value);
        for (j = 0; j < (9 - numDigits); j++)
        {
            printf(" ");
        }
        printf("%d\n", stat.max_used_value);
    }
    tivxMutexUnlock(g_tivx_log_resource_lock);
#endif
}

static char *test_file_path(void)
{
    char *tivxPlatformGetEnv(char *env_var);

    #if defined(SYSBIOS) || defined(FREERTOS) || defined(SAFERTOS)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
}

vx_status tivxExportAllResourceMaxUsedValueToFile(void)
{
    vx_status status = (vx_status)VX_FAILURE;
#ifdef TIVX_RESOURCE_LOG_ENABLE
    tivxMutexLock(g_tivx_log_resource_lock);

    int32_t i;
    FILE *ofp;
    tivx_resource_stats_t stat;
    status = (vx_status)VX_SUCCESS;

    char outputFilename[TIVX_CONFIG_PATH_LENGTH];

    if (NULL != test_file_path())
    {
        snprintf(outputFilename, TIVX_CONFIG_PATH_LENGTH, "%s/%s", test_file_path(), "output/tivx_config_generated.h");

        ofp = fopen(outputFilename, "w");

        if (ofp == NULL)
        {
            fprintf(stderr, "Can't open output file!\n");
            status = (vx_status)VX_FAILURE;
            tivxMutexUnlock(g_tivx_log_resource_lock);
        }
        else
        {
            fprintf(ofp, "#ifndef TIVX_CONFIG_H_\n");
            fprintf(ofp, "#define TIVX_CONFIG_H_\n\n");

            fprintf(ofp, "#ifdef __cplusplus\n");
            fprintf(ofp, "extern \"C\" {\n");
            fprintf(ofp, "#endif\n\n");

            for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
            {
                stat = g_tivx_resource_stats_table[i];
                fprintf(ofp, "#define ");
                fprintf(ofp, "%s ", stat.name);
                fprintf(ofp, "(%du)\n\n", stat.max_used_value);
            }

            fprintf(ofp, "#ifdef __cplusplus\n");
            fprintf(ofp, "}\n");
            fprintf(ofp, "#endif\n\n");
            fprintf(ofp, "#endif\n");

            fclose(ofp);
            tivxMutexUnlock(g_tivx_log_resource_lock);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "VX_TEST_DATA_PATH has not been set!\n");
        tivxMutexUnlock(g_tivx_log_resource_lock);
    }
#endif
    return status;
}

