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

#include <tivx_target_config.h>

#if defined(MPU)
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (160U*1024U)
#elif defined(R5F)
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (8U*1024U)
#elif defined(C7X_FAMILY) || defined(C66)
#define TIVX_TARGET_DEFAULT_STACK_SIZE  (64U*1024U)
#elif defined(PC)
#define TIVX_TARGET_DEFAULT_STACK_SIZE  ((2U * 1024U) * 1024U)
#endif


#if defined(C7X_FAMILY) || defined(R5F) || defined(C66)

#if defined(SAFERTOS)
#define TIVX_TARGET_DEFAULT_STACK_ALIGNMENT  TIVX_TARGET_DEFAULT_STACK_SIZE
#else
#define TIVX_TARGET_DEFAULT_STACK_ALIGNMENT  8192
#endif /* SAFERTOS */

#if defined(C7X_FAMILY)
#define TIVX_MAX_TARGET                        8
#elif defined(R5F)
#define TIVX_MAX_TARGET                        TIVX_TARGET_R5F_MAX
#elif defined(C66)
#define TIVX_MAX_TARGET                        1
#endif
/**
 *******************************************************************************
 * \brief Target Stack
 *******************************************************************************
 */
/* IMPORTANT NOTE: For C7x,
 * - stack size and stack ptr MUST be 8KB aligned
 * - AND min stack size MUST be 16KB
 * - AND stack assigned for task context is "size - 8KB"
 *       - 8KB chunk for the stack area is used for interrupt handling in this task context
 */
static uint8_t gTarget_tskStack[TIVX_MAX_TARGET][TIVX_TARGET_DEFAULT_STACK_SIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(TIVX_TARGET_DEFAULT_STACK_ALIGNMENT)))
    ;
#endif /* #if defined(C7X_FAMILY) || defined(R5F) || defined(C66) */

/*! \brief Structure for mapping target id and target name
 * \ingroup group_tivx_platform
 */
typedef struct tivx_target_info
{
    /*! \brief Name of the target, defined in include/TI/soc/tivx_soc_<soc>.h file
     */
    char target_name[TIVX_TARGET_MAX_NAME];
    /*! \brief Id of the target defined in #tivx_target_id_e in the
     *   file soc/tivx_target_config_<soc>.h
     */
    vx_enum target_id;
} tivx_target_info_t;

/*! \brief Global instance of platform information
 * \ingroup group_tivx_platform
 */
static tivx_target_info_t g_tivx_target_info[] = TIVX_TARGET_INFO;

/*! \brief Maximum number of targets and thus targetid supported
 *         MUST be <= TIVX_TARGET_MAX_TARGETS_IN_CPU defined in
 *         include/TI/soc/tivx_config_<soc>.h
 * \ingroup group_tivx_platform
 */
#define TIVX_PLATFORM_MAX_TARGETS  dimof(g_tivx_target_info)

/* Function declarations for these are in source/platform/common/targets/tivx_target_config.h
 * since they are used inside of platform directory */

void tivxPlatformCreateTargetId(vx_enum target_id, uint32_t i, const char *name, uint32_t task_pri)
{
    vx_status status;
    tivx_target_create_params_t target_create_prms;

#ifndef PC
    if(ownTargetGetCpuId(target_id) == tivxGetSelfCpuId() )
#endif
    {
        char target_name[TIVX_TARGET_MAX_NAME];

        ownTargetSetDefaultCreateParams(&target_create_prms);

        #if defined(MPU) || defined(PC)
        target_create_prms.task_stack_ptr = NULL;
        #else
        target_create_prms.task_stack_ptr = gTarget_tskStack[i];
        #endif
        target_create_prms.task_stack_size = TIVX_TARGET_DEFAULT_STACK_SIZE;
        target_create_prms.task_core_affinity = TIVX_TASK_AFFINITY_ANY;
        target_create_prms.task_priority = task_pri;
        (void)strncpy(target_create_prms.task_name, name,TIVX_TARGET_MAX_TASK_NAME);
        target_create_prms.task_name[TIVX_TARGET_MAX_TASK_NAME-1U] = (char)0;

        status = ownTargetCreate(target_id, &target_create_prms);
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Could not Add Target\n");
        }
        else
        {
            ownPlatformGetTargetName(target_id, target_name);
            VX_PRINT(VX_ZONE_INFO, "Added target %s \n", target_name);
        }
    }
}

void tivxPlatformDeleteTargetId(vx_enum target_id)
{
    vx_status status;

    status = ownTargetDelete(target_id);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_CONFIG_UBR001
<justification end>*/
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Delete Target\n");
    }
/* LDRA_JUSTIFY_END */
}

void tivxPlatformSetHostTargetId(vx_enum host_cpu_id)
{
    uint32_t i;
    uint32_t host_target_id;
    vx_status status = (vx_status)VX_FAILURE;

    /* This assumes the convention that the 0th target on each
     * core is the one which would receive messages for the host
     * if the host is enabled for that core.  See #tivx_target_id_e
     * in the file soc/tivx_target_config_<soc>.h to confirm */
    host_target_id = TIVX_MAKE_TARGET_ID((uint32_t)host_cpu_id, 0u);

    /* Since the target_id was calculated, check if it is a valid
       target_id in the system */
    for (i = 0; i < TIVX_PLATFORM_MAX_TARGETS; i ++)
    {
        if ((vx_enum)host_target_id == g_tivx_target_info[i].target_id)
        {
            status = (vx_status)VX_SUCCESS;
            break;
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = (vx_status)VX_FAILURE;
        for (i = 0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_TARGET_CONFIG_UBR002
<justification end> */
        i < TIVX_PLATFORM_MAX_TARGETS;
/* LDRA_JUSTIFY_END */
        i ++)
        {
            if (0 == strncmp(
                    g_tivx_target_info[i].target_name,
                    TIVX_TARGET_HOST,
                    TIVX_TARGET_MAX_NAME))
            {
                /* update target_id for TIVX_TARGET_HOST */
                g_tivx_target_info[i].target_id = (vx_enum)host_target_id;
                status = (vx_status)VX_SUCCESS;
                break;
            }
        }
    }

    if((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Could not Set Host Target ID\n");
    }
}

/* Function declarations for these are in source/include/tivx_platform.h
 * since they are used outside of platform directory */

vx_enum ownPlatformGetTargetId(const char *target_name)
{
    uint32_t i;
    vx_enum target_id = (vx_enum)TIVX_TARGET_ID_INVALID;

    if (NULL != target_name)
    {
        for (i = 0; i < TIVX_PLATFORM_MAX_TARGETS; i ++)
        {
            if (0 == strncmp(g_tivx_target_info[i].target_name,
                    target_name,
                    TIVX_TARGET_MAX_NAME))
            {
                target_id = g_tivx_target_info[i].target_id;
                break;
            }
        }
    }

    return (target_id);
}

void ownPlatformGetTargetName(vx_enum target_id, char *target_name)
{
    uint32_t i;

    (void)snprintf(target_name, TIVX_TARGET_MAX_NAME, "UNKNOWN");

    if(target_id != (vx_enum)TIVX_TARGET_ID_INVALID)
    {
        for (i = 0; i < TIVX_PLATFORM_MAX_TARGETS; i ++)
        {
            if (target_id == g_tivx_target_info[i].target_id)
            {
                (void)snprintf(target_name, TIVX_TARGET_MAX_NAME, "%s", g_tivx_target_info[i].target_name);
                break;
            }
        }
    }
}
