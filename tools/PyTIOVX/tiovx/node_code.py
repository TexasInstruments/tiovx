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
        code_gen.write_line("vx_node node = NULL;")
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

        if self.ref.get_vx_kernel_enum() == "VX_USER_KERNEL" :
            code_gen.write_open_brace()
            code_gen.write_line("vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), \"%s\");" % self.ref.kernel)
            code_gen.write_newline()
            code_gen.write_line("if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)");
            code_gen.write_open_brace();
            code_gen.write_line("node = tivxCreateNodeByKernel(graph, kernel, params, %d);" % (num_params))
            code_gen.write_close_brace()
            code_gen.write_close_brace()
        else :
            code_gen.write_line("node = tivxCreateNodeByStructure(graph, %s, params, %d);" % (self.ref.get_vx_kernel_enum(), num_params))
        code_gen.write_newline()
        code_gen.write_line("return node;")
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
        self.set_ref_name(code_gen)
        code_gen.write_line("vxSetNodeTarget(usecase->%s, VX_TARGET_STRING, %s);" % (self.ref.name, Target.get_vx_enum_name(self.ref.target)))
        code_gen.write_close_brace();

    def call_delete(self, code_gen) :
        num_params = len(self.ref.ref)

        code_gen.write_if_status();
        code_gen.write_open_brace();
        code_gen.write_line("status = vxReleaseNode( &usecase->%s );" % (self.ref.name))
        code_gen.write_close_brace();
