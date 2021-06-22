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

/* set to 1 only if 'dot' tool can be invoke with "system" command */
#ifdef PC /* PC host emulation mode */
#define TIVX_EXPORT_GRAPH_AS_JPG    (1u)
#else
#define TIVX_EXPORT_GRAPH_AS_JPG    (0u)
#endif

#define TIVX_EXPORT_MAX_FILENAME    (256u)
#define TIVX_EXPORT_MAX_LINE_SIZE   (1024u)
#define TIVX_EXPORT_MAX_NODE_COLOR_NAME (64u)

#define TIVX_EXPORT_WRITELN(fp, message, ...) do { \
    snprintf(line, TIVX_EXPORT_MAX_FILENAME, message"\n", ##__VA_ARGS__); \
    fwrite(line, 1, strlen(line), fp); \
    } while (1 == 0)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
static void getNodeColor(vx_node node, char *node_color_name);
static int32_t exportAsJpg(const char *output_file_path, const char *output_file_prefix, const char *out_file_id_str, const char *in_filename);
#ifndef PC
static void exportTargetLegend(FILE *fp, vx_graph graph);
#endif
static void exportDataRef(FILE *fp, vx_reference ref);
static void exportDataRefObjDesc(FILE *fp, vx_reference ref);
static void exportDataRefQueue(FILE *fp, tivx_data_ref_queue ref, uint32_t num_buf, vx_bool is_graph_parameter);
static void exportDataRefQueueObjDesc(FILE *fp, tivx_data_ref_queue ref,
            uint32_t num_buf, vx_bool is_graph_parameter, vx_bool show_delay_links, uint32_t pipeline_depth);
static void exportNodeObjDesc(FILE *fp, vx_node node, uint32_t pipe_id, const char *prefix);
static void tivxExportGraphDataRefQueueToDot(FILE *fp, vx_graph graph,
                    tivx_data_ref_queue data_ref_q, uint32_t graph_parameter_index );
static vx_status tivxExportGraphDataRefQueuesToDot(vx_graph graph, const char *output_file_path, const char *output_file_prefix);
static vx_status tivxExportGraphFirstPipelineToDot(vx_graph graph, const char *output_file_path, const char *output_file_prefix);
static vx_status tivxExportGraphPipelineToDot(vx_graph graph, const char *output_file_path, const char *output_file_prefix);
static vx_status tivxExportGraphTopLevelToDot(vx_graph graph, const char *output_file_path, const char *output_file_prefix);

static void getNodeColor(vx_node node, char *node_color_name)
{
    char target_name[TIVX_TARGET_MAX_NAME] = {0};

    snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "white");

    if(node->obj_desc[0] != NULL)
    {
        tivxPlatformGetTargetName((int32_t)node->obj_desc[0]->target_id, target_name);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"node obj desc is NULL\n");
    }

    if(node->obj_desc[0] != NULL)
    {
        if(strncmp(target_name, "DSP-1", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "palegreen");
        }
        else
        if(strncmp(target_name, "DSP-2", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "darkturquoise");
        }
        else
        if(strncmp(target_name, "EVE-1", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "yellow");
        }
        else
        if(strncmp(target_name, "EVE-2", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "gold");
        }
        else
        if(strncmp(target_name, "EVE-3", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "orange");
        }
        else
        if(strncmp(target_name, "EVE-4", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "goldenrod4");
        }
        else
        if(strncmp(target_name, "A15-0", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "lightblue");
        }
        else
        if(strncmp(target_name, "IPU1-0", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "grey");
        }
        else
        if(strncmp(target_name, "IPU1-1", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "LightSalmon");
        }
        else
        if(strncmp(target_name, "IPU2", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "MediumOrchid");
        }
        else
        if(strncmp(target_name, "IPU2", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "MediumOrchid");
        }
        else
        if(strncmp(target_name, "A72-0", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "lightblue");
        }
        else
        if(strncmp(target_name, "DSP_C7-1", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "yellow");
        }
        else
        if(strncmp(target_name, "VPAC_NF", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "chocolate");
        }
        else
        if(strncmp(target_name, "VPAC_LDC1", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "aquamarine");
        }
        else
        if(strncmp(target_name, "VPAC_MSC1", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "antiquewhite");
        }
        else
        if(strncmp(target_name, "VPAC_MSC2", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "azure");
        }
        else
        if(strncmp(target_name, "DMPAC_SDE", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "beige");
        }
        else
        if(strncmp(target_name, "DMPAC_DOF", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "bisque");
        }
        else
        if(strncmp(target_name, "VPAC_VISS1", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "blanchedalmond");
        }
        else
        if(strncmp(target_name, "CAPTURE1", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "blue");
        }
        else
        if(strncmp(target_name, "CAPTURE2", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "brown");
        }
        else
        if(strncmp(target_name, "DISPLAY1", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "burlywood");
        }
        else
        if(strncmp(target_name, "DISPLAY2", TIVX_TARGET_MAX_NAME) == 0)
        {
            snprintf(node_color_name, TIVX_EXPORT_MAX_NODE_COLOR_NAME, "cadetblue");
        }
        else
        {
            /* do nothing */
        }
    }
}

static int32_t exportAsJpg(const char *output_file_path, const char *output_file_prefix, const char *out_file_id_str, const char *in_filename)
{
    int32_t status = 0;
    #if TIVX_EXPORT_GRAPH_AS_JPG
    char command[4096];

    snprintf(command, 4096, "dot -Tjpg -o%s/%s%s_img.jpg %s",
        output_file_path, output_file_prefix, out_file_id_str,
        in_filename);
    status = system(command);

    #endif
    return status;
}

