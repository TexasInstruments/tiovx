'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class NodeCode (ReferenceCode) :
    def __init__(self, ref) :
        ReferenceCode.__init__(self, ref)

    def declare_var(self, code_gen) :
        code_gen.write_line('vx_node %s;' % self.ref.name)

    def define_create(self, code_gen) :
        code_gen.write_line("static vx_node usecase_node_create_%s (" % self.ref.name)
        num_params = len(self.ref.ref)

        end_char = ","
        if num_params == 0 :
            end_char = ""
        code_gen.write_line("  vx_graph graph %s" % (end_char))

        i=0
        for ref in self.ref.ref :
            end_char = ","
            if i == (num_params-1) :
                end_char = ""
            code_gen.write_line("  %s %s_%d %s" % (Type.get_vx_name(ref.type), ref.type.name.lower(), i, end_char))
            i = i+1
        code_gen.write_line("  )")

        code_gen.write_open_brace()

        code_gen.write_line("vx_reference params[] =")
        code_gen.write_open_brace()
        i=0
        for ref in self.ref.ref :
            end_char = ","
            if i == (num_params-1) :
                end_char = ""
            code_gen.write_line("  (vx_reference)%s_%d %s" % (ref.type.name.lower(), i, end_char))
            i = i+1
        code_gen.write_close_brace(";")

        code_gen.write_line("return tivxCreateNodeByStructure(graph, %s, params, %d);" % (self.ref.get_vx_kernel_enum(), num_params))

        code_gen.write_close_brace()
        code_gen.write_newline()

    def call_create(self, code_gen) :
        num_params = len(self.ref.ref)

        code_gen.write_if_status();
        code_gen.write_open_brace();
        code_gen.write_line("usecase->%s = usecase_node_create_%s (" % (self.ref.name, self.ref.name))
        end_char = ","
        if num_params == 0 :
            end_char = ""
        code_gen.write_line("    graph %s" % (end_char))
        i=0
        for ref in self.ref.ref :
            end_char = ","
            if i == (num_params-1) :
                end_char = ""
            code_gen.write_line("    usecase->%s %s" % (ref.name, end_char))
            i = i+1
        code_gen.write_line("  );")
        code_gen.write_close_brace();

    def call_delete(self, code_gen) :
        num_params = len(self.ref.ref)

        code_gen.write_if_status();
        code_gen.write_open_brace();
        code_gen.write_line("status = vxReleaseNode( &usecase->%s );" % (self.ref.name))
        code_gen.write_close_brace();
