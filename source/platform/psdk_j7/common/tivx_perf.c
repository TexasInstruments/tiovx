/*
 *******************************************************************************
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <tivx_platform_psdk.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <tivx_platform.h>


void tivxGetTiovxStats(app_perf_stats_tiovx_stats_t *tiovx_stats);

extern tivx_resource_stats_t g_tivx_resource_stats_table[TIVX_RESOURCE_STATS_TABLE_SIZE];

app_perf_registration_t get_tiovx_stats = {&tivxGetTiovxStats};
app_perf_registration_t * perf_fxns_list = &get_tiovx_stats;

void ownPlatformGetTargetPerfStats(uint32_t app_cpu_id, uint32_t target_values[TIVX_TARGET_RESOURCE_COUNT])
{
    app_perf_stats_tiovx_stats_t tiovx_stats;
    int32_t status;

    status = appPerfStatsCpuTiovxStatsGet(app_cpu_id, &tiovx_stats);

    if (status == (vx_status)VX_SUCCESS)
    {
        target_values[0] = tiovx_stats.targets_in_cpu;
        target_values[1] = tiovx_stats.max_targets_per_kernel;
        target_values[2] = tiovx_stats.target_kernel_instance_max;
        target_values[3] = tiovx_stats.target_kernel_max;
    }
}

void tivxGetTiovxStats(app_perf_stats_tiovx_stats_t *tiovx_stats)
{
    if (NULL != tiovx_stats)
    {
        uint32_t i;

        for (i = 0; i < TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
        {
            if ( strncmp(g_tivx_resource_stats_table[i].name, "TIVX_TARGET_MAX_TARGETS_IN_CPU", TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                tiovx_stats->targets_in_cpu = g_tivx_resource_stats_table[i].max_used_value;
            }
            if ( strncmp(g_tivx_resource_stats_table[i].name, "TIVX_MAX_TARGETS_PER_KERNEL", TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                tiovx_stats->max_targets_per_kernel = g_tivx_resource_stats_table[i].max_used_value;
            }
            if ( strncmp(g_tivx_resource_stats_table[i].name, "TIVX_TARGET_KERNEL_INSTANCE_MAX", TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                tiovx_stats->target_kernel_instance_max = g_tivx_resource_stats_table[i].cur_used_value;
            }
            if ( strncmp(g_tivx_resource_stats_table[i].name, "TIVX_TARGET_KERNEL_MAX", TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                tiovx_stats->target_kernel_max = g_tivx_resource_stats_table[i].max_used_value;
            }
        }
    }
}