#ifndef PC
static void exportTargetLegend(FILE *fp, vx_graph graph)
{
    char line[TIVX_EXPORT_MAX_LINE_SIZE];
    char target_name[TIVX_TARGET_MAX_NAME];
    char target_name_list[TIVX_TARGET_MAX_TARGETS_IN_CPU][TIVX_TARGET_MAX_NAME];
    char node_color_name_list[TIVX_TARGET_MAX_TARGETS_IN_CPU][TIVX_EXPORT_MAX_NODE_COLOR_NAME];
    uint32_t node_id, target_id, num_targets = 0;
    vx_bool new_target = (vx_bool)vx_true_e;

    TIVX_EXPORT_WRITELN(fp, "  ColorScheme [shape=none, margin=0, label=<\n");
    TIVX_EXPORT_WRITELN(fp, "        <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n");

    for(node_id=0; node_id<graph->num_nodes; node_id++)
    {
        new_target = (vx_bool)vx_true_e;

        if(graph->nodes[node_id]->obj_desc[0] != NULL)
        {
            tivxPlatformGetTargetName((int32_t)graph->nodes[node_id]->obj_desc[0]->target_id, target_name);
            for (target_id = 0; target_id < num_targets; target_id++)
            {
                if(strcmp(target_name, target_name_list[target_id]) == 0)
                {
                    new_target = (vx_bool)vx_false_e;
                    break;
                }
            }

            if ((vx_bool)vx_true_e == new_target)
            {
                num_targets++;
                getNodeColor(graph->nodes[node_id], node_color_name_list[target_id]);
                snprintf(target_name_list[target_id], TIVX_EXPORT_MAX_NODE_COLOR_NAME, target_name);
            }
        }
    }

    for (target_id = 0; target_id < num_targets; target_id++)
    {
        TIVX_EXPORT_WRITELN(fp, "        <TR><TD bgcolor=\"%s\">%s</TD></TR>", node_color_name_list[target_id], target_name_list[target_id]);
    }

    TIVX_EXPORT_WRITELN(fp, "        </TABLE>>];\n");
    TIVX_EXPORT_WRITELN(fp, "");
    TIVX_EXPORT_WRITELN(fp, "");
}
#endif

static void exportDataRef(FILE *fp, vx_reference ref)
{
    if(ref != NULL)
    {
        char is_virtual[64]="";
        char is_virtual_label[64]="";
        char is_replicated_label[80]="";
        char line[TIVX_EXPORT_MAX_LINE_SIZE];

        if(ref->is_virtual != 0)
        {
            snprintf(is_virtual, 64, ", style=filled, fillcolor=lightgrey");
            snprintf(is_virtual_label, 64, "| virtual");
        }

        {
            if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                    ||
                (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
               )
            {
                snprintf(is_replicated_label, 80, "| [in] %s", ref->scope->name);
            }
        }
        TIVX_EXPORT_WRITELN(fp, "%s [shape=record %s, label=\"{%s %s %s}\"]",
                ref->name,
                is_virtual,
                ref->name,
                is_virtual_label,
                is_replicated_label
                );
        if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                ||
            (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
           )
        {
            exportDataRef(fp, ref->scope);
        }
    }
}

static void exportNodeObjDesc(FILE *fp, vx_node node, uint32_t pipe_id, const char *prefix)
{
    tivx_obj_desc_node_t *node_desc;
    char node_color_name[TIVX_EXPORT_MAX_NODE_COLOR_NAME];
    char line[TIVX_EXPORT_MAX_LINE_SIZE];

    if(node != NULL)
    {
        getNodeColor(node, node_color_name);
        node_desc = node->obj_desc[pipe_id];
        if(node_desc != NULL)
        {
            TIVX_EXPORT_WRITELN(fp, "%s%d [shape=record, label=\"{%s|pipe %d|desc %d}\", style=filled, fillcolor=%s]",
                prefix,
                node_desc->base.obj_desc_id,
                node->base.name,
                node_desc->pipeline_id,
                node_desc->base.obj_desc_id,
                node_color_name);
        }
    }
}

static void exportDataRefObjDesc(FILE *fp, vx_reference ref)
{
    if(ref != NULL)
    {
        char is_virtual[64]="";
        char is_virtual_label[64]="";
        char is_replicated_label[80]="";
        char line[TIVX_EXPORT_MAX_LINE_SIZE];

        if(ref->is_virtual != 0)
        {
            snprintf(is_virtual, 64, ", style=filled, fillcolor=lightgrey");
            snprintf(is_virtual_label, 64, "| virtual");
        }
        if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
            ||
            (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
        )
        {
            snprintf(is_replicated_label, 80, "| [in] %s", ref->scope->name);
        }
        if(ref->obj_desc != NULL)
        {
            TIVX_EXPORT_WRITELN(fp, "d_%d [shape=record %s, label=\"{%s %s %s | desc %d}\"]",
                ref->obj_desc->obj_desc_id,
                is_virtual,
                ref->name,
                is_virtual_label,
                is_replicated_label,
                ref->obj_desc->obj_desc_id
                );
        }
        if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
            ||
            (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
        )
        {
            exportDataRefObjDesc(fp, ref->scope);
        }
    }
}

