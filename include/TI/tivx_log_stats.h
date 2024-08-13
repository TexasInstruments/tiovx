/*
*
* Copyright (c) 2024 Texas Instruments Incorporated
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

#ifndef TIVX_LOG_STATS_H_
#define TIVX_LOG_STATS_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Max number of data objects that contribute to a configuration
 *         parameter's memory footprint
 *
 * \ingroup group_tivx_log_resource
 */
#define TIVX_MAX_CONFIG_PARAM_OBJECTS (12u)

/*! \brief Number of dimensions of a configuration parameter's object type array
 *
 * \ingroup group_tivx_log_resource
 */
#define TIVX_CONFIG_PARAM_OBJECT_TYPE_DIM (2u)


/*!
 * \brief Max size of macro
 *
 * \ingroup group_tivx_log_resource
 */
#define TIVX_RESOURCE_NAME_MAX (39u)

/*! \brief Number of resources of a TIOVX perf stats object
 *
 * \ingroup group_tivx_log_resource
 */
#define TIVX_TARGET_RESOURCE_COUNT (4u)

/*! \brief Number of resources of a TIOVX perf stats object
 *
 * \ingroup group_tivx_log_resource
 */
#define TIVX_RESOURCE_STATS_TABLE_SIZE (57u)


/*! \brief Struct containing config parameters of given static resource. Allows
 *         for a log to be kept of the resources used throughout runtime.
 *
 * \ingroup group_tivx_log_resource
 */
typedef struct _tivx_resource_stats_t {

    uint32_t  max_value; /**< Maximum system quantity of the resource */

    uint32_t  cur_used_value; /**< Count of times the resource is currently defined in statically allocated arrays */

    uint32_t  max_used_value; /**< Highest resource count during current runtime */

    uint32_t  min_required_value; /**< Minimum value required for framework/tests to compile */

    char      name[TIVX_RESOURCE_NAME_MAX]; /**< Name of the resource */

    uint32_t   object_types[TIVX_CONFIG_PARAM_OBJECT_TYPE_DIM][TIVX_MAX_CONFIG_PARAM_OBJECTS];
    /**< Data objects with arrays of current parameter's length and multiplication factors */

    vx_bool   is_obj_desc; /**< Flag to indicate parameter's object descriptor status */

    vx_bool   is_size_cumulative; /**< Flag to indicate if a parameter is a total size or part of a bigger size */

} tivx_resource_stats_t;

/*!
 * \brief Query resource for resource stats
 *
 * \param [in] resource_name Name of the resource to query
 * \param [out] stats Pointer to resource statistic returned from the query
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 * \retval VX_FAILURE Unable to find "resource_name" in list of resources
 *
 * \ingroup group_tivx_log_resource
 */
vx_status tivxQueryResourceStats(const char *resource_name, tivx_resource_stats_t *stats);

/*!
 * \brief Prints out resource stats
 *
 * \ingroup group_tivx_log_resource
 */
void tivxPrintAllResourceStats(void);

/*!
 * \brief Exports the max used values to a file
 *
 *        Note 1: For PC emulation mode, the exported files will not have a legend with the correct target
 *        Note 2: For target mode, only the .txt files will be generated.  These can be converted to JPEG's using the following command:
 *
 *        dot -Tjpg -o<output file path>/<jpg name>.jpg <text file name>
 *
 *        For example, to generate a JPEG named "graph.jpg" for the file "graph.txt" in the current directory, the following command would be used.
 *
 *        dot -Tjpg -o./graph.jpg graph.txt
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS Output was successfully written to file
 * \retval VX_FAILURE Unable to open output file, or TIVX_LOG_RESOURCE_ENABLE was not defined
 *
 * \ingroup group_tivx_log_resource
 */
vx_status tivxExportAllResourceMaxUsedValueToFile(void);

/*!
 * \brief Exports memory consumption information to console or a specified file.
 *        Note that the object descriptor values returned by this API are local only to the
 *        process/core this is being called on.
 *
 * \param [in] outputFile Character pointer to indicate name of file where output is desired.
 *        The file name can be set to NULL if console output is desired.
 * \param [in] unit Character pointer to indicate the units desired in memory output.
 *        The unit options are "B", "KB", "MB", or "all". Any other choice is set to "all".
 * \param [in] displayMode Enum representing the desired mode of display.
 *        The mode options are defined in \ref tivx_memory_logging_e
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS Output was output to the console or was successfully written to file
 * \retval VX_FAILURE Unable to open output file, or TIVX_LOG_RESOURCE_ENABLE was not defined
 *
 *
 * \ingroup group_tivx_log_resource
 */
vx_status tivxExportMemoryConsumption(char * outputFile, const char * unit, vx_enum displayMode);

/*!
 * \brief Export graph representation as DOT graph file
 *
 * Multiple representation of the graph are exported to different files
 * using 'output_file_prefix' as filename prefix.
 * The output files are stored at path 'output_file_path'
 * Note: This must be called after vxVerifyGraph()
 *
 * \param [in] graph Graph reference
 * \param [in] output_file_path String specifying the ouput file path
 * \param [in] output_file_prefix String specifying the filename prefix of the output file
 *
 * \ingroup group_tivx_log_resource
 */
vx_status VX_API_CALL tivxExportGraphToDot(vx_graph graph, const char *output_file_path, const char *output_file_prefix);

/*! \brief Enumerations of memory consumption tool's display modes
 *
 * \ingroup group_tivx_log_resource
 */
typedef enum _tivx_memory_logging_e {

    /*! \brief Default display mode; Outputs only total global and obj desc memory */
    TIVX_MEM_LOG_DEFAULT = 0,

    /*! \brief Obj Desc display mode; Outputs only the object descriptor memory info */
    TIVX_MEM_LOG_OBJECT_DESCRIPTOR = 1,

    /*! \brief Global display mode; Outputs only the TIOVX global memory info */
    TIVX_MEM_LOG_GLOBAL = 2,

    /*! \brief All display mode; Outputs all possible information */
    TIVX_MEM_LOG_ALL = 3

} tivx_memory_logging_e;

#ifdef __cplusplus
}
#endif

#endif
