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

import os, sys, re

from . import *
from glob import glob

## Code object used to generate custom kernel
#
# \par Example Usage: Setting up file paths for custom kernel
# Note: if leaving the optional field for env_var blank, you must set CUSTOM_KERNEL_PATH
#
# \code
# from tiovx import *
#
# code = KernelExportCode(Module.IMAGING, Core.C66, "CUSTOM_KERNEL_PATH")
# <set up kernel parameters>
# code.export(kernel)
# \endcode
#
# Output files from the above parameters:
# \code
# <CUSTOM_KERNEL_PATH>/DEVELOPER_TODO.txt (generated first time only for given parameters)
# <CUSTOM_KERNEL_PATH>/include/TI/tivx_imaging.h (generated first time only for given parameters)
# <CUSTOM_KERNEL_PATH>/include/TI/tivx_imaging_kernels.h (generated first time only for given parameters)
# <CUSTOM_KERNEL_PATH>/include/TI/tivx_imaging_nodes.h (generated first time only for given parameters)
# \endcode
#
# Output folder from the above parameters:
# \code
# <CUSTOM_KERNEL_PATH>/imaging/
# \endcode
#
# This folder contains the following:
# \code
# c66/concerto.mak (generated first time only for given parameters)
# c66/vx_<kernel_name>_target.c
# c66/vx_kernels_imaging_target.c (generated first time only for given parameters)
# c66/bam/vx_bam_<kernel_name>_target.c (if using the C66 DSP)
# c66/bam/concerto.mak (generated first time only for given parameters, if using the C66 DSP)
# host/concerto.mak (generated first time only for given parameters)
# tivx_imaging_node_api.c (generated first time only for given parameters)
# host/vx_<kernel_name>_host.c
# host/vx_kernels_imaging_host.c (generated first time only for given parameters)
# include/tivx_imaging_kernels.h (generated first time only for given parameters)
# include/tivx_kernel_<kernel_name>.h
# test/concerto.mak (generated first time only for given parameters)
# test/test_main.h (generated first time only for given parameters)
# \endcode
#
# \par Below is an example using the CUSTOM_APPLICATION_PATH
#
# \code
# from tiovx import *
#
# code = KernelExportCode(Module.IMAGING, Core.C66, "CUSTOM_APPLICATION_PATH")
# <set up kernel parameters>
# code.export(kernel)
# \endcode
#
# Output files from the above parameters:
# \code
# <CUSTOM_APPLICATION_PATH>/DEVELOPER_TODO.txt
# <CUSTOM_APPLICATION_PATH>/concerto_inc.mak
# <CUSTOM_APPLICATION_PATH>/custom_tools_path.mak
# <CUSTOM_APPLICATION_PATH>/kernels/imaging/include/TI/tivx_imaging.h
# <CUSTOM_APPLICATION_PATH>/kernels/imaging/include/TI/tivx_imaging_kernels.h
# <CUSTOM_APPLICATION_PATH>/kernels/imaging/include/TI/tivx_imaging_nodes.h
# \endcode
#
# Output folder from the above parameters:
# \code
# <CUSTOM_APPLICATION_PATH>/kernels/imaging/
# \endcode
#
# This folder contains the following:
# \code
# c66/concerto.mak (generated first time only for given parameters)
# c66/vx_<kernel_name>_target.c
# c66/vx_kernels_imaging_target.c (generated first time only for given parameters)
# c66/bam/vx_bam_<kernel_name>_target.c (if using the C66 DSP)
# c66/bam/concerto.mak (generated first time only for given parameters, if using the C66 DSP)
# host/concerto.mak (generated first time only for given parameters)
# tivx_imaging_node_api.c (generated first time only for given parameters)
# host/vx_<kernel_name>_host.c
# host/vx_kernels_imaging_host.c (generated first time only for given parameters)
# include/tivx_imaging_kernels.h (generated first time only for given parameters)
# include/tivx_kernel_<kernel_name>.h
# test/concerto.mak (generated first time only for given parameters)
# test/test_main.h (generated first time only for given parameters)
# \endcode
#
# \par Below is an example using the VISION_APPS_PATH
#
# \code
# from tiovx import *
#
# code = KernelExportCode(Module.IMAGING, Core.C66, "VISION_APPS_PATH")
# <set up kernel parameters>
# code.export(kernel)
# \endcode
#
# Output files from the above parameters:
# \code
# <VISION_APPS_PATH>/kernels/DEVELOPER_TODO.txt
# <VISION_APPS_PATH>/kernels/imaging/include/TI/tivx_imaging.h
# <VISION_APPS_PATH>/kernels/imaging/include/TI/tivx_imaging_kernels.h
# <VISION_APPS_PATH>/kernels/imaging/include/TI/tivx_imaging_nodes.h
# \endcode
#
# Output folder from the above parameters:
# \code
# <VISION_APPS_PATH>/kernels/imaging/
# \endcode
#
# This folder contains the following:
# \code
# c66/concerto.mak (generated first time only for given parameters)
# c66/vx_<kernel_name>_target.c
# c66/vx_kernels_imaging_target.c (generated first time only for given parameters)
# c66/bam/vx_bam_<kernel_name>_target.c (if using the C66 DSP)
# c66/bam/concerto.mak (generated first time only for given parameters, if using the C66 DSP)
# host/concerto.mak (generated first time only for given parameters)
# tivx_imaging_node_api.c (generated first time only for given parameters)
# host/vx_<kernel_name>_host.c
# host/vx_kernels_imaging_host.c (generated first time only for given parameters)
# include/tivx_imaging_kernels.h (generated first time only for given parameters)
# include/tivx_kernel_<kernel_name>.h
# test/concerto.mak (generated first time only for given parameters)
# test/test_main.h (generated first time only for given parameters)
# \endcode
#
# \ingroup KERNEL_CODE
#
class KernelExportCode :
    ## Constructor used to create this object
    #
    # \param module           [in] [optional] Module name for the kernel; Default="ext1"
    # \param core             [in] [optional] Name of the core for the kernel to run on; Default="c66"
    # \param env_var          [in] [optional] Path to the directory where these should be outputted; Default="VISION_APPS_PATH"
    # \param include_subpath  [in] [optional] Company name which serves as a subpath for an include directory; Default="TI"
    # \param include_filename [in] [optional] Variable to overwrite the include filename, otherwise include filename set to <lowercase(include_subpath)>vx_<module>; Default=""
    def __init__(self, module="ext1", core="c66", env_var='VISION_APPS_PATH', include_subpath="TI", include_filename="") :
        self.company = include_subpath
        self.module = ""
        if type(module) is Module :
            self.module = module.value
        else :
            print("WARNING: module argument should use the Module class to avoid potential name clashes")
            self.module = module.lower()
        if include_filename :
            self.top_header_name = include_filename
        else :
            self.top_header_name = include_subpath.lower()+"vx_"+self.module.lower()
        if type(core) is Core :
            self.core = core.value
        elif type(core) is str :
            print("WARNING: core argument should use the Core class to avoid potential name clashes")
            self.core = core
        else :
            sys.exit("core argument has invalid type.")
        self.env_var = env_var

        self.workarea = os.environ.get(self.env_var)

        if module == Module.TEST_KERNELS:
            self.idirs_path = "$(HOST_ROOT)/conformance_tests/kernels"
        else:
            self.idirs_path = "$("+self.env_var+")"

        if self.workarea == None or self.workarea == "":
            print("ERROR: You must define %s environment variable as the root of the kernel workarea." % self.env_var);
            sys.exit("Try typing “export CUSTOM_APPLICATION_PATH=<path to where you want the output kernels generated>” in your terminal window and try again.");

        if self.env_var == 'CUSTOM_KERNEL_PATH':
            self.kernels_header_extension = "";
        else:
            self.kernels_header_extension = "_priv";

    def getDataColor(self, ref) :
        return "GhostWhite"

    def outputData(self, data) :
        self.file.write('  %s [color=%s, style=filled]\n' % (data.name_lower, self.getDataColor(data)))

    def outputDataList(self, kernel) :
        self.file.write('\n')
        self.file.write('  /* DATA OBJECTS */\n')
        for ref in kernel.params :
            self.outputData(ref)
        self.file.write('\n')

    def getTargetColor(self, target) :
        if target == Target.DSP1 :
            return "palegreen"
        if target == Target.DSP2 :
            return "darkturquoise"
        if target == Target.EVE1 :
            return "yellow"
        if target == Target.EVE2 :
            return "gold"
        if target == Target.EVE3 :
            return "orange"
        if target == Target.EVE4 :
            return "goldenrod4"
        if target == Target.A15_0 :
            return "lightblue"
        if target == Target.MCU2_0 :
            return "grey"
        if target == Target.MCU2_1 :
            return "LightSalmon"
        if target == Target.IPU2 :
            return "MediumOrchid"
        return "white"

    def outputNode(self, kernel) :
        if kernel.targets :
            self.file.write('  %s [label=\"%s\", color=%s, style=filled]\n' % (kernel.name_lower, kernel.name_lower, self.getTargetColor(kernel.targets[0])) )
        else :
            self.file.write('  %s [label=\"%s\", color=%s, style=filled]\n' % (kernel.name_lower, kernel.name_lower, self.getTargetColor("white")) )

    def outputNodeList(self, kernel) :
        self.file.write('\n')
        self.file.write('  /* NODE OBJECTS */\n')
        self.outputNode(kernel)
        self.file.write('\n')

    def outputNodeConnection(self, kernel) :
        idx = 0
        for prm in kernel.params :
            if prm.direction == Direction.INPUT :
                self.file.write('  %s -> %s [taillabel=%d, labeldistance=3]\n' % (prm.name_lower, kernel.name_lower, idx))
            else :
                self.file.write('  %s -> %s [headlabel=%d, labeldistance=3]\n' % (kernel.name_lower, prm.name_lower, idx))
            idx = idx + 1

    def outputNodeConnectionList(self, kernel) :
        self.file.write('\n')
        self.file.write('  /* NODE CONNECTIONS */\n')
        self.outputNodeConnection(kernel)
        self.file.write('\n')

    ## Export object as C source code
    #
    def exportDiagram(self, kernel) :
        print ('Generating image from OpenVX kernel ...')
        self.filename_prefix = kernel.name_lower
        self.filename = kernel.name_lower + "_img.txt"
        self.filenameJpg = kernel.name_lower + ".jpg"
        self.file = None

        self.file = open(self.filename, 'w')
        self.file.write('digraph %s {\n' % kernel.name_lower)
        self.file.write('\n')
        self.file.write('  label = \"%s\"\n' % kernel.name_lower)
        self.outputDataList(kernel)
        self.outputNodeList(kernel)
        self.outputNodeConnectionList(kernel)
        self.file.write('\n')
        self.file.write('}\n')
        self.file.close()

        try :
            command_str = 'dot %s -Tjpg -o%s' % (self.filename, self.filenameJpg)
            command_args = ['dot', self.filename, '-Tjpg','-o%s' % self.filenameJpg]
            print('Executing dot tool command ... [' + command_str + ']')
            subprocess.call(command_args)
            print ('Generating image from OpenVX context ... DONE !!!')
        except FileNotFoundError:
            print('ERROR: \'dot\' tool not found. Make sure \'graphviz\' is installed and \'dot\' command is added to system PATH !!!')
            print('ERROR: Cannot generate .jpg file !!!')


    def setCompanyDirectory(self, company) :
        self.company = company

    def setTopHeaderName(self, header) :
        self.top_header_name = header

    def setModuleDirectory(self, module) :
        if type(module) is Module :
            self.module = module.value
        elif type(core) is str :
            self.module is module
        self.top_header_name = include_subpath.lower()+"vx_"+self.module.lower()

    def setCoreDirectory(self, core) :
        if type(core) is Core :
            self.core = core.value
        elif type(core) is str :
            self.core = core
        else :
            sys.exit("core argument has invalid type.")

    def create_all_directories(self):
        self.create_directory(self.workarea)

        if self.env_var == 'CUSTOM_KERNEL_PATH' :
            self.workarea_include = self.workarea + "/include"
        else :
            self.workarea_include = self.workarea + "/kernels/" + self.module + "/include"

        self.create_directory(self.workarea_include)

        self.workarea_include_company = self.workarea_include + "/" + self.company
        self.create_directory(self.workarea_include_company)

        if self.env_var == 'CUSTOM_KERNEL_PATH' :
            self.workarea_module = self.workarea + "/" + self.module
        else :
            self.workarea_module = self.workarea + "/kernels/" + self.module

        self.create_directory(self.workarea_module)

        if self.env_var == 'CUSTOM_KERNEL_PATH' :
            self.workarea_module_include = self.workarea_module + "/include"
        else :
            self.workarea_module_include = self.workarea_module + "/host"

        self.create_directory(self.workarea_module_include)

        self.workarea_module_host = self.workarea_module + "/host"
        self.create_directory(self.workarea_module_host)

        self.workarea_module_core = self.workarea_module + "/" + self.core
        self.create_directory(self.workarea_module_core)

        if self.target_uses_dsp :
            self.workarea_module_core_bam = self.workarea_module + "/" + self.core + "/bam"
            self.create_directory(self.workarea_module_core_bam)

        self.workarea_module_test = self.workarea_module + "/test"
        self.create_directory(self.workarea_module_test)

    def create_directory(self, directory):
        self.directory = directory
        if not os.path.exists(self.directory):
            print("Creating " + self.directory)
            os.makedirs(self.directory)

    def generate_h_file_code(self):
        print("Creating " + self.workarea_module_include + "/" + self.h_filename)
        self.h_code = CodeGenerate(self.workarea_module_include + "/" + self.h_filename)
        self.h_code.write_ifndef_define("_" + self.kernel.enum_str_prefix + self.kernel.name_upper + "_")
        self.h_code.write_extern_c_top()
        self.h_code.write_newline();
        for prm in self.kernel.params :
            self.h_code.write_line("#define %s%s_%s_IDX (%dU)" % (self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper, prm.index))
        self.h_code.write_newline();
        self.h_code.write_line("#define %s%s_MAX_PARAMS (%dU)" % (self.kernel.enum_str_prefix, self.kernel.name_upper, len(self.kernel.params)))
        self.h_code.write_newline();
        self.h_code.write_extern_c_bottom()
        self.h_code.write_newline()
        self.h_code.write_endif("_" + self.kernel.enum_str_prefix + self.kernel.name_upper + "_")
        self.h_code.close()

    def generate_host_c_add_func_code(self):
        self.host_c_code.write_line("vx_status tivxAddKernel%s(vx_context context)" % (self.kernel.name_camel))
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("vx_kernel kernel;")
        self.host_c_code.write_line("vx_status status;")
        self.host_c_code.write_line("uint32_t index;")
        self.host_c_code.write_line("vx_enum kernel_id;")
        self.host_c_code.write_newline()
        self.host_c_code.write_line("status = vxAllocateUserKernelId(context, &kernel_id);")
        self.host_c_code.write_line("if(status != (vx_status)VX_SUCCESS)")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"Unable to allocate user kernel ID\\n\");")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()
        self.host_c_code.write_if_status()
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("kernel = vxAddUserKernel(");
        self.host_c_code.write_line("            context,")
        self.host_c_code.write_line("            %s%s_NAME," % (self.kernel.enum_str_prefix, self.kernel.name_upper))
        self.host_c_code.write_line("            kernel_id," )
        self.host_c_code.write_line("            NULL,")
        self.host_c_code.write_line("            %s%s_MAX_PARAMS," % (self.kernel.enum_str_prefix, self.kernel.name_upper) )
        self.host_c_code.write_line("            tivxAddKernel%sValidate," % (self.kernel.name_camel) )
        self.host_c_code.write_line("            tivxAddKernel%sInitialize," % (self.kernel.name_camel) )
        self.host_c_code.write_line("            NULL);")
        self.host_c_code.write_newline()
        self.host_c_code.write_line("status = vxGetStatus((vx_reference)kernel);")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_if_status()
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("index = 0;")
        self.host_c_code.write_newline()
        for prm in self.kernel.params :
            if prm != self.kernel.params[0] :
                self.host_c_code.write_if_status()
            self.host_c_code.write_open_brace()
            self.host_c_code.write_line("status = vxAddParameterToKernel(kernel,")
            self.host_c_code.write_line("            index,")
            self.host_c_code.write_line("            (vx_enum)%s," % (Direction.get_vx_enum_name(prm.direction)) )
            if Type.is_scalar_type(prm.type) :
                self.host_c_code.write_line("            (vx_enum)VX_TYPE_SCALAR,")
            else :
                self.host_c_code.write_line("            (vx_enum)%s," % (Type.get_vx_enum_name(prm.type)) )
            self.host_c_code.write_line("            (vx_enum)%s" % (ParamState.get_vx_enum_name(prm.state)) )
            self.host_c_code.write_line(");")
            self.host_c_code.write_line("index++;")
            self.host_c_code.write_close_brace()

        self.host_c_code.write_if_status()
        self.host_c_code.write_open_brace()
        self.host_c_code.write_comment_line("add supported target's")
        for target in self.kernel.targets :
            self.host_c_code.write_line("tivxAddKernelTarget(kernel, %s);" % (Target.get_vx_enum_name(target)))
        self.host_c_code.write_close_brace()

        self.host_c_code.write_if_status()
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("status = vxFinalizeKernel(kernel);")
        self.host_c_code.write_close_brace()

        self.host_c_code.write_line("if (status != (vx_status)VX_SUCCESS)")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("vxReleaseKernel(&kernel);")
        self.host_c_code.write_line("kernel = NULL;")
        self.host_c_code.write_close_brace()

        self.host_c_code.write_close_brace()
        self.host_c_code.write_line("else")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("kernel = NULL;")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_line("vx_%s_kernel = kernel;" % (self.kernel.name_lower))
        self.host_c_code.write_newline()
        self.host_c_code.write_line("return status;")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

    def generate_host_c_remove_func_code(self):
        self.host_c_code.write_line("vx_status tivxRemoveKernel%s(vx_context context)" % (self.kernel.name_camel))
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("vx_status status;")
        self.host_c_code.write_line("vx_kernel kernel = vx_%s_kernel;" % self.kernel.name_lower)
        self.host_c_code.write_newline()
        self.host_c_code.write_line("status = vxRemoveKernel(kernel);")
        self.host_c_code.write_line("vx_%s_kernel = NULL;" % self.kernel.name_lower)
        self.host_c_code.write_newline()
        self.host_c_code.write_line("return status;")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

    def verify_parameter_relationship_items(self, relationship_list, prm, attribute, name) :
        for rel in relationship_list :
            if prm in rel.prm_list :
                if attribute in rel.attribute_list :
                    return True
                # Sometimes we want to compare attributes from different object types (e.g. image and pyramid width)
                for local_name in rel.attribute_list :
                    if local_name.name == name :
                        return True
        if attribute == ImageAttribute.FORMAT :
            return True
        elif attribute == UserDataObjectAttribute.NAME :
            return True
        elif attribute == UserDataObjectAttribute.SIZE :
            return True
        elif attribute == ArrayAttribute.ITEMSIZE :
            return True
        elif attribute == ArrayAttribute.ITEMTYPE :
            return True
        elif attribute == PyramidAttribute.FORMAT :
            return True
        elif attribute == MatrixAttribute.TYPE :
            return True
        elif attribute == LutAttribute.TYPE :
            return True
        return False

    # performs conversion from string to array type
    def convert_string_to_array_type(self, print_type):
        if print_type.startswith('VX_TYPE_') :
            string_length = len(print_type)
            substring = print_type[8:string_length]
            for t in Type :
                if t.name == substring :
                    return t
        return Type.NULL

    # performs check on array type to see if it is a non-enum type
    def check_array_type(self, print_type):
        array_type = self.convert_string_to_array_type(print_type)
        return Type.is_array_type(array_type)

    def generate_host_c_validate_func_code(self):
        self.host_c_code.write_line("static vx_status VX_CALLBACK tivxAddKernel%sValidate(vx_node node," % self.kernel.name_camel)
        self.host_c_code.write_line("            const vx_reference parameters[ ],")
        self.host_c_code.write_line("            vx_uint32 num,")
        self.host_c_code.write_line("            vx_meta_format metas[])")
        self.host_c_code.write_open_brace()

        # Initial parameters
        self.host_c_code.write_line("vx_status status = (vx_status)VX_SUCCESS;")
        self.num_params = 0
        for prm in self.kernel.params :
            self.host_c_code.write_newline()

            attr = Attribute.from_type(prm.type)
            if Type.is_scalar_type(prm.type) :
                self.host_c_code.write_line("vx_scalar %s = NULL;" % prm.name_lower)
                self.host_c_code.write_line("vx_enum %s_scalar_type;" % (prm.name_lower))
            else :
                self.host_c_code.write_line("%s %s = NULL;" % (Type.get_vx_name(prm.type), prm.name_lower))
                for name, member in attr.__members__.items() :
                    if self.verify_parameter_relationship_items(self.kernel.relationship_list, prm, member, name) :
                        if member == UserDataObjectAttribute.NAME :
                            self.host_c_code.write_line("%s %s_%s[VX_MAX_REFERENCE_NAME];" % (member.value[1], prm.name_lower, member.value[0]))
                        else :
                            self.host_c_code.write_line("%s %s_%s;" % (member.value[1], prm.name_lower, member.value[0]))
            self.num_params += 1

        self.host_c_code.write_newline()
        self.host_c_code.write_line("if ( (num != %s%s_MAX_PARAMS)" % (self.kernel.enum_str_prefix, self.kernel.name_upper) )
        for prm in self.kernel.params :
            if prm.state is ParamState.REQUIRED :
                self.host_c_code.write_line("    || (NULL == parameters[%s%s_%s_IDX])" % (self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper))
        self.host_c_code.write_line(")")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("status = (vx_status)VX_ERROR_INVALID_PARAMETERS;")
        self.host_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"One or more REQUIRED parameters are set to NULL\\n\");")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

        # Query all types here
        self.host_c_code.write_line("if ((vx_status)VX_SUCCESS == status)")
        self.host_c_code.write_open_brace()
        # find code from target for here
        # assigned descriptors to local variables
        for prm in self.kernel.params :
            if Type.is_scalar_type(prm.type) :
                self.host_c_code.write_line("%s = (%s)parameters[%s%s_%s_IDX];" %
                    (prm.name_lower, Type.get_vx_name(Type.SCALAR), self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
            else :
                self.host_c_code.write_line("%s = (%s)parameters[%s%s_%s_IDX];" %
                    (prm.name_lower, Type.get_vx_name(prm.type), self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

        self.host_c_code.write_newline()
        self.host_c_code.write_line("/* PARAMETER ATTRIBUTE FETCH */")
        self.host_c_code.write_newline()

        # for loop writing each query here around if statements checking the status
        num_image = 0
        num_nonimage = 0
        num_scalar = 0
        self.host_c_code.write_line("if ((vx_status)VX_SUCCESS == status)")
        self.host_c_code.write_open_brace()
        for prm in self.kernel.params :
            if prm.state is ParamState.OPTIONAL :
                self.host_c_code.write_line("if (NULL != %s)" % prm.name_lower)
                self.host_c_code.write_open_brace()

            attr = Attribute.from_type(prm.type)
            if Type.is_scalar_type(prm.type) :
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryScalar(%s, (vx_enum)VX_SCALAR_TYPE, &%s_scalar_type, sizeof(%s_scalar_type)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            else :
                for name, member in attr.__members__.items() :
                    if self.verify_parameter_relationship_items(self.kernel.relationship_list, prm, member, name) :
                        if prm.type == Type.RAW_IMAGE :
                            self.host_c_code.write_line("tivxCheckStatus(&status, tivxQuery%s(%s, (vx_enum)TIVX_%s_%s, &%s_%s, sizeof(%s_%s)));" % (toCamelCase(prm.type.name), prm.name_lower, prm.type.name, name, prm.name_lower, member.value[0], prm.name_lower, member.value[0]))
                        elif prm.type == Type.LUT:
                            self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryLUT(%s, (vx_enum)VX_%s_%s, &%s_%s, sizeof(%s_%s)));" % (prm.name_lower, prm.type.name, name, prm.name_lower, member.value[0], prm.name_lower, member.value[0]))
                        else :
                            self.host_c_code.write_line("tivxCheckStatus(&status, vxQuery%s(%s, (vx_enum)VX_%s_%s, &%s_%s, sizeof(%s_%s)));" % (toCamelCase(prm.type.name), prm.name_lower, prm.type.name, name, prm.name_lower, member.value[0], prm.name_lower, member.value[0]))

            if prm.state is ParamState.OPTIONAL :
                self.host_c_code.write_close_brace()
            if prm is not self.kernel.params[-1] :
                self.host_c_code.write_newline()
        self.host_c_code.write_close_brace()

        self.host_c_code.write_newline()
        self.host_c_code.write_line("/* PARAMETER CHECKING */")
        self.host_c_code.write_newline()

        # Check for sizeof array, and data type (format) of other objects
        self.host_c_code.write_line("if ((vx_status)VX_SUCCESS == status)")
        self.host_c_code.write_open_brace()
        for prm in self.kernel.params :
            if Type.IMAGE == prm.type or Type.PYRAMID == prm.type or Type.ARRAY == prm.type or Type.MATRIX == prm.type or Type.LUT == prm.type or Type.USER_DATA_OBJECT == prm.type or Type.is_scalar_type(prm.type) is True :
                if prm.state is ParamState.OPTIONAL :
                    self.host_c_code.write_line("if (NULL != %s)" % (prm.name_lower))
                    self.host_c_code.write_open_brace()
                if len(prm.data_types) == 0 :
                    self.host_c_code.write_comment_line("< DEVELOPER_TODO: Replace <Add type here> with correct data type >")
                    self.print_data_type = ['<Add type here>']
                else :
                    self.print_data_type = prm.data_types
                if Type.IMAGE == prm.type :
                    if len(prm.data_types) > 1 :
                        self.host_c_code.write_line("if( ((vx_df_image)%s != %s_fmt) &&" % (self.print_data_type[0], prm.name_lower))
                        for dt in self.print_data_type[1:-1] :
                            self.host_c_code.write_line("    ((vx_df_image)%s != %s_fmt) &&" % (dt, prm.name_lower))
                        self.host_c_code.write_line("    ((vx_df_image)%s != %s_fmt))" % (self.print_data_type[-1], prm.name_lower))
                    else :
                        self.host_c_code.write_line("if ((vx_df_image)%s != %s_fmt)" % (self.print_data_type[0], prm.name_lower))
                elif Type.PYRAMID == prm.type :
                    if len(prm.data_types) > 1 :
                        self.host_c_code.write_line("if( ((vx_df_image)%s != %s_fmt) &&" % (self.print_data_type[0], prm.name_lower))
                        for dt in self.print_data_type[1:-1] :
                            self.host_c_code.write_line("    ((vx_df_image)%s != %s_fmt) &&" % (dt, prm.name_lower))
                        self.host_c_code.write_line("    ((vx_df_image)%s != %s_fmt))" % (self.print_data_type[-1], prm.name_lower))
                    else :
                        self.host_c_code.write_line("if ((vx_df_image)%s != %s_fmt)" % (self.print_data_type[0], prm.name_lower))
                elif Type.ARRAY == prm.type :
                    if len(prm.data_types) > 1 :
                        if self.check_array_type(self.print_data_type[0]) :
                            self.host_c_code.write_line("if( ((vx_enum)%s != %s_item_type) &&" % (self.print_data_type[0], prm.name_lower))
                        else :
                            self.host_c_code.write_line("if( (%s_item_size != sizeof(%s)) &&" % (prm.name_lower, self.print_data_type[0]))
                        for dt in self.print_data_type[1:-1] :
                            if self.check_array_type(dt) :
                                self.host_c_code.write_line("    (%s_item_type != %s) &&" % (prm.name_lower, dt))
                            else :
                                self.host_c_code.write_line("    (%s_item_size != sizeof(%s)) &&" % (prm.name_lower, dt))
                        if self.check_array_type(self.print_data_type[-1]) :
                            self.host_c_code.write_line("    (%s_item_type != %s))" % (prm.name_lower, self.print_data_type[-1]))
                        else :
                            self.host_c_code.write_line("    (%s_item_size != sizeof(%s)))" % (prm.name_lower, self.print_data_type[-1]))
                    else :
                        if self.check_array_type(self.print_data_type[0]) :
                            self.host_c_code.write_line("if (%s != %s_item_type )" % (self.print_data_type[0], prm.name_lower))
                        else :
                            self.host_c_code.write_line("if ( %s_item_size != sizeof(%s))" % (prm.name_lower, self.print_data_type[0]))
                elif Type.MATRIX == prm.type or Type.LUT == prm.type:
                    if len(prm.data_types) > 1 :
                        self.host_c_code.write_line("if( ((vx_enum)%s != %s_type) &&" % (self.print_data_type[0], prm.name_lower))
                        for dt in self.print_data_type[1:-1] :
                            self.host_c_code.write_line("    ((vx_enum)%s != %s_type) &&" % (dt, prm.name_lower))
                        self.host_c_code.write_line("    ((vx_enum)%s != %s_type))" % (self.print_data_type[-1], prm.name_lower))
                    else :
                        self.host_c_code.write_line("if ((vx_enum)%s != %s_type)" % (self.print_data_type[0], prm.name_lower))
                elif Type.is_scalar_type(prm.type) :
                    if len(prm.data_types) > 1 :
                        self.host_c_code.write_line("if( ((vx_enum)%s != %s_scalar_type) &&" % (self.print_data_type[0], prm.name_lower))
                        for dt in self.print_data_type[1:-1] :
                            self.host_c_code.write_line("    ((vx_enum)%s != %s_scalar_type) &&" % (dt, prm.name_lower))
                        self.host_c_code.write_line("    ((vx_enum)%s != %s_scalar_type))" % (self.print_data_type[-1], prm.name_lower))
                    else :
                        self.host_c_code.write_line("if ((vx_enum)%s != %s_scalar_type)" % (self.print_data_type[0], prm.name_lower))
                elif Type.USER_DATA_OBJECT == prm.type :
                    if len(prm.data_types) > 1 :
                        self.host_c_code.write_line("if( ((%s_size != sizeof(%s)) ||" % (prm.name_lower, self.print_data_type[0]))
                        self.host_c_code.write_line("     (strncmp(%s_name, \"%s\", sizeof(%s_name)) != 0)) &&" % (prm.name_lower, self.print_data_type[0], prm.name_lower))
                        for dt in self.print_data_type[1:-1] :
                            self.host_c_code.write_line("    ((%s_size != sizeof(%s)) ||" % (prm.name_lower, dt))
                            self.host_c_code.write_line("     (strncmp(%s_name, \"%s\", sizeof(%s_name)) != 0)) &&" % (prm.name_lower, dt, prm.name_lower))
                        self.host_c_code.write_line("    ((%s_size != sizeof(%s)) ||" % (prm.name_lower, self.print_data_type[-1]))
                        self.host_c_code.write_line("     (strncmp(%s_name, \"%s\", sizeof(%s_name)) != 0)))" % (prm.name_lower, self.print_data_type[-1], prm.name_lower))
                    else :
                        self.host_c_code.write_line("if ((%s_size != sizeof(%s)) ||" % (prm.name_lower, self.print_data_type[0]))
                        self.host_c_code.write_line("    (strncmp(%s_name, \"%s\", sizeof(%s_name)) != 0))" % (prm.name_lower, self.print_data_type[0], prm.name_lower))

                self.host_c_code.write_open_brace()
                self.host_c_code.write_line("status = (vx_status)VX_ERROR_INVALID_PARAMETERS;")
                vowel = ["a","e","i","o"]
                if Type.is_scalar_type(prm.type) :
                    self.host_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"'%s' should be a scalar of type:\\n " % (prm.name_lower), new_line=False)
                else :
                    article = 'a'
                    if prm.type.name[0].lower() in vowel :
                        article = 'an'
                    self.host_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"'%s' should be %s %s of type:\\n " % (prm.name_lower, article, prm.type.name.lower()), new_line=False)
                self.host_c_code.write_line("%s " % (self.print_data_type[0]), new_line=False, indent=False)
                for dt in self.print_data_type[1:] :
                    self.host_c_code.write_line("or %s " % (dt), new_line=False, indent=False)
                self.host_c_code.write_line("\\n\");", indent=False)
                self.host_c_code.write_close_brace()
                if prm.state is ParamState.OPTIONAL :
                    self.host_c_code.write_close_brace()
            if prm is not self.kernel.params[-1] :
                self.host_c_code.write_newline()
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

        if len(self.kernel.relationship_list) :
            self.host_c_code.write_newline()
            self.host_c_code.write_line("/* PARAMETER RELATIONSHIP CHECKING */")
            self.host_c_code.write_newline()
            self.host_c_code.write_line("if ((vx_status)VX_SUCCESS == status)")
            self.host_c_code.write_open_brace()

            for rel in self.kernel.relationship_list :
                if rel.state is ParamState.OPTIONAL :
                    self.host_c_code.write_line("if (NULL != %s)" % (rel.prm_list[0].name_lower))
                    self.host_c_code.write_open_brace()
                for attr in rel.attribute_list :
                    if attr.vx_enum_name() == "VX_MATRIX_ORIGIN":
                        self.host_c_code.write_line("if( (%s_%s.x != %s_%s.x) ||" % (rel.prm_list[0].name_lower, attr.value[0], rel.prm_list[1].name_lower, attr.value[0]))
                        self.host_c_code.write_line("    (%s_%s.y != %s_%s.y) )" % (rel.prm_list[0].name_lower, attr.value[0], rel.prm_list[1].name_lower, attr.value[0]))
                    else :
                        if len(rel.prm_list) > 2 :
                            self.host_c_code.write_line("if( (%s_%s != %s_%s) ||" % (rel.prm_list[0].name_lower, attr.value[0], rel.prm_list[1].name_lower, attr.value[0]))
                            for prm in rel.prm_list[2:-1] :
                                self.host_c_code.write_line("    (%s_%s != %s_%s) ||" % (rel.prm_list[0].name_lower, attr.value[0], prm.name_lower, attr.value[0]))
                            self.host_c_code.write_line("    (%s_%s != %s_%s))" % (rel.prm_list[0].name_lower, attr.value[0], rel.prm_list[-1].name_lower, attr.value[0]))
                        elif len(rel.prm_list) == 2 :
                            self.host_c_code.write_line("if (%s_%s != %s_%s)" % (rel.prm_list[0].name_lower, attr.value[0], rel.prm_list[1].name_lower, attr.value[0]))

                    self.host_c_code.write_open_brace()
                    self.host_c_code.write_line("status = (vx_status)VX_ERROR_INVALID_PARAMETERS;")
                    self.host_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"Parameters '%s' and '%s' " % (rel.prm_list[0].name_lower, rel.prm_list[1].name_lower), new_line=False)
                    for prm in rel.prm_list[2:] :
                        self.host_c_code.write_line("and '%s' " % (prm.name_lower), new_line=False, indent=False)
                    self.host_c_code.write_line("should have the same value for %s\\n\");" % attr.vx_enum_name(), indent=False)
                    self.host_c_code.write_close_brace()
                if rel.state is ParamState.OPTIONAL :
                    self.host_c_code.write_close_brace()
                if rel is not self.kernel.relationship_list[-1] :
                    self.host_c_code.write_newline()
            self.host_c_code.write_close_brace()

        self.host_c_code.write_newline()
        self.host_c_code.write_line("/* CUSTOM PARAMETER CHECKING */")
        self.host_c_code.write_newline()
        self.host_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Add any custom parameter type or range checking not")
        self.host_c_code.write_comment_line("                  covered by the code-generation script.) >")
        self.host_c_code.write_newline()

        self.host_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) If intending to use a virtual data object, set metas using appropriate TI API.")
        self.host_c_code.write_comment_line("                  For a code example, please refer to the validate callback of the follow file:")
        self.host_c_code.write_comment_line("                  tiovx/kernels/openvx-core/host/vx_absdiff_host.c. For further information regarding metas,")
        self.host_c_code.write_comment_line("                  please refer to the OpenVX 1.1 spec p. 260, or search for vx_kernel_validate_f. >")
        self.host_c_code.write_newline()

        self.host_c_code.write_line("return status;")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

    def generate_host_c_initialize_func_code(self):
        self.host_c_code.write_line("static vx_status VX_CALLBACK tivxAddKernel%sInitialize(vx_node node," % self.kernel.name_camel)
        self.host_c_code.write_line("            const vx_reference parameters[ ],")
        self.host_c_code.write_line("            vx_uint32 num_params)")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("vx_status status = (vx_status)VX_SUCCESS;")
        if self.kernel.getNumImages() > 0 :
            self.host_c_code.write_line("tivxKernelValidRectParams prms;")
        self.host_c_code.write_newline()

        # Check if null params
        self.host_c_code.write_line("if ( (num_params != %s%s_MAX_PARAMS)" % (self.kernel.enum_str_prefix, self.kernel.name_upper) )
        for prm in self.kernel.params :
            if prm.state is ParamState.REQUIRED :
                self.host_c_code.write_line("    || (NULL == parameters[%s%s_%s_IDX])" % (self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper))
        self.host_c_code.write_line(")")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("status = (vx_status)VX_ERROR_INVALID_PARAMETERS;")
        self.host_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"One or more REQUIRED parameters are set to NULL\\n\");")
        self.host_c_code.write_close_brace()

        # Set images
        num_input_image = 0
        num_output_image = 0
        self.temp_buffer = ""
        for prm in self.kernel.params :
            if Type.IMAGE == prm.type:
                if Direction.INPUT == prm.direction:
                    self.temp_buffer += ("        prms.in_img[%sU] = (vx_image)parameters[%s%s_%s_IDX];\n" %
                        (num_input_image, self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
                    num_input_image+=1
                if Direction.OUTPUT == prm.direction:
                    self.temp_buffer += ("        prms.out_img[%sU] = (vx_image)parameters[%s%s_%s_IDX];\n" %
                        (num_output_image, self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
                    num_output_image+=1

        # Config valid rectangle
        if num_input_image > 0 or num_output_image > 0 :
            self.host_c_code.write_line("if ((vx_status)VX_SUCCESS == status)")
            self.host_c_code.write_open_brace()
            self.host_c_code.write_line("tivxKernelValidRectParams_init(&prms);")
            self.host_c_code.write_newline()
            self.host_c_code.write_block(self.temp_buffer)
            self.host_c_code.write_line("prms.num_input_images = %s;" % self.kernel.getNumInputImages())
            self.host_c_code.write_line("prms.num_output_images = %s;" % self.kernel.getNumOutputImages())
            self.host_c_code.write_newline()
            self.host_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Set padding values based on valid region if border mode is")
            self.host_c_code.write_comment_line("                   set to VX_BORDER_UNDEFINED and remove the #if 0 and #endif lines.")
            self.host_c_code.write_comment_line("                   Else, remove this entire #if 0 ... #endif block >")
            self.host_c_code.write_line("#if 0")
            self.host_c_code.write_line("prms.top_pad = 0;")
            self.host_c_code.write_line("prms.bot_pad = 0;")
            self.host_c_code.write_line("prms.left_pad = 0;")
            self.host_c_code.write_line("prms.right_pad = 0;")
            self.host_c_code.write_line("prms.border_mode = VX_BORDER_UNDEFINED;")
            self.host_c_code.write_line("#endif")
            self.host_c_code.write_newline()
            self.host_c_code.write_line("status = tivxKernelConfigValidRect(&prms);")
            self.host_c_code.write_close_brace()
            self.host_c_code.write_newline()

        self.host_c_code.write_line("return status;")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

    def generate_host_c_file_code(self):
        print("Creating " + self.workarea_module_host + "/" + self.host_c_filename)
        self.host_c_code = CodeGenerate(self.workarea_module_host + "/" + self.host_c_filename)
        self.host_c_code.write_include("TI/tivx.h")
        self.host_c_code.write_include(self.company + "/" + self.top_header_name + ".h")
        self.host_c_code.write_include("tivx_" + self.module.lower() + "_kernels" + self.kernels_header_extension + ".h")
        self.host_c_code.write_include(self.h_filename)
        self.host_c_code.write_include("TI/tivx_target_kernel.h")
        self.host_c_code.write_newline()
        self.host_c_code.write_line("static vx_kernel vx_%s_kernel = NULL;" % (self.kernel.name_lower))
        self.host_c_code.write_newline()

        self.host_c_code.write_line("static vx_status VX_CALLBACK tivxAddKernel%sValidate(vx_node node," % self.kernel.name_camel)
        self.host_c_code.write_line("            const vx_reference parameters[ ],")
        self.host_c_code.write_line("            vx_uint32 num,")
        self.host_c_code.write_line("            vx_meta_format metas[]);")
        self.host_c_code.write_line("static vx_status VX_CALLBACK tivxAddKernel%sInitialize(vx_node node," % self.kernel.name_camel)
        self.host_c_code.write_line("            const vx_reference parameters[ ],")
        self.host_c_code.write_line("            vx_uint32 num_params);")
        self.host_c_code.write_line("vx_status tivxAddKernel%s(vx_context context);" % (self.kernel.name_camel))
        self.host_c_code.write_line("vx_status tivxRemoveKernel%s(vx_context context);" % (self.kernel.name_camel))

        self.host_c_code.write_newline()
        self.generate_host_c_validate_func_code()
        self.generate_host_c_initialize_func_code()
        self.generate_host_c_add_func_code()
        self.generate_host_c_remove_func_code()
        self.host_c_code.close()

    def generate_target_c_add_func_code(self):
        self.target_c_code.write_line("void tivxAddTargetKernel%s(void)" % self.kernel.name_camel, files=0)
        self.target_c_code.write_line("void tivxAddTargetKernelBam%s(void)" % self.kernel.name_camel, files=1)
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = (vx_status)VX_FAILURE;")
        self.target_c_code.write_line("char target_name[TIVX_TARGET_MAX_NAME];")
        self.target_c_code.write_line("vx_enum self_cpu;")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("self_cpu = tivxGetSelfCpuId();")
        self.target_c_code.write_newline()
        for target in self.kernel.targets :
            if type(target) is Target :
                cpu = Target.get_cpu(target)
                self.target_c_code.write_line("if ( self_cpu == (vx_enum)%s )" % Cpu.get_vx_enum_name(cpu) )
            self.target_c_code.write_open_brace()
            self.target_c_code.write_line("strncpy(target_name, %s, TIVX_TARGET_MAX_NAME);" % Target.get_vx_enum_name(target))
            self.target_c_code.write_line("status = (vx_status)VX_SUCCESS;")
            self.target_c_code.write_close_brace()
            self.target_c_code.write_line("else")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("status = (vx_status)VX_FAILURE;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

        self.target_c_code.write_if_status()
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_%s_target_kernel = tivxAddTargetKernelByName(" % self.kernel.name_lower)
        self.target_c_code.write_line("                    %s%s_NAME," % (self.kernel.enum_str_prefix, self.kernel.name_upper))
        self.target_c_code.write_line("                    target_name,")
        self.target_c_code.write_line("                    tivx%sProcess," % self.kernel.name_camel)
        self.target_c_code.write_line("                    tivx%sCreate," % self.kernel.name_camel)
        self.target_c_code.write_line("                    tivx%sDelete," % self.kernel.name_camel)
        self.target_c_code.write_line("                    tivx%sControl," % self.kernel.name_camel)
        self.target_c_code.write_line("                    NULL);")
        self.target_c_code.write_close_brace()

        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_remove_func_code(self):
        self.target_c_code.write_line("void tivxRemoveTargetKernel%s(void)" % self.kernel.name_camel, files=0)
        self.target_c_code.write_line("void tivxRemoveTargetKernelBam%s(void)" % self.kernel.name_camel, files=1)
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = (vx_status)VX_SUCCESS;")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("status = tivxRemoveTargetKernel(vx_%s_target_kernel);" % self.kernel.name_lower)
        self.target_c_code.write_if_status()
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_%s_target_kernel = NULL;" % self.kernel.name_lower)
        self.target_c_code.write_close_brace()
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    # performs error checking on string keywords within the attribute string
    def extract_local_mem_string_error_check(self, new_str, type, name):
        found = False
        invalid_str = ""
        if type != Type.ARRAY :
            if "capacity" in new_str :
                found = True
                invalid_str = "capacity"
            elif "itemsize" in new_str :
                found = True
                invalid_str = "itemsize"
            elif "itemtype" in new_str :
                found = True
                invalid_str = "itemtype"
        if type != Type.ARRAY and type != Type.OBJECT_ARRAY:
            if "numitems" in new_str :
                found = True
                invalid_str = "numitems"
        if type != Type.IMAGE and type != Type.REMAP:
            if "width" in new_str :
                found = True
                invalid_str = "width"
            elif "height" in new_str :
                found = True
                invalid_str = "height"
        if type != Type.IMAGE :
            if "stride_x" in new_str :
                found = True
                invalid_str = "stride_x"
            elif "stride_y" in new_str :
                found = True
                invalid_str = "stride_y"
        if type != Type.PYRAMID :
            if "levels" in new_str :
                found = True
                invalid_str = "levels"
        if type != Type.MATRIX and type != Type.CONVOLUTION:
            if "rows" in new_str :
                found = True
                invalid_str = "rows"
            elif "columns" in new_str :
                found = True
                invalid_str = "columns"
        if type != Type.DISTRIBUTION :
            if "dimensions" in new_str :
                found = True
                invalid_str = "dimensions"
            elif "range" in new_str :
                found = True
                invalid_str = "range"
            elif "bins" in new_str :
                found = True
                invalid_str = "bins"
            elif "win" in new_str :
                found = True
                invalid_str = "win"
        if type != Type.DISTRIBUTION and type != Type.LUT:
            if "offset" in new_str :
                found = True
                invalid_str = "offset"
        if type != Type.LUT:
            if "count" in new_str :
                found = True
                invalid_str = "count"
        assert found == False, "'%s' is in invalid string for parameter %s" % (invalid_str, name)

    # extracts from string written by user for local mem allocation
    def extract_local_mem_string(self, type, attribute, local):
        self.extract_local_mem_string_error_check(attribute, type, local.name)
        if type == Type.IMAGE :
            new_str = attribute
            new_str = new_str.replace("width", "%s_desc->imagepatch_addr[0].dim_x" % local.prm.name_lower)
            new_str = new_str.replace("height", "%s_desc->imagepatch_addr[0].dim_y" % local.prm.name_lower)
            new_str = new_str.replace("stride_x", "%s_desc->imagepatch_addr[0].stride_x" % local.prm.name_lower)
            new_str = new_str.replace("stride_y", "%s_desc->imagepatch_addr[0].stride_y" % local.prm.name_lower)
            return new_str
        elif type == Type.ARRAY :
            new_str = attribute
            new_str = new_str.replace("capacity", "%s_desc->capacity" % local.prm.name_lower)
            new_str = new_str.replace("itemsize", "%s_desc->item_size" % local.prm.name_lower)
            new_str = new_str.replace("itemtype", "%s_desc->item_type" % local.prm.name_lower)
            new_str = new_str.replace("numitems", "%s_desc->num_items" % local.prm.name_lower)
            return new_str
        elif type == Type.PYRAMID :
            new_str = attribute
            # should this support width/height?
            new_str = new_str.replace("levels", "%s_desc->num_levels" % local.prm.name_lower)
            return new_str
        elif type == Type.MATRIX :
            new_str = attribute
            new_str = new_str.replace("rows", "%s_desc->rows" % local.prm.name_lower)
            new_str = new_str.replace("columns", "%s_desc->columns" % local.prm.name_lower)
            new_str = new_str.replace("size", "%s_desc->mem_size" % local.prm.name_lower)
            return new_str
        elif type == Type.DISTRIBUTION :
            new_str = attribute
            new_str = new_str.replace("dimensions", "1")
            new_str = new_str.replace("offset", "%s_desc->offset" % local.prm.name_lower)
            new_str = new_str.replace("range", "%s_desc->range" % local.prm.name_lower)
            new_str = new_str.replace("bins", "%s_desc->num_bins" % local.prm.name_lower)
            new_str = new_str.replace("window", "%s_desc->num_win" % local.prm.name_lower)
            new_str = new_str.replace("size", "%s_desc->mem_size" % local.prm.name_lower)
            return new_str
        elif type == Type.LUT :
            new_str = attribute
            new_str = new_str.replace("count", "%s_desc->num_items" % local.prm.name_lower)
            new_str = new_str.replace("size", "%s_desc->mem_size" % local.prm.name_lower)
            return new_str
        elif type == Type.REMAP :
            new_str = attribute
            new_str = new_str.replace("source_width", "%s_desc->src_width" % local.prm.name_lower)
            new_str = new_str.replace("source_height", "%s_desc->src_height" % local.prm.name_lower)
            new_str = new_str.replace("destination_width", "%s_desc->dst_width" % local.prm.name_lower)
            new_str = new_str.replace("destination_height", "%s_desc->dst_height" % local.prm.name_lower)
            return new_str
        elif type == Type.CONVOLUTION :
            new_str = attribute
            new_str = new_str.replace("rows", "%s_desc->rows" % local.prm.name_lower)
            new_str = new_str.replace("columns", "%s_desc->columns" % local.prm.name_lower)
            new_str = new_str.replace("size", "%s_desc->mem_size" % local.prm.name_lower)
            new_str = new_str.replace("scale", "%s_desc->scale" % local.prm.name_lower)
            return new_str
        elif type == Type.OBJECT_ARRAY :
            new_str = attribute
            new_str = new_str.replace("numitems", "%s_desc->num_items" % local.prm.name_lower)
            return new_str

    # extracts from string written by user for local mem allocation
    def is_supported_type(self, type):
        if type == Type.IMAGE :
            return True
        elif type == Type.ARRAY :
            return True
        elif type == Type.PYRAMID :
            return True
        elif type == Type.MATRIX :
            return True
        elif type == Type.DISTRIBUTION :
            return True
        elif type == Type.REMAP :
            return True
        elif type == Type.CONVOLUTION :
            return True
        elif type == Type.LUT :
            return True
        elif type == Type.OBJECT_ARRAY :
            return True
        elif type == Type.NULL :
            return True
        else :
            return False

    # extracts from string written by user for local mem allocation
    def extract_attribute(self, local, is_first_prm):
        invalid_type = False
        if not is_first_prm :
            self.target_c_code.write_line("if ((vx_status)VX_SUCCESS == status)")
            self.target_c_code.write_open_brace()
        if local.prm.type != Type.NULL :
            # verifying that the optional parameter is being used
            if ParamState.OPTIONAL == local.state :
                self.target_c_code.write_line("if( %s_desc != NULL)" % local.prm.name_lower)
                self.target_c_code.write_open_brace()
        if local.prm.type == Type.IMAGE :
             size_str = ""
             append_str = ""
             for num, attr in enumerate(local.attribute_list, start=1):
                 if type(attr) is ImageAttribute :
                     if Attribute.Image.WIDTH == attr :
                         append_str = ("%s_desc->imagepatch_addr[0].dim_x" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Image.HEIGHT == attr :
                         append_str = ("%s_desc->imagepatch_addr[0].dim_y" % local.prm.name_lower)
                         size_str+=append_str
                 elif type(attr) is int :
                     append_str = ("%s" % attr)
                     size_str+=append_str
                 elif type(attr) is str :
                     append_str = self.extract_local_mem_string(local.prm.type, attr, local)
                     size_str+=append_str
                 else :
                    invalid_type = True
                 if num < len(local.attribute_list) :
                     append_str = " * "
                     size_str+=append_str
                 assert invalid_type == False, "'%s' contains an invalid attribute" % (local.prm.name_lower)
        elif local.prm.type == Type.ARRAY :
             size_str = ""
             append_str = ""
             for num, attr in enumerate(local.attribute_list, start=1):
                 if type(attr) is ArrayAttribute :
                     if Attribute.Array.CAPACITY == attr :
                         append_str = ("%s_desc->capacity" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Array.ITEMTYPE == attr :
                         append_str = ("%s_desc->item_type" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Array.NUMITEMS == attr :
                         append_str = ("%s_desc->num_items" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Array.ITEMSIZE == attr :
                         append_str = ("%s_desc->item_size" % local.prm.name_lower)
                         size_str+=append_str
                 elif type(attr) is int :
                     append_str = ("%s" % attr)
                     size_str+=append_str
                 elif type(attr) is str :
                     append_str = self.extract_local_mem_string(local.prm.type, attr, local)
                     size_str+=append_str
                 else :
                    invalid_type = True
                 if num < len(local.attribute_list) :
                     append_str = " * "
                     size_str+=append_str
                 assert invalid_type == False, "'%s' contains an invalid attribute" % (local.prm.name_lower)
        elif local.prm.type == Type.PYRAMID :
             size_str = ""
             append_str = ""
             for num, attr in enumerate(local.attribute_list, start=1):
                 if type(attr) is PyramidAttribute :
                     if Attribute.Pyramid.LEVELS == attr :
                         append_str = ("%s_desc->num_levels" % local.prm.name_lower)
                         size_str+=append_str
                 elif type(attr) is int :
                     append_str = ("%s" % attr)
                     size_str+=append_str
                 elif type(attr) is str :
                     append_str = self.extract_local_mem_string(local.prm.type, attr, local)
                     size_str+=append_str
                 else :
                    invalid_type = True
                 if num < len(local.attribute_list) :
                     append_str = " * "
                     size_str+=append_str
                 assert invalid_type == False, "'%s' contains an invalid attribute" % (local.prm.name_lower)
        elif local.prm.type == Type.MATRIX :
             size_str = ""
             append_str = ""
             for num, attr in enumerate(local.attribute_list, start=1):
                 if type(attr) is MatrixAttribute :
                     if Attribute.Matrix.ROWS == attr :
                         append_str = ("%s_desc->rows" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Matrix.COLUMNS == attr :
                         append_str = ("%s_desc->columns" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Matrix.SIZE == attr :
                         append_str = ("%s_desc->mem_size" % local.prm.name_lower)
                         size_str+=append_str
                 elif type(attr) is int :
                     append_str = ("%s" % attr)
                     size_str+=append_str
                 elif type(attr) is str :
                     append_str = self.extract_local_mem_string(local.prm.type, attr, local)
                     size_str+=append_str
                 else :
                    invalid_type = True
                 if num < len(local.attribute_list) :
                     append_str = " * "
                     size_str+=append_str
                 assert invalid_type == False, "'%s' contains an invalid attribute" % (local.prm.name_lower)
        elif local.prm.type == Type.DISTRIBUTION :
             size_str = ""
             append_str = ""
             for num, attr in enumerate(local.attribute_list, start=1):
                 if type(attr) is DistributionAttribute :
                     if Attribute.Distribution.DIMENSIONS == attr :
                         append_str = ("1")
                         size_str+=append_str
                     elif Attribute.Distribution.OFFSET == attr :
                         append_str = ("%s_desc->offset" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Distribution.RANGE == attr :
                         append_str = ("%s_desc->range" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Distribution.BINS == attr :
                         append_str = ("%s_desc->num_bins" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Distribution.WINDOW == attr :
                         append_str = ("%s_desc->num_win" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Distribution.SIZE == attr :
                         append_str = ("%s_desc->mem_size" % local.prm.name_lower)
                         size_str+=append_str
                 elif type(attr) is int :
                     append_str = ("%s" % attr)
                     size_str+=append_str
                 elif type(attr) is str :
                     append_str = self.extract_local_mem_string(local.prm.type, attr, local)
                     size_str+=append_str
                 else :
                    invalid_type = True
                 if num < len(local.attribute_list) :
                     append_str = " * "
                     size_str+=append_str
                 assert invalid_type == False, "'%s' contains an invalid attribute" % (local.prm.name_lower)
        elif local.prm.type == Type.LUT :
             size_str = ""
             append_str = ""
             for num, attr in enumerate(local.attribute_list, start=1):
                 if type(attr) is LutAttribute :
                     if Attribute.Lut.COUNT == attr :
                         append_str = ("%s_desc->num_items" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Lut.SIZE == attr :
                         append_str = ("%s_desc->mem_size" % local.prm.name_lower)
                         size_str+=append_str
                 elif type(attr) is int :
                     append_str = ("%s" % attr)
                     size_str+=append_str
                 elif type(attr) is str :
                     append_str = self.extract_local_mem_string(local.prm.type, attr, local)
                     size_str+=append_str
                 else :
                    invalid_type = True
                 if num < len(local.attribute_list) :
                     append_str = " * "
                     size_str+=append_str
                 assert invalid_type == False, "'%s' contains an invalid attribute" % (local.prm.name_lower)
        elif local.prm.type == Type.REMAP :
             size_str = ""
             append_str = ""
             for num, attr in enumerate(local.attribute_list, start=1):
                 if type(attr) is RemapAttribute :
                     if Attribute.Remap.SOURCE_WIDTH == attr :
                         append_str = ("%s_desc->src_width" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Remap.SOURCE_HEIGHT == attr :
                         append_str = ("%s_desc->src_height" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Remap.DESTINATION_WIDTH == attr :
                         append_str = ("%s_desc->dst_width" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Remap.DESTINATION_HEIGHT == attr :
                         append_str = ("%s_desc->dst_height" % local.prm.name_lower)
                         size_str+=append_str
                 elif type(attr) is int :
                     append_str = ("%s" % attr)
                     size_str+=append_str
                 elif type(attr) is str :
                     append_str = self.extract_local_mem_string(local.prm.type, attr, local)
                     size_str+=append_str
                 else :
                    invalid_type = True
                 if num < len(local.attribute_list) :
                     append_str = " * "
                     size_str+=append_str
                 assert invalid_type == False, "'%s' contains an invalid attribute" % (local.prm.name_lower)
        elif local.prm.type == Type.OBJECT_ARRAY :
             size_str = ""
             append_str = ""
             for num, attr in enumerate(local.attribute_list, start=1):
                 if type(attr) is ObjectArrayAttribute :
                     if Attribute.ObjectArray.NUMITEMS == attr :
                         append_str = ("%s_desc->num_items" % local.prm.name_lower)
                         size_str+=append_str
                 elif type(attr) is int :
                     append_str = ("%s" % attr)
                     size_str+=append_str
                 elif type(attr) is str :
                     append_str = self.extract_local_mem_string(local.prm.type, attr, local)
                     size_str+=append_str
                 else :
                    invalid_type = True
                 if num < len(local.attribute_list) :
                     append_str = " * "
                     size_str+=append_str
                 assert invalid_type == False, "'%s' contains an invalid attribute" % (local.prm.name_lower)
        elif local.prm.type == Type.CONVOLUTION :
             size_str = ""
             append_str = ""
             for num, attr in enumerate(local.attribute_list, start=1):
                 if type(attr) is ConvolutionAttribute :
                     if Attribute.Convolution.ROWS == attr :
                         append_str = ("%s_desc->rows" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Convolution.COLUMNS == attr :
                         append_str = ("%s_desc->columns" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Convolution.SCALE == attr :
                         append_str = ("%s_desc->scale" % local.prm.name_lower)
                         size_str+=append_str
                     elif Attribute.Convolution.SIZE == attr :
                         append_str = ("%s_desc->mem_size" % local.prm.name_lower)
                         size_str+=append_str
                 elif type(attr) is int :
                     append_str = ("%s" % attr)
                     size_str+=append_str
                 elif type(attr) is str :
                     append_str = self.extract_local_mem_string(local.prm.type, attr, local)
                     size_str+=append_str
                 else :
                    invalid_type = True
                 if num < len(local.attribute_list) :
                     append_str = " * "
                     size_str+=append_str
                 assert invalid_type == False, "'%s' contains an invalid attribute" % (local.prm.name_lower)
        elif local.prm.type == Type.NULL :
             size_str = ""
             append_str = ""
             for num, attr in enumerate(local.attribute_list, start=1):
                 if type(attr) is int :
                     append_str = ("%s" % attr)
                     size_str+=append_str
                 elif type(attr) is str :
                     append_str = attr
                     size_str+=append_str
                 if num < len(local.attribute_list) :
                     append_str = " * "
                     size_str+=append_str
        # setting 0 is for allocating mem; setting 1 is for setting mem to 0
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: Verify correct amount of memory is allocated >")
        self.target_c_code.write_line("prms->%s_size = %s;" % (local.name, size_str) )
        self.target_c_code.write_line("prms->%s_ptr = tivxMemAlloc(prms->%s_size, (vx_enum)TIVX_MEM_EXTERNAL);" % (local.name, local.name) )
        self.target_c_code.write_newline()
        self.target_c_code.write_line("if (NULL == prms->%s_ptr)" % (local.name) )
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("status = (vx_status)VX_ERROR_NO_MEMORY;")
        self.target_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"Unable to allocate local memory\\n\");")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_line("else")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: Verify memory setting to clear the correct amount of memory >")
        self.target_c_code.write_line("memset(prms->%s_ptr, 0, %s);" % (local.name, size_str) )
        self.target_c_code.write_close_brace()
        if not is_first_prm :
            self.target_c_code.write_close_brace()
        else :
            self.target_c_code.write_newline()
        if local.prm.type != Type.NULL :
             # verifying that the optional parameter is being used
             if ParamState.OPTIONAL == local.state :
                 self.target_c_code.write_close_brace()

    def generate_target_c_create_func_code(self):
        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sCreate(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = (vx_status)VX_SUCCESS;")
        contains_user_data_object = False
        for prm in self.kernel.params :
            if Type.USER_DATA_OBJECT == prm.type :
                contains_user_data_object = True
        if self.prms_needed:
            if self.prms_commented_out:
                self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment if kernel context is needed >")
                self.target_c_code.write_line("#if 0" , files=self.prms_write)
            self.target_c_code.write_line("tivx%sParams *prms = NULL;" % self.kernel.name_camel, files=self.prms_write)
            if self.prms_commented_out:
                self.target_c_code.write_line("#endif" , files=self.prms_write)
        self.target_c_code.write_newline()
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Add any target kernel create code here (e.g. allocating")
        self.target_c_code.write_comment_line("                  local memory buffers, one time initialization, etc) >")
        if self.prms_needed or contains_user_data_object :
            # checks function parameters
            self.target_c_code.write_line("if ( (num_params != %s%s_MAX_PARAMS)" % (self.kernel.enum_str_prefix, self.kernel.name_upper) , files=self.prms_write)
            for prm in self.kernel.params :
                if prm.state is ParamState.REQUIRED :
                    self.target_c_code.write_line("    || (NULL == obj_desc[%s%s_%s_IDX])" % (self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper), files=self.prms_write)
            self.target_c_code.write_line(")", files=self.prms_write)
            self.target_c_code.write_open_brace(files=self.prms_write)
            # function parameters status check failure case
            self.target_c_code.write_line("status = (vx_status)VX_FAILURE;", files=self.prms_write)
            self.target_c_code.write_close_brace(files=self.prms_write)
            self.target_c_code.write_line("else", files=self.prms_write)
            self.target_c_code.write_open_brace(files=self.prms_write)

            # declaring variables
            duplicates = []
            for local in self.kernel.local_mem_list :
                 if local.prm.type != Type.NULL :
                     if not (local.prm.name_lower in duplicates) :
                         self.target_c_code.write_line("%s *%s_desc;" % (Type.get_obj_desc_name(local.prm.type), local.prm.name_lower) , files=self.prms_write)
                         duplicates.append(local.prm.name_lower)
            for prm in self.kernel.params :
                 if Type.USER_DATA_OBJECT == prm.type :
                     if not (prm.name_lower in duplicates) :
                         self.target_c_code.write_line("%s *%s_desc;" % (Type.get_obj_desc_name(prm.type), prm.name_lower) , files=self.prms_write)
                         duplicates.append(prm.name_lower)
            self.target_c_code.write_newline(files=self.prms_write)

            # populating object descriptors
            duplicates = []
            for local in self.kernel.local_mem_list :
                 if local.prm.type != Type.NULL :
                     if not (local.prm.name_lower in duplicates) :
                         self.target_c_code.write_line("%s_desc = (%s *)obj_desc[%s%s_%s_IDX];" %
                            (local.prm.name_lower, Type.get_obj_desc_name(local.prm.type), self.kernel.enum_str_prefix, self.kernel.name_upper, local.prm.name_upper) , files=self.prms_write)
                         duplicates.append(local.prm.name_lower)
            for prm in self.kernel.params :
                 if Type.USER_DATA_OBJECT == prm.type :
                     if not (prm.name_lower in duplicates) :
                         self.target_c_code.write_line("%s_desc = (%s *)obj_desc[%s%s_%s_IDX];" %
                            (prm.name_lower, Type.get_obj_desc_name(prm.type), self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) , files=self.prms_write)
                         duplicates.append(prm.name_lower)
            self.target_c_code.write_newline(files=self.prms_write)

            if contains_user_data_object :
                for prm in self.kernel.params :
                    if Type.USER_DATA_OBJECT == prm.type :
                        if len(prm.data_types) == 0 :
                            self.target_c_code.write_comment_line("< DEVELOPER_TODO: Replace <Add type here> with correct data type >", files=self.prms_write)
                            self.print_data_type = ['<Add type here>']
                        else :
                            self.print_data_type = prm.data_types
                        self.target_c_code.write_line("if (%s_desc->mem_size != sizeof(%s))" % (prm.name_lower, self.print_data_type[0]) , files=self.prms_write)
                        self.target_c_code.write_open_brace(files=self.prms_write)
                        self.target_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"User data object size on target does not match the size on host, possibly due to misalignment in data structure\\n\");", files=self.prms_write)
                        self.target_c_code.write_line("status = (vx_status)VX_FAILURE;", files=self.prms_write)
                        self.target_c_code.write_close_brace(files=self.prms_write)

            # Allocating memory for local structure
            if self.prms_needed:
                if self.prms_commented_out:
                    self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment if kernel context is needed >")
                    self.target_c_code.write_line("#if 0" , files=self.prms_write)
                self.target_c_code.write_line("prms = tivxMemAlloc(sizeof(tivx%sParams), (vx_enum)TIVX_MEM_EXTERNAL);" % self.kernel.name_camel, files=self.prms_write)
                self.target_c_code.write_line("if (NULL != prms)", files=self.prms_write)
                self.target_c_code.write_open_brace(files=self.prms_write)
                # Allocating local memory data
                is_first_prm = True
                for local in self.kernel.local_mem_list :
                     if self.is_supported_type(local.prm.type) :
                         self.extract_attribute(local, is_first_prm)
                         is_first_prm = False
                self.target_c_code.write_newline(files=self.prms_write)

                self.target_c_code.write_close_brace(files=self.prms_write)
                self.target_c_code.write_line("else", files=self.prms_write)
                self.target_c_code.write_open_brace(files=self.prms_write)
                self.target_c_code.write_line("status = (vx_status)VX_ERROR_NO_MEMORY;", files=self.prms_write)
                self.target_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"Unable to allocate local memory\\n\");", files=self.prms_write)
                self.target_c_code.write_close_brace(files=self.prms_write)
                self.target_c_code.write_newline(files=self.prms_write)

                # Place to create BAM graph
                self.target_c_code.write_line("if (NULL != prms)", files=1)
                self.target_c_code.write_open_brace(files=1)
                self.target_c_code.write_comment_line("< DEVELOPER_TODO: Create BAM graph using graph_handle >", files=1)
                self.target_c_code.write_close_brace(files=1)
                self.target_c_code.write_newline(files=1)

                self.target_c_code.write_line("if ((vx_status)VX_SUCCESS == status)", files=self.prms_write)
                self.target_c_code.write_open_brace(files=self.prms_write)
                self.target_c_code.write_line("tivxSetTargetKernelInstanceContext(kernel, prms,", files=self.prms_write)
                self.target_c_code.write_line("    sizeof(tivx%sParams));" % self.kernel.name_camel, files=self.prms_write)
                self.target_c_code.write_close_brace(files=self.prms_write)
                self.target_c_code.write_line("else", files=self.prms_write)
                self.target_c_code.write_open_brace(files=self.prms_write)
                self.target_c_code.write_line("status = (vx_status)VX_ERROR_NO_MEMORY;", files=self.prms_write)
                self.target_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"Unable to allocate local memory\\n\");", files=self.prms_write)
                self.target_c_code.write_close_brace(files=self.prms_write)
                if self.prms_commented_out:
                    self.target_c_code.write_line("#endif" , files=self.prms_write)
            self.target_c_code.write_close_brace(files=self.prms_write)


        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_delete_func_code(self):
        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sDelete(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = (vx_status)VX_SUCCESS;")
        if self.prms_needed :
            if self.prms_commented_out:
                self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment if kernel context is needed >")
                self.target_c_code.write_line("#if 0" , files=self.prms_write)
            self.target_c_code.write_line("tivx%sParams *prms = NULL;" % self.kernel.name_camel, files=self.prms_write)
            self.target_c_code.write_line("uint32_t size;", files=self.prms_write)
            if self.prms_commented_out:
                self.target_c_code.write_line("#endif" , files=self.prms_write)
        self.target_c_code.write_newline(files=self.prms_write)
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing")
        self.target_c_code.write_comment_line("                  local memory buffers, etc) >")
        if self.prms_needed :
            if self.prms_commented_out:
                self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment if kernel context is needed >")
                self.target_c_code.write_line("#if 0" , files=self.prms_write)
            # checks function parameters
            self.target_c_code.write_line("if ( (num_params != %s%s_MAX_PARAMS)" % (self.kernel.enum_str_prefix, self.kernel.name_upper) , files=self.prms_write)
            for prm in self.kernel.params :
                if prm.state is ParamState.REQUIRED :
                    self.target_c_code.write_line("    || (NULL == obj_desc[%s%s_%s_IDX])" % (self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper), files=self.prms_write)
            self.target_c_code.write_line(")", files=self.prms_write)
            self.target_c_code.write_open_brace(files=self.prms_write)
            # function parameters status check failure case
            self.target_c_code.write_line("status = (vx_status)VX_FAILURE;", files=self.prms_write)
            self.target_c_code.write_close_brace(files=self.prms_write)
            self.target_c_code.write_line("else", files=self.prms_write)
            self.target_c_code.write_open_brace(files=self.prms_write)
            self.target_c_code.write_line("tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);", files=self.prms_write)
            self.target_c_code.write_newline(files=self.prms_write)
            self.target_c_code.write_line("if ((NULL != prms) &&", files=self.prms_write)
            self.target_c_code.write_line("    (sizeof(tivx%sParams) == size))" % self.kernel.name_camel, files=self.prms_write)
            self.target_c_code.write_open_brace(files=self.prms_write)
            for local in self.kernel.local_mem_list :
                 if self.is_supported_type(local.prm.type) :
                     self.target_c_code.write_line("if (NULL != prms->%s_ptr)" % (local.name), files=self.prms_write)
                     self.target_c_code.write_open_brace(files=self.prms_write)
                     self.target_c_code.write_line("tivxMemFree(prms->%s_ptr, prms->%s_size, (vx_enum)TIVX_MEM_EXTERNAL);" %
                         (local.name, local.name) , files=self.prms_write)
                     self.target_c_code.write_close_brace(files=self.prms_write)
                     self.target_c_code.write_newline(files=self.prms_write)

            self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment once BAM graph has been created >", files=1)
            self.target_c_code.write_comment_line("tivxBamDestroyHandle(prms->graph_handle);", files=1)
            self.target_c_code.write_line("tivxMemFree(prms, size, (vx_enum)TIVX_MEM_EXTERNAL);", files=self.prms_write)
            self.target_c_code.write_close_brace(files=self.prms_write)
            self.target_c_code.write_close_brace(files=self.prms_write)
            if self.prms_commented_out:
                self.target_c_code.write_line("#endif" , files=self.prms_write)
        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_control_func_code(self):
        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sControl(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = (vx_status)VX_SUCCESS;")
        self.target_c_code.write_newline()
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Add any target kernel control code here (e.g. commands")
        self.target_c_code.write_comment_line("                  the user can call to modify the processing of the kernel at run-time) >")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_struct(self):
        self.target_c_code.write_line("typedef struct", files=self.prms_write)
        self.target_c_code.write_line("{", files=self.prms_write)
        for local in self.kernel.local_mem_list :
            self.target_c_code.write_line("    void     *%s_ptr;" % local.name , files=self.prms_write)
            self.target_c_code.write_line("    uint32_t %s_size;" % local.name , files=self.prms_write)
        if self.target_uses_dsp :
            self.target_c_code.write_line("    tivx_bam_graph_handle graph_handle;" , files=1)
        self.target_c_code.write_line("} tivx%sParams;" % self.kernel.name_camel, files=self.prms_write)
        self.target_c_code.write_newline(files=self.prms_write)

    def generate_bam_pointers(self, kernel_params):
        idx = 0
        for prm in kernel_params :
            if prm.type == Type.IMAGE :
                self.target_c_code.write_line("img_ptrs[%s] = %s_addr;" % (idx, prm.name_lower), files=1)
                idx += 1
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment once BAM graph has been created >", files=1)
        self.target_c_code.write_comment_line("tivxBamUpdatePointers(prms->graph_handle, %sU, %sU, img_ptrs);" % (self.kernel.getNumInputImages(), self.kernel.getNumOutputImages()), files=1)
        self.target_c_code.write_newline(files=1)

    def generate_optional_bam_pointers(self, num_scenarios) :
        first_scenario = True
        first_param = True
        binary_mask = num_scenarios
        for scenario in range(num_scenarios) :
            first_param = True
            included_str = []
            num_included_optional_input = 0
            num_included_optional_output = 0
            for idx, prm in enumerate(self.optional_img_params) :
                if binary_mask & (2**idx) :
                    included_str.append(prm.name_lower)
                    if first_scenario == True and first_param == True:
                        self.target_c_code.write_line ("if ( (%s_desc != NULL)" % prm.name_lower, files=1)
                        first_scenario = False
                        first_param = False
                    elif first_param == True :
                        self.target_c_code.write_line ("else if ( (%s_desc != NULL)" % prm.name_lower, files=1)
                        first_scenario = False
                        first_param = False
                    else :
                        self.target_c_code.write_line ("    && (%s_desc != NULL)" % prm.name_lower, files=1)
                    if prm.direction == Direction.INPUT :
                        num_included_optional_input += 1
                    elif prm.direction == Direction.OUTPUT :
                        num_included_optional_output += 1
            self.target_c_code.write_line (")", files=1)
            self.target_c_code.write_open_brace(files=1)
            idx = 0
            for prm in self.kernel.params :
                if (prm.type == Type.IMAGE and prm.state == ParamState.REQUIRED) or \
                   (prm.name_lower in included_str):
                    self.target_c_code.write_line("img_ptrs[%s] = %s_addr;" % (idx, prm.name_lower), files=1)
                    idx += 1
            self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment once BAM graph has been created >", files=1)
            self.target_c_code.write_comment_line("tivxBamUpdatePointers(prms->graph_handle, %sU, %sU, img_ptrs);" % (self.kernel.getNumRequiredInputImages()+num_included_optional_input, \
                self.kernel.getNumRequiredOutputImages()+num_included_optional_output), files=1)
            self.target_c_code.write_close_brace(files=1)
            binary_mask -= 1

        self.target_c_code.write_line ("else", files=1)
        self.target_c_code.write_open_brace(files=1)
        idx = 0
        for prm in self.kernel.params :
            if (prm.type == Type.IMAGE and prm.state == ParamState.REQUIRED) :
                self.target_c_code.write_line("img_ptrs[%s] = %s_addr;" % (idx, prm.name_lower), files=1)
                idx += 1
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment once BAM graph has been created >", files=1)
        self.target_c_code.write_comment_line("tivxBamUpdatePointers(prms->graph_handle, %sU, %sU, img_ptrs);" % (self.kernel.getNumRequiredInputImages(), \
            self.kernel.getNumRequiredOutputImages()), files=1)
        self.target_c_code.write_close_brace(files=1)


    def generate_optional_list(self, kernel_params) :
        self.optional_img_params = []
        for prm in kernel_params :
            if prm.type == Type.IMAGE and prm.state == ParamState.OPTIONAL :
                self.optional_img_params.append(prm)

    def generate_target_c_process_func_code(self):
        # define function name, and parameters
        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sProcess(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()

        # define status variables and obj descriptor variable
        self.target_c_code.write_line("vx_status status = (vx_status)VX_SUCCESS;")
        if self.prms_needed :
            if self.prms_commented_out:
                self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment if kernel context is needed >")
                self.target_c_code.write_line("#if 0" , files=self.prms_write)
            self.target_c_code.write_line("tivx%sParams *prms = NULL;" % self.kernel.name_camel, files=self.prms_write)
            if self.prms_commented_out:
                self.target_c_code.write_line("#endif" , files=self.prms_write)
        need_plane_idx_var = False
        need_exposure_idx_var = False
        need_pyramid_idx_var = False
        printed_incrementer = False
        for prm in self.kernel.params :
            if Type.IMAGE == prm.type or Type.PYRAMID == prm.type:
                if len(prm.data_types) > 1 :
                    for dt in prm.data_types[0:-1] :
                        if DfImage.get_num_planes(DfImage.get_df_enum_from_string(dt)) > 1 :
                            need_plane_idx_var = True
                            break
                if Type.PYRAMID == prm.type:
                    need_pyramid_idx_var = True
            if Type.RAW_IMAGE == prm.type :
                need_exposure_idx_var = True
            self.target_c_code.write_line("%s *%s_desc;" % (Type.get_obj_desc_name(prm.type), prm.name_lower) )
            if prm.type == Type.PYRAMID :
                self.target_c_code.write_line("%s *img_%s_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];" % (Type.get_obj_desc_name(Type.IMAGE), prm.name_lower) )
                if printed_incrementer is False :
                    self.target_c_code.write_line("vx_uint32 i;")
                    printed_incrementer = True
            #TODO: Object Array is hardcoded to image ... modify for proper type
            if prm.type == Type.OBJECT_ARRAY :
                self.target_c_code.write_line("%s *img_%s_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];" % (Type.get_obj_desc_name(Type.IMAGE), prm.name_lower) )
                if printed_incrementer is False :
                    self.target_c_code.write_line("vx_uint32 i;")
                    printed_incrementer = True
        if need_plane_idx_var is True :
            self.target_c_code.write_line("uint16_t plane_idx;")
        if need_exposure_idx_var is True :
            self.target_c_code.write_line("uint16_t exposure_idx;")
        if need_pyramid_idx_var is True and printed_incrementer is False :
            self.target_c_code.write_line("vx_uint32 i;")

        self.target_c_code.write_newline()

        # checks function parameters
        self.target_c_code.write_line("if ( (num_params != %s%s_MAX_PARAMS)" % (self.kernel.enum_str_prefix, self.kernel.name_upper) )
        for prm in self.kernel.params :
            if prm.state is ParamState.REQUIRED :
                self.target_c_code.write_line("    || (NULL == obj_desc[%s%s_%s_IDX])" % (self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper))
        self.target_c_code.write_line(")")

        self.target_c_code.write_open_brace()

        # function parameters status check failure case
        self.target_c_code.write_line("status = (vx_status)VX_FAILURE;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

        self.target_c_code.write_line("if((vx_status)VX_SUCCESS == status)")
        self.target_c_code.write_open_brace()

        if self.prms_needed :
            if self.prms_commented_out:
                self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment if kernel context is needed >")
                self.target_c_code.write_line("#if 0" , files=self.prms_write)
            self.target_c_code.write_line("uint32_t size;", files=self.prms_write)
            if self.prms_commented_out:
                self.target_c_code.write_line("#endif" , files=self.prms_write)

        # assigned descriptors to local variables
        for prm in self.kernel.params :
            self.target_c_code.write_line("%s_desc = (%s *)obj_desc[%s%s_%s_IDX];" %
                (prm.name_lower, Type.get_obj_desc_name(prm.type), self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
        self.target_c_code.write_newline()

        # retrieving prms struct for use
        if self.prms_needed :
            if self.prms_commented_out:
                self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment if kernel context is needed >")
                self.target_c_code.write_line("#if 0" , files=self.prms_write)
            self.target_c_code.write_line("status = tivxGetTargetKernelInstanceContext(kernel,", files=self.prms_write)
            self.target_c_code.write_line("    (void **)&prms, &size);", files=self.prms_write)
            self.target_c_code.write_line("if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||", files=self.prms_write)
            self.target_c_code.write_line("    (sizeof(tivx%sParams) != size))" % self.kernel.name_camel, files=self.prms_write)
            self.target_c_code.write_open_brace(files=self.prms_write)
            self.target_c_code.write_line("status = (vx_status)VX_FAILURE;", files=self.prms_write)
            self.target_c_code.write_close_brace(files=self.prms_write)
            if self.prms_commented_out:
                self.target_c_code.write_line("#endif" , files=self.prms_write)

        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()
        # function parameters status check success case

        self.target_c_code.write_line("if((vx_status)VX_SUCCESS == status)")
        self.target_c_code.write_open_brace()

        # define variables to hold scalar values
        for prm in self.kernel.params :
            if Type.is_scalar_type(prm.type) is True :
                self.target_c_code.write_line("%s %s_value;" % (Type.get_vx_name(prm.type), prm.name_lower ))
        self.target_c_code.write_newline()

        # assigned descriptors to local variables
        for prm in self.kernel.params :
            if prm.type == Type.PYRAMID:
                self.target_c_code.write_line("void *%s_target_ptr[TIVX_PYRAMID_MAX_LEVEL_OBJECTS] = {NULL};" % prm.name_lower )
            elif prm.type == Type.OBJECT_ARRAY :
                self.target_c_code.write_line("void *%s_target_ptr[TIVX_OBJECT_ARRAY_MAX_ITEMS] = {NULL};" % prm.name_lower )
            elif prm.type == Type.RAW_IMAGE :
                self.target_c_code.write_line("void *%s_target_ptr[TIVX_RAW_IMAGE_MAX_EXPOSURES] = {NULL};" % prm.name_lower )
            elif prm.type == Type.IMAGE :
                if len(prm.data_types) > 1 :
                    for dt in prm.data_types[0:-1] :
                        if DfImage.get_num_planes(DfImage.get_df_enum_from_string(dt)) > 1 :
                            self.target_c_code.write_line("void *%s_target_ptr[TIVX_IMAGE_MAX_PLANES] = {NULL};" % prm.name_lower )
                        else :
                            self.target_c_code.write_line("void *%s_target_ptr;" % prm.name_lower )
                else :
                    self.target_c_code.write_line("void *%s_target_ptr;" % prm.name_lower )
            else :
                self.target_c_code.write_line("void *%s_target_ptr;" % prm.name_lower )
        self.target_c_code.write_newline()

        # convert descriptors pointer to target pointers
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if Type.is_scalar_type(prm.type) is False :
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_line("if( %s != NULL)" % desc)
                    self.target_c_code.write_open_brace()
                if Type.IMAGE == prm.type or Type.PYRAMID == prm.type or Type.OBJECT_ARRAY == prm.type or Type.RAW_IMAGE == prm.type:
                    # Check if data type has multi-planes
                    self.multiplane = False
                    if len(prm.data_types) > 1 :
                        for dt in prm.data_types[0:-1] :
                            if DfImage.get_num_planes(DfImage.get_df_enum_from_string(dt)) > 1 :
                                self.multiplane = True
                                break
                    if prm.type == Type.IMAGE :
                        if self.multiplane :
                            self.target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("%s_target_ptr[plane_idx] = tivxMemShared2TargetPtr(&%s->mem_ptr[plane_idx]);" % (prm.name_lower, desc))
                            if prm.do_map :
                                self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferMap(%s_target_ptr[plane_idx]," % prm.name_lower )
                                self.target_c_code.write_line("   %s->mem_size[plane_idx], (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                                self.target_c_code.write_line("   (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                            self.target_c_code.write_close_brace()
                        else :
                            self.target_c_code.write_line("%s_target_ptr = tivxMemShared2TargetPtr(&%s->mem_ptr[0]);" % (prm.name_lower, desc))
                            if prm.do_map :
                                self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferMap(%s_target_ptr," % prm.name_lower )
                                self.target_c_code.write_line("   %s->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                                self.target_c_code.write_line("   (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                    elif prm.type == Type.RAW_IMAGE :
                        self.target_c_code.write_line("for(exposure_idx=0; exposure_idx<%s->params.num_exposures; exposure_idx++)" % desc )
                        self.target_c_code.write_open_brace()
                        self.target_c_code.write_line("%s_target_ptr[exposure_idx] = tivxMemShared2TargetPtr(&%s->mem_ptr[exposure_idx]);" % (prm.name_lower, desc))
                        if prm.do_map :
                            self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferMap(%s_target_ptr[exposure_idx]," % prm.name_lower )
                            self.target_c_code.write_line("   %s->mem_size[exposure_idx], (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                            self.target_c_code.write_line("   (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                        self.target_c_code.write_close_brace()
                    elif prm.type == Type.PYRAMID or prm.type == Type.OBJECT_ARRAY:
                        if prm.type == Type.PYRAMID :
                            self.target_c_code.write_line("tivxGetObjDescList(%s->obj_desc_id, (tivx_obj_desc_t**)img_%s, %s->num_levels);" % (desc, desc, desc) )
                        else :
                            self.target_c_code.write_line("tivxGetObjDescList(%s->obj_desc_id, (tivx_obj_desc_t**)img_%s, %s->num_items);" % (desc, desc, desc) )
                        if self.multiplane :
                            if prm.type == Type.PYRAMID :
                                self.target_c_code.write_line("for(i=0U; i<%s->num_levels; i++)" % desc )
                            else :
                                self.target_c_code.write_line("for(i=0U; i<%s->num_items; i++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("%s_target_ptr[i] = tivxMemShared2TargetPtr(&img_%s[i]->mem_ptr[plane_idx]);" % (prm.name_lower, desc))
                            if prm.do_map :
                                self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferMap(%s_target_ptr[i]," % prm.name_lower )
                                self.target_c_code.write_line("   img_%s[i]->mem_size[plane_idx], (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                                self.target_c_code.write_line("   (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                            self.target_c_code.write_close_brace()
                            self.target_c_code.write_close_brace()
                        else :
                            if prm.type == Type.PYRAMID :
                                self.target_c_code.write_line("for(i=0U; i<%s->num_levels; i++)" % desc )
                            else :
                                self.target_c_code.write_line("for(i=0U; i<%s->num_items; i++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("%s_target_ptr[i] = tivxMemShared2TargetPtr(&img_%s[i]->mem_ptr[0]);" % (prm.name_lower, desc))
                            if prm.do_map :
                                self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferMap(%s_target_ptr[i]," % prm.name_lower )
                                self.target_c_code.write_line("   img_%s[i]->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                                self.target_c_code.write_line("   (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                            self.target_c_code.write_close_brace()
                elif prm.type != Type.THRESHOLD:
                    self.target_c_code.write_line("%s_target_ptr = tivxMemShared2TargetPtr(&%s->mem_ptr);" % (prm.name_lower, desc))
                    if prm.do_map :
                        self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferMap(%s_target_ptr," % prm.name_lower )
                        self.target_c_code.write_line("   %s->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                        self.target_c_code.write_line("   (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_close_brace()
                self.target_c_code.write_newline()
        self.target_c_code.write_newline()

        # set scalar values to local variables for input type scalars
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if (Type.is_scalar_type(prm.type) is True) and prm.direction != Direction.OUTPUT :
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_line("if( %s != NULL)" % desc)
                    self.target_c_code.write_open_brace()
                if "invalid" != Type.get_scalar_obj_desc_data_name(prm.type):
                    self.target_c_code.write_line("%s_value = %s->data.%s;" % (prm.name_lower, desc, Type.get_scalar_obj_desc_data_name(prm.type)))
                else :
                    self.target_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Modify 'scalar data type' below to be correct type >")
                    self.target_c_code.write_line("/*%s_value = %s->data.<scalar data type>;*/" % (prm.name_lower, desc))
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

        self.target_c_code.write_open_brace()
        if self.target_uses_dsp :
            if self.kernel.getNumImages() > 0:
                self.target_c_code.write_line("void *img_ptrs[%s];" % self.kernel.getNumImages(), files=1)

        # Setting up bufparams and pointer location in case of image
        for prm in self.kernel.params :
            if prm.type == Type.IMAGE and (prm.do_map or prm.do_unmap):
                self.target_c_code.write_line("VXLIB_bufParams2D_t vxlib_%s;" % prm.name_lower)
                self.target_c_code.write_line("uint8_t *%s_addr = NULL;" % prm.name_lower)
        self.target_c_code.write_newline()
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if prm.type == Type.IMAGE and (prm.do_map or prm.do_unmap):
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_line("if( %s != NULL)" % desc)
                    self.target_c_code.write_open_brace()
                self.target_c_code.write_line("tivxInitBufParams(%s, &vxlib_%s);" % (desc, prm.name_lower) )
                self.target_c_code.write_line("tivxSetPointerLocation(%s, &%s_target_ptr, &%s_addr);" % (desc, prm.name_lower, prm.name_lower) )
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_close_brace()
                self.target_c_code.write_newline()

        if self.target_uses_dsp :
            if self.kernel.getNumImages() > 0:
                self.generate_optional_list(self.kernel.params)
                if self.kernel.getNumOptionalImages() > 0 :
                    num_scenarios = (2 ** self.kernel.getNumOptionalImages()) - 1
                else :
                    num_scenarios = 1
                if num_scenarios == 1 and self.kernel.getNumOptionalImages() == 0 :
                    self.generate_bam_pointers(self.kernel.params)
                elif num_scenarios == 1 and self.kernel.getNumOptionalImages() > 0 :
                    self.generate_optional_bam_pointers(num_scenarios)
                else :
                    self.generate_optional_bam_pointers(num_scenarios)
                self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment once BAM graph has been created >", files=1)
                self.target_c_code.write_comment_line("status  = tivxBamProcessGraph(prms->graph_handle);", files=1)
                self.target_c_code.write_newline(files=1)

        self.target_c_code.write_comment_line("call kernel processing function")
        self.target_c_code.write_newline()
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: Add target kernel processing code here >")
        self.target_c_code.write_newline()
        self.target_c_code.write_comment_line("kernel processing function complete")
        self.target_c_code.write_newline()
        self.target_c_code.write_close_brace()

        # unmap descriptors pointer
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if prm.do_unmap :
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_line("if( %s != NULL)" % desc)
                    self.target_c_code.write_open_brace()
                if Type.IMAGE == prm.type or Type.PYRAMID == prm.type or Type.OBJECT_ARRAY == prm.type or Type.RAW_IMAGE == prm.type:
                    # Check if data type has multi-planes
                    self.multiplane = False
                    if len(prm.data_types) > 1 :
                        for dt in prm.data_types[0:-1] :
                            if DfImage.get_num_planes(DfImage.get_df_enum_from_string(dt)) > 1 :
                                self.multiplane = True
                                break
                    if prm.type == Type.IMAGE :
                        if self.multiplane :
                            self.target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferUnmap(%s_target_ptr[plane_idx]," % prm.name_lower )
                            self.target_c_code.write_line("   %s->mem_size[plane_idx], (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                            self.target_c_code.write_line("    (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                            self.target_c_code.write_close_brace()
                        else :
                            self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferUnmap(%s_target_ptr," % prm.name_lower )
                            self.target_c_code.write_line("   %s->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                            self.target_c_code.write_line("    (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                    elif prm.type == Type.RAW_IMAGE :
                        self.target_c_code.write_line("for(exposure_idx=0; exposure_idx<%s->params.num_exposures; exposure_idx++)" % desc )
                        self.target_c_code.write_open_brace()
                        self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferUnmap(%s_target_ptr[exposure_idx]," % prm.name_lower )
                        self.target_c_code.write_line("   %s->mem_size[exposure_idx], (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                        self.target_c_code.write_line("    (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                        self.target_c_code.write_close_brace()
                    elif prm.type == Type.PYRAMID or prm.type == Type.OBJECT_ARRAY :
                        if self.multiplane :
                            if prm.type == Type.PYRAMID :
                                self.target_c_code.write_line("for(i=0U; i<%s->num_levels; i++)" % desc )
                            else :
                                self.target_c_code.write_line("for(i=0U; i<%s->num_items; i++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferUnmap(%s_target_ptr[i]," % prm.name_lower )
                            self.target_c_code.write_line("   img_%s[i]->mem_size[plane_idx], (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                            self.target_c_code.write_line("    (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                            self.target_c_code.write_close_brace()
                            self.target_c_code.write_close_brace()
                        else :
                            if prm.type == Type.PYRAMID :
                                self.target_c_code.write_line("for(i=0U; i<%s->num_levels; i++)" % desc )
                            else :
                                self.target_c_code.write_line("for(i=0U; i<%s->num_items; i++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferUnmap(%s_target_ptr[i]," % prm.name_lower )
                            self.target_c_code.write_line("   img_%s[i]->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                            self.target_c_code.write_line("    (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                            self.target_c_code.write_close_brace()
                elif prm.type != Type.THRESHOLD :
                    self.target_c_code.write_line("tivxCheckStatus(&status, tivxMemBufferUnmap(%s_target_ptr," % prm.name_lower )
                    self.target_c_code.write_line("   %s->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST," % desc)
                    self.target_c_code.write_line("    (vx_enum)%s));" % Direction.get_access_type(prm.direction))
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_close_brace()
                self.target_c_code.write_newline()
        self.target_c_code.write_newline()

        # set scalar values from local variables for output type scalars
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if (Type.is_scalar_type(prm.type) is True) and prm.direction != Direction.INPUT :
                if "invalid" != Type.get_scalar_obj_desc_data_name(prm.type):
                    self.target_c_code.write_line("%s->data.%s = %s_value;" % (desc, Type.get_scalar_obj_desc_data_name(prm.type), prm.name_lower))
                else :
                    self.target_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Modify 'scalar data type' below to be correct type >")
                    self.target_c_code.write_line("/*%s->data.<scalar data type> = %s_value;*/" % (prm.name_lower, desc))
        self.target_c_code.write_newline()

        self.target_c_code.write_close_brace()

        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_file_code(self):
        print("Creating " + self.workarea_module_core + "/" + self.target_c_filename)
        if self.target_uses_dsp :
            self.target_c_code = CodeGenerate(self.workarea_module_core + "/" + self.target_c_filename, True, self.workarea_module_core_bam + "/" + self.bam_target_c_filename)
            print("Creating " + self.workarea_module_core_bam + "/" + self.bam_target_c_filename)
        else :
            self.target_c_code = CodeGenerate(self.workarea_module_core + "/" + self.target_c_filename)
        self.target_c_code.write_include("TI/tivx.h")
        self.target_c_code.write_include(self.company + "/" + self.top_header_name + ".h")
        self.target_c_code.write_include("VX/vx.h")
        self.target_c_code.write_include("tivx_" + self.module.lower()  + "_kernels" + self.kernels_header_extension + ".h")
        self.target_c_code.write_include(self.h_filename)
        self.target_c_code.write_include("TI/tivx_target_kernel.h")
        self.target_c_code.write_include("tivx_kernels_target_utils.h")
        if self.target_uses_dsp :
            self.target_c_code.write_include("tivx_bam_kernel_wrapper.h", files=1)
        self.target_c_code.write_newline()
        # Calling method for creating struct based on if localMem is needing to be allocated
        if self.prms_needed == True :
            if self.prms_commented_out:
                self.target_c_code.write_comment_line("< DEVELOPER_TODO: Uncomment if kernel context is needed >")
                self.target_c_code.write_line("#if 0" , files=self.prms_write)
            self.generate_target_c_struct()
            if self.prms_commented_out:
                self.target_c_code.write_line("#endif" , files=self.prms_write)
        self.target_c_code.write_line("static tivx_target_kernel vx_%s_target_kernel = NULL;" % (self.kernel.name_lower))
        self.target_c_code.write_newline()

        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sProcess(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg);")
        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sCreate(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg);")
        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sDelete(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg);")
        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sControl(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg);")

        self.target_c_code.write_newline()
        self.generate_target_c_process_func_code()
        self.generate_target_c_create_func_code()
        self.generate_target_c_delete_func_code()
        self.generate_target_c_control_func_code()
        self.generate_target_c_add_func_code()
        self.generate_target_c_remove_func_code()
        self.target_c_code.close()

    def generate_make_files(self, kernel) :
        if self.env_var == 'CUSTOM_KERNEL_PATH' or self.env_var == 'CUSTOM_APPLICATION_PATH':
            self.concerto_inc_filename = self.workarea + "/concerto_inc.mak"
            if not os.path.exists(self.concerto_inc_filename):
                print("Creating " + self.concerto_inc_filename)
                self.concerto_inc_code = CodeGenerate(self.concerto_inc_filename, header=False)
                self.concerto_inc_code.write_line("# This file contains a list of extension kernel specific static libraries")
                self.concerto_inc_code.write_line("# to be included in the PC executables.  It is put in this separate file")
                self.concerto_inc_code.write_line("# to make it easier to add/extend kernels without needing to modify")
                self.concerto_inc_code.write_line("# several concerto.mak files which depend on kernel libraries.")
                self.concerto_inc_code.write_newline()
                self.concerto_inc_code.write_line("STATIC_LIBS += vx_kernels_" + self.module + "_tests " + "vx_kernels_" + self.module)
                if self.env_var == 'CUSTOM_KERNEL_PATH' :
                    self.concerto_inc_code.write_line("STATIC_LIBS += vx_target_kernels_" + self.core)
                    if self.target_uses_dsp :
                        self.concerto_inc_code.write_line("ifeq ($(BUILD_BAM),yes)")
                        self.concerto_inc_code.write_line("STATIC_LIBS += vx_target_kernels_" + self.core + "_bam")
                        self.concerto_inc_code.write_line("endif")
                else:
                    self.concerto_inc_code.write_line("STATIC_LIBS += vx_target_kernels_" + self.module + "_" + self.core)
                    if self.target_uses_dsp :
                        self.concerto_inc_code.write_line("ifeq ($(BUILD_BAM),yes)")
                        self.concerto_inc_code.write_line("STATIC_LIBS += vx_target_kernels_" + self.module + "_" + self.core + "_bam")
                        self.concerto_inc_code.write_line("endif")
                self.concerto_inc_code.write_line("STATIC_LIBS += vx_conformance_engine")
                self.concerto_inc_code.write_line("# < DEVELOPER_TODO: Add any additional dependent libraries >")
                self.concerto_inc_code.close()

        if self.env_var == 'CUSTOM_KERNEL_PATH' or self.env_var == 'CUSTOM_APPLICATION_PATH':
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

        self.module_host_concerto_filename = self.workarea_module_host + "/concerto.mak"
        if not os.path.exists(self.module_host_concerto_filename):
            print("Creating " + self.module_host_concerto_filename)
            self.module_host_concerto_code = CodeGenerate(self.module_host_concerto_filename, header=False)
            self.module_host_concerto_code.write_newline()
            self.module_host_concerto_code.write_line("ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72 R5F))")
            self.module_host_concerto_code.write_newline()
            self.module_host_concerto_code.write_line("include $(PRELUDE)")
            self.module_host_concerto_code.write_line("TARGET      := vx_kernels_" + self.module)
            self.module_host_concerto_code.write_line("TARGETTYPE  := library")
            self.module_host_concerto_code.write_line("CSOURCES    := $(call all-c-files)")
            if self.env_var == 'CUSTOM_KERNEL_PATH' :
                self.module_host_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/include")
                self.module_host_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/" + self.module + "/include")
            else:
                self.module_host_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/kernels/" + self.module + "/include")
            self.module_host_concerto_code.write_newline()
            self.module_host_concerto_code.write_line("include $(FINALE)")
            self.module_host_concerto_code.write_newline()
            self.module_host_concerto_code.write_line("endif")
            self.module_host_concerto_code.close()

        self.module_target_concerto_filename = self.workarea_module_core + "/concerto.mak"
        if not os.path.exists(self.module_target_concerto_filename):
            print("Creating " + self.module_target_concerto_filename)
            self.module_target_concerto_code = CodeGenerate(self.module_target_concerto_filename, header=False)
            dspAdded = False
            eveAdded = False
            armAdded = False
            ipuAdded = False
            c7xAdded = False
            targetCpuListString = "X86 x86_64 "
            for tar in kernel.targets :
                if (tar == Target.DSP1 or tar == Target.DSP2) and (dspAdded == False) :
                    targetCpuListString+="C66 "
                    dspAdded = True
                if (tar == Target.EVE1 or tar == Target.EVE2 or tar == Target.EVE3 or tar == Target.EVE4) and (eveAdded == False) :
                    targetCpuListString+="EVE "
                    eveAdded = True
                if (tar == Target.A15_0) and (armAdded == False) :
                    targetCpuListString+="A15 A72 "
                    armAdded = True
                if (tar == Target.MCU2_0 or tar == Target.MCU2_1 or tar == Target.IPU2) and (ipuAdded == False) :
                    targetCpuListString+="M4 R5F "
                    ipuAdded = True
                if (tar == Target.DSP_C7_1) and (c7xAdded == False) :
                    targetCpuListString+="C71 "
                    c7xAdded = True
            self.module_target_concerto_code.write_newline()
            self.module_target_concerto_code.write_line("ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), " + targetCpuListString + "))")
            self.module_target_concerto_code.write_newline()
            self.module_target_concerto_code.write_line("include $(PRELUDE)")
            if self.env_var == 'CUSTOM_KERNEL_PATH' :
                self.module_target_concerto_code.write_line("TARGET      := vx_target_kernels_" + self.core)
            else:
                self.module_target_concerto_code.write_line("TARGET      := vx_target_kernels_" + self.module + "_" + self.core);
            self.module_target_concerto_code.write_line("TARGETTYPE  := library")
            self.module_target_concerto_code.write_line("CSOURCES    := $(call all-c-files)")
            if self.env_var == 'CUSTOM_KERNEL_PATH' :
                self.module_target_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/include")
                self.module_target_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/" + self.module + "/include")
            else:
                self.module_target_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/kernels/" + self.module + "/include")
                self.module_target_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/kernels/" + self.module + "/host")
            if self.env_var != "VISION_APPS_PATH" :
                self.module_target_concerto_code.write_line("IDIRS       += $(HOST_ROOT)/kernels/include")
            else :
                self.module_target_concerto_code.write_line("IDIRS       += $(TIOVX_PATH)/kernels/include")
            self.module_target_concerto_code.write_line("IDIRS       += $(VXLIB_PATH)/packages")
            self.module_target_concerto_code.write_line("# < DEVELOPER_TODO: Add any custom include paths using 'IDIRS' >")
            self.module_target_concerto_code.write_line("# < DEVELOPER_TODO: Add any custom preprocessor defines or build options needed using")
            self.module_target_concerto_code.write_line("#                   'CFLAGS'. >")
            self.module_target_concerto_code.write_line("# < DEVELOPER_TODO: Adjust which cores this library gets built on using 'SKIPBUILD'. >")
            self.module_target_concerto_code.write_newline()
            self.module_target_concerto_code.write_line("ifeq ($(TARGET_CPU),C66)")
            self.module_target_concerto_code.write_line("DEFS += CORE_DSP")
            self.module_target_concerto_code.write_line("endif")
            self.module_target_concerto_code.write_newline();
            self.module_target_concerto_code.write_line("ifeq ($(BUILD_BAM),yes)")
            self.module_target_concerto_code.write_line("DEFS += BUILD_BAM")
            self.module_target_concerto_code.write_line("endif")
            self.module_target_concerto_code.write_newline();
            self.module_target_concerto_code.write_line("ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))")
            self.module_target_concerto_code.write_line("DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION")
            self.module_target_concerto_code.write_line("endif")
            self.module_target_concerto_code.write_newline()
            self.module_target_concerto_code.write_line("include $(FINALE)")
            self.module_target_concerto_code.write_line("endif")
            self.module_target_concerto_code.close()

        if self.target_uses_dsp :
            self.module_target_bam_concerto_filename = self.workarea_module_core_bam + "/concerto.mak"
            if not os.path.exists(self.module_target_bam_concerto_filename):
                print("Creating " + self.module_target_bam_concerto_filename)
                self.module_target_bam_concerto_code = CodeGenerate(self.module_target_bam_concerto_filename, header=False)
                self.module_target_bam_concerto_code.write_line("ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C66))")
                self.module_target_bam_concerto_code.write_line("include $(PRELUDE)")
                if self.env_var == 'CUSTOM_KERNEL_PATH' :
                    self.module_target_bam_concerto_code.write_line("TARGET      := vx_target_kernels_" + self.core + "_bam")
                else:
                    self.module_target_bam_concerto_code.write_line("TARGET      := vx_target_kernels_" + self.module + "_" + self.core + "_bam");
                self.module_target_bam_concerto_code.write_line("TARGETTYPE  := library")
                self.module_target_bam_concerto_code.write_line("CSOURCES    := $(call all-c-files)")
                if self.env_var == 'CUSTOM_KERNEL_PATH' :
                    self.module_target_bam_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/include")
                    self.module_target_bam_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/" + self.module + "/include")
                else:
                    self.module_target_bam_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/kernels/" + self.module + "/include")
                    self.module_target_bam_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/kernels/" + self.module + "/host")
                if self.env_var != "VISION_APPS_PATH" :
                    self.module_target_bam_concerto_code.write_line("IDIRS       += $(HOST_ROOT)/kernels/include")
                else :
                    self.module_target_bam_concerto_code.write_line("IDIRS       += $(TIOVX_PATH)/kernels/include")
                self.module_target_bam_concerto_code.write_line("IDIRS       += $(ALGFRAMEWORK_PATH)/inc")
                self.module_target_bam_concerto_code.write_line("IDIRS       += $(ALGFRAMEWORK_PATH)/src/bam_dma_nodes")
                self.module_target_bam_concerto_code.write_line("IDIRS       += $(DMAUTILS_PATH)/inc")
                self.module_target_bam_concerto_code.write_line("IDIRS       += $(DMAUTILS_PATH)")
                self.module_target_bam_concerto_code.write_line("IDIRS       += $(DMAUTILS_PATH)/inc/edma_utils")
                self.module_target_bam_concerto_code.write_line("IDIRS       += $(DMAUTILS_PATH)/inc/edma_csl")
                self.module_target_bam_concerto_code.write_line("IDIRS       += $(DMAUTILS_PATH)/inc/baseaddress/vayu/dsp")
                self.module_target_bam_concerto_code.write_line("IDIRS       += $(VXLIB_PATH)/packages")
                self.module_target_bam_concerto_code.write_line("# < DEVELOPER_TODO: Add any custom include paths using 'IDIRS' >")
                self.module_target_bam_concerto_code.write_line("# < DEVELOPER_TODO: Add any custom preprocessor defines or build options needed using")
                self.module_target_bam_concerto_code.write_line("#                   'CFLAGS'. >")
                self.module_target_bam_concerto_code.write_line("# < DEVELOPER_TODO: Adjust which cores this library gets built on using 'SKIPBUILD'. >")
                self.module_target_bam_concerto_code.write_newline();
                self.module_target_bam_concerto_code.write_line("DEFS += CORE_DSP")
                self.module_target_bam_concerto_code.write_newline();
                self.module_target_bam_concerto_code.write_line("ifeq ($(BUILD_BAM),yes)")
                self.module_target_bam_concerto_code.write_line("DEFS += BUILD_BAM")
                self.module_target_bam_concerto_code.write_line("endif")
                self.module_target_bam_concerto_code.write_newline();
                self.module_target_bam_concerto_code.write_line("ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))")
                self.module_target_bam_concerto_code.write_line("DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION")
                self.module_target_bam_concerto_code.write_line("endif")
                self.module_target_bam_concerto_code.write_newline();
                self.module_target_bam_concerto_code.write_line("ifeq ($(BUILD_BAM),yes)")
                self.module_target_bam_concerto_code.write_line("SKIPBUILD=0")
                self.module_target_bam_concerto_code.write_line("else")
                self.module_target_bam_concerto_code.write_line("SKIPBUILD=1")
                self.module_target_bam_concerto_code.write_line("endif")
                self.module_target_bam_concerto_code.write_newline()
                self.module_target_bam_concerto_code.write_line("include $(FINALE)")
                self.module_target_bam_concerto_code.write_newline()
                self.module_target_bam_concerto_code.write_line("endif")
                self.module_target_bam_concerto_code.close()

        self.module_test_concerto_filename = self.workarea_module_test + "/concerto.mak"
        if not os.path.exists(self.module_test_concerto_filename):
            print("Creating " + self.module_test_concerto_filename)
            self.module_test_concerto_code = CodeGenerate(self.module_test_concerto_filename, header=False)
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72 R5F))")
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("include $(PRELUDE)")
            self.module_test_concerto_code.write_line("TARGET      := vx_kernels_" + self.module + "_tests")
            self.module_test_concerto_code.write_line("TARGETTYPE  := library")
            self.module_test_concerto_code.write_line("CSOURCES    := $(call all-c-files)")
            if self.env_var != "VISION_APPS_PATH" :
                self.module_test_concerto_code.write_line("IDIRS       += $(HOST_ROOT)/conformance_tests")
                self.module_test_concerto_code.write_line("IDIRS       += $(HOST_ROOT)/source/include")
            else :
                self.module_test_concerto_code.write_line("IDIRS       += $(TIOVX_PATH)/include")
                self.module_test_concerto_code.write_line("IDIRS       += $(TIOVX_PATH)/conformance_tests")
            if self.env_var == 'CUSTOM_KERNEL_PATH' :
                self.module_test_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/include")
                self.module_test_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/" + self.module + "/include")
            else:
                self.module_test_concerto_code.write_line("IDIRS       += "+self.idirs_path+"/kernels/" + self.module + "/include")
            if self.env_var == "CUSTOM_KERNEL_PATH" :
                self.module_test_concerto_code.write_line("IDIRS       += $(CUSTOM_KERNEL_PATH)")
            if self.env_var == "CUSTOM_APPLICATION_PATH" :
                self.module_test_concerto_code.write_line("IDIRS       += $(CUSTOM_APPLICATION_PATH)")
            self.module_test_concerto_code.write_line("CFLAGS      += -DHAVE_VERSION_INC")
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("ifeq ($(HOST_COMPILER),TIARMCGT)")
            self.module_test_concerto_code.write_line("CFLAGS += --display_error_number")
            self.module_test_concerto_code.write_line("CFLAGS += --diag_suppress=179")
            self.module_test_concerto_code.write_line("CFLAGS += --diag_suppress=112")
            self.module_test_concerto_code.write_line("CFLAGS += --diag_suppress=552")
            self.module_test_concerto_code.write_line("endif")
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_WINDOWS GCC_LINUX GCC_SYSBIOS_ARM GCC_LINUX_ARM))")
            self.module_test_concerto_code.write_line("CFLAGS += -Wno-unused-function")
            self.module_test_concerto_code.write_line("CFLAGS += -Wno-unused-variable")
            self.module_test_concerto_code.write_line("CFLAGS += -Wno-format-security")
            self.module_test_concerto_code.write_line("endif")
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("ifeq ($(TARGET_CPU),x86_64)")
            self.module_test_concerto_code.write_line("CFLAGS      += -DTARGET_X86_64")
            self.module_test_concerto_code.write_line("endif")
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("include $(FINALE)")
            self.module_test_concerto_code.write_line("endif")
            self.module_test_concerto_code.close()

    def generate_headers(self) :
        if( self.env_var == 'CUSTOM_KERNEL_PATH' ) :
            self.include_custom_kernel_library_tests_filename = self.workarea_include + "/custom_kernel_library_tests.h"
        else :
            self.include_custom_kernel_library_tests_filename = self.workarea + "/kernels" + "/custom_app_kernel_library_tests.h"

        if not os.path.exists(self.include_custom_kernel_library_tests_filename):
            print("Creating " + self.include_custom_kernel_library_tests_filename)
            self.include_custom_kernel_library_tests_code = CodeGenerate(self.include_custom_kernel_library_tests_filename)
            self.include_custom_kernel_library_tests_code.write_line("#include \"" + self.module + "/test/test_main.h\"")
            self.include_custom_kernel_library_tests_code.close()

        self.module_test_main_filename = self.workarea_module_test + "/test_main.h"
        if not os.path.exists(self.module_test_main_filename):
            print("Creating " + self.module_test_main_filename)
            self.module_test_main_code = CodeGenerate(self.module_test_main_filename)
            self.module_test_main_code.write_line("#if 0")
            self.module_test_main_code.write_line("TESTCASE(tivx" + toCamelCase(self.module) + self.kernel.name_camel + ")")
            self.module_test_main_code.write_line("#endif")
            self.module_test_main_code.close()

        self.include_customer_header_filename = self.workarea_include_company + "/" + self.top_header_name + ".h"
        if not os.path.exists(self.include_customer_header_filename):
            print("Creating " + self.include_customer_header_filename)
            self.include_customer_header_code = CodeGenerate(self.include_customer_header_filename)
            self.include_customer_header_code.write_ifndef_define(self.top_header_name.upper() + "_H_")
            self.include_customer_header_code.write_line("#include <TI/tivx.h>")
            self.include_customer_header_code.write_line("#include <" + self.company + "/" + self.top_header_name +
                                                          "_kernels.h>")
            self.include_customer_header_code.write_line("#include <" + self.company + "/" + self.top_header_name +
                                                          "_nodes.h>")
            self.include_customer_header_code.write_newline()
            self.include_customer_header_code.write_endif(self.top_header_name.upper() + "_H_")
            self.include_customer_header_code.close()

        self.include_customer_kernels_filename = self.workarea_include_company + "/" + self.top_header_name + "_kernels.h"
        if not os.path.exists(self.include_customer_kernels_filename):
            print("Creating " + self.include_customer_kernels_filename)
            self.include_customer_kernels_code = CodeGenerate(self.include_customer_kernels_filename)
            self.include_customer_kernels_code.write_ifndef_define(self.top_header_name.upper() + "_KERNELS_H_")
            self.include_customer_kernels_code.write_line("#include <VX/vx.h>")
            self.include_customer_kernels_code.write_line("#include <VX/vx_kernels.h>")
            self.include_customer_kernels_code.write_newline()
            self.include_customer_kernels_code.write_extern_c_top()
            self.include_customer_kernels_code.write_line("/*!")
            self.include_customer_kernels_code.write_line(" * \\file")
            self.include_customer_kernels_code.write_line(" * \\brief The list of supported kernels in this kernel extension.")
            self.include_customer_kernels_code.write_line(" */")
            self.include_customer_kernels_code.write_newline()
            self.include_customer_kernels_code.write_line("/*! \\brief Name for OpenVX Extension kernel module: " + self.module)
            self.include_customer_kernels_code.write_line(" * \\ingroup group_tivx_ext")
            self.include_customer_kernels_code.write_line(" */")
            self.include_customer_kernels_code.write_line("#define TIVX_MODULE_NAME_" + self.module.upper() + "    \"" + self.module + "\"")
            self.include_customer_kernels_code.write_newline()
            self.include_customer_kernels_code.write_line("/*! \\brief The list of kernels supported in " + self.module + " module")
            self.include_customer_kernels_code.write_line(" *")
            self.include_customer_kernels_code.write_line(" * Each kernel listed here can be used with the <tt>\\ref vxGetKernelByName</tt> call.")
            self.include_customer_kernels_code.write_line(" * When programming the parameters, use")
            self.include_customer_kernels_code.write_line(" * \\arg <tt>\\ref VX_INPUT</tt> for [in]")
            self.include_customer_kernels_code.write_line(" * \\arg <tt>\\ref VX_OUTPUT</tt> for [out]")
            self.include_customer_kernels_code.write_line(" * \\arg <tt>\\ref VX_BIDIRECTIONAL</tt> for [in,out]")
            self.include_customer_kernels_code.write_line(" *")
            self.include_customer_kernels_code.write_line(" * When programming the parameters, use")
            self.include_customer_kernels_code.write_line(" * \\arg <tt>\\ref VX_TYPE_IMAGE</tt> for a <tt>\\ref vx_image</tt> in the size field of <tt>\\ref vxGetParameterByIndex</tt> or <tt>\\ref vxSetParameterByIndex</tt>")
            self.include_customer_kernels_code.write_line(" * \\arg <tt>\\ref VX_TYPE_ARRAY</tt> for a <tt>\\ref vx_array</tt> in the size field of <tt>\\ref vxGetParameterByIndex</tt> or <tt>\\ref vxSetParameterByIndex</tt>")
            self.include_customer_kernels_code.write_line(" * \\arg or other appropriate types in \\ref vx_type_e.")
            self.include_customer_kernels_code.write_line(" * \\ingroup group_kernel")
            self.include_customer_kernels_code.write_line(" */")
            self.include_customer_kernels_code.write_newline()
            self.include_customer_kernels_code.write_line("/*! \\brief " + self.kernel.name_lower + " kernel name")
            self.include_customer_kernels_code.write_line(" *  \\see group_vision_function_" + self.module)
            self.include_customer_kernels_code.write_line(" */")
            self.include_customer_kernels_code.write_line("#define " + self.kernel.enum_str_prefix + self.kernel.name_upper + "_NAME     \"%s%s.%s\"" % (self.kernel.name_str_prefix, self.module.lower(), self.kernel.name_lower))
            self.include_customer_kernels_code.write_newline()
            self.include_customer_kernels_code.write_line("/*! End of group_vision_function_" + self.module + " */")
            self.include_customer_kernels_code.write_newline()
            self.include_customer_kernels_code.write_line("/*!")
            self.include_customer_kernels_code.write_line(" * \\brief Used for the Application to load the " + self.module + " kernels into the context.")
            self.include_customer_kernels_code.write_line(" * \\ingroup group_kernel")
            self.include_customer_kernels_code.write_line(" */")
            self.include_customer_kernels_code.write_line("void tivx" + toCamelCase(self.module) + "LoadKernels(vx_context context);")
            self.include_customer_kernels_code.write_newline()
            self.include_customer_kernels_code.write_line("/*!")
            self.include_customer_kernels_code.write_line(" * \\brief Used for the Application to unload the " + self.module + " kernels from the context.")
            self.include_customer_kernels_code.write_line(" * \\ingroup group_kernel")
            self.include_customer_kernels_code.write_line(" */")
            self.include_customer_kernels_code.write_line("void tivx" + toCamelCase(self.module) + "UnLoadKernels(vx_context context);")
            self.include_customer_kernels_code.write_newline()
            self.include_customer_kernels_code.write_line("/*!")
            self.include_customer_kernels_code.write_line(" * \\brief Used to print the performance of the kernels.")
            self.include_customer_kernels_code.write_line(" * \\ingroup group_kernel")
            self.include_customer_kernels_code.write_line(" */")
            self.include_customer_kernels_code.write_line("void tivx" + toCamelCase(self.module) + "PrintPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName);")
            self.include_customer_kernels_code.write_newline()
            self.include_customer_kernels_code.write_extern_c_bottom()
            self.include_customer_kernels_code.write_endif(self.top_header_name.upper() + "_KERNELS_H_")
            self.include_customer_kernels_code.close()

        self.include_customer_nodes_filename = self.workarea_include_company + "/" + self.top_header_name + "_nodes.h"
        if not os.path.exists(self.include_customer_nodes_filename):
            print("Creating " + self.include_customer_nodes_filename)
            self.include_customer_nodes_code = CodeGenerate(self.include_customer_nodes_filename)
            self.include_customer_nodes_code.write_ifndef_define(self.top_header_name.upper() + "_NODES_H_")
            self.include_customer_nodes_code.write_line("#include <VX/vx.h>")
            self.include_customer_nodes_code.write_newline()
            self.include_customer_nodes_code.write_extern_c_top()
            self.include_customer_nodes_code.write_line("/*! \\brief [Graph] Creates a " + self.kernel.name_upper + " Node.")
            self.include_customer_nodes_code.write_line(" * \\param [in] graph The reference to the graph.")
            for prm in self.kernel.params :
                if(prm.state == ParamState.OPTIONAL) :
                    self.paramstate = " (optional)"
                else :
                    self.paramstate = ""
                self.include_customer_nodes_code.write_line(" * \param [" + prm.direction.get_doxygen_name() + "] " + prm.name_lower + self.paramstate)
            self.include_customer_nodes_code.write_line(" * \\see <tt>" + self.kernel.enum_str_prefix + self.kernel.name_upper + "_NAME</tt>")
            self.include_customer_nodes_code.write_line(" * \\ingroup group_vision_function_" + self.kernel.name_lower)
            self.include_customer_nodes_code.write_line(" * \\return <tt>\\ref vx_node</tt>.")
            self.include_customer_nodes_code.write_line(" * \\retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\\ref vxGetStatus</tt>")
            self.include_customer_nodes_code.write_line(" */")
            self.include_customer_nodes_code.write_line("VX_API_ENTRY vx_node VX_API_CALL tivx" + self.kernel.name_camel + "Node(vx_graph graph,")
            for prm in self.kernel.params[:-1] :
                if Type.is_scalar_type(prm.type) :
                    self.include_customer_nodes_code.write_line("%-37s %-20s %s," % ("", "vx_scalar", prm.name_lower))
                else :
                    self.include_customer_nodes_code.write_line("%-37s %-20s %s," % ("", prm.type.get_vx_name(), prm.name_lower))
            self.include_customer_nodes_code.write_line("%-37s %-20s %s);" % ("", self.kernel.params[-1].type.get_vx_name(), self.kernel.params[-1].name_lower))
            self.include_customer_nodes_code.write_newline()
            self.include_customer_nodes_code.write_extern_c_bottom()
            self.include_customer_nodes_code.write_endif(self.top_header_name.upper() + "_NODES_H_")
            self.include_customer_nodes_code.close()

        self.module_include_kernels_filename = self.workarea_module_include + "/tivx_" + self.module + "_kernels" + self.kernels_header_extension + ".h"
        if not os.path.exists(self.module_include_kernels_filename):
            print("Creating " + self.module_include_kernels_filename)
            self.module_include_kernels_code = CodeGenerate(self.module_include_kernels_filename)
            self.module_include_kernels_code.write_ifndef_define("VX_" + self.module.upper() + "_KERNELS" + self.kernels_header_extension.upper() + "_H_")
            self.module_include_kernels_code.write_line("#include \"tivx_kernels_host_utils.h\"")
            self.module_include_kernels_code.write_newline()
            self.module_include_kernels_code.write_extern_c_top()
            self.module_include_kernels_code.write_line("/*!")
            self.module_include_kernels_code.write_line(" * \\file")
            self.module_include_kernels_code.write_line(" * \\brief Interface file for the " + self.module.upper() + " kernels")
            self.module_include_kernels_code.write_line(" */")
            self.module_include_kernels_code.write_newline()
            self.module_include_kernels_code.write_newline()
            self.module_include_kernels_code.write_line("/*!")
            self.module_include_kernels_code.write_line(" * \\brief Function to register " + self.module.upper() + " Kernels on the Host")
            self.module_include_kernels_code.write_line(" * \\ingroup group_tivx_ext")
            self.module_include_kernels_code.write_line(" */")
            self.module_include_kernels_code.write_line("void tivxRegister" + toCamelCase(self.module) + "Kernels(void);")
            self.module_include_kernels_code.write_newline()
            self.module_include_kernels_code.write_line("/*!")
            self.module_include_kernels_code.write_line(" * \\brief Function to un-register " + self.module.upper() + " Kernels on the Host")
            self.module_include_kernels_code.write_line(" * \\ingroup group_tivx_ext")
            self.module_include_kernels_code.write_line(" */")
            self.module_include_kernels_code.write_line("void tivxUnRegister" + toCamelCase(self.module) + "Kernels(void);")
            self.module_include_kernels_code.write_newline()
            self.module_include_kernels_code.write_line("/*!")
            self.module_include_kernels_code.write_line(" * \\brief Function to register " + self.module.upper() + " Kernels on the " + self.core + " Target")
            self.module_include_kernels_code.write_line(" * \\ingroup group_tivx_ext")
            self.module_include_kernels_code.write_line(" */")
            self.module_include_kernels_code.write_line("void tivxRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels(void);")
            self.module_include_kernels_code.write_newline()
            self.module_include_kernels_code.write_line("/*!")
            self.module_include_kernels_code.write_line(" * \\brief Function to un-register " + self.module.upper() + " Kernels on the " + self.core + " Target")
            self.module_include_kernels_code.write_line(" * \\ingroup group_tivx_ext")
            self.module_include_kernels_code.write_line(" */")
            self.module_include_kernels_code.write_line("void tivxUnRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels(void);")
            self.module_include_kernels_code.write_newline()
            self.module_include_kernels_code.write_newline()
            self.module_include_kernels_code.write_extern_c_bottom()
            self.module_include_kernels_code.write_endif("VX_" + self.module.upper() + "_KERNELS" + self.kernels_header_extension.upper() + "_H_")
            self.module_include_kernels_code.close()

    def generate_sources(self) :
        self.host_node_api_filename = self.workarea_module_host + "/tivx_" + self.module + "_node_api.c"
        if not os.path.exists(self.host_node_api_filename):
            print("Creating " + self.host_node_api_filename)
            self.host_node_api_code = CodeGenerate(self.host_node_api_filename)
            self.host_node_api_code.write_line("#include <TI/tivx.h>")
            self.host_node_api_code.write_line("#include <" + self.company + "/" + self.top_header_name + ".h>")
            self.host_node_api_code.write_newline()
            self.host_node_api_code.write_line("VX_API_ENTRY vx_node VX_API_CALL tivx" + self.kernel.name_camel + "Node(vx_graph graph,")
            for prm in self.kernel.params[:-1] :
                if Type.is_scalar_type(prm.type) :
                    self.host_node_api_code.write_line("%-37s %-20s %s," % ("", "vx_scalar", prm.name_lower))
                else :
                    self.host_node_api_code.write_line("%-37s %-20s %s," % ("", prm.type.get_vx_name(), prm.name_lower))
            self.host_node_api_code.write_line("%-37s %-20s %s)" % ("", self.kernel.params[-1].type.get_vx_name(), self.kernel.params[-1].name_lower))
            self.host_node_api_code.write_open_brace()
            self.host_node_api_code.write_line("vx_reference prms[] = {")
            for prm in self.kernel.params[:-1] :
                self.host_node_api_code.write_line("%-7s (vx_reference)%s," % ("", prm.name_lower))
            self.host_node_api_code.write_line("%-7s (vx_reference)%s" % ("", self.kernel.params[-1].name_lower))
            self.host_node_api_code.write_line("};")
            self.host_node_api_code.write_line("vx_node node = tivxCreateNodeByKernelName(graph,")
            self.host_node_api_code.write_line("%-38s %s_NAME," % ("", self.kernel.enum_str_prefix + self.kernel.name_upper))
            self.host_node_api_code.write_line("%-38s prms," % (""))
            self.host_node_api_code.write_line("%-38s dimof(prms));" % (""))
            self.host_node_api_code.write_line("return node;")
            self.host_node_api_code.write_close_brace()
            self.host_node_api_code.close()

        self.host_kernels_filename = self.workarea_module_host + "/vx_kernels_" + self.module.lower() + "_host.c"
        if not os.path.exists(self.host_kernels_filename):
            print("Creating " + self.host_kernels_filename)
            self.host_kernels_code = CodeGenerate(self.host_kernels_filename)
            self.host_kernels_code.write_line("#include <TI/tivx.h>")
            self.host_kernels_code.write_line("#include <" + self.company + "/" + self.top_header_name + ".h>")
            self.host_kernels_code.write_line("#include \"tivx_" + self.module.lower() + "_kernels" + self.kernels_header_extension + ".h\"")
            self.host_kernels_code.write_line("#include \"tivx_kernels_host_utils.h\"")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("static vx_status VX_CALLBACK publishKernels(vx_context context);")
            self.host_kernels_code.write_line("static vx_status VX_CALLBACK unPublishKernels(vx_context context);")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("static uint32_t gIs" + toCamelCase(self.module) + "KernelsLoad = 0u;")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("vx_status tivxAddKernel" + self.kernel.name_camel + "(vx_context context);")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("vx_status tivxRemoveKernel" + self.kernel.name_camel + "(vx_context context);")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("static Tivx_Host_Kernel_List  gTivx_host_kernel_list[] = {")
            self.host_kernels_code.write_line("    {&tivxAddKernel" + self.kernel.name_camel + ", &tivxRemoveKernel" + self.kernel.name_camel + "},")
            self.host_kernels_code.write_line("};")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("static vx_status VX_CALLBACK publishKernels(vx_context context)")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("return tivxPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));")
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("static vx_status VX_CALLBACK unPublishKernels(vx_context context)")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("return tivxUnPublishKernels(context, gTivx_host_kernel_list, dimof(gTivx_host_kernel_list));")
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("void tivxRegister" + toCamelCase(self.module) + "Kernels(void)")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("tivxRegisterModule(TIVX_MODULE_NAME_" + self.module.upper() + ", publishKernels, unPublishKernels);")
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("void tivxUnRegister" + toCamelCase(self.module) + "Kernels(void)")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("tivxUnRegisterModule(TIVX_MODULE_NAME_" + self.module.upper() + ");")
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("void tivx" + toCamelCase(self.module) + "LoadKernels(vx_context context)")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("if ((0 == gIs" + toCamelCase(self.module) + "KernelsLoad) && (NULL != context))")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("tivxRegister" + toCamelCase(self.module) + "Kernels();")
            self.host_kernels_code.write_line("vxLoadKernels(context, TIVX_MODULE_NAME_" + self.module.upper() + ");")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("/* These three lines only work on PC emulation mode ...")
            self.host_kernels_code.write_line(" * this will need to be updated when moving to target */")
            self.host_kernels_code.write_line("#ifdef x86_64")
            self.host_kernels_code.write_line("void tivxSetSelfCpuId(vx_enum cpu_id);")
            self.host_kernels_code.write_newline()
            for target in self.kernel.targets :
                self.host_kernels_code.write_line("tivxSetSelfCpuId(%s);" % Cpu.get_vx_enum_name(Target.get_cpu(target)))
                self.host_kernels_code.write_line("tivxRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels();")
            self.host_kernels_code.write_line("#endif")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_line("gIs" + toCamelCase(self.module) + "KernelsLoad++;")
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("void tivx" + toCamelCase(self.module) + "UnLoadKernels(vx_context context)")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("if (gIs" + toCamelCase(self.module) + "KernelsLoad > 0)")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("gIs" + toCamelCase(self.module) + "KernelsLoad--;")
            self.host_kernels_code.write_line("if ((0u == gIs" + toCamelCase(self.module) + "KernelsLoad) && (NULL != context))")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("vxUnloadKernels(context, TIVX_MODULE_NAME_" + self.module.upper() + ");")
            self.host_kernels_code.write_line("tivxUnRegister" + toCamelCase(self.module) + "Kernels();")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("/* This line only work on PC emulation mode ...")
            self.host_kernels_code.write_line(" * this will need to be updated when moving to target */")
            self.host_kernels_code.write_line("#ifdef x86_64")
            self.host_kernels_code.write_line("tivxUnRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels();")
            self.host_kernels_code.write_line("#endif")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.close()

        self.target_kernels_filename = self.workarea_module_core + "/vx_kernels_" + self.module.lower() + "_target.c"
        if not os.path.exists(self.target_kernels_filename):
            self.target_kernels_created = True
            print("Creating " + self.target_kernels_filename)
            self.target_kernels_code = CodeGenerate(self.target_kernels_filename)
            self.target_kernels_code.write_line("#include <TI/tivx.h>")
            self.target_kernels_code.write_line("#include <TI/tivx_target_kernel.h>")
            self.target_kernels_code.write_line("#include \"tivx_" + self.module.lower() + "_kernels" + self.kernels_header_extension + ".h\"")
            self.target_kernels_code.write_line("#include \"tivx_kernels_target_utils.h\"")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("#ifdef BUILD_BAM")
            self.target_kernels_code.write_newline()
            if self.target_uses_dsp :
                self.target_kernels_code.write_line("void tivxAddTargetKernelBam" + self.kernel.name_camel + "(void);")
            else :
                self.target_kernels_code.write_line("void tivxAddTargetKernel" + self.kernel.name_camel + "(void);")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("#else")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("void tivxAddTargetKernel" + self.kernel.name_camel + "(void);")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("#endif")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("#ifdef BUILD_BAM")
            self.target_kernels_code.write_newline()
            if self.target_uses_dsp :
                self.target_kernels_code.write_line("void tivxRemoveTargetKernelBam" + self.kernel.name_camel + "(void);")
            else :
                self.target_kernels_code.write_line("void tivxRemoveTargetKernel" + self.kernel.name_camel + "(void);")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("#else")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("void tivxRemoveTargetKernel" + self.kernel.name_camel + "(void);")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("#endif")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("static Tivx_Target_Kernel_List  gTivx_target_kernel_list[] = {")
            self.target_kernels_code.write_line("#ifdef BUILD_BAM")
            self.target_kernels_code.write_newline()
            if self.target_uses_dsp :
                self.target_kernels_code.write_line("    {&tivxAddTargetKernelBam" + self.kernel.name_camel + ", &tivxRemoveTargetKernelBam" + self.kernel.name_camel + "},")
            else :
                self.target_kernels_code.write_line("    {&tivxAddTargetKernel" + self.kernel.name_camel + ", &tivxRemoveTargetKernel" + self.kernel.name_camel + "},")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("#else")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("    {&tivxAddTargetKernel" + self.kernel.name_camel + ", &tivxRemoveTargetKernel" + self.kernel.name_camel + "},")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("#endif")
            self.target_kernels_code.write_line("};")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("void tivxRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels(void)")
            self.target_kernels_code.write_open_brace()
            self.target_kernels_code.write_line("tivxRegisterTargetKernels(gTivx_target_kernel_list, dimof(gTivx_target_kernel_list));")
            self.target_kernels_code.write_close_brace()
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("void tivxUnRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels(void)")
            self.target_kernels_code.write_open_brace()
            self.target_kernels_code.write_line("tivxUnRegisterTargetKernels(gTivx_target_kernel_list, dimof(gTivx_target_kernel_list));")
            self.target_kernels_code.write_close_brace()
            self.target_kernels_code.close()
        else :
            self.target_kernels_created = False

    def modify_files(self) :
        self.modify_make_file()
        self.modify_kernel_header_file()
        self.modify_node_header_file()
        self.modify_node_api_source_file()
        self.modify_module_host_header_file()
        self.modify_module_host_source_file()
        self.modify_module_target_source_file()
        self.modify_test_header()

    def modify_test_header(self) :
        print("Modifying " + self.module_test_main_filename)
        CodeModify().block_insert(self.module_test_main_filename,
                      "#if ",
                      "#endif",
                      "TESTCASE(tivx" + toCamelCase(self.module) + self.kernel.name_camel + ")",
                      "#endif",
                      "#endif",
                      "TESTCASE(tivx" + toCamelCase(self.module) + self.kernel.name_camel + ")\n")

    def modify_make_file(self) :
        if self.env_var == 'CUSTOM_KERNEL_PATH' :
            print("Modifying " + self.concerto_inc_filename)
            CodeModify().block_insert(self.concerto_inc_filename,
                          "vx_kernels_" + self.module,
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_target_kernels_" + self.core + "\n",
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_target_kernels_" + self.core + "\n")
            if self.target_uses_dsp :
                CodeModify().block_insert(self.concerto_inc_filename,
                          "vx_kernels_" + self.module,
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_target_kernels_" + self.core + "_bam\n",
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_target_kernels_" + self.core + "_bam\n")
        elif self.env_var == 'CUSTOM_APPLICATION_PATH' :
            print("Modifying " + self.concerto_inc_filename)
            CodeModify().block_insert(self.concerto_inc_filename,
                          "vx_kernels_" + self.module,
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_target_kernels_" + self.module + "_" + self.core + "\n",
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_target_kernels_" + self.module + "_" + self.core + "\n")
            if self.target_uses_dsp :
                CodeModify().block_insert(self.concerto_inc_filename,
                          "vx_kernels_" + self.module,
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_target_kernels_" + self.module + "_" + self.core + "_bam\n",
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_target_kernels_" + self.module + "_" + self.core + "_bam\n")


    def modify_kernel_header_file(self) :
        print("Modifying " + self.include_customer_kernels_filename)
        # Update for new modules
        self.insert = (r"! \\brief Name for OpenVX Extension kernel module: " + self.module + "\n" +
                        " * \\ingroup group_tivx_ext\n" +
                        " */\n" +
                        "#define TIVX_MODULE_NAME_" + self.module.upper() + "    \"" + self.module + "\"\n\n")
        CodeModify().block_insert(self.include_customer_kernels_filename,
                          "extern \"C\" {",
                          "vxGetKernelByName",
                          "#define TIVX_MODULE_NAME_" + self.module.upper() + "    \"" + self.module + "\"",
                          r"/*! \\brief The list of kernels supported",
                          r"/*! \\brief The list of kernels supported",
                          self.insert)

        self.insert = (
            r" \\brief The list of kernels supported in " + self.module + " module" + "\n" +
            " *" + "\n" +
            r" * Each kernel listed here can be used with the <tt>\\ref vxGetKernelByEnum</tt> call." + "\n" +
            " * When programming the parameters, use" + "\n" +
            r" * \\arg <tt>\\ref VX_INPUT</tt> for [in]" + "\n" +
            r" * \\arg <tt>\\ref VX_OUTPUT</tt> for [out]" + "\n" +
            r" * \\arg <tt>\\ref VX_BIDIRECTIONAL</tt> for [in,out]" + "\n" +
            " *" + "\n" +
            " * When programming the parameters, use" + "\n" +
            r" * \\arg <tt>\\ref VX_TYPE_IMAGE</tt> for a <tt>\\ref vx_image</tt> in the size field of <tt>\\ref vxGetParameterByIndex</tt> or <tt>\\ref vxSetParameterByIndex</tt>" + "\n" +
            r" * \\arg <tt>\\ref VX_TYPE_ARRAY</tt> for a <tt>\\ref vx_array</tt> in the size field of <tt>\\ref vxGetParameterByIndex</tt> or <tt>\\ref vxSetParameterByIndex</tt>" + "\n" +
            r" * \\arg or other appropriate types in \\ref vx_type_e." + "\n" +
            " * \\ingroup group_kernel" + "\n" +
            " */" + "\n" +
            r"/*! \\brief " + self.kernel.name_lower + " kernel name" + "\n" +
            " * \\see group_vision_function_" + self.module + "\n" +
            "*/" + "\n" +
            "#define " + self.kernel.enum_str_prefix + self.kernel.name_upper + "_NAME \"" + self.kernel.name_str_prefix + self.module.lower() + "." + self.kernel.name_lower + "\"\n\n" +
            "/*! End of group_vision_function_" + self.module + " */\n\n"
            "/*! \n")
        CodeModify().block_insert(self.include_customer_kernels_filename,
                          "The list of kernels supported in",
                          " Used for the Application to load the",
                          "The list of kernels supported in " + self.module + " module",
                          r" * \\brief Used for the Application to load the",
                          r" * \\brief Used for the Application to load the",
                          self.insert)

        self.insert = (
            "/*!" + "\n" +
            r" * \\brief Used for the Application to load the " + self.module + " kernels into the context." + "\n" +
            " * \\ingroup group_kernel" + "\n" +
            " */" + "\n" +
            "void tivx" + toCamelCase(self.module) + "LoadKernels(vx_context context);" + "\n" + "\n" +
            "/*!" + "\n" +
            r" * \\brief Used for the Application to unload the " + self.module + " kernels from the context." + "\n" +
            " * \\ingroup group_kernel" + "\n" +
            " */" + "\n" +
            "void tivx" + toCamelCase(self.module) + "UnLoadKernels(vx_context context);" + "\n" + "\n" +
            "/*!" + "\n" +
            r" * \\brief Used to print the performance of the kernels." + "\n" +
            " * \\ingroup group_kernel" + "\n" +
            " */" + "\n" +
            "void tivx" + toCamelCase(self.module) + "PrintPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName);" + "\n" + "\n"
            )
        CodeModify().block_insert(self.include_customer_kernels_filename,
                          "LoadKernels",
                          "#ifdef __cplusplus",
                          "void tivx" + toCamelCase(self.module) + "LoadKernels",
                          "#ifdef __cplusplus",
                          "#ifdef __cplusplus",
                          self.insert)

        # Update for new kernels
        self.insert = (r"/*! \\brief " + self.kernel.name_lower + " kernel name\n" +
                        " *  \\see group_vision_function_" + self.module + "\n" +
                        " */\n" +
                        "#define " + self.kernel.enum_str_prefix + self.kernel.name_upper + "_NAME     \"" + self.kernel.name_str_prefix + self.module.lower() + "." + self.kernel.name_lower + "\"\n\n")
        CodeModify().block_insert(self.include_customer_kernels_filename,
                        "The list of kernels supported in " + self.module + " module",
                        " End of group_vision_function_" + self.module,
                        " " +self.kernel.enum_str_prefix + self.kernel.name_upper + "_NAME",
                        r"\/\*\! End of group_vision_function_" + self.module + " \*\/",
                        r"/*! End of group_vision_function_" + self.module + " */",
                        self.insert)

    def modify_node_header_file(self) :
        print("Modifying " + self.include_customer_nodes_filename)
        self.insert = (r"/*! \\brief [Graph] Creates a " + self.kernel.name_upper + " Node.\n")
        self.insert += (r" * \\param [in] graph The reference to the graph.\n")
        for prm in self.kernel.params :
            if(prm.state == ParamState.OPTIONAL) :
                self.paramstate = " (optional)"
            else :
                self.paramstate = ""
            self.insert += (" * \param [" + prm.direction.get_doxygen_name() + "] " + prm.name_lower + self.paramstate + "\n")
        self.insert += (r" * \\see <tt>" + self.kernel.enum_str_prefix + self.kernel.name_upper + "_NAME</tt>" + "\n")
        self.insert += (r" * \\ingroup group_vision_function_" + self.kernel.name_lower + "\n")
        self.insert += (r" * \\return <tt>\\ref vx_node</tt>.\n")
        self.insert += (r" * \\retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\\ref vxGetStatus</tt>\n")
        self.insert += (r" */\n")
        self.insert += ("VX_API_ENTRY vx_node VX_API_CALL tivx" + self.kernel.name_camel + "Node(vx_graph graph,\n")
        for prm in self.kernel.params[:-1] :
            if Type.is_scalar_type(prm.type) :
                self.insert += ("%-37s %-20s %s,\n" % ("", "vx_scalar", prm.name_lower))
            else :
                self.insert += ("%-37s %-20s %s,\n" % ("", prm.type.get_vx_name(), prm.name_lower))
        if Type.is_scalar_type(self.kernel.params[-1].type) :
            self.insert += ("%-37s %-20s %s);\n" % ("", "vx_scalar", self.kernel.params[-1].name_lower))
        else :
            self.insert += ("%-37s %-20s %s);\n" % ("", self.kernel.params[-1].type.get_vx_name(), self.kernel.params[-1].name_lower))
        self.insert += ("\n")

        CodeModify().block_insert(self.include_customer_nodes_filename,
                          "VX_API_ENTRY",
                          "#ifdef __cplusplus",
                          " tivx" + self.kernel.name_camel + "Node(vx_graph graph,",
                          "#ifdef __cplusplus",
                          "#ifdef __cplusplus",
                          self.insert)

    def modify_node_api_source_file(self) :
        print("Modifying " + self.host_node_api_filename)
        if not CodeModify().file_search(self.host_node_api_filename, " tivx" + self.kernel.name_camel + "Node(vx_graph graph,") :
            self.insert = ("VX_API_ENTRY vx_node VX_API_CALL tivx" + self.kernel.name_camel + "Node(vx_graph graph,\n")
            for prm in self.kernel.params[:-1] :
                if Type.is_scalar_type(prm.type) :
                    self.insert += ("%-37s %-20s %s,\n" % ("", "vx_scalar", prm.name_lower))
                else :
                    self.insert += ("%-37s %-20s %s,\n" % ("", prm.type.get_vx_name(), prm.name_lower))
            if Type.is_scalar_type(self.kernel.params[-1].type) :
                self.insert += ("%-37s %-20s %s)\n" % ("", "vx_scalar", self.kernel.params[-1].name_lower))
            else :
                self.insert += ("%-37s %-20s %s)\n" % ("", self.kernel.params[-1].type.get_vx_name(), self.kernel.params[-1].name_lower))
            self.insert += ("{\n")
            self.insert += ("    vx_reference prms[] = {\n")
            for prm in self.kernel.params[:-1] :
                self.insert += ("%-11s (vx_reference)%s,\n" % ("", prm.name_lower))
            self.insert += ("%-11s (vx_reference)%s\n" % ("", self.kernel.params[-1].name_lower))
            self.insert += ("    };\n")
            self.insert += ("    vx_node node = tivxCreateNodeByKernelName(graph,\n")
            self.insert += ("%-42s %s_NAME,\n" % ("", self.kernel.enum_str_prefix + self.kernel.name_upper))
            self.insert += ("%-42s prms,\n" % (""))
            self.insert += ("%-42s dimof(prms));\n" % (""))
            self.insert += ("    return node;\n}\n\n")

            CodeModify().file_append(self.host_node_api_filename, self.insert)

    def modify_module_host_header_file(self) :
        print("Modifying " + self.module_include_kernels_filename)

        self.insert = (r"/*!\n")
        self.insert += (r" * \\brief Function to register " + self.module.upper() + " Kernels on the " + self.core + " Target\n")
        self.insert += (r" * \\ingroup group_tivx_ext\n")
        self.insert += (r" */\n")
        self.insert += (r"void tivxRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels(void);\n\n")
        self.insert += (r"/*!\n")
        self.insert += (r" * \\brief Function to un-register " + self.module.upper() + " Kernels on the " + self.core + " Target\n")
        self.insert += (r" * \\ingroup group_tivx_ext\n")
        self.insert += (r" */\n")
        self.insert += (r"void tivxUnRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels(void);\n\n")
        CodeModify().block_insert(self.module_include_kernels_filename,
                          "Interface file for the " + self.module.upper() + " kernels",
                          "#ifdef __cplusplus",
                          "void tivxRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels(void);",
                          "\n#ifdef __cplusplus",
                          "\n#ifdef __cplusplus",
                          self.insert)

    def modify_module_host_source_file(self) :
        print("Modifying " + self.host_kernels_filename)
        CodeModify().block_insert(self.host_kernels_filename,
                          "vx_status tivxAddKernel",
                          "vx_status tivxRemoveKernel",
                          "vx_status tivxAddKernel" + self.kernel.name_camel + "(vx_context context);",
                          "\nvx_status tivxRemoveKernel",
                          "\nvx_status tivxRemoveKernel",
                          "vx_status tivxAddKernel" + self.kernel.name_camel + "(vx_context context);\n")

        CodeModify().block_insert(self.host_kernels_filename,
                          "vx_status tivxRemoveKernel",
                          "Tivx_Host_Kernel_List",
                          "vx_status tivxRemoveKernel" + self.kernel.name_camel + "(vx_context context);",
                          "\nstatic Tivx_Host_Kernel_List",
                          "\nstatic Tivx_Host_Kernel_List",
                          "vx_status tivxRemoveKernel" + self.kernel.name_camel + "(vx_context context);\n")

        CodeModify().block_insert(self.host_kernels_filename,
                          "Tivx_Host_Kernel_List",
                          "};",
                          "    {&tivxAddKernel" + self.kernel.name_camel + ", &tivxRemoveKernel" + self.kernel.name_camel + "}",
                          "};",
                          "};",
                          "    {&tivxAddKernel" + self.kernel.name_camel + ", &tivxRemoveKernel" + self.kernel.name_camel + "},\n")

        CodeModify().block_insert(self.host_kernels_filename,
                          "tivx" + toCamelCase(self.module) + "LoadKernels",
                          "}",
                          "        tivxRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels();",
                          "        tivxSetSelfCpuId\(TIVX_CPU_ID_DSP1\);",
                          "        tivxSetSelfCpuId(TIVX_CPU_ID_DSP1);",
                          "        tivxRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels();\n")

        CodeModify().block_insert(self.host_kernels_filename,
                          "tivx" + toCamelCase(self.module) + "UnLoadKernels",
                          "}",
                          "        tivxUnRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels();",
                          "\n        gIs" + toCamelCase(self.module) + "KernelsLoad",
                          "\n        gIs" + toCamelCase(self.module) + "KernelsLoad",
                          "        tivxUnRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels();\n")

    def modify_module_target_source_file(self) :
        print("Modifying " + self.target_kernels_filename)

        if self.target_uses_dsp :
            CodeModify().block_insert(self.target_kernels_filename,
                          "void tivxAddTargetKernel",
                          "#ifdef BUILD_BAM",
                          "void tivxAddTargetKernelBam" + self.kernel.name_camel + "(void);",
                          "\n#else\n\n",
                          "\n#else\n\n",
                          "void tivxAddTargetKernelBam" + self.kernel.name_camel + "(void);\n")
        else :
            CodeModify().block_insert(self.target_kernels_filename,
                          "void tivxAddTargetKernel",
                          "#ifdef BUILD_BAM",
                          "void tivxAddTargetKernel" + self.kernel.name_camel + "(void);",
                          "\n#else\n\n",
                          "\n#else\n\n",
                          "void tivxAddTargetKernel" + self.kernel.name_camel + "(void);\n")

        CodeModify().block_insert(self.target_kernels_filename,
                          "void tivxAddTargetKernel",
                          "#ifdef BUILD_BAM",
                          "void tivxAddTargetKernel" + self.kernel.name_camel + "(void);",
                          "\n#endif",
                          "\n#endif",
                          "void tivxAddTargetKernel" + self.kernel.name_camel + "(void);\n", overrideFind=(not self.target_kernels_created))

        if self.target_uses_dsp :
            CodeModify().block_insert(self.target_kernels_filename,
                          "void tivxRemoveTargetKernel",
                          "static Tivx_Target_Kernel_List",
                          "void tivxRemoveTargetKernelBam" + self.kernel.name_camel + "(void);",
                          "\n#else\n\n",
                          "\n#else\n\n",
                          "void tivxRemoveTargetKernelBam" + self.kernel.name_camel + "(void);\n")
        else :
            CodeModify().block_insert(self.target_kernels_filename,
                          "void tivxRemoveTargetKernel",
                          "static Tivx_Target_Kernel_List",
                          "void tivxRemoveTargetKernel" + self.kernel.name_camel + "(void);",
                          "\n#else\n\n",
                          "\n#else\n\n",
                          "void tivxRemoveTargetKernel" + self.kernel.name_camel + "(void);\n")

        CodeModify().block_insert(self.target_kernels_filename,
                          "void tivxRemoveTargetKernel",
                          "#endif",
                          "void tivxRemoveTargetKernel" + self.kernel.name_camel + "(void);",
                          "\n#endif",
                          "\n#endif",
                          "void tivxRemoveTargetKernel" + self.kernel.name_camel + "(void);\n", overrideFind=(not self.target_kernels_created))

        if self.target_uses_dsp :
            CodeModify().block_insert(self.target_kernels_filename,
                              "Tivx_Target_Kernel_List",
                              "#else",
                              "    {&tivxAddTargetKernelBam" + self.kernel.name_camel + ", &tivxRemoveTargetKernelBam" + self.kernel.name_camel + "},",
                              "\n#else",
                              "\n#else",
                              "    {&tivxAddTargetKernelBam" + self.kernel.name_camel + ", &tivxRemoveTargetKernelBam" + self.kernel.name_camel + "},\n")
        else :
            CodeModify().block_insert(self.target_kernels_filename,
                              "Tivx_Target_Kernel_List",
                              "#else",
                              "    {&tivxAddTargetKernel" + self.kernel.name_camel + ", &tivxRemoveTargetKernel" + self.kernel.name_camel + "},",
                              "\n#else",
                              "\n#else",
                              "    {&tivxAddTargetKernel" + self.kernel.name_camel + ", &tivxRemoveTargetKernel" + self.kernel.name_camel + "},\n")

        CodeModify().block_insert(self.target_kernels_filename,
                          "Tivx_Target_Kernel_List",
                          "#endif",
                          "    {&tivxAddTargetKernel" + self.kernel.name_camel + ", &tivxRemoveTargetKernel" + self.kernel.name_camel + "},",
                          "\n#endif",
                          "\n#endif",
                          "    {&tivxAddTargetKernel" + self.kernel.name_camel + ", &tivxRemoveTargetKernel" + self.kernel.name_camel + "},\n", overrideFind=(not self.target_kernels_created))

    def todo(self) :
        if self.env_var == 'CUSTOM_KERNEL_PATH' or self.env_var == 'CUSTOM_APPLICATION_PATH':
            self.todo_filename = self.workarea + "/DEVELOPER_TODO.txt"
        else :
            self.todo_filename = self.workarea_module + "/DEVELOPER_TODO.txt"
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
        if self.env_var == 'CUSTOM_KERNEL_PATH' or self.env_var == 'CUSTOM_APPLICATION_PATH':
            self.all_files = [y for x in os.walk(self.workarea) for y in glob(os.path.join(x[0], '*.*'))]
        else :
            self.all_files = [y for x in os.walk(self.workarea_module) for y in glob(os.path.join(x[0], '*.*'))]
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


    ## Method for generating files from kernel
    #
    # \param kernel  [in] Kernel to be exported to a file
    def export(self, kernel) :
        self.kernel = kernel
        self.h_filename = "tivx_kernel_" + kernel.name_lower + ".h";
        self.host_c_filename = "vx_" + kernel.name_lower + "_host.c";
        self.target_c_filename = "vx_" + kernel.name_lower + "_target.c";
        # Disabling BAM generation while this is not supported
        #self.bam_target_c_filename = "vx_bam_" + kernel.name_lower + "_target.c";

        self.target_uses_dsp = False
        # Disabling BAM generation while this is not supported
        '''
        for target in self.kernel.targets :
            if target == Target.DSP1 or target == Target.DSP2 :
                self.target_uses_dsp = True
        '''

        self.prms_write = 2
        if self.kernel.localMem == True :
            self.prms_write = 2
        elif self.target_uses_dsp and self.kernel.localMem == False :
            self.prms_write = 1

        # TIOVX-815: Setting the kernel instance context regardless of mem being used
        self.prms_needed = True

        # TIOVX-815: Adding an #if 0 so the mem alloc doesn't fail on target
        self.prms_commented_out = True

        if self.target_uses_dsp or self.kernel.localMem == True :
            self.prms_commented_out = False

        print ('Generating C code for OpenVX kernel ...')
        print ()
        print ('Creating new directories ...')
        self.create_all_directories()
        print ('Creating new makefiles ...')
        self.generate_make_files(kernel)
        print ('Creating new headers ...')
        self.generate_headers()
        print ('Creating new module-level sources ...')
        self.generate_sources()

        self.modify_files()

        print ('Creating new kernel-specific files ...')
        self.generate_h_file_code()
        self.generate_host_c_file_code()
        self.generate_target_c_file_code()

        print ()
        print (self.kernel)
        print ('Generating C code for OpenVX kernel ... DONE !!!')
        self.todo()