static void exportDataRefQueueObjDesc(FILE *fp, tivx_data_ref_queue ref,
            uint32_t num_buf, vx_bool is_graph_parameter, vx_bool show_delay_links, uint32_t pipeline_depth)
{
    if(ref != NULL)
    {
        char line[TIVX_EXPORT_MAX_LINE_SIZE];
        char graph_parameter_label[64]="";
        uint32_t pipe_id;

        if(is_graph_parameter != 0)
        {
            snprintf(graph_parameter_label, 64, ", fillcolor=yellow");
        }
        else
        {
            snprintf(graph_parameter_label, 64, ", fillcolor=lightgrey");
        }

        for(pipe_id=0; pipe_id<pipeline_depth; pipe_id++)
        {
            if(ref->obj_desc[pipe_id] != NULL)
            {
                char auto_age_delay_string[64]="";
                uint32_t flags;

                flags = ref->obj_desc[pipe_id]->flags;

                if(tivxFlagIsBitSet(flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_IN_DELAY) != 0)
                {
                    if(tivxFlagIsBitSet(flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_DELAY_SLOT_AUTO_AGE) != 0)
                    {
                        snprintf(auto_age_delay_string, 64, "|delay_slot_auto_age");
                    }
                }
                TIVX_EXPORT_WRITELN(fp, "d_%d [shape=record, style=filled %s, label=\"{%s|bufs %d|pipe %d|in_nodes %d|desc %d %s}\"]",
                    ref->obj_desc[pipe_id]->base.obj_desc_id,
                    graph_parameter_label,
                    ref->base.name,
                    num_buf,
                    pipe_id,
                    ref->obj_desc[pipe_id]->num_in_nodes,
                    ref->obj_desc[pipe_id]->base.obj_desc_id,
                    auto_age_delay_string
                    );

                if(show_delay_links != 0)
                {
                    if(tivxFlagIsBitSet(ref->obj_desc[pipe_id]->flags, TIVX_OBJ_DESC_DATA_REF_Q_FLAG_IS_IN_DELAY) != 0)
                    {
                        tivx_obj_desc_data_ref_q_t *next_obj_desc;

                        next_obj_desc = (tivx_obj_desc_data_ref_q_t *)
                                            tivxObjDescGet(ref->obj_desc[pipe_id]->next_obj_desc_id_in_delay);

                        if(next_obj_desc != NULL)
                        {
                            TIVX_EXPORT_WRITELN(fp, "d_%d -> d_%d [label = %d, style=dashed, color=gray]",
                                ref->obj_desc[pipe_id]->base.obj_desc_id,
                                next_obj_desc->base.obj_desc_id,
                                -(((int32_t)ref->obj_desc[pipe_id]->delay_slot_index+1)%(int32_t)ref->obj_desc[pipe_id]->delay_slots));
                        }
                    }
                }
            }
        }
    }
}

static void exportDataRefQueue(FILE *fp, tivx_data_ref_queue ref, uint32_t num_buf, vx_bool is_graph_parameter)
{
    if(ref != NULL)
    {
        char line[TIVX_EXPORT_MAX_LINE_SIZE];
        char graph_parameter_label[64]="";

        if(is_graph_parameter != 0)
        {
            snprintf(graph_parameter_label, 64, ", fillcolor=yellow");
        }
        else
        {
            snprintf(graph_parameter_label, 64, ", fillcolor=lightgrey");
        }

        TIVX_EXPORT_WRITELN(fp, "d_%s [shape=record, style=filled %s, label=\"{%s|bufs %d}\"]",
            ref->base.name,
            graph_parameter_label,
            ref->base.name,
            num_buf
            );
    }
}

