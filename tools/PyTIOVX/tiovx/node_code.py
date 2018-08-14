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
            if ref.type != Type.NULL :
                if ((i == (num_params-2)) and (self.ref.ref[num_params-1].type == Type.NULL)) or \
                   ((i == (num_params-3)) and (self.ref.ref[num_params-2].type == Type.NULL) and \
                    (self.ref.ref[num_params-1].type == Type.NULL)):
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
            if ref.type != Type.NULL :
                code_gen.write_line("  (vx_reference)%s_%d %s" % (ref.type.name.lower(), i, end_char))
            else :
                code_gen.write_line("  NULL %s" % (end_char))
            i = i+1
        code_gen.write_close_brace(";")

        code_gen.write_open_brace()
        code_gen.write_line("vx_kernel kernel = vxGetKernelByName(vxGetContext((vx_reference)graph), \"%s\");" % self.ref.kernel)
        code_gen.write_newline()
        code_gen.write_line("if (vxGetStatus((vx_reference)kernel)==VX_SUCCESS)");
        code_gen.write_open_brace();
        code_gen.write_line("node = tivxCreateNodeByKernelRef(graph, kernel, params, %d);" % (num_params))
        code_gen.write_close_brace()
        code_gen.write_line("vxReleaseKernel(&kernel);")
        code_gen.write_close_brace()
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
            if ((i == (num_params-2)) and (self.ref.ref[num_params-1].type == Type.NULL)) or \
               ((i == (num_params-3)) and (self.ref.ref[num_params-2].type == Type.NULL) and \
                (self.ref.ref[num_params-1].type == Type.NULL)):
                end_char = ""
            if ref.type != Type.NULL :
                code_gen.write_line("    usecase->%s %s" % (ref.name, end_char))
            i = i+1
        code_gen.write_line("  );")
        self.set_ref_name(code_gen)
        code_gen.write_if_status();
        code_gen.write_open_brace();
        code_gen.write_line("status = vxSetNodeTarget(usecase->%s, VX_TARGET_STRING, %s);" % (self.ref.name, Target.get_vx_enum_name(self.ref.target)))
        code_gen.write_close_brace();
        code_gen.write_close_brace();

    def call_delete(self, code_gen) :
        num_params = len(self.ref.ref)

        code_gen.write_if_status();
        code_gen.write_open_brace();
        code_gen.write_line("status = vxReleaseNode( &usecase->%s );" % (self.ref.name))
        code_gen.write_close_brace();
