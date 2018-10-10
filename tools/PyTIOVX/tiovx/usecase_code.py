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
from glob import glob
import os, sys, re

class UsecaseCode :
    def __init__(self, context, env_var) :
        self.env_var = env_var
        self.workarea = os.environ.get(self.env_var)
        if self.workarea == None or self.workarea == "":
            sys.exit("ERROR: You must define %s environment variable as the root of the kernel workarea." % self.env_var);
        if self.env_var == "CUSTOM_APPLICATION_PATH" :
            self.workarea_app = self.workarea + "/" + "app_" + context.name
        else :
            self.workarea_app = self.workarea + "/apps/" + context.name + "/app_" + context.name
        self.create_directory(self.workarea_app)
        self.h_file = CodeGenerate(self.workarea_app + "/" + context.name + '.h')
        self.c_file = CodeGenerate(self.workarea_app + "/" + context.name + '.c')
        self.concerto_file = CodeGenerate(self.workarea_app + "/" + 'concerto.mak', header=False)
        self.workarea_kernel = self.workarea + "/kernels"

        # Generating necessary files if they don't exist
        if self.env_var == "CUSTOM_APPLICATION_PATH" :
            self.include_custom_kernel_library_tests_filename = self.workarea_kernel + "/custom_app_kernel_library_tests.h"
            self.create_directory(self.workarea_kernel)
            if not os.path.exists(self.include_custom_kernel_library_tests_filename):
                print("Creating " + self.include_custom_kernel_library_tests_filename)
                self.include_custom_kernel_library_tests_code = CodeGenerate(self.include_custom_kernel_library_tests_filename)
                self.include_custom_kernel_library_tests_code.close()

        if self.env_var == "CUSTOM_APPLICATION_PATH" :
            self.tools_path_filename = self.workarea + "/custom_tools_path.mak"
            if not os.path.exists(self.tools_path_filename):
                print("Creating " + self.tools_path_filename)
                self.tools_path_code = CodeGenerate(self.tools_path_filename, header=False)
                self.tools_path_code.write_line("# This file can optionally be used to define environment variables which")
                self.tools_path_code.write_line("# are needed by the kernel libraries defined in this folder, or can be")
                self.tools_path_code.write_line("# used to overwrite environment variables from the psdk_tools_path.mak")
                self.tools_path_code.write_line("# and vsdk_tools_path.mak files from the tiovx directory.")
                self.tools_path_code.write_newline()
                self.tools_path_code.write_line("# < DEVELOPER_TODO: Add any custom PATH environment variables >")
                self.tools_path_code.close()

        if self.env_var == "CUSTOM_APPLICATION_PATH" :
            self.concerto_inc_filename = self.workarea + "/concerto_inc.mak"
        else :
            self.concerto_inc_filename_folder = self.workarea + "/apps/" + context.name
            self.concerto_inc_filename = self.concerto_inc_filename_folder + "/concerto_inc.mak"
        if not os.path.exists(self.concerto_inc_filename):
            print("Creating " + self.concerto_inc_filename)
            self.concerto_inc_code = CodeGenerate(self.concerto_inc_filename, header=False)
            self.concerto_inc_code.write_line("# This file contains a list of extension kernel specific static libraries")
            self.concerto_inc_code.write_line("# to be included in the PC executables.  It is put in this separate file")
            self.concerto_inc_code.write_line("# to make it easier to add/extend kernels without needing to modify")
            self.concerto_inc_code.write_line("# several concerto.mak files which depend on kernel libraries.")
            self.concerto_inc_code.write_newline()
            self.concerto_inc_code.write_line("STATIC_LIBS += vx_conformance_engine")
            self.concerto_inc_code.write_line("# < DEVELOPER_TODO: Add any additional dependent libraries >")
            self.concerto_inc_code.close()

        self.context = context
        self.context_code = ContextCode(context)

    def create_directory(self, directory):
        self.directory = directory
        if not os.path.exists(self.directory):
            print("Creating " + self.directory)
            os.makedirs(self.directory)

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

    def generate_main_code(self) :
        self.c_file.write_line("/**")
        self.c_file.write_line(" * Main function")
        self.c_file.write_line(" *")
        self.c_file.write_line(" */")
        self.c_file.write_line("int main(int argc, char* argv[])")
        self.c_file.write_open_brace()
        self.c_file.write_define_status()
        self.c_file.write_newline()
        self.c_file.write_line("%s_t uc;" % self.context.name)
        self.c_file.write_newline()
        self.c_file.write_line("tivxInit();")
        self.c_file.write_newline()
        self.c_file.write_line("status = %s_create(&uc);" % self.context.name)
        self.c_file.write_newline()
        self.c_file.write_if_status()
        self.c_file.write_open_brace()
        self.c_file.write_line("status = %s_verify(&uc);" % self.context.name)
        self.c_file.write_close_brace()
        self.c_file.write_newline()
        self.c_file.write_if_status()
        self.c_file.write_open_brace()
        self.c_file.write_line("status = %s_run(&uc);" % self.context.name)
        self.c_file.write_close_brace()
        self.c_file.write_newline()
        self.c_file.write_if_status()
        self.c_file.write_open_brace()
        self.c_file.write_line("status = %s_delete(&uc);" % self.context.name)
        self.c_file.write_close_brace()
        self.c_file.write_newline()
        self.c_file.write_line("tivxDeInit();")
        self.c_file.write_newline()
        self.c_file.write_line("return 0;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

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
        self.c_file.write_newline();
        self.c_file.write_line("/* < DEVELOPER_TODO: (Optional) Load any custom kernel modules needed for this use case > */")
        self.c_file.write_newline();
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

        self.c_file.write_newline();
        self.c_file.write_line("/* < DEVELOPER_TODO: (Optional) Unload any custom kernel modules needed for this use case >*/")
        self.c_file.write_newline();

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
            if data.type != Type.NULL :
                self.c_file.write_line("status = vxReleaseReference((vx_reference*)&usecase->%s);" % (data.name) )
            self.c_file.write_close_brace()
        self.c_file.write_newline()
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_c_code(self) :
        self.c_file.write_include(self.context.name + '.h')
        self.c_file.write_line("/* < DEVELOPER_TODO: (Optional) Include any custom kernel module header files needed for this use case >*/")
        self.c_file.write_newline();
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
        self.generate_main_code()
        self.c_file.close()

    def generate_concerto(self) :
        if self.env_var == "VISION_APPS_PATH" :
            self.concerto_file.write_line("ifeq ($(TARGET_PLATFORM),PC)")
            self.concerto_file.write_newline()
            self.concerto_file.write_line("include $(PRELUDE)")
            self.concerto_file.write_line("TARGET      := vx_app_%s" % self.context.name)
            self.concerto_file.write_line("TARGETTYPE  := exe")
            self.concerto_file.write_line("CSOURCES    := $(call all-c-files)")
            self.concerto_file.write_newline()
            self.concerto_file.write_line("include $(VISION_APPS_PATH)/apps/concerto_inc.mak")
            self.concerto_file.write_line("include $(VISION_APPS_PATH)/apps/%s/concerto_inc.mak" % self.context.name)
            self.concerto_file.write_newline()
            self.concerto_file.write_line("include $(FINALE)")
            self.concerto_file.write_newline()
            self.concerto_file.write_line("endif")
            self.concerto_file.close()
        else :
            self.concerto_file.write_line("include $(PRELUDE)")
            self.concerto_file.write_line("TARGET      := vx_app_%s" % self.context.name)
            self.concerto_file.write_line("TARGETTYPE  := exe")
            self.concerto_file.write_line("CSOURCES    := $(call all-c-files)")
            self.concerto_file.write_newline()
            self.concerto_file.write_line("IDIRS       += $(TIOVX_PATH)/utils/include")
            self.concerto_file.write_line("# < DEVELOPER_TODO: Add any custom IDIRS paths, similar to below: >")
            self.concerto_file.write_line("# IDIRS       += $(CUSTOM_APPLICATION_PATH)/kernels/<module>/include")
            self.concerto_file.write_newline()
            self.concerto_file.write_newline()
            self.concerto_file.write_line("include $(HOST_ROOT)/kernels/concerto_inc.mak")
            self.concerto_file.write_newline()
            self.concerto_file.write_line("STATIC_LIBS += vx_vxu vx_framework")
            self.concerto_file.write_line("STATIC_LIBS += vx_platform_pc vx_framework")
            self.concerto_file.write_line("STATIC_LIBS += vx_kernels_openvx_core vx_target_kernels_openvx_core")
            self.concerto_file.write_newline()
            self.concerto_file.write_line("ifeq ($(BUILD_TUTORIAL),yes)")
            self.concerto_file.write_line("STATIC_LIBS += vx_target_kernels_tutorial")
            self.concerto_file.write_line("endif")
            self.concerto_file.write_newline()
            self.concerto_file.write_line("ifeq ($(BUILD_BAM),yes)")
            self.concerto_file.write_line("STATIC_LIBS += vx_target_kernels_openvx_core_bam vx_target_kernels_openvx_core")
            self.concerto_file.write_line("endif")
            self.concerto_file.write_newline()
            self.concerto_file.write_line("STATIC_LIBS += vx_kernels_host_utils")
            self.concerto_file.write_line("STATIC_LIBS += vx_kernels_target_utils")
            self.concerto_file.write_line("STATIC_LIBS += vx_framework")
            self.concerto_file.write_line("ifeq ($(BUILD_BAM),yes)")
            self.concerto_file.write_line("STATIC_LIBS += vxlib_bamplugin_$(TARGET_CPU)")
            self.concerto_file.write_line("endif")
            self.concerto_file.write_line("STATIC_LIBS += vxlib_$(TARGET_CPU) c6xsim_$(TARGET_CPU)_C66")
            self.concerto_file.write_line("SYS_SHARED_LIBS += rt")
            self.concerto_file.write_line("ifeq ($(BUILD_BAM),yes)")
            self.concerto_file.write_line("STATIC_LIBS += algframework_$(TARGET_CPU) dmautils_$(TARGET_CPU)")
            self.concerto_file.write_line("endif")
            self.concerto_file.write_newline()
            self.concerto_file.write_line("include $(FINALE)")
            self.concerto_file.close()

    def todo(self) :
        if self.env_var == "CUSTOM_APPLICATION_PATH" :
            self.todo_filename = self.workarea + "/DEVELOPER_TODO.txt"
        else :
            self.todo_filename = self.concerto_inc_filename_folder + "/DEVELOPER_TODO.txt"
        print("Creating " + self.todo_filename)
        self.todo_code = CodeGenerate(self.todo_filename, header=False)

        self.lineNum = -1
        self.fileName = None
        self.state = False

        self.todo_code.write_line("# This file lists the places in the generated code where the developer is expected")
        self.todo_code.write_line("# to add custom code beyond what the script can generate.  This is generated as ")
        self.todo_code.write_line("# part of the KernelExportCode.export() function, but may also be called independently ")
        self.todo_code.write_line("# by calling the KernelExportCode.todo() function with the requirement that the ")
        self.todo_code.write_line("# "+self.env_var+" environment variable is defined. This function simply searches")
        self.todo_code.write_line("# for the \"< DEVELOPER_TODO ...>\" string in all the files from this path, and lists them.")
        self.todo_code.write_line("# Removing the \"< DEVELOPER_TODO ...>\" comment block from the files will effectively remove those")
        self.todo_code.write_line("# lines from showing up in this file the next time KernelExportCode.todo() is run.")
        if self.env_var == "CUSTOM_APPLICATION_PATH" :
            self.all_files = [y for x in os.walk(self.workarea) for y in glob(os.path.join(x[0], '*.*'))]
        else :
            self.all_files = [y for x in os.walk(self.concerto_inc_filename_folder) for y in glob(os.path.join(x[0], '*.*'))]
        for file in self.all_files :
            with open(file, 'rb') as f:
                for num, line in enumerate(f, 1):
                    if 'DEVELOPER_TODO'.encode() in line:
                        if '>'.encode() in line:
                            self.state = False
                        else:
                            self.state = True
                        self.modLine = re.sub("^.*?DEVELOPER_TODO:".encode(),"".encode(), line)
                        self.modLine = re.sub("^\s+".encode(),"".encode(), self.modLine)
                        self.modLine = re.sub("\*\/".encode(),"".encode(), self.modLine)
                        self.modLine = re.sub("\>".encode(),"".encode(), self.modLine)
                        if self.fileName != file :
                            self.todo_code.write_line("\n" + file, new_line=False)
                        self.todo_code.write_line("\n    " + str(num) + ": " + self.modLine.decode('utf-8'), new_line=False)
                        self.lineNum = num
                        self.fileName = file
                    elif self.state :
                        if '>'.encode() in line :
                            self.state = False
                        self.modLine = re.sub("^.*?DEVELOPER_TODO:".encode(),"".encode(), line)
                        self.modLine = re.sub("#".encode(),"".encode(), self.modLine)
                        self.modLine = re.sub("\/\/".encode(),"".encode(), self.modLine)
                        self.modLine = re.sub("\/\*".encode(),"".encode(), self.modLine)
                        self.modLine = re.sub("\*\/".encode(),"".encode(), self.modLine)
                        self.modLine = re.sub("^\s+".encode(),"".encode(), self.modLine)
                        self.modLine = re.sub("\>".encode(),"".encode(), self.modLine)
                        self.todo_code.write_line("    " + str(num) + ": " + self.modLine.decode('utf-8'), new_line=False)
                        self.lineNum = num
        self.todo_code.close()

    def generate_code(self) :
        self.generate_h_code()
        self.generate_c_code()
        self.generate_concerto()
        self.todo()