static vx_status tivxExportGraphTopLevelToDot(vx_graph graph, const char *output_file_path, const char *output_file_prefix)
{
    vx_status status = (vx_status)VX_SUCCESS;

    char filename[TIVX_EXPORT_MAX_FILENAME];

    snprintf(filename, TIVX_EXPORT_MAX_FILENAME,
        "%s/%s_1_img.txt",
        output_file_path,
        output_file_prefix);

    FILE *fp = fopen(filename, "wb");

    if(fp!=NULL)
    {
        char line[TIVX_EXPORT_MAX_LINE_SIZE];
        char node_color_name[TIVX_EXPORT_MAX_NODE_COLOR_NAME];
        uint32_t node_id, data_id;
        vx_reference ref;
        vx_node node;
        vx_enum dir;

        TIVX_EXPORT_WRITELN(fp, "digraph %s {", output_file_prefix);
        TIVX_EXPORT_WRITELN(fp, "");

        /* Note: everything is set to DSP1 for PC, and is therefore misleading */
        #ifndef PC
        exportTargetLegend(fp, graph);
        #endif

        /* List nodes within a graph */
        TIVX_EXPORT_WRITELN(fp, "/* List of Nodes */");
        for(node_id=0; node_id<graph->num_nodes; node_id++)
        {
            node = graph->nodes[node_id];
            if(node!=NULL)
            {
                getNodeColor(node, node_color_name);
                TIVX_EXPORT_WRITELN(fp,
                    "_%s [label = \"%s\", shape=box, color=%s, style=filled]",
                    node->base.name,
                    node->base.name,
                    node_color_name);
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");

        /* List data references within a graph */
        TIVX_EXPORT_WRITELN(fp, "/* List of Data References */");
        for(data_id=0; data_id<graph->num_data_ref; data_id++)
        {
            ref = graph->data_ref[data_id];

            exportDataRef(fp, ref);

        }
        TIVX_EXPORT_WRITELN(fp, "");
        /* link delays */
        data_id = 0;
        while(graph->delays[data_id]!=NULL)
        {
            vx_delay delay = graph->delays[data_id];
            uint32_t slot_id;

            for(slot_id=0U; slot_id<delay->count; slot_id++)
            {
                if((delay->refs[slot_id] != NULL)
                    &&
                    ((delay->refs[(slot_id+1U)%delay->count] != NULL))
                )
                {
                    exportDataRef(fp, delay->refs[slot_id]);

                    TIVX_EXPORT_WRITELN(fp, "%s -> %s [label = %d, style=dashed, color=gray]\n",
                        delay->refs[slot_id]->name,
                        delay->refs[(slot_id+1U)%delay->count]->name,
                        -(((int32_t)slot_id+1)%(int32_t)delay->count)
                        );
                }
            }
            data_id++;
        }
        TIVX_EXPORT_WRITELN(fp, "");

        /* link nodes to data references within a graph */
        for(node_id=0; node_id<graph->num_nodes; node_id++)
        {
            node = graph->nodes[node_id];
            if(node != NULL)
            {
                for(data_id=0; data_id<ownNodeGetNumParameters(node); data_id++)
                {
                    char replicated_label[32]="";
                    vx_bool is_replicated;

                    dir = ownNodeGetParameterDir(node, data_id);
                    is_replicated = ownNodeIsPrmReplicated(node, data_id);
                    if(is_replicated != 0)
                    {
                        snprintf(replicated_label, 32, "[label=\" replicated\"]");
                    }

                    ref = ownNodeGetParameterRef(node, data_id);
                    if(ref != NULL)
                    {
                        if(is_replicated != 0)
                        {
                            ref = ref->scope;
                            exportDataRef(fp, ref);
                        }
                        if(dir==(vx_enum)VX_INPUT)
                        {
                            TIVX_EXPORT_WRITELN(fp, "%s -> _%s %s",
                                ref->name,
                                node->base.name,
                                replicated_label);
                        }
                        else
                        {
                            TIVX_EXPORT_WRITELN(fp, "_%s -> %s %s",
                                node->base.name,
                                ref->name,
                                replicated_label );
                        }
                    }
                    else
                    {
                        TIVX_EXPORT_WRITELN(fp, "null_%s_%d [label=\"NULL\"]",
                                node->base.name,
                                data_id
                                );

                        /* optional parameter */
                        if(dir==(vx_enum)VX_INPUT)
                        {
                            TIVX_EXPORT_WRITELN(fp, "null_%s_%d -> _%s %s",
                                node->base.name,
                                data_id,
                                node->base.name,
                                replicated_label);
                        }
                        else
                        {
                            TIVX_EXPORT_WRITELN(fp, "_%s -> null_%s_%d %s",
                                node->base.name,
                                node->base.name,
                                data_id,
                                replicated_label);
                        }
                    }
                }
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");
        TIVX_EXPORT_WRITELN(fp, "}");

        fclose(fp);

        exportAsJpg(output_file_path, output_file_prefix, "_1", filename);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to open file [%s]", filename);
        status = (vx_status)VX_ERROR_NO_RESOURCES;
    }
    return status;
}

static void tivxExportGraphDataRefQueueToDot(FILE *fp, vx_graph graph,
                    tivx_data_ref_queue data_ref_q, uint32_t graph_parameter_index )
{
    uint32_t pipe_id;
    char line[TIVX_EXPORT_MAX_LINE_SIZE];

    if(data_ref_q  != NULL)
    {
        for(pipe_id=0; pipe_id<data_ref_q->pipeline_depth; pipe_id++)
        {
            if(data_ref_q->obj_desc[pipe_id]!=NULL)
            {
                if((vx_enum)data_ref_q->obj_desc[pipe_id]->ref_consumed_cmd_obj_desc_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                {
                    TIVX_EXPORT_WRITELN(fp, "dq_cmd_%d [shape=record, label=\"{response cmd|desc %d}\", style=filled]",
                        data_ref_q->obj_desc[pipe_id]->ref_consumed_cmd_obj_desc_id,
                        data_ref_q->obj_desc[pipe_id]->base.obj_desc_id
                    );

                    TIVX_EXPORT_WRITELN(fp, "d_%d -> dq_cmd_%d\n",
                        data_ref_q->obj_desc[pipe_id]->base.obj_desc_id,
                        data_ref_q->obj_desc[pipe_id]->ref_consumed_cmd_obj_desc_id
                        );
                }
                if((vx_enum)data_ref_q->obj_desc[pipe_id]->release_q_obj_desc_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                {
                    TIVX_EXPORT_WRITELN(fp, "d_%d -> queue_%d [label=\"release\"]\n",
                        data_ref_q->obj_desc[pipe_id]->base.obj_desc_id,
                        data_ref_q->obj_desc[pipe_id]->release_q_obj_desc_id
                        );
                }

                if((vx_enum)data_ref_q->obj_desc[pipe_id]->acquire_q_obj_desc_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                {
                    TIVX_EXPORT_WRITELN(fp, "queue_%d -> d_%d [label=\"acquire\"]\n",
                        data_ref_q->obj_desc[pipe_id]->acquire_q_obj_desc_id,
                        data_ref_q->obj_desc[pipe_id]->base.obj_desc_id
                        );
                }
            }
        }

        if((vx_enum)data_ref_q->done_q_obj_desc_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
        {
            TIVX_EXPORT_WRITELN(fp, "queue_%d -> dequeue_%d [label=\"done\"]\n",
                data_ref_q->done_q_obj_desc_id,
                graph_parameter_index
                );
        }

        if((vx_enum)data_ref_q->ready_q_obj_desc_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
        {
            TIVX_EXPORT_WRITELN(fp, "enqueue_%d -> queue_%d [label=\"ready\"]\n",
                graph_parameter_index,
                data_ref_q->ready_q_obj_desc_id
                );
        }
        if((vx_enum)data_ref_q->acquire_q_obj_desc_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
        {
            tivx_obj_desc_queue_t *obj_desc_queue
                = (tivx_obj_desc_queue_t *)tivxObjDescGet(data_ref_q->acquire_q_obj_desc_id);

            if((obj_desc_queue != NULL) && (obj_desc_queue->count > 0U))
            {
                uint32_t data_id;

                /* list initially enqueued ref obj desc */
                for(data_id=0; data_id<obj_desc_queue->count; data_id++)
                {
                    uint16_t obj_desc_id = obj_desc_queue->queue_mem[data_id];
                    vx_reference ref = ownReferenceGetHandleFromObjDescId(obj_desc_id);

                    if(ref!=NULL)
                    {
                        if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                            ||
                            (   ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
                        )
                        {
                            ref = ref->scope;
                        }
                        exportDataRefObjDesc(fp, ref);
                    }

                    if(ref && ref->obj_desc)
                    {
                        TIVX_EXPORT_WRITELN(fp, "d_%d -> queue_%d\n",
                            ref->obj_desc->obj_desc_id,
                            data_ref_q->acquire_q_obj_desc_id);
                    }
                }
            }
        }
    }
}

static vx_status tivxExportGraphDataRefQueuesToDot(vx_graph graph, const char *output_file_path, const char *output_file_prefix)
{
    vx_status status = (vx_status)VX_SUCCESS;
    char filename[TIVX_EXPORT_MAX_FILENAME];

    snprintf(filename, TIVX_EXPORT_MAX_FILENAME, "%s/%s_3_data_ref_q_img.txt", output_file_path, output_file_prefix);

    FILE *fp = fopen(filename, "wb");

    if(fp!=NULL)
    {
        char line[TIVX_EXPORT_MAX_LINE_SIZE];
        uint32_t i;

        TIVX_EXPORT_WRITELN(fp, "digraph %s {", output_file_prefix);
        TIVX_EXPORT_WRITELN(fp, "");

        for(i=0; i<graph->num_params; i++)
        {
            if(graph->parameters[i].queue_enable != 0)
            {
                if (graph->parameters[i].data_ref_queue != NULL)
                {
                    exportDataRefQueueObjDesc(fp, graph->parameters[i].data_ref_queue, graph->parameters[i].num_buf,
                        (vx_bool)vx_true_e, (vx_bool)vx_false_e, graph->parameters[i].data_ref_queue->pipeline_depth);
                    tivxExportGraphDataRefQueueToDot(fp, graph, graph->parameters[i].data_ref_queue, i);
                }
            }
        }
        for(i=0; i<graph->num_data_ref_q; i++)
        {
            if (graph->data_ref_q_list[i].data_ref_queue != NULL)
            {
                exportDataRefQueueObjDesc(fp, graph->data_ref_q_list[i].data_ref_queue, graph->data_ref_q_list[i].num_buf,
                    (vx_bool)vx_false_e, (vx_bool)vx_false_e, graph->data_ref_q_list[i].data_ref_queue->pipeline_depth);
                tivxExportGraphDataRefQueueToDot(fp, graph, graph->data_ref_q_list[i].data_ref_queue,
                    graph->num_params+i);
            }
        }
        for(i=0; i<graph->num_delay_data_ref_q; i++)
        {
            if (graph->delay_data_ref_q_list[i].data_ref_queue != NULL)
            {
                exportDataRefQueueObjDesc(fp, graph->delay_data_ref_q_list[i].data_ref_queue, 1,
                    (vx_bool)vx_false_e, (vx_bool)vx_false_e, graph->delay_data_ref_q_list[i].data_ref_queue->pipeline_depth);
                tivxExportGraphDataRefQueueToDot(fp, graph, graph->delay_data_ref_q_list[i].data_ref_queue,
                    graph->num_params+graph->num_data_ref_q+i);
            }
        }

        TIVX_EXPORT_WRITELN(fp, "");
        TIVX_EXPORT_WRITELN(fp, "}");

        fclose(fp);

        exportAsJpg(output_file_path, output_file_prefix, "_3_data_ref_q", filename);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to open file [%s]", filename);
        status = (vx_status)VX_ERROR_NO_RESOURCES;
    }
    return status;
}

static vx_status tivxExportGraphFirstPipelineToDot(vx_graph graph, const char *output_file_path, const char *output_file_prefix)
{
    vx_status status = (vx_status)VX_SUCCESS;
    char filename[TIVX_EXPORT_MAX_FILENAME];

    snprintf(filename, TIVX_EXPORT_MAX_FILENAME,
                "%s/%s_4_pipe0_img.txt",
                output_file_path,
                output_file_prefix);

    FILE *fp = fopen(filename, "wb");

    if(fp!=NULL)
    {
        char line[TIVX_EXPORT_MAX_LINE_SIZE];
        uint32_t node_id, data_id, pipe_id = 0, linked_node_idx;
        tivx_obj_desc_node_t *node_desc;
        vx_node node;

        TIVX_EXPORT_WRITELN(fp, "digraph %s {", output_file_prefix);
        TIVX_EXPORT_WRITELN(fp, "");

        /* Note: everything is set to DSP1 for PC, and is therefore misleading */
        #ifndef PC
        exportTargetLegend(fp, graph);
        #endif

        /* List node descriptor for each node and each pipeline in a graph */
        TIVX_EXPORT_WRITELN(fp, "/* List of nodes ( Pipeline = %d )*/", pipe_id);
        for(node_id=0; node_id<graph->num_nodes; node_id++)
        {
            node = graph->nodes[node_id];
            exportNodeObjDesc(fp, node, pipe_id, "n_");
        }
        TIVX_EXPORT_WRITELN(fp, "");

        /* link the node descriptors within a pipeline to each other based on graph structure */
        TIVX_EXPORT_WRITELN(fp, "/* Dependancy of nodes within pipeline (Pipeline = %d) */",
            pipe_id);
        for(node_id=0; node_id<graph->num_nodes; node_id++)
        {
            node = graph->nodes[node_id];
            if(node != NULL)
            {
                node_desc = node->obj_desc[pipe_id];
                if(node_desc != NULL)
                {
                    #if 1
                    for(linked_node_idx=0; linked_node_idx<node_desc->num_in_nodes; linked_node_idx++)
                    {
                        TIVX_EXPORT_WRITELN(fp, "n_%d -> n_%d [style=dashed, color=gray]",
                            node_desc->in_node_id[linked_node_idx],
                            node_desc->base.obj_desc_id);
                    }
                    #endif
                    for(linked_node_idx=0; linked_node_idx<node_desc->num_out_nodes; linked_node_idx++)
                    {
                        TIVX_EXPORT_WRITELN(fp, "n_%d -> n_%d [style=dashed]",
                            node_desc->base.obj_desc_id,
                            node_desc->out_node_id[linked_node_idx]);
                    }
                }
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");

        #if 1
        TIVX_EXPORT_WRITELN(fp, "/* List of data reference queues */");
        for(data_id=0; data_id<graph->num_params; data_id++)
        {
            if(graph->parameters[data_id].queue_enable != 0)
            {
                uint32_t num_buf = graph->parameters[data_id].num_buf;
                tivx_data_ref_queue ref = graph->parameters[data_id].data_ref_queue;

                exportDataRefQueueObjDesc(fp, ref, num_buf, (vx_bool)vx_true_e, (vx_bool)vx_true_e, 1);
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");
        for(data_id=0; data_id<graph->num_data_ref_q; data_id++)
        {
            uint32_t num_buf = graph->data_ref_q_list[data_id].num_buf;
            tivx_data_ref_queue ref = graph->data_ref_q_list[data_id].data_ref_queue;

            exportDataRefQueueObjDesc(fp, ref, num_buf, (vx_bool)vx_false_e, (vx_bool)vx_true_e, 1);
        }
        TIVX_EXPORT_WRITELN(fp, "");
        for(data_id=0; data_id<graph->num_delay_data_ref_q; data_id++)
        {
            tivx_data_ref_queue ref = graph->delay_data_ref_q_list[data_id].data_ref_queue;

            exportDataRefQueueObjDesc(fp, ref, 1, (vx_bool)vx_false_e, (vx_bool)vx_true_e, 1);
        }
        TIVX_EXPORT_WRITELN(fp, "");
        #endif
        #if 1
        /* link data descriptor to node descriptors depending on the graph structure */
        TIVX_EXPORT_WRITELN(fp, "/* Dependancy of nodes within pipeline (Pipeline = %d) */",
            pipe_id);
        for(node_id=0; node_id<graph->num_nodes; node_id++)
        {
            node = graph->nodes[node_id];
            if(node != NULL)
            {
                node_desc = node->obj_desc[pipe_id];
                if(node_desc != NULL)
                {
                    for(data_id=0; data_id<node_desc->num_params; data_id++)
                    {
                        char replicated_label[32]="";
                        vx_bool is_replicated;

                        is_replicated = tivxFlagIsBitSet(node_desc->is_prm_replicated, ((uint32_t)1<<(uint32_t)data_id));
                        if(is_replicated != 0)
                        {
                            snprintf(replicated_label, 32, "[label=\" replicated\"]");
                        }

                        if(tivxFlagIsBitSet(node_desc->is_prm_input, ((uint32_t)1<<(uint32_t)data_id)) != 0)
                        {
                            if(tivxFlagIsBitSet(node_desc->is_prm_data_ref_q, ((uint32_t)1<<(uint32_t)data_id)) != 0)
                            {
                                if((vx_enum)node_desc->data_ref_q_id[data_id]!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                                {
                                    TIVX_EXPORT_WRITELN(fp, "d_%d -> n_%d %s", node_desc->data_ref_q_id[data_id], node_desc->base.obj_desc_id, replicated_label);
                                }
                            }
                            else
                            {
                                if((vx_enum)node_desc->data_id[data_id]!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                                {
                                    TIVX_EXPORT_WRITELN(fp, "d_%d -> n_%d %s", node_desc->data_id[data_id], node_desc->base.obj_desc_id, replicated_label);
                                }
                            }
                        }
                        else
                        {
                            if(tivxFlagIsBitSet(node_desc->is_prm_data_ref_q, ((uint32_t)1<<(uint32_t)data_id)) != 0)
                            {
                                if((vx_enum)node_desc->data_ref_q_id[data_id]!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                                {
                                    TIVX_EXPORT_WRITELN(fp, "n_%d -> d_%d %s", node_desc->base.obj_desc_id, node_desc->data_ref_q_id[data_id], replicated_label);
                                }
                            }
                            else
                            {
                                if((vx_enum)node_desc->data_id[data_id]!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                                {
                                    TIVX_EXPORT_WRITELN(fp, "n_%d -> d_%d %s", node_desc->base.obj_desc_id, node_desc->data_id[data_id], replicated_label);
                                }
                            }
                        }
                    }
                }
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");
        #endif

        TIVX_EXPORT_WRITELN(fp, "");
        TIVX_EXPORT_WRITELN(fp, "}");

        fclose(fp);

        exportAsJpg(output_file_path, output_file_prefix, "_4_pipe0", filename);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to open file [%s]", filename);
        status = (vx_status)VX_ERROR_NO_RESOURCES;
    }
    return status;
}

static vx_status tivxExportGraphPipelineToDot(vx_graph graph, const char *output_file_path, const char *output_file_prefix)
{
    vx_status status = (vx_status)VX_SUCCESS;
    char filename[TIVX_EXPORT_MAX_FILENAME];

    snprintf(filename, TIVX_EXPORT_MAX_FILENAME,
                "%s/%s_2_pipe_img.txt",
                output_file_path,
                output_file_prefix);

    FILE *fp = fopen(filename, "wb");

    if(fp!=NULL)
    {
        char line[TIVX_EXPORT_MAX_LINE_SIZE];
        uint32_t node_id, data_id, pipe_id, linked_node_idx, prm_id, buf_id;
        tivx_obj_desc_node_t *node_desc;
        vx_reference ref;
        vx_node node;

        TIVX_EXPORT_WRITELN(fp, "digraph %s {", output_file_prefix);
        TIVX_EXPORT_WRITELN(fp, "");

        /* Note: everything is set to DSP1 for PC, and is therefore misleading */
        #ifndef PC
        exportTargetLegend(fp, graph);
        #endif

        /* List node descriptor for each node and each pipeline in a graph */
        for(pipe_id=0; pipe_id<graph->pipeline_depth; pipe_id++)
        {
            TIVX_EXPORT_WRITELN(fp, "/* List of nodes ( Pipeline = %d )*/", pipe_id);
            for(node_id=0; node_id<graph->num_nodes; node_id++)
            {
                node = graph->nodes[node_id];
                exportNodeObjDesc(fp, node, pipe_id, "n_");
            }
            TIVX_EXPORT_WRITELN(fp, "");
        }
        /* link the node descriptors within a pipeline to each other based on graph structure */
        for(pipe_id=0; pipe_id<graph->pipeline_depth; pipe_id++)
        {
            TIVX_EXPORT_WRITELN(fp, "/* Dependancy of nodes within pipeline (Pipeline = %d) */",
                pipe_id);
            for(node_id=0; node_id<graph->num_nodes; node_id++)
            {
                node = graph->nodes[node_id];
                if(node != NULL)
                {
                    node_desc = node->obj_desc[pipe_id];
                    if(node_desc != NULL)
                    {
                        #if 1
                        for(linked_node_idx=0; linked_node_idx<node_desc->num_in_nodes; linked_node_idx++)
                        {
                            TIVX_EXPORT_WRITELN(fp, "n_%d -> n_%d [style=dashed, color=gray]",
                                node_desc->in_node_id[linked_node_idx],
                                node_desc->base.obj_desc_id);
                        }
                        #endif
                        for(linked_node_idx=0; linked_node_idx<node_desc->num_out_nodes; linked_node_idx++)
                        {
                            TIVX_EXPORT_WRITELN(fp, "n_%d -> n_%d [style=dashed]",
                                node_desc->base.obj_desc_id,
                                node_desc->out_node_id[linked_node_idx]);
                        }
                    }
                }
            }
            TIVX_EXPORT_WRITELN(fp, "");
        }
        /* List node descriptor for each node and each pipeline in a graph, this time with a different name "ln" */
        for(pipe_id=0; pipe_id<graph->pipeline_depth; pipe_id++)
        {
            TIVX_EXPORT_WRITELN(fp, "/* List of nodes ( Pipeline = %d )*/",
                pipe_id);
            for(node_id=0; node_id<graph->num_nodes; node_id++)
            {
                node = graph->nodes[node_id];
                exportNodeObjDesc(fp, node, pipe_id, "ln_");
            }
            TIVX_EXPORT_WRITELN(fp, "");
        }

        /* link the node descriptors across pipelines to each other based on dependancy across pipelines */
        for(pipe_id=0; pipe_id<graph->pipeline_depth; pipe_id++)
        {
            TIVX_EXPORT_WRITELN(fp, "/* Dependancy of nodes across pipeline ( Pipeline = %d ) */",
                pipe_id);
            for(node_id=0; node_id<graph->num_nodes; node_id++)
            {
                node = graph->nodes[node_id];
                if(node != NULL)
                {
                    node_desc = node->obj_desc[pipe_id];
                    if(node_desc != NULL)
                    {
                        if((vx_enum)node_desc->prev_pipe_node_id!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                        {
                            TIVX_EXPORT_WRITELN(fp, "ln_%d -> ln_%d [style=dashed]",
                                node_desc->prev_pipe_node_id,
                                node_desc->base.obj_desc_id);
                        }
                    }
                }
            }
            TIVX_EXPORT_WRITELN(fp, "");
        }
        /* List data references within a graph */
        TIVX_EXPORT_WRITELN(fp, "/* List of Data References */");
        for(data_id=0; data_id<graph->num_data_ref; data_id++)
        {
            ref = graph->data_ref[data_id];

            exportDataRefObjDesc(fp, ref);
        }
        TIVX_EXPORT_WRITELN(fp, "");


        for(prm_id=0; prm_id<graph->num_params; prm_id++)
        {
            if(graph->parameters[prm_id].queue_enable && graph->parameters[prm_id].data_ref_queue)
            {
                exportDataRefQueue(fp,
                    graph->parameters[prm_id].data_ref_queue,
                    graph->parameters[prm_id].num_buf,
                    (vx_bool)vx_true_e);

                for(buf_id=0; buf_id<graph->parameters[prm_id].num_buf; buf_id++)
                {
                    ref = graph->parameters[prm_id].refs_list[buf_id];

                    if(ref!=NULL)
                    {
                        if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                            ||
                            (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
                        )
                        {
                            ref = ref->scope;
                        }

                        exportDataRefObjDesc(fp, ref);
                    }
                    if(ref && ref->obj_desc)
                    {
                        TIVX_EXPORT_WRITELN(fp, "d_%d -> d_%s",
                            ref->obj_desc->obj_desc_id,
                            graph->parameters[prm_id].data_ref_queue->base.name);
                    }
                }
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");

        for(prm_id=0; prm_id<graph->num_data_ref_q; prm_id++)
        {
            if(graph->data_ref_q_list[prm_id].data_ref_queue != NULL)
            {
                exportDataRefQueue(fp,
                    graph->data_ref_q_list[prm_id].data_ref_queue,
                    graph->data_ref_q_list[prm_id].num_buf,
                    (vx_bool)vx_false_e);

                for(buf_id=0; buf_id<graph->data_ref_q_list[prm_id].num_buf; buf_id++)
                {
                    ref = graph->data_ref_q_list[prm_id].refs_list[buf_id];

                    if ((ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                        ||
                        (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
                    )
                    {
                        ref = ref->scope;
                    }

                    exportDataRefObjDesc(fp, ref);
                    if(ref && ref->obj_desc)
                    {
                        TIVX_EXPORT_WRITELN(fp, "d_%d -> d_%s",
                            ref->obj_desc->obj_desc_id,
                            graph->data_ref_q_list[prm_id].data_ref_queue->base.name);
                    }
                }
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");

        for(prm_id=0; prm_id<graph->num_delay_data_ref_q; prm_id++)
        {
            if(graph->delay_data_ref_q_list[prm_id].data_ref_queue != NULL)
            {
                exportDataRefQueue(fp,
                    graph->delay_data_ref_q_list[prm_id].data_ref_queue,
                    1,
                    (vx_bool)vx_false_e);

                if(graph->delay_data_ref_q_list[prm_id].node != NULL)
                {
                    ref = ownNodeGetParameterRef(graph->delay_data_ref_q_list[prm_id].node,
                            graph->delay_data_ref_q_list[prm_id].index);

                    exportDataRefObjDesc(fp, ref);
                    if(ref && ref->obj_desc)
                    {
                        TIVX_EXPORT_WRITELN(fp, "d_%d -> d_%s",
                            ref->obj_desc->obj_desc_id,
                            graph->delay_data_ref_q_list[prm_id].data_ref_queue->base.name);
                    }
                }
                else
                {
                    vx_delay delay = graph->delay_data_ref_q_list[prm_id].delay_ref;
                    uint32_t delay_slot_index = graph->delay_data_ref_q_list[prm_id].delay_slot_index;

                    if(delay != NULL)
                    {
                        ref = delay->refs[delay_slot_index];
                        if(ref != NULL)
                        {
                            exportDataRefObjDesc(fp, ref);
                            if(ref->obj_desc != NULL)
                            {
                                TIVX_EXPORT_WRITELN(fp, "d_%d -> d_%s",
                                    ref->obj_desc->obj_desc_id,
                                    graph->delay_data_ref_q_list[prm_id].data_ref_queue->base.name);
                            }
                        }
                    }
                }
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");
        #if 1
        TIVX_EXPORT_WRITELN(fp, "/* List of data reference queues */");
        for(data_id=0; data_id<graph->num_params; data_id++)
        {
            if(graph->parameters[data_id].queue_enable != 0)
            {
                uint32_t num_buf = graph->parameters[data_id].num_buf;
                tivx_data_ref_queue ref = graph->parameters[data_id].data_ref_queue;

                if (ref != NULL)
                {
                    exportDataRefQueueObjDesc(fp, ref, num_buf, (vx_bool)vx_true_e, (vx_bool)vx_true_e, ref->pipeline_depth);
                }
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");
        for(data_id=0; data_id<graph->num_data_ref_q; data_id++)
        {
            uint32_t num_buf = graph->data_ref_q_list[data_id].num_buf;
            tivx_data_ref_queue ref = graph->data_ref_q_list[data_id].data_ref_queue;

            if (ref != NULL)
            {
                exportDataRefQueueObjDesc(fp, ref, num_buf, (vx_bool)vx_false_e, (vx_bool)vx_true_e, ref->pipeline_depth);
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");
        for(data_id=0; data_id<graph->num_delay_data_ref_q; data_id++)
        {
            tivx_data_ref_queue ref = graph->delay_data_ref_q_list[data_id].data_ref_queue;

            if (ref != NULL)
            {
                exportDataRefQueueObjDesc(fp, ref, 1, (vx_bool)vx_false_e, (vx_bool)vx_true_e, ref->pipeline_depth);
            }
        }
        TIVX_EXPORT_WRITELN(fp, "");

        /* link data descriptor to node descriptors depending on the graph structure */
        for(pipe_id=0; pipe_id<graph->pipeline_depth; pipe_id++)
        {
            TIVX_EXPORT_WRITELN(fp, "/* Dependancy of nodes within pipeline (Pipeline = %d) */",
                pipe_id);
            for(node_id=0; node_id<graph->num_nodes; node_id++)
            {
                node = graph->nodes[node_id];
                if(node != NULL)
                {
                    node_desc = node->obj_desc[pipe_id];
                    if(node_desc != NULL)
                    {
                        for(data_id=0; data_id<node_desc->num_params; data_id++)
                        {
                            char replicated_label[32]="";
                            vx_bool is_replicated;

                            is_replicated = tivxFlagIsBitSet(node_desc->is_prm_replicated, ((uint32_t)1<<(uint32_t)data_id));
                            if(is_replicated != 0)
                            {
                                snprintf(replicated_label, 32, "[label=\" replicated\"]");
                            }

                            if(tivxFlagIsBitSet(node_desc->is_prm_input, ((uint32_t)1<<(uint32_t)data_id)) != 0)
                            {
                                if(tivxFlagIsBitSet(node_desc->is_prm_data_ref_q, ((uint32_t)1<<(uint32_t)data_id)) != 0)
                                {
                                    if((vx_enum)node_desc->data_ref_q_id[data_id]!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                                    {
                                        TIVX_EXPORT_WRITELN(fp, "d_%d -> n_%d %s", node_desc->data_ref_q_id[data_id], node_desc->base.obj_desc_id, replicated_label);
                                    }
                                }
                                else
                                {
                                    if((vx_enum)node_desc->data_id[data_id]!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                                    {
                                        TIVX_EXPORT_WRITELN(fp, "d_%d -> n_%d %s", node_desc->data_id[data_id], node_desc->base.obj_desc_id, replicated_label);
                                    }
                                }
                            }
                            else
                            {
                                if(tivxFlagIsBitSet(node_desc->is_prm_data_ref_q, ((uint32_t)1<<(uint32_t)data_id)) != 0)
                                {
                                    if((vx_enum)node_desc->data_ref_q_id[data_id]!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                                    {
                                        TIVX_EXPORT_WRITELN(fp, "n_%d -> d_%d %s", node_desc->base.obj_desc_id, node_desc->data_ref_q_id[data_id], replicated_label);
                                    }
                                }
                                else
                                {
                                    if((vx_enum)node_desc->data_id[data_id]!=(vx_enum)TIVX_OBJ_DESC_INVALID)
                                    {
                                        TIVX_EXPORT_WRITELN(fp, "n_%d -> d_%d %s", node_desc->base.obj_desc_id, node_desc->data_id[data_id], replicated_label);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            TIVX_EXPORT_WRITELN(fp, "");
        }
        #endif

        TIVX_EXPORT_WRITELN(fp, "");
        TIVX_EXPORT_WRITELN(fp, "}");

        fclose(fp);

        exportAsJpg(output_file_path, output_file_prefix, "_2_pipe", filename);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to open file [%s]", filename);
        status = (vx_status)VX_ERROR_NO_RESOURCES;
    }
    return status;
}

vx_status tivxExportGraphToDot(vx_graph graph, const char *output_file_path, const char *output_file_prefix)
{
    vx_status status = (vx_status)VX_FAILURE;

    if (   (NULL != graph)
        && (output_file_path!=NULL)
        && (output_file_prefix!=NULL)
        && (ownIsValidSpecificReference(&graph->base, (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
        && (graph->verified == (vx_bool)vx_true_e))
    {
        status = tivxExportGraphTopLevelToDot(graph, output_file_path, output_file_prefix);
        if(status==(vx_status)VX_SUCCESS)
        {
            status = tivxExportGraphPipelineToDot(graph, output_file_path, output_file_prefix);
        }
        if(status==(vx_status)VX_SUCCESS)
        {
            status = tivxExportGraphDataRefQueuesToDot(graph, output_file_path, output_file_prefix);
        }
        if(status==(vx_status)VX_SUCCESS)
        {
            status = tivxExportGraphFirstPipelineToDot(graph, output_file_path, output_file_prefix);
        }
        if(status!=(vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to export graph to dot");
            status = (vx_status)VX_FAILURE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters or graph node not verified");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}
