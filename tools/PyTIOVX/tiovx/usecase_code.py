#
# Copyright (c) 2017 Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided
# that the following conditions are met:
#
#       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
#       any redistribution and use are licensed by TI for use only with TI Devices.
#
#       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
#       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
#       any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

from . import *

class UsecaseCode :
    def __init__(self, context) :
        self.h_file = CodeGenerate(context.name + '.h')
        self.c_file = CodeGenerate(context.name + '.c')
        self.context = context
        self.context_code = ContextCode(context)

    def generate_h_code(self) :
        self.h_file.write_ifndef_define(self.context.name.upper())
        self.h_file.write_include("VX/vx.h")
        self.h_file.write_include("TI/tivx.h")
        self.h_file.write_newline()
        self.h_file.write_line('typedef struct _%s_t *%s;' % (self.context.name, self.context.name))
        self.h_file.write_newline()
        self.h_file.write_line('typedef struct _%s_t' % self.context.name)
        self.h_file.write_open_brace()
        self.context_code.declare_var(self.h_file)
        self.h_file.write_close_brace(' %s_t;' % self.context.name)
        self.h_file.write_newline()
        self.generate_declare_data_function("create")
        self.generate_declare_data_function("delete")
        self.h_file.write_newline()
        for graph in self.context.graph_list :
            self.generate_declare_graph_functions(graph)
        self.generate_declare_usecase_function("create")
        self.generate_declare_usecase_function("delete")
        self.generate_declare_usecase_function("verify")
        self.generate_declare_usecase_function("run")
        self.h_file.write_newline()
        self.h_file.write_endif(self.context.name.upper())
        self.h_file.close()

    def generate_declare_usecase_function(self, function_name) :
        self.h_file.write_line("vx_status %s_%s(%s usecase);" % (self.context.name, function_name, self.context.name) )

    def generate_declare_data_function(self, function_name) :
        self.h_file.write_line("vx_status %s_data_%s(%s usecase);" % (self.context.name, function_name, self.context.name) )

    def generate_declare_graph_functions(self, graph) :
        self.h_file.write_line("vx_status %s_%s_create(%s usecase);" % (self.context.name, graph.name, self.context.name) )
        self.h_file.write_line("vx_status %s_%s_delete(%s usecase);" % (self.context.name, graph.name, self.context.name) )
        self.h_file.write_line("vx_status %s_%s_verify(%s usecase);" % (self.context.name, graph.name, self.context.name) )
        self.h_file.write_line("vx_status %s_%s_run(%s usecase);" % (self.context.name, graph.name, self.context.name) )
        self.h_file.write_newline()

    def generate_define_node_code(self) :
        for node in self.context.node_list :
            NodeCode(node).define_create(self.c_file)

    def generate_create_graph_code(self, graph):
        self.c_file.write_line("vx_status %s_%s_create(%s usecase)" % (self.context.name, graph.name, self.context.name) )
        self.c_file.write_open_brace()
        self.c_file.write_define_status();
        self.c_file.write_newline()
        self.c_file.write_line("vx_graph graph = usecase->%s;" % graph.name)
        self.c_file.write_newline()
        GraphCode(graph).call_create(self.c_file)
        self.c_file.write_if_status();
        self.c_file.write_open_brace();
        GraphCode(graph).set_ref_name(self.c_file)
        self.c_file.write_close_brace()
        self.c_file.write_newline()
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_delete_graph_code(self, graph):
        self.c_file.write_line("vx_status %s_%s_delete(%s usecase)" % (self.context.name, graph.name, self.context.name) )
        self.c_file.write_open_brace()
        self.c_file.write_define_status();
        self.c_file.write_newline()
        self.c_file.write_line("vx_graph graph = usecase->%s;" % (graph.name))
        self.c_file.write_newline()
        GraphCode(graph).call_delete(self.c_file)
        self.c_file.write_line("usecase->%s = graph;" % (graph.name))
        self.c_file.write_newline()
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_verify_graph_code(self, graph):
        self.c_file.write_line("vx_status %s_%s_verify(%s usecase)" % (self.context.name, graph.name, self.context.name) )
        self.c_file.write_open_brace()
        self.c_file.write_define_status();
        self.c_file.write_newline()
        self.c_file.write_line("vx_graph graph = usecase->%s;" % (graph.name))
        self.c_file.write_newline()
        GraphCode(graph).call_verify(self.c_file)
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_run_graph_code(self, graph):
        self.c_file.write_line("vx_status %s_%s_run(%s usecase)" % (self.context.name, graph.name, self.context.name) )
        self.c_file.write_open_brace()
        self.c_file.write_define_status();
        self.c_file.write_newline()
        self.c_file.write_line("vx_graph graph = usecase->%s;" % (graph.name))
        self.c_file.write_newline()
        GraphCode(graph).call_run(self.c_file)
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_create_usecase_code(self) :
        self.c_file.write_line("vx_status %s_create(%s usecase)" % (self.context.name, self.context.name) )
        self.c_file.write_open_brace()
        self.c_file.write_define_status();
        self.c_file.write_newline()
        self.c_file.write_line("memset(usecase, 0, sizeof(%s_t));" % self.context.name);
        self.c_file.write_newline();
        self.c_file.write_if_status();
        self.c_file.write_open_brace()
        self.c_file.write_line("usecase->context = vxCreateContext();")
        self.c_file.write_line("if (usecase->context == NULL)");
        self.c_file.write_open_brace()
        self.c_file.write_line("status = VX_ERROR_NO_RESOURCES;");
        self.c_file.write_close_brace()
        self.c_file.write_close_brace()
        for graph in self.context.graph_list:
            # Create graph
            self.c_file.write_if_status();
            self.c_file.write_open_brace()
            self.c_file.write_line("usecase->%s = vxCreateGraph(usecase->context);" % graph.name)
            self.c_file.write_line("if (usecase->%s == NULL)" % graph.name);
            self.c_file.write_open_brace()
            self.c_file.write_line("status = VX_ERROR_NO_RESOURCES;");
            self.c_file.write_close_brace()
            self.c_file.write_close_brace()
        self.c_file.write_if_status();
        self.c_file.write_open_brace()
        self.c_file.write_line("status = %s_data_create(usecase);" % (self.context.name) )
        self.c_file.write_close_brace()

        for graph in self.context.graph_list :
            self.c_file.write_if_status();
            self.c_file.write_open_brace()
            self.c_file.write_line("status = %s_%s_create(usecase);" % (self.context.name, graph.name) )
            self.c_file.write_close_brace()

        self.c_file.write_newline()
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_delete_usecase_code(self):
        self.c_file.write_line("vx_status %s_delete(%s usecase)" % (self.context.name, self.context.name) )
        self.c_file.write_open_brace()
        self.c_file.write_define_status();
        self.c_file.write_newline()

        for graph in self.context.graph_list :
            self.c_file.write_if_status();
            self.c_file.write_open_brace()
            self.c_file.write_line("status = %s_%s_delete(usecase);" % (self.context.name, graph.name) )
            self.c_file.write_close_brace()

        self.c_file.write_if_status();
        self.c_file.write_open_brace()
        self.c_file.write_line("status = %s_data_delete(usecase);" % (self.context.name) )
        self.c_file.write_close_brace()

        self.c_file.write_if_status();
        self.c_file.write_open_brace()
        self.c_file.write_line("status = vxReleaseContext(&usecase->context);" )
        self.c_file.write_close_brace()

        self.c_file.write_newline()
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_usecase_function_code(self, function_name):
        self.c_file.write_line("vx_status %s_%s(%s usecase)" % (self.context.name, function_name, self.context.name) )
        self.c_file.write_open_brace()
        self.c_file.write_define_status();
        self.c_file.write_newline()

        for graph in self.context.graph_list :
            self.c_file.write_if_status();
            self.c_file.write_open_brace()
            self.c_file.write_line("status = %s_%s_%s(usecase);" % (self.context.name, graph.name, function_name) )
            self.c_file.write_close_brace()

        if (function_name=="delete") :
            self.c_file.write_if_status();
            self.c_file.write_open_brace()
            self.c_file.write_line("status = %s_data_%s(usecase);" % (self.context.name, function_name) )
            self.c_file.write_close_brace()

        self.c_file.write_newline()
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_create_data_code(self) :
        self.c_file.write_line("vx_status %s_data_create(%s usecase)" % (self.context.name, self.context.name) )
        self.c_file.write_open_brace()
        self.c_file.write_define_status();
        self.c_file.write_newline()
        self.c_file.write_line("vx_context context = usecase->context;")
        self.c_file.write_newline()
        for data in self.context.data_list :
            ContextCode.get_data_code_obj(data).call_create(self.c_file)
        self.c_file.write_newline()
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_delete_data_code(self) :
        self.c_file.write_line("vx_status %s_data_delete(%s usecase)" % (self.context.name, self.context.name) )
        self.c_file.write_open_brace()
        self.c_file.write_define_status();
        self.c_file.write_newline()
        for data in self.context.data_list :
            self.c_file.write_if_status();
            self.c_file.write_open_brace()
            self.c_file.write_line("status = vxReleaseReference((vx_reference*)&usecase->%s);" % (data.name) )
            self.c_file.write_close_brace()
        self.c_file.write_newline()
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_c_code(self) :
        self.c_file.write_include(self.context.name + '.h')
        self.c_file.write_newline()
        self.generate_create_usecase_code()
        self.generate_usecase_function_code("verify")
        self.generate_usecase_function_code("run")
        self.generate_delete_usecase_code()
        self.generate_create_data_code()
        self.generate_delete_data_code()
        self.generate_define_node_code()
        for graph in self.context.graph_list :
            self.generate_create_graph_code(graph)
            self.generate_delete_graph_code(graph)
            self.generate_verify_graph_code(graph)
            self.generate_run_graph_code(graph)
        self.c_file.close()

    def generate_code(self) :
        self.generate_h_code()
        self.generate_c_code()


