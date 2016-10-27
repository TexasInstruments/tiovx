'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

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
        self.c_file.write_line("vx_context context = usecase->context;")
        self.c_file.write_line("vx_graph graph = NULL;")
        self.c_file.write_newline()
        GraphCode(graph).call_create(self.c_file)
        self.c_file.write_line("usecase->%s = graph;" % (graph.name))
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
        self.c_file.write_newline()
        self.c_file.write_line("return status;")
        self.c_file.write_close_brace()
        self.c_file.write_newline()

    def generate_c_code(self) :
        self.c_file.write_include(self.context.name + '.h')
        self.c_file.write_newline()
        self.generate_usecase_function_code("create")
        self.generate_usecase_function_code("verify")
        self.generate_usecase_function_code("run")
        self.generate_usecase_function_code("delete")
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


