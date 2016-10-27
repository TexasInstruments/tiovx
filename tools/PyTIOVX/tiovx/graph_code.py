'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class GraphCode (ReferenceCode) :
    def __init__(self, ref) :
        ReferenceCode.__init__(self, ref)

    def declare_var(self, code_gen) :
        code_gen.write_line('vx_graph %s;' % self.ref.name)

    def call_create(self, code_gen) :
        code_gen.write_if_status();
        code_gen.write_open_brace()
        code_gen.write_line("graph = vxCreateGraph(context);");
        code_gen.write_line("if (graph == NULL)");
        code_gen.write_open_brace()
        code_gen.write_line("status = VX_ERROR_NO_RESOURCES;");
        code_gen.write_close_brace()
        code_gen.write_close_brace()
        for node in self.ref.ref :
            NodeCode(node).call_create(code_gen)
        code_gen.write_newline()

    def call_function(self, code_gen, function_name) :
        code_gen.write_if_status();
        code_gen.write_open_brace()
        code_gen.write_line("status = vx%sGraph(graph);" % (function_name));
        code_gen.write_close_brace()
        code_gen.write_newline()

    def call_delete(self, code_gen) :
        code_gen.write_if_status();
        code_gen.write_open_brace()
        code_gen.write_line("status = vxReleaseGraph(&graph);");
        code_gen.write_close_brace()
        for node in self.ref.ref :
            NodeCode(node).call_delete(code_gen)
        code_gen.write_newline()


    def call_verify(self, code_gen) :
        self.call_function(code_gen, "Verify")

    def call_run(self, code_gen) :
        self.call_function(code_gen, "Process")
