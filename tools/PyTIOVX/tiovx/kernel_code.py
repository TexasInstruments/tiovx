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

class KernelExportCode :
    def __init__(self, include_subpath="TI", include_filename="extvx", module="ext1", core="c66", env_var='CUSTOM_KERNEL_PATH') :
        self.company = include_subpath
        self.top_header_name = include_filename
        self.module = module
        self.core = core
        self.env_var = env_var

        self.workarea = os.environ.get(self.env_var)

        if self.workarea == None :
            sys.exit("ERROR: You must define %s environment variable as the root of the kernel workarea." % self.env_var);

        if self.env_var == 'CUSTOM_KERNEL_PATH':
            self.kernels_header_extension = "";
        else:
            self.kernels_header_extension = "_priv";


    def setCompanyDirectory(self, company) :
        self.company = company

    def setTopHeaderName(self, header) :
        self.top_header_name = header

    def setModuleDirectory(self, module) :
        self.module = module

    def setCoreDirectory(self, core) :
        self.core = core

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
        self.host_c_code.write_newline()
        self.host_c_code.write_line("kernel = vxAddUserKernel(");
        self.host_c_code.write_line("            context,")
        self.host_c_code.write_line("            \"%s%s.%s\"," % (self.kernel.name_str_prefix, self.module.lower(), self.kernel.name_lower))
        self.host_c_code.write_line("            %s%s," % (self.kernel.enum_str_prefix, self.kernel.name_upper))
        self.host_c_code.write_line("            NULL,")
        self.host_c_code.write_line("            %s%s_MAX_PARAMS," % (self.kernel.enum_str_prefix, self.kernel.name_upper) )
        self.host_c_code.write_line("            tivxAddKernel%sValidate," % (self.kernel.name_camel) )
        self.host_c_code.write_line("            tivxAddKernel%sInitialize," % (self.kernel.name_camel) )
        self.host_c_code.write_line("            NULL);")
        self.host_c_code.write_newline()
        self.host_c_code.write_line("status = vxGetStatus((vx_reference)kernel);")
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
            self.host_c_code.write_line("            %s," % (Direction.get_vx_enum_name(prm.direction)) )
            self.host_c_code.write_line("            %s," % (Type.get_vx_enum_name(prm.type)) )
            self.host_c_code.write_line("            %s" % (ParamState.get_vx_enum_name(prm.state)) )
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

        self.host_c_code.write_line("if (status != VX_SUCCESS)")
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

    def generate_host_c_validate_func_code(self):
        self.host_c_code.write_line("static vx_status VX_CALLBACK tivxAddKernel%sValidate(vx_node node," % self.kernel.name_camel)
        self.host_c_code.write_line("            const vx_reference parameters[ ],")
        self.host_c_code.write_line("            vx_uint32 num,")
        self.host_c_code.write_line("            vx_meta_format metas[])")
        self.host_c_code.write_open_brace()

        # Initial parameters
        self.host_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.num_params = 0
        for prm in self.kernel.params :
            self.host_c_code.write_newline()
            if not Type.is_scalar_type(prm.type) :
                self.host_c_code.write_line("%s %s = NULL;" % (Type.get_vx_name(prm.type), prm.name_lower))
            if prm.type == Type.IMAGE :
                self.host_c_code.write_line("vx_df_image %s_fmt;" % prm.name_lower)
                self.host_c_code.write_line("vx_uint32 %s_w, %s_h;" % (prm.name_lower, prm.name_lower))
            elif prm.type == Type.ARRAY :
                self.host_c_code.write_line("vx_enum %s_item_type;" % prm.name_lower)
                self.host_c_code.write_line("vx_size %s_capacity, %s_item_size;" % (prm.name_lower, prm.name_lower))
            elif prm.type == Type.PYRAMID :
                self.host_c_code.write_line("vx_df_image %s_fmt;" % prm.name_lower)
                self.host_c_code.write_line("vx_size %s_levels;" % prm.name_lower)
                self.host_c_code.write_line("vx_float32 %s_scale;" % prm.name_lower)
                self.host_c_code.write_line("vx_uint32 %s_w, %s_h;" % (prm.name_lower, prm.name_lower))
            elif prm.type == Type.MATRIX :
                self.host_c_code.write_line("vx_enum %s_type;" % prm.name_lower)
                self.host_c_code.write_line("vx_size %s_w, %s_h;" % (prm.name_lower, prm.name_lower))
            elif prm.type == Type.CONVOLUTION :
                self.host_c_code.write_line("vx_size %s_col, %s_row;" % (prm.name_lower, prm.name_lower))
            elif prm.type == Type.DISTRIBUTION :
                self.host_c_code.write_line("vx_int32 %s_offset = 0;" % prm.name_lower)
                self.host_c_code.write_line("vx_uint32 %s_range = 0;" % prm.name_lower)
                self.host_c_code.write_line("vx_size %s_numBins = 0;" % prm.name_lower)
            elif prm.type == Type.LUT :
                self.host_c_code.write_line("vx_enum %s_type;" % prm.name_lower)
                self.host_c_code.write_line("vx_size %s_count;" % prm.name_lower)
            elif prm.type == Type.THRESHOLD:
                self.host_c_code.write_line("vx_enum %s_threshold_type, %s_threshold_data_type;" % (prm.name_lower, prm.name_lower))
            elif prm.type == Type.OBJECT_ARRAY:
                self.host_c_code.write_line("vx_enum %s_type;" % prm.name_lower)
                self.host_c_code.write_line("vx_size %s_num_items;" % prm.name_lower)
            elif prm.type == Type.REMAP :
                self.host_c_code.write_line("vx_uint32 %s_src_w, %s_src_h;" % (prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("vx_uint32 %s_dst_w, %s_dst_h;" % (prm.name_lower, prm.name_lower))
            elif Type.is_scalar_type(prm.type) :
                self.host_c_code.write_line("vx_scalar %s = NULL;" % prm.name_lower)
                self.host_c_code.write_line("vx_enum %s_scalar_type;" % prm.name_lower)
            self.num_params += 1

        self.host_c_code.write_newline()
        self.host_c_code.write_line("if ( (num != %s%s_MAX_PARAMS)" % (self.kernel.enum_str_prefix, self.kernel.name_upper) )
        for prm in self.kernel.params :
            if prm.state is ParamState.REQUIRED :
                self.host_c_code.write_line("    || (NULL == parameters[%s%s_%s_IDX])" % (self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper))
        self.host_c_code.write_line(")")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("status = VX_ERROR_INVALID_PARAMETERS;")
        self.host_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"One or more REQUIRED parameters are set to NULL\\n\");")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

        # Query all types here
        self.host_c_code.write_line("if (VX_SUCCESS == status)")
        self.host_c_code.write_open_brace()
        # find code from target for here
        # assigned descriptors to local variables
        for prm in self.kernel.params :
            if Type.is_scalar_type(prm.type) :
                self.host_c_code.write_line("%s = (const %s)parameters[%s%s_%s_IDX];" %
                    (prm.name_lower, Type.get_vx_name(Type.SCALAR), self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
            else :
                self.host_c_code.write_line("%s = (const %s)parameters[%s%s_%s_IDX];" %
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
        self.host_c_code.write_line("if (VX_SUCCESS == status)")
        self.host_c_code.write_open_brace()
        for prm in self.kernel.params :
            if prm.state is ParamState.OPTIONAL :
                self.host_c_code.write_line("if (NULL != %s)" % prm.name_lower)
                self.host_c_code.write_open_brace()

            # TODO:
            # Now that we have attributes defined, these commented out lines *could* replace the if..else list below, however this would fetch ALL of the attributes, which we don't yet do.
            #    also, we would need to do something similar for the variable declaration section above, and the parameter checking section below.
            #
            #attr = Attribute.from_type(prm.type)
            #if Type.is_scalar_type(prm.type) :
            #    self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryScalar(%s, VX_SCALAR_TYPE, &%s_scalar_type, sizeof(%s_scalar_type)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            #else :
            #    for name, member in attr.__members__.items() :
            #        self.host_c_code.write_line("tivxCheckStatus(&status, vxQuery%s(%s, VX_%s_%s, &%s_%s, sizeof(%s_%s)));" % (toCamelCase(prm.type.name), prm.name_lower, prm.type.name, name, prm.name_lower, member.value, prm.name_lower, member.value))

            if Type.is_scalar_type(prm.type) :
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryScalar(%s, VX_SCALAR_TYPE, &%s_scalar_type, sizeof(%s_scalar_type)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            elif Type.IMAGE == prm.type :
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryImage(%s, VX_IMAGE_FORMAT, &%s_fmt, sizeof(%s_fmt)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryImage(%s, VX_IMAGE_WIDTH, &%s_w, sizeof(%s_w)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryImage(%s, VX_IMAGE_HEIGHT, &%s_h, sizeof(%s_h)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            elif Type.ARRAY == prm.type :
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryArray(%s, VX_ARRAY_ITEMTYPE, &%s_item_type, sizeof(%s_item_type)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryArray(%s, VX_ARRAY_CAPACITY, &%s_capacity, sizeof(%s_capacity)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryArray(%s, VX_ARRAY_ITEMSIZE, &%s_item_size, sizeof(%s_item_size)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            elif Type.MATRIX == prm.type :
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryMatrix(%s, VX_MATRIX_TYPE, &%s_type, sizeof(%s_type)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryMatrix(%s, VX_MATRIX_COLUMNS, &%s_w, sizeof(%s_w)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryMatrix(%s, VX_MATRIX_ROWS, &%s_h, sizeof(%s_h)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            elif Type.PYRAMID == prm.type :
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryPyramid(%s, VX_PYRAMID_FORMAT, &%s_fmt, sizeof(%s_fmt)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryPyramid(%s, VX_PYRAMID_LEVELS, &%s_levels, sizeof(%s_levels)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryPyramid(%s, VX_PYRAMID_SCALE, &%s_scale, sizeof(%s_scale)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryPyramid(%s, VX_PYRAMID_WIDTH, &%s_w, sizeof(%s_w)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryPyramid(%s, VX_PYRAMID_HEIGHT, &%s_h, sizeof(%s_h)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            elif Type.CONVOLUTION == prm.type :
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryConvolution(%s, VX_CONVOLUTION_COLUMNS, &%s_col, sizeof(%s_col)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryConvolution(%s, VX_CONVOLUTION_ROWS, &%s_row, sizeof(%s_row)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            elif Type.DISTRIBUTION == prm.type :
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryDistribution(%s, VX_DISTRIBUTION_BINS, &%s_numBins, sizeof(%s_numBins)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryDistribution(%s, VX_DISTRIBUTION_RANGE, &%s_range, sizeof(%s_range)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryDistribution(%s, VX_DISTRIBUTION_OFFSET, &%s_offset, sizeof(%s_offset)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            elif Type.LUT == prm.type:
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryLUT(%s, VX_LUT_TYPE, &%s_type, sizeof(%s_type)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryLUT(%s, VX_LUT_COUNT, &%s_count, sizeof(%s_count)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            elif Type.THRESHOLD == prm.type:
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryThreshold(%s, VX_THRESHOLD_TYPE, &%s_threshold_type, sizeof(%s_threshold_type)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryThreshold(%s, VX_THRESHOLD_DATA_TYPE, &%s_threshold_data_type, sizeof(%s_threshold_data_type)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            elif Type.OBJECT_ARRAY == prm.type:
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryObjectArray(%s, VX_OBJECT_ARRAY_ITEMTYPE, &%s_type, sizeof(%s_type)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryObjectArray(%s, VX_OBJECT_ARRAY_NUMITEMS, &%s_num_items, sizeof(%s_num_items)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            elif Type.REMAP == prm.type:
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryRemap(%s, VX_REMAP_SOURCE_WIDTH, &%s_src_w, sizeof(%s_src_w)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryRemap(%s, VX_REMAP_SOURCE_HEIGHT, &%s_src_h, sizeof(%s_src_h)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryRemap(%s, VX_REMAP_DESTINATION_WIDTH, &%s_dst_w, sizeof(%s_dst_w)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
                self.host_c_code.write_line("tivxCheckStatus(&status, vxQueryRemap(%s, VX_REMAP_DESTINATION_HEIGHT, &%s_dst_h, sizeof(%s_dst_h)));" % (prm.name_lower, prm.name_lower, prm.name_lower))
            if prm.state is ParamState.OPTIONAL :
                self.host_c_code.write_close_brace()
            if prm is not self.kernel.params[-1] :
                self.host_c_code.write_newline()
        self.host_c_code.write_close_brace()

        self.host_c_code.write_newline()
        self.host_c_code.write_line("/* PARAMETER CHECKING */")
        self.host_c_code.write_newline()

        # Check for sizeof array, and data type (format) of other objects
        self.host_c_code.write_line("if (VX_SUCCESS == status)")
        self.host_c_code.write_open_brace()
        for prm in self.kernel.params :
            if Type.IMAGE == prm.type or Type.PYRAMID == prm.type or Type.ARRAY == prm.type or Type.MATRIX == prm.type or Type.LUT == prm.type or Type.is_scalar_type(prm.type) is True :
                if prm.state is ParamState.OPTIONAL :
                    self.host_c_code.write_line("if (NULL != %s)" % (prm.name_lower))
                    self.host_c_code.write_open_brace()
                if len(prm.data_types) == 0 :
                    self.print_data_type = ['<Add type here>']
                else :
                    self.print_data_type = prm.data_types
                if Type.IMAGE == prm.type :
                    if len(prm.data_types) > 1 :
                        self.host_c_code.write_line("if( (%s != %s_fmt) &&" % (self.print_data_type[0], prm.name_lower))
                        for dt in self.print_data_type[1:-1] :
                            self.host_c_code.write_line("    (%s != %s_fmt) &&" % (dt, prm.name_lower))
                        self.host_c_code.write_line("    (%s != %s_fmt))" % (self.print_data_type[-1], prm.name_lower))
                    else :
                        self.host_c_code.write_line("if (%s != %s_fmt)" % (self.print_data_type[0], prm.name_lower))
                elif Type.PYRAMID == prm.type :
                    if len(prm.data_types) > 1 :
                        self.host_c_code.write_line("if( (%s != %s_fmt) &&" % (self.print_data_type[0], prm.name_lower))
                        for dt in self.print_data_type[1:-1] :
                            self.host_c_code.write_line("    (%s != %s_fmt) &&" % (dt, prm.name_lower))
                        self.host_c_code.write_line("    (%s != %s_fmt))" % (self.print_data_type[-1], prm.name_lower))
                    else :
                        self.host_c_code.write_line("if (%s != %s_fmt)" % (self.print_data_type[0], prm.name_lower))
                elif Type.ARRAY == prm.type :
                    if len(prm.data_types) > 1 :
                        self.host_c_code.write_line("if( (%s_item_size != sizeof(%s)) &&" % (prm.name_lower, self.print_data_type[0]))
                        for dt in self.print_data_type[1:-1] :
                            self.host_c_code.write_line("    (%s_item_size != sizeof(%s)) &&" % (prm.name_lower, dt))
                        self.host_c_code.write_line("    (%s_item_size != sizeof(%s)))" % (prm.name_lower, self.print_data_type[-1]))
                    else :
                        self.host_c_code.write_line("if ( %s_item_size != sizeof(%s))" % (prm.name_lower, self.print_data_type[0]))
                elif Type.MATRIX == prm.type or Type.LUT == prm.type:
                    if len(prm.data_types) > 1 :
                        self.host_c_code.write_line("if( (%s != %s_type) &&" % (self.print_data_type[0], prm.name_lower))
                        for dt in self.print_data_type[1:-1] :
                            self.host_c_code.write_line("    (%s != %s_type) &&" % (dt, prm.name_lower))
                        self.host_c_code.write_line("    (%s != %s_type))" % (self.print_data_type[-1], prm.name_lower))
                    else :
                        self.host_c_code.write_line("if (%s != %s_type)" % (self.print_data_type[0], prm.name_lower))
                elif Type.is_scalar_type(prm.type) :
                    if len(prm.data_types) > 1 :
                        self.host_c_code.write_line("if( (%s != %s_scalar_type) &&" % (self.print_data_type[0], prm.name_lower))
                        for dt in self.print_data_type[1:-1] :
                            self.host_c_code.write_line("    (%s != %s_scalar_type) &&" % (dt, prm.name_lower))
                        self.host_c_code.write_line("    (%s != %s_scalar_type))" % (self.print_data_type[-1], prm.name_lower))
                    else :
                        self.host_c_code.write_line("if (%s != %s_scalar_type)" % (self.print_data_type[0], prm.name_lower))
                self.host_c_code.write_open_brace()
                self.host_c_code.write_line("status = VX_ERROR_INVALID_PARAMETERS;")
                vowel = ["a","e","i","o","u"]
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
            self.host_c_code.write_line("if (VX_SUCCESS == status)")
            self.host_c_code.write_open_brace()

            for rel in self.kernel.relationship_list :
                if rel.state is ParamState.OPTIONAL :
                    self.host_c_code.write_line("if (NULL != %s)" % (rel.prm_list[0].name_lower))
                    self.host_c_code.write_open_brace()
                for attr in rel.attribute_list :
                    if len(rel.prm_list) > 2 :
                        self.host_c_code.write_line("if( (%s_%s != %s_%s) ||" % (rel.prm_list[0].name_lower, attr.value, rel.prm_list[1].name_lower, attr.value))
                        for prm in rel.prm_list[2:-1] :
                            self.host_c_code.write_line("    (%s_%s != %s_%s) ||" % (rel.prm_list[0].name_lower, attr.value, prm.name_lower, attr.value))
                        self.host_c_code.write_line("    (%s_%s != %s_%s))" % (rel.prm_list[0].name_lower, attr.value, rel.prm_list[-1].name_lower, attr.value))
                    elif len(rel.prm_list) == 2 :
                        self.host_c_code.write_line("if (%s_%s != %s_%s)" % (rel.prm_list[0].name_lower, attr.value, rel.prm_list[1].name_lower, attr.value))

                    self.host_c_code.write_open_brace()
                    self.host_c_code.write_line("status = VX_ERROR_INVALID_PARAMETERS;")
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

        self.host_c_code.write_line("return status;")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

    def generate_host_c_initialize_func_code(self):
        self.host_c_code.write_line("static vx_status VX_CALLBACK tivxAddKernel%sInitialize(vx_node node," % self.kernel.name_camel)
        self.host_c_code.write_line("            const vx_reference parameters[ ],")
        self.host_c_code.write_line("            vx_uint32 num_params)")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("vx_status status = VX_SUCCESS;")
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
        self.host_c_code.write_line("status = VX_ERROR_INVALID_PARAMETERS;")
        self.host_c_code.write_line("VX_PRINT(VX_ZONE_ERROR, \"One or more REQUIRED parameters are set to NULL\\n\");")
        self.host_c_code.write_close_brace()

        # Set images
        num_input_image = 0
        num_output_image = 0
        self.temp_buffer = ""
        for prm in self.kernel.params :
            if Type.IMAGE == prm.type:
                if Direction.INPUT == prm.direction:
                    self.temp_buffer += ("        prms.in_img[%sU] = (const vx_image)parameters[%s%s_%s_IDX];\n" %
                        (num_input_image, self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
                    num_input_image+=1
                if Direction.OUTPUT == prm.direction:
                    self.temp_buffer += ("        prms.out_img[%sU] = (const vx_image)parameters[%s%s_%s_IDX];\n" %
                        (num_output_image, self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
                    num_output_image+=1

        # Config valid rectangle
        if num_input_image > 0 or num_output_image > 0 :
            self.host_c_code.write_line("if (VX_SUCCESS == status)")
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
        self.target_c_code.write_line("void tivxAddTargetKernel%s(void)" % self.kernel.name_camel)
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = VX_FAILURE;")
        self.target_c_code.write_line("char target_name[TIVX_TARGET_MAX_NAME];")
        self.target_c_code.write_line("vx_enum self_cpu;")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("self_cpu = tivxGetSelfCpuId();")
        self.target_c_code.write_newline()
        for target in self.kernel.targets :
            cpu = Target.get_cpu(target)
            self.target_c_code.write_line("if ( self_cpu == %s )" % Cpu.get_vx_enum_name(cpu) )
            self.target_c_code.write_open_brace()
            self.target_c_code.write_line("strncpy(target_name, %s, TIVX_TARGET_MAX_NAME);" % Target.get_vx_enum_name(target))
            self.target_c_code.write_line("status = VX_SUCCESS;")
            self.target_c_code.write_close_brace()
            self.target_c_code.write_line("else")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("status = VX_FAILURE;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

        self.target_c_code.write_if_status()
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_%s_target_kernel = tivxAddTargetKernel(" % self.kernel.name_lower)
        self.target_c_code.write_line("                    %s%s," % (self.kernel.enum_str_prefix, self.kernel.name_upper))
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
        self.target_c_code.write_line("void tivxRemoveTargetKernel%s(void)" % self.kernel.name_camel)
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("status = tivxRemoveTargetKernel(vx_%s_target_kernel);" % self.kernel.name_lower)
        self.target_c_code.write_if_status()
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_%s_target_kernel = NULL;" % self.kernel.name_lower)
        self.target_c_code.write_close_brace()
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_create_func_code(self):
        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sCreate(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.target_c_code.write_newline()
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Add any target kernel create code here (e.g. allocating")
        self.target_c_code.write_comment_line("                  local memory buffers, one time initialization, etc) >")
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
        self.target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.target_c_code.write_newline()
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Add any target kernel delete code here (e.g. freeing")
        self.target_c_code.write_comment_line("                  local memory buffers, etc) >")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_control_func_code(self):
        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sControl(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.target_c_code.write_newline()
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Add any target kernel control code here (e.g. commands")
        self.target_c_code.write_comment_line("                  the user can call to modify the processing of the kernel at run-time) >")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_process_func_code(self):
        # define function name, and parameters
        self.target_c_code.write_line("static vx_status VX_CALLBACK tivx%sProcess(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()

        # define status variables and obj descriptor variable
        self.target_c_code.write_line("vx_status status = VX_SUCCESS;")
        need_plane_idx_var = False
        need_pyramid_idx_var = False
        for prm in self.kernel.params :
            if prm.do_map or prm.do_unmap or (Type.is_scalar_type(prm.type) is True):
                if Type.IMAGE == prm.type or Type.PYRAMID == prm.type:
                    if len(prm.data_types) > 1 :
                        for dt in prm.data_types[0:-1] :
                            if DfImage.get_num_planes(DfImage.get_df_enum_from_string(dt)) > 1 :
                                need_plane_idx_var = True
                                break
                    if Type.PYRAMID == prm.type:
                        need_pyramid_idx_var = True
                self.target_c_code.write_line("%s *%s_desc;" % (Type.get_obj_desc_name(prm.type), prm.name_lower) )
                if prm.type == Type.PYRAMID :
                    self.target_c_code.write_line("%s *img_%s_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];" % (Type.get_obj_desc_name(Type.IMAGE), prm.name_lower) )
                #TODO: Object Array is hardcoded to image ... modify for proper type
                if prm.type == Type.OBJECT_ARRAY :
                    self.target_c_code.write_line("%s *img_%s_desc[TIVX_OBJECT_ARRAY_MAX_ITEMS];" % (Type.get_obj_desc_name(Type.IMAGE), prm.name_lower) )
        if need_plane_idx_var is True :
            self.target_c_code.write_line("uint16_t plane_idx;")
        if need_pyramid_idx_var is True :
            self.target_c_code.write_line("uint32_t i;")

        self.target_c_code.write_newline()

        # checks function parameters
        self.target_c_code.write_line("if ( num_params != %s%s_MAX_PARAMS" % (self.kernel.enum_str_prefix, self.kernel.name_upper) )
        for prm in self.kernel.params :
            if prm.state is ParamState.REQUIRED :
                self.target_c_code.write_line("    || (NULL == obj_desc[%s%s_%s_IDX])" % (self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper))
        self.target_c_code.write_line(")")

        self.target_c_code.write_open_brace()

        # function parameters status check failure case
        self.target_c_code.write_line("status = VX_FAILURE;")
        self.target_c_code.write_close_brace()

        self.target_c_code.write_line("else")

        self.target_c_code.write_open_brace()

        # function parameters status check success case

        # define variables to hold scalar values
        for prm in self.kernel.params :
            if Type.is_scalar_type(prm.type) is True :
                self.target_c_code.write_line("%s %s_value;" % (Type.get_vx_name(prm.type), prm.name_lower ))
        self.target_c_code.write_newline()

        # assigned descriptors to local variables
        for prm in self.kernel.params :
            self.target_c_code.write_line("%s_desc = (%s *)obj_desc[%s%s_%s_IDX];" %
                (prm.name_lower, Type.get_obj_desc_name(prm.type), self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
        self.target_c_code.write_newline()

        # convert descriptors pointer to target pointers
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if Type.is_scalar_type(prm.type) is False :
                if prm.do_map or prm.do_unmap :
                    if prm.state is ParamState.OPTIONAL:
                        self.target_c_code.write_line("if( %s != NULL)" % desc)
                        self.target_c_code.write_open_brace()
                    if Type.IMAGE == prm.type or Type.PYRAMID == prm.type:
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
                                self.target_c_code.write_line("%s->mem_ptr[plane_idx].target_ptr = tivxMemShared2TargetPtr(" % desc )
                                self.target_c_code.write_line("  %s->mem_ptr[plane_idx].shared_ptr, %s->mem_ptr[plane_idx].mem_type);" % (desc, desc))
                                if prm.do_map :
                                    self.target_c_code.write_line("tivxMemBufferMap(%s->mem_ptr[plane_idx].target_ptr," % desc )
                                    self.target_c_code.write_line("   %s->mem_size[plane_idx], %s->mem_ptr[plane_idx].mem_type," % (desc, desc))
                                    self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                                self.target_c_code.write_close_brace()
                            else :
                                self.target_c_code.write_line("%s->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(" % desc )
                                self.target_c_code.write_line("  %s->mem_ptr[0].shared_ptr, %s->mem_ptr[0].mem_type);" % (desc, desc))
                                if prm.do_map :
                                    self.target_c_code.write_line("tivxMemBufferMap(%s->mem_ptr[0].target_ptr," % desc )
                                    self.target_c_code.write_line("   %s->mem_size[0], %s->mem_ptr[0].mem_type," % (desc, desc))
                                    self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                        elif prm.type == Type.PYRAMID :
                            self.target_c_code.write_line("tivxGetObjDescList(%s->obj_desc_id, (tivx_obj_desc_t**)img_%s, %s->num_levels);" % (desc, desc, desc) )
                            if self.multiplane :
                                self.target_c_code.write_line("for(i=0; i<%s->num_levels; i++)" % desc )
                                self.target_c_code.write_open_brace()
                                self.target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                                self.target_c_code.write_open_brace()
                                self.target_c_code.write_line("img_%s[i]->mem_ptr[plane_idx].target_ptr = tivxMemShared2TargetPtr(" % desc )
                                self.target_c_code.write_line("  img_%s[i]->mem_ptr[plane_idx].shared_ptr, img_%s[i]->mem_ptr[plane_idx].mem_type);" % (desc, desc))
                                if prm.do_map :
                                    self.target_c_code.write_line("tivxMemBufferMap(img_%s[i]->mem_ptr[plane_idx].target_ptr," % desc )
                                    self.target_c_code.write_line("   img_%s[i]->mem_size[plane_idx], img_%s[i]->mem_ptr[plane_idx].mem_type," % (desc, desc))
                                    self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                                self.target_c_code.write_close_brace()
                                self.target_c_code.write_close_brace()
                            else :
                                self.target_c_code.write_line("for(i=0; i<%s->num_levels; i++)" % desc )
                                self.target_c_code.write_open_brace()
                                self.target_c_code.write_line("img_%s[i]->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(" % desc )
                                self.target_c_code.write_line("  img_%s[i]->mem_ptr[0].shared_ptr, img_%s[i]->mem_ptr[0].mem_type);" % (desc, desc))
                                if prm.do_map :
                                    self.target_c_code.write_line("tivxMemBufferMap(img_%s[i]->mem_ptr[0].target_ptr," % desc )
                                    self.target_c_code.write_line("   img_%s[i]->mem_size[0], img_%s[i]->mem_ptr[0].mem_type," % (desc, desc))
                                    self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                                self.target_c_code.write_close_brace()
                    else :
                        self.target_c_code.write_line("%s->mem_ptr.target_ptr = tivxMemShared2TargetPtr(" % desc )
                        self.target_c_code.write_line("  %s->mem_ptr.shared_ptr, %s->mem_ptr.mem_type);" % (desc, desc))
                        if prm.do_map :
                            self.target_c_code.write_line("tivxMemBufferMap(%s->mem_ptr.target_ptr," % desc )
                            self.target_c_code.write_line("   %s->mem_size, %s->mem_ptr.mem_type," % (desc, desc))
                            self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
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
                self.target_c_code.write_line("%s_value = %s->data.%s;" % (prm.name_lower, desc, Type.get_scalar_obj_desc_data_name(prm.type)))
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

        self.target_c_code.write_comment_line("call kernel processing function")
        self.target_c_code.write_newline()
        self.target_c_code.write_comment_line("< DEVELOPER_TODO: Add target kernel processing code here >")
        self.target_c_code.write_newline()
        self.target_c_code.write_comment_line("kernel processing function complete")
        self.target_c_code.write_newline()

        # unmap descriptors pointer
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if prm.do_unmap :
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_line("if( %s != NULL)" % desc)
                    self.target_c_code.write_open_brace()
                if Type.IMAGE == prm.type or Type.PYRAMID == prm.type:
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
                            self.target_c_code.write_line("tivxMemBufferUnmap(%s->mem_ptr[plane_idx].target_ptr," % desc )
                            self.target_c_code.write_line("   %s->mem_size[plane_idx], %s->mem_ptr[plane_idx].mem_type," % (desc, desc))
                            self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                            self.target_c_code.write_close_brace()
                        else :
                            self.target_c_code.write_line("tivxMemBufferUnmap(%s->mem_ptr[0].target_ptr," % desc )
                            self.target_c_code.write_line("   %s->mem_size[0], %s->mem_ptr[0].mem_type," % (desc, desc))
                            self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                    elif prm.type == Type.PYRAMID :
                        if self.multiplane :
                            self.target_c_code.write_line("for(i=0; i<%s->num_levels; i++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("tivxMemBufferUnmap(img_%s[i]->mem_ptr[plane_idx].target_ptr," % desc )
                            self.target_c_code.write_line("   img_%s[i]->mem_size[plane_idx], img_%s[i]->mem_ptr[plane_idx].mem_type," % (desc, desc))
                            self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                            self.target_c_code.write_close_brace()
                            self.target_c_code.write_close_brace()
                        else :
                            self.target_c_code.write_line("for(i=0; i<%s->num_levels; i++)" % desc )
                            self.target_c_code.write_open_brace()
                            self.target_c_code.write_line("tivxMemBufferUnmap(img_%s[i]->mem_ptr[0].target_ptr," % desc )
                            self.target_c_code.write_line("   img_%s[i]->mem_size[0], img_%s[i]->mem_ptr[0].mem_type," % (desc, desc))
                            self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                            self.target_c_code.write_close_brace()
                else :
                    self.target_c_code.write_line("tivxMemBufferUnmap(%s->mem_ptr.target_ptr," % desc )
                    self.target_c_code.write_line("   %s->mem_size, %s->mem_ptr.mem_type," % (desc, desc))
                    self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_close_brace()
                self.target_c_code.write_newline()
        self.target_c_code.write_newline()

        # set scalar values from local variables for output type scalars
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if (Type.is_scalar_type(prm.type) is True) and prm.direction != Direction.INPUT :
                self.target_c_code.write_line("%s->data.%s = %s_value;" % (desc, Type.get_scalar_obj_desc_data_name(prm.type), prm.name_lower))
        self.target_c_code.write_newline()

        self.target_c_code.write_close_brace()

        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_file_code(self):
        print("Creating " + self.workarea_module_core + "/" + self.target_c_filename)
        self.target_c_code = CodeGenerate(self.workarea_module_core + "/" + self.target_c_filename)
        self.target_c_code.write_include("TI/tivx.h")
        self.target_c_code.write_include(self.company + "/" + self.top_header_name + ".h")
        self.target_c_code.write_include("VX/vx.h")
        self.target_c_code.write_include("tivx_" + self.module.lower()  + "_kernels" + self.kernels_header_extension + ".h")
        self.target_c_code.write_include(self.h_filename)
        self.target_c_code.write_include("TI/tivx_target_kernel.h")
        self.target_c_code.write_include("tivx_kernels_target_utils.h")
        self.target_c_code.write_newline()
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
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg);")

        self.target_c_code.write_newline()
        self.generate_target_c_process_func_code()
        self.generate_target_c_create_func_code()
        self.generate_target_c_delete_func_code()
        self.generate_target_c_control_func_code()
        self.generate_target_c_add_func_code()
        self.generate_target_c_remove_func_code()
        self.target_c_code.close()

    def generate_bam_target_c_add_func_code(self):
        self.bam_target_c_code.write_line("void tivxAddTargetKernelBam%s(void)" % self.kernel.name_camel)
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("vx_status status = VX_FAILURE;")
        self.bam_target_c_code.write_line("char target_name[TIVX_TARGET_MAX_NAME];")
        self.bam_target_c_code.write_line("vx_enum self_cpu;")
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("self_cpu = tivxGetSelfCpuId();")
        self.bam_target_c_code.write_newline()
        for target in self.kernel.targets :
            cpu = Target.get_cpu(target)
            self.bam_target_c_code.write_line("if ( self_cpu == %s )" % Cpu.get_vx_enum_name(cpu) )
            self.bam_target_c_code.write_open_brace()
            self.bam_target_c_code.write_line("strncpy(target_name, %s, TIVX_TARGET_MAX_NAME);" % Target.get_vx_enum_name(target))
            self.bam_target_c_code.write_line("status = VX_SUCCESS;")
            self.bam_target_c_code.write_close_brace()
            self.bam_target_c_code.write_line("else")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("status = VX_FAILURE;")
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

        self.bam_target_c_code.write_if_status()
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("vx_%s_target_kernel = tivxAddTargetKernel(" % self.kernel.name_lower)
        self.bam_target_c_code.write_line("                    %s%s," % (self.kernel.enum_str_prefix, self.kernel.name_upper))
        self.bam_target_c_code.write_line("                    target_name,")
        self.bam_target_c_code.write_line("                    tivx%sProcess," % self.kernel.name_camel)
        self.bam_target_c_code.write_line("                    tivx%sCreate," % self.kernel.name_camel)
        self.bam_target_c_code.write_line("                    tivx%sDelete," % self.kernel.name_camel)
        self.bam_target_c_code.write_line("                    tivx%sControl," % self.kernel.name_camel)
        self.bam_target_c_code.write_line("                    NULL);")
        self.bam_target_c_code.write_close_brace()

        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

    def generate_bam_target_c_remove_func_code(self):
        self.bam_target_c_code.write_line("void tivxRemoveTargetKernelBam%s(void)" % self.kernel.name_camel)
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("status = tivxRemoveTargetKernel(vx_%s_target_kernel);" % self.kernel.name_lower)
        self.bam_target_c_code.write_if_status()
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("vx_%s_target_kernel = NULL;" % self.kernel.name_lower)
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

    def generate_bam_target_c_process_func_code(self):
        # define function name, and parameters
        self.bam_target_c_code.write_line("static vx_status VX_CALLBACK tivx%sProcess(" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.bam_target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.bam_target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.bam_target_c_code.write_open_brace()

        # define status variables and obj descriptor variable
        self.bam_target_c_code.write_line("vx_status status = VX_SUCCESS;")
        need_plane_idx_var = False
        for prm in self.kernel.params :
            if prm.do_map or prm.do_unmap :
                if prm.do_map_unmap_all_planes :
                    need_plane_idx_var = True
            self.bam_target_c_code.write_line("%s *%s_desc;" % (Type.get_obj_desc_name(prm.type), prm.name_lower) )
        if need_plane_idx_var is True :
            self.bam_target_c_code.write_line("uint16_t plane_idx;")
        # TODO figure out a way to get image format and create addr pointers here
        self.bam_target_c_code.write_line("tivx%sParams *prms = NULL;" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("uint32_t size;")
        self.bam_target_c_code.write_newline()

        # checks function parameters
        self.bam_target_c_code.write_line("status = tivxCheckNullParams(obj_desc, num_params,")
        self.bam_target_c_code.write_line("        %s%s_MAX_PARAMS);" % (self.kernel.enum_str_prefix, self.kernel.name_upper))
        self.bam_target_c_code.write_newline()

        # get target kernel instance context
        self.bam_target_c_code.write_line("if (VX_SUCCESS == status)")
        self.bam_target_c_code.write_open_brace()

        # define variables to hold scalar values
        for prm in self.kernel.params :
            if Type.is_scalar_type(prm.type) is True :
                self.bam_target_c_code.write_line("%s %s_value;" % (Type.get_vx_name(prm.type), prm.name_lower ))
        self.bam_target_c_code.write_newline()

        # assigned descriptors to local variables
        for prm in self.kernel.params :
            self.bam_target_c_code.write_line("%s_desc = (%s *)obj_desc[%s%s_%s_IDX];" %
                (prm.name_lower, Type.get_obj_desc_name(prm.type), self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
        self.bam_target_c_code.write_newline()

        self.bam_target_c_code.write_line("status = tivxGetTargetKernelInstanceContext(kernel,")
        self.bam_target_c_code.write_line("    (void **)&prms, &size);")
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("if ((VX_SUCCESS != status) || (NULL == prms) ||")
        self.bam_target_c_code.write_line("    (sizeof(tivx%sParams) != size))" % self.kernel.name_camel)
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("status = VX_FAILURE;")
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_close_brace()

        self.bam_target_c_code.write_line("if (VX_SUCCESS == status)")
        self.bam_target_c_code.write_open_brace()

        self.bam_target_c_code.write_line("void *img_ptrs[%s];" % self.kernel.getNumImages())
        self.bam_target_c_code.write_newline()

        # convert descriptors pointer to target pointers
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if Type.is_scalar_type(prm.type) is False :
                if prm.state is ParamState.OPTIONAL:
                    self.bam_target_c_code.write_line("if( %s != NULL)" % desc)
                    self.bam_target_c_code.write_open_brace()
                if prm.do_map or prm.do_unmap :
                    if prm.do_map_unmap_all_planes :
                        self.bam_target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                        self.bam_target_c_code.write_open_brace()
                        self.bam_target_c_code.write_line("%s->mem_ptr[plane_idx].target_ptr = tivxMemShared2TargetPtr(" % desc )
                        self.bam_target_c_code.write_line("  %s->mem_ptr[plane_idx].shared_ptr, %s->mem_ptr[plane_idx].mem_type);" % (desc, desc))
                        self.bam_target_c_code.write_close_brace()
                    else:
                        if prm.type == Type.IMAGE : #TODO: test with other types
                            self.bam_target_c_code.write_line("%s->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(" % desc )
                            self.bam_target_c_code.write_line("  %s->mem_ptr[0].shared_ptr, %s->mem_ptr[0].mem_type);" % (desc, desc))
                        else:
                            self.bam_target_c_code.write_line("%s->mem_ptr.target_ptr = tivxMemShared2TargetPtr(" % desc )
                            self.bam_target_c_code.write_line("  %s->mem_ptr.shared_ptr, %s->mem_ptr.mem_type);" % (desc, desc))
                if prm.state is ParamState.OPTIONAL:
                    self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

        # map descriptors pointer
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if prm.do_map :
                if prm.state is ParamState.OPTIONAL:
                    self.bam_target_c_code.write_line("if( %s != NULL)" % desc)
                    self.bam_target_c_code.write_open_brace()
                if prm.do_map_unmap_all_planes :
                    self.bam_target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                    self.bam_target_c_code.write_open_brace()
                    self.bam_target_c_code.write_line("tivxMemBufferMap(%s->mem_ptr[plane_idx].target_ptr," % desc )
                    self.bam_target_c_code.write_line("   %s->mem_size[plane_idx], %s->mem_ptr[plane_idx].mem_type," % (desc, desc))
                    self.bam_target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                    self.bam_target_c_code.write_close_brace()
                else:
                    if prm.type == Type.IMAGE : #TODO: test with other types
                        self.bam_target_c_code.write_line("tivxMemBufferMap(%s->mem_ptr[0].target_ptr," % desc )
                        self.bam_target_c_code.write_line("   %s->mem_size[0], %s->mem_ptr[0].mem_type," % (desc, desc))
                        self.bam_target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                    else:
                        self.bam_target_c_code.write_line("tivxMemBufferMap(%s->mem_ptr.target_ptr," % desc )
                        self.bam_target_c_code.write_line("   %s->mem_size, %s->mem_ptr.mem_type," % (desc, desc))
                        self.bam_target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))

                if prm.state is ParamState.OPTIONAL:
                    self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

        # set scalar values to local variables for input type scalars
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if (Type.is_scalar_type(prm.type) is True) and prm.direction != Direction.OUTPUT :
                if prm.state is ParamState.OPTIONAL:
                    self.bam_target_c_code.write_line("if( %s != NULL)" % desc)
                    self.bam_target_c_code.write_open_brace()
                self.bam_target_c_code.write_line("%s_value = %s->data.%s;" % (prm.name_lower, desc, Type.get_scalar_obj_desc_data_name(prm.type)))
                if prm.state is ParamState.OPTIONAL:
                    self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

        #TODO Set pointer location
        #TODO Set img_ptrs
        self.bam_target_c_code.write_line("tivxBamUpdatePointers(prms->graph_handle, %sU, %sU, img_ptrs);" % (self.kernel.getNumInputImages(), self.kernel.getNumOutputImages()))
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("status  = tivxBamProcessGraph(prms->graph_handle);")
        self.bam_target_c_code.write_newline()

        # unmap descriptors pointer
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if prm.do_unmap :
                if prm.state is ParamState.OPTIONAL:
                    self.bam_target_c_code.write_line("if( %s != NULL)" % desc)
                    self.bam_target_c_code.write_open_brace()
                if prm.do_map_unmap_all_planes :
                    self.bam_target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                    self.bam_target_c_code.write_open_brace()
                    self.bam_target_c_code.write_line("tivxMemBufferUnmap(%s->mem_ptr[plane_idx].target_ptr," % desc )
                    self.bam_target_c_code.write_line("   %s->mem_size[plane_idx], %s->mem_ptr[plane_idx].mem_type," % (desc, desc))
                    self.bam_target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                    self.bam_target_c_code.write_close_brace()
                else:
                    if prm.type == Type.IMAGE : #TODO: test with other types
                        self.bam_target_c_code.write_line("tivxMemBufferUnmap(%s->mem_ptr[0].target_ptr," % desc )
                        self.bam_target_c_code.write_line("   %s->mem_size[0], %s->mem_ptr[0].mem_type," % (desc, desc))
                        self.bam_target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                    else:
                        self.bam_target_c_code.write_line("tivxMemBufferUnmap(%s->mem_ptr.target_ptr," % desc )
                        self.bam_target_c_code.write_line("   %s->mem_size, %s->mem_ptr.mem_type," % (desc, desc))
                        self.bam_target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                if prm.state is ParamState.OPTIONAL:
                    self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

        self.bam_target_c_code.write_close_brace()

        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("return status;")
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

    def generate_bam_target_c_create_func_code(self):
        self.bam_target_c_code.write_line("static vx_status VX_CALLBACK tivx%sCreate(" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.bam_target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.bam_target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.bam_target_c_code.write_open_brace()

        # define status variables and obj descriptor variable
        self.bam_target_c_code.write_line("vx_status status = VX_SUCCESS;")
        need_plane_idx_var = False
        for prm in self.kernel.params :
            if prm.do_map or prm.do_unmap :
                if prm.do_map_unmap_all_planes :
                    need_plane_idx_var = True
            self.bam_target_c_code.write_line("%s *%s_desc;" % (Type.get_obj_desc_name(prm.type), prm.name_lower) )
        if need_plane_idx_var is True :
            self.bam_target_c_code.write_line("uint16_t plane_idx;")
        self.bam_target_c_code.write_line("tivx%sParams *prms = NULL;" % self.kernel.name_camel)
        self.bam_target_c_code.write_newline()

        # checks function parameters
        self.bam_target_c_code.write_line("status = tivxCheckNullParams(obj_desc, num_params,")
        self.bam_target_c_code.write_line("        %s%s_MAX_PARAMS);" % (self.kernel.enum_str_prefix, self.kernel.name_upper))
        self.bam_target_c_code.write_newline()

        self.bam_target_c_code.write_line("if (VX_SUCCESS == status)")
        self.bam_target_c_code.write_open_brace()

        # define variables to hold scalar values
        for prm in self.kernel.params :
            if Type.is_scalar_type(prm.type) is True :
                self.bam_target_c_code.write_line("%s %s_value;" % (Type.get_vx_name(prm.type), prm.name_lower ))
        self.bam_target_c_code.write_newline()

        # assigned descriptors to local variables
        for prm in self.kernel.params :
            self.bam_target_c_code.write_line("%s_desc = (%s *)obj_desc[%s%s_%s_IDX];" %
                (prm.name_lower, Type.get_obj_desc_name(prm.type), self.kernel.enum_str_prefix, self.kernel.name_upper, prm.name_upper) )
        self.bam_target_c_code.write_newline()

        # allocate memory for tivxParams
        self.bam_target_c_code.write_line("prms = tivxMemAlloc(sizeof(tivx%sParams)," % self.kernel.name_camel)
        self.bam_target_c_code.write_line("    TIVX_MEM_EXTERNAL);")
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("if (NULL != prms)")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("tivx_bam_kernel_details_t kernel_details;")
        # TODO: Probably could use some kind of logic here to write bufparams
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("memset(prms, 0, sizeof(tivxAddParams));")
        # TODO: Use same logic from buf params to write owninitbufparams and fill in frame level sizes
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("/* Fill in the frame level sizes of buffers here. If the port")
        self.bam_target_c_code.write_line(" * is optionally disabled, put NULL */")

        self.bam_target_c_code.write_close_brace()

        # if function parameter check fails
        self.bam_target_c_code.write_line("else")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("status = VX_ERROR_NO_MEMORY;")
        self.bam_target_c_code.write_close_brace()

        self.bam_target_c_code.write_line("if (VX_SUCCESS == status)")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("tivxSetTargetKernelInstanceContext(kernel, prms,")
        self.bam_target_c_code.write_line("    sizeof(tivx%sParams));" % self.kernel.name_camel)
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_line("else")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("if (NULL != prms)")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("tivxMemFree(prms, sizeof(tivx%sParams), TIVX_MEM_EXTERNAL);" % self.kernel.name_camel)
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_line("return status;")
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

    def generate_bam_target_c_delete_func_code(self):
        self.bam_target_c_code.write_line("static vx_status VX_CALLBACK tivx%sDelete(" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.bam_target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.bam_target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.bam_target_c_code.write_line("uint32_t size;")
        self.bam_target_c_code.write_line("tivx%sParams *prms = NULL;" % self.kernel.name_camel)
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("/* Check number of buffers and NULL pointers */")
        self.bam_target_c_code.write_line("status = tivxCheckNullParams(obj_desc, num_params,")
        self.bam_target_c_code.write_line("        %s%s_MAX_PARAMS);" % (self.kernel.enum_str_prefix, self.kernel.name_upper))
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("if (VX_SUCCESS == status)")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("status = tivxGetTargetKernelInstanceContext(kernel,")
        self.bam_target_c_code.write_line("    (void **)&prms, &size);")
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("if ((VX_SUCCESS == status) && (NULL != prms) &&")
        self.bam_target_c_code.write_line("    (sizeof(tivx%sParams) == size))" % self.kernel.name_camel)
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("tivxBamDestroyHandle(prms->graph_handle);")
        self.bam_target_c_code.write_line("tivxMemFree(prms, sizeof(tivx%sParams), TIVX_MEM_EXTERNAL);" % self.kernel.name_camel)
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("return status;")
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

    def generate_bam_target_c_control_func_code(self):
        self.bam_target_c_code.write_line("static vx_status VX_CALLBACK tivx%sControl(" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.bam_target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.bam_target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_comment_line("< DEVELOPER_TODO: (Optional) Add any target kernel control code here (e.g. commands")
        self.bam_target_c_code.write_comment_line("                  the user can call to modify the processing of the kernel at run-time) >")
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("return status;")
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

    def generate_bam_target_c_file_code(self):
        self.bam_target_c_code = CodeGenerate(self.bam_target_c_filename)
        self.bam_target_c_code.write_include("TI/tivx.h")
        self.bam_target_c_code.write_include("VX/vx.h")
        self.bam_target_c_code.write_include("tivx_openvx_core_kernels.h")
        self.bam_target_c_code.write_include(self.h_filename)
        self.bam_target_c_code.write_include("TI/tivx_target_kernel.h")
        self.bam_target_c_code.write_include("ti/vxlib/vxlib.h")
        self.bam_target_c_code.write_include("tivx_kernels_target_utils.h")
        self.bam_target_c_code.write_include("<tivx_bam_kernel_wrapper.h")
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("typedef struct")
        self.bam_target_c_code.write_line("{")
        self.bam_target_c_code.write_line("    tivx_bam_graph_handle graph_handle;")
        self.bam_target_c_code.write_line("} tivx%sParams;" % (self.kernel.name_camel))
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("static tivx_target_kernel vx_%s_target_kernel = NULL;" % (self.kernel.name_lower))
        self.bam_target_c_code.write_newline()

        self.bam_target_c_code.write_line("static vx_status VX_CALLBACK tivx%sProcess(" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.bam_target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.bam_target_c_code.write_line("       uint16_t num_params, void *priv_arg);")
        self.bam_target_c_code.write_line("static vx_status VX_CALLBACK tivx%sCreate(" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.bam_target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.bam_target_c_code.write_line("       uint16_t num_params, void *priv_arg);")
        self.bam_target_c_code.write_line("static vx_status VX_CALLBACK tivx%sDelete(" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.bam_target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.bam_target_c_code.write_line("       uint16_t num_params, void *priv_arg);")
        self.bam_target_c_code.write_line("static vx_status VX_CALLBACK tivx%sControl(" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.bam_target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.bam_target_c_code.write_line("       uint16_t num_params, void *priv_arg);")

        self.bam_target_c_code.write_newline()
        self.generate_bam_target_c_process_func_code()
        self.generate_bam_target_c_create_func_code()
        self.generate_bam_target_c_delete_func_code()
        self.generate_bam_target_c_control_func_code()
        self.generate_bam_target_c_add_func_code()
        self.generate_bam_target_c_remove_func_code()
        self.bam_target_c_code.close()

    def generate_make_files(self) :
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
            else:
                self.concerto_inc_code.write_line("STATIC_LIBS += vx_target_kernels_" + self.module + "_" + self.core)
            self.concerto_inc_code.write_line("STATIC_LIBS += vx_conformance_engine")
            self.concerto_inc_code.write_line("# < DEVELOPER_TODO: Add any additional dependent libraries >")
            self.concerto_inc_code.close()

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
            self.module_host_concerto_code.write_line("include $(PRELUDE)")
            self.module_host_concerto_code.write_line("TARGET      := vx_kernels_" + self.module)
            self.module_host_concerto_code.write_line("TARGETTYPE  := library")
            self.module_host_concerto_code.write_line("CSOURCES    := $(call all-c-files)")
            if self.env_var == 'CUSTOM_KERNEL_PATH' :
                self.module_host_concerto_code.write_line("IDIRS       += $("+self.env_var+")/" + self.module + "/include")
            else:
                self.module_host_concerto_code.write_line("IDIRS       += $("+self.env_var+")/kernels/" + self.module + "/include")
            self.module_host_concerto_code.write_newline()
            self.module_host_concerto_code.write_line("ifeq ($(TARGET_CPU),C66)")
            self.module_host_concerto_code.write_line("SKIPBUILD=1")
            self.module_host_concerto_code.write_line("endif")
            self.module_host_concerto_code.write_newline()
            self.module_host_concerto_code.write_line("ifeq ($(TARGET_CPU),EVE)")
            self.module_host_concerto_code.write_line("SKIPBUILD=1")
            self.module_host_concerto_code.write_line("endif")
            self.module_host_concerto_code.write_newline()
            self.module_host_concerto_code.write_line("ifeq ($(TARGET_CPU),A15)")
            self.module_host_concerto_code.write_line("SKIPBUILD=1")
            self.module_host_concerto_code.write_line("endif")
            self.module_host_concerto_code.write_newline()
            self.module_host_concerto_code.write_line("ifeq ($(TARGET_CPU),M4)")
            self.module_host_concerto_code.write_line("SKIPBUILD=1")
            self.module_host_concerto_code.write_line("endif")
            self.module_host_concerto_code.write_newline()
            self.module_host_concerto_code.write_line("include $(FINALE)")
            self.module_host_concerto_code.close()

        self.module_target_concerto_filename = self.workarea_module_core + "/concerto.mak"
        if not os.path.exists(self.module_target_concerto_filename):
            print("Creating " + self.module_target_concerto_filename)
            self.module_target_concerto_code = CodeGenerate(self.module_target_concerto_filename, header=False)
            self.module_target_concerto_code.write_line("include $(PRELUDE)")
            if self.env_var == 'CUSTOM_KERNEL_PATH' :
                self.module_target_concerto_code.write_line("TARGET      := vx_target_kernels_" + self.core)
            else:
                self.module_target_concerto_code.write_line("TARGET      := vx_target_kernels_" + self.module + "_" + self.core);
            self.module_target_concerto_code.write_line("TARGETTYPE  := library")
            self.module_target_concerto_code.write_line("CSOURCES    := $(call all-c-files)")
            if self.env_var == 'CUSTOM_KERNEL_PATH' :
                self.module_target_concerto_code.write_line("IDIRS       += $("+self.env_var+")/" + self.module + "/include")
            else:
                self.module_target_concerto_code.write_line("IDIRS       += $("+self.env_var+")/kernels/" + self.module + "/include")
                self.module_target_concerto_code.write_line("IDIRS       += $("+self.env_var+")/kernels/" + self.module + "/host")
            self.module_target_concerto_code.write_line("IDIRS       += $(HOST_ROOT)/kernels/include")
            self.module_target_concerto_code.write_line("IDIRS       += $(VXLIB_PATH)/packages")
            self.module_target_concerto_code.write_line("# < DEVELOPER_TODO: Add any custom include paths using 'IDIRS' >")
            self.module_target_concerto_code.write_line("# < DEVELOPER_TODO: Add any custom preprocessor defines or build options needed using")
            self.module_target_concerto_code.write_line("#                   'CFLAGS'. >")
            self.module_target_concerto_code.write_line("# < DEVELOPER_TODO: Adjust which cores this library gets built on using 'SKIPBUILD'. >")
            self.module_target_concerto_code.write_newline();
            self.module_target_concerto_code.write_line("DEFS += CORE_DSP")
            self.module_target_concerto_code.write_newline();
            self.module_target_concerto_code.write_line("ifeq ($(BUILD_BAM),yes)")
            self.module_target_concerto_code.write_line("DEFS += BUILD_BAM")
            self.module_target_concerto_code.write_line("endif")
            self.module_target_concerto_code.write_newline();
            self.module_target_concerto_code.write_line("ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))")
            self.module_target_concerto_code.write_line("DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION")
            self.module_target_concerto_code.write_line("endif")
            self.module_target_concerto_code.write_newline();
            self.module_target_concerto_code.write_line("ifeq ($(TARGET_CPU),C66)")
            self.module_target_concerto_code.write_line("SKIPBUILD=1")
            self.module_target_concerto_code.write_line("endif")
            self.module_target_concerto_code.write_newline();
            self.module_target_concerto_code.write_line("ifeq ($(TARGET_CPU),EVE)")
            self.module_target_concerto_code.write_line("SKIPBUILD=1")
            self.module_target_concerto_code.write_line("endif")
            self.module_target_concerto_code.write_newline();
            self.module_target_concerto_code.write_line("ifeq ($(TARGET_CPU),A15)")
            self.module_target_concerto_code.write_line("SKIPBUILD=1")
            self.module_target_concerto_code.write_line("endif")
            self.module_target_concerto_code.write_newline();
            self.module_target_concerto_code.write_line("ifeq ($(TARGET_CPU),M4)")
            self.module_target_concerto_code.write_line("SKIPBUILD=1")
            self.module_target_concerto_code.write_line("endif")
            self.module_target_concerto_code.write_newline();
            self.module_target_concerto_code.write_line("include $(FINALE)")
            self.module_target_concerto_code.close()

        self.module_test_concerto_filename = self.workarea_module_test + "/concerto.mak"
        if not os.path.exists(self.module_test_concerto_filename):
            print("Creating " + self.module_test_concerto_filename)
            self.module_test_concerto_code = CodeGenerate(self.module_test_concerto_filename, header=False)
            self.module_test_concerto_code.write_line("include $(PRELUDE)")
            self.module_test_concerto_code.write_line("TARGET      := vx_kernels_" + self.module + "_tests")
            self.module_test_concerto_code.write_line("TARGETTYPE  := library")
            self.module_test_concerto_code.write_line("CSOURCES    := $(call all-c-files)")
            self.module_test_concerto_code.write_line("IDIRS       += $(HOST_ROOT)/conformance_tests")
            self.module_test_concerto_code.write_line("IDIRS       += $(HOST_ROOT)/source/include")
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("ifeq ($(TARGET_CPU),C66)")
            self.module_test_concerto_code.write_line("SKIPBUILD=1")
            self.module_test_concerto_code.write_line("endif")
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("ifeq ($(TARGET_CPU),EVE)")
            self.module_test_concerto_code.write_line("SKIPBUILD=1")
            self.module_test_concerto_code.write_line("endif")
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("ifeq ($(TARGET_CPU),A15)")
            self.module_test_concerto_code.write_line("SKIPBUILD=1")
            self.module_test_concerto_code.write_line("endif")
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("ifeq ($(TARGET_CPU),M4)")
            self.module_test_concerto_code.write_line("SKIPBUILD=1")
            self.module_test_concerto_code.write_line("endif")
            self.module_test_concerto_code.write_newline()
            self.module_test_concerto_code.write_line("include $(FINALE)")
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
            self.include_customer_kernels_code.write_line("/*! \\brief The list of available libraries in this extension */")
            self.include_customer_kernels_code.write_line("enum " + self.top_header_name + "_library_e {")
            self.include_customer_kernels_code.write_line("   /*! \\brief The set of kernels supported in " + self.module + " module  */")
            self.include_customer_kernels_code.write_line("   TIVX_LIBRARY_" + self.module.upper() + "_BASE = 0,")
            self.include_customer_kernels_code.write_line("};")
            self.include_customer_kernels_code.write_newline()
            self.include_customer_kernels_code.write_line("/*!")
            self.include_customer_kernels_code.write_line(" * \\brief The list of kernels supported in " + self.module + " module")
            self.include_customer_kernels_code.write_line(" *")
            self.include_customer_kernels_code.write_line(" * Each kernel listed here can be used with the <tt>\\ref vxGetKernelByEnum</tt> call.")
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
            self.include_customer_kernels_code.write_line("enum tivx_kernel_" + self.module + "_e {")
            self.include_customer_kernels_code.write_line("    /*! \\brief The " + self.kernel.name_lower + " kernel")
            self.include_customer_kernels_code.write_line("     * \\see group_vision_function_" + self.module)
            self.include_customer_kernels_code.write_line("     */")
            self.include_customer_kernels_code.write_line("    " + self.kernel.enum_str_prefix + self.kernel.name_upper + " = VX_KERNEL_BASE(VX_ID_DEFAULT, TIVX_LIBRARY_" + self.module.upper() + "_BASE) + 0,")
            self.include_customer_kernels_code.write_line("    " + self.kernel.enum_str_prefix + self.module.upper() + "_MAX_1_0, /*!< \internal Used for bounds checking in the conformance test. */")
            self.include_customer_kernels_code.write_line("};")
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
            self.include_customer_nodes_code.write_line(" * \\see <tt>" + self.kernel.enum_str_prefix + self.kernel.name_upper + "</tt>")
            self.include_customer_nodes_code.write_line(" * \\ingroup group_vision_function_" + self.kernel.name_lower)
            self.include_customer_nodes_code.write_line(" * \\return <tt>\\ref vx_node</tt>.")
            self.include_customer_nodes_code.write_line(" * \\retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\\ref vxGetStatus</tt>")
            self.include_customer_nodes_code.write_line(" */")
            self.include_customer_nodes_code.write_line("VX_API_ENTRY vx_node VX_API_CALL tivx" + self.kernel.name_camel + "Node(vx_graph graph,")
            for prm in self.kernel.params[:-1] :
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
                self.host_node_api_code.write_line("%-37s %-20s %s," % ("", prm.type.get_vx_name(), prm.name_lower))
            self.host_node_api_code.write_line("%-37s %-20s %s)" % ("", self.kernel.params[-1].type.get_vx_name(), self.kernel.params[-1].name_lower))
            self.host_node_api_code.write_open_brace()
            self.host_node_api_code.write_line("vx_reference prms[] = {")
            for prm in self.kernel.params[:-1] :
                self.host_node_api_code.write_line("%-7s (vx_reference)%s," % ("", prm.name_lower))
            self.host_node_api_code.write_line("%-7s (vx_reference)%s" % ("", self.kernel.params[-1].name_lower))
            self.host_node_api_code.write_line("};")
            self.host_node_api_code.write_line("vx_node node = tivxCreateNodeByKernelEnum(graph,")
            self.host_node_api_code.write_line("%-38s %s," % ("", self.kernel.enum_str_prefix + self.kernel.name_upper))
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
            self.host_kernels_code.write_line("tivxSetSelfCpuId(TIVX_CPU_ID_IPU1_0);")
            self.host_kernels_code.write_line("tivxRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels();")
            self.host_kernels_code.write_line("tivxSetSelfCpuId(TIVX_CPU_ID_DSP1);")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("gIs" + toCamelCase(self.module) + "KernelsLoad = 1U;")
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("void tivx" + toCamelCase(self.module) + "UnLoadKernels(vx_context context)")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("if ((1u == gIs" + toCamelCase(self.module) + "KernelsLoad) && (NULL != context))")
            self.host_kernels_code.write_open_brace()
            self.host_kernels_code.write_line("vxUnloadKernels(context, TIVX_MODULE_NAME_" + self.module.upper() + ");")
            self.host_kernels_code.write_line("tivxUnRegister" + toCamelCase(self.module) + "Kernels();")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("/* This line only work on PC emulation mode ...")
            self.host_kernels_code.write_line(" * this will need to be updated when moving to target */")
            self.host_kernels_code.write_line("tivxUnRegister" + toCamelCase(self.module) + "Target" + toCamelCase(self.core) + "Kernels();")
            self.host_kernels_code.write_newline()
            self.host_kernels_code.write_line("gIs" + toCamelCase(self.module) + "KernelsLoad = 0U;")
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.write_close_brace()
            self.host_kernels_code.close()

        self.target_kernels_filename = self.workarea_module_core + "/vx_kernels_" + self.module.lower() + "_target.c"
        if not os.path.exists(self.target_kernels_filename):
            print("Creating " + self.target_kernels_filename)
            self.target_kernels_code = CodeGenerate(self.target_kernels_filename)
            self.target_kernels_code.write_line("#include <TI/tivx.h>")
            self.target_kernels_code.write_line("#include <TI/tivx_target_kernel.h>")
            self.target_kernels_code.write_line("#include \"tivx_" + self.module.lower() + "_kernels" + self.kernels_header_extension + ".h\"")
            self.target_kernels_code.write_line("#include \"tivx_kernels_target_utils.h\"")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("void tivxAddTargetKernel" + self.kernel.name_camel + "(void);")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("void tivxRemoveTargetKernel" + self.kernel.name_camel + "(void);")
            self.target_kernels_code.write_newline()
            self.target_kernels_code.write_line("static Tivx_Target_Kernel_List  gTivx_target_kernel_list[] = {")
            self.target_kernels_code.write_line("    {&tivxAddTargetKernel" + self.kernel.name_camel + ", &tivxRemoveTargetKernel" + self.kernel.name_camel + "},")
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
        print("Modifying " + self.concerto_inc_filename)
        if self.env_var == 'CUSTOM_KERNEL_PATH' :
            CodeModify().block_insert(self.concerto_inc_filename,
                          "vx_kernels_" + self.module,
                          "DEVELOPER_TODO",
                          "vx_target_kernels_" + self.core,
                          "STATIC_LIBS \+= vx_conformance_engine",
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_target_kernels_" + self.core + "\n")
        else:
            CodeModify().block_insert(self.concerto_inc_filename,
                          "vx_kernels_" + self.module,
                          "DEVELOPER_TODO",
                          "vx_target_kernels_" + self.core,
                          "STATIC_LIBS \+= vx_conformance_engine",
                          "STATIC_LIBS += vx_conformance_engine",
                          "STATIC_LIBS += vx_target_kernels_" + self.module + "_" + self.core + "\n")


    def modify_kernel_header_file(self) :
        print("Modifying " + self.include_customer_kernels_filename)
        # Update for new modules
        self.insert = (r"! \\brief Name for OpenVX Extension kernel module: " + self.module + "\n" +
                        " * \\ingroup group_tivx_ext\n" +
                        " */\n" +
                        "#define TIVX_MODULE_NAME_" + self.module.upper() + "    \"" + self.module + "\"\n\n")
        CodeModify().block_insert(self.include_customer_kernels_filename,
                          "extern \"C\" {",
                          "enum",
                          "#define TIVX_MODULE_NAME_" + self.module.upper() + "    \"" + self.module + "\"",
                          r"/*! \\brief The list of available libraries in this extension",
                          r"/*! \\brief The list of available libraries in this extension",
                          self.insert)

        self.prev_enum_value = CodeModify().block_search(self.include_customer_kernels_filename,
                                                  "enum " + self.top_header_name + "_library_e",
                                                  "};",
                                                  " (\d)")
        self.insert = (r"   /*! \\brief The set of kernels supported in " + self.module + " module  */\n" +
                        "   TIVX_LIBRARY_" + self.module.upper() +
                        "_BASE = %d,\n" % (int(self.prev_enum_value)+1))
        CodeModify().block_insert(self.include_customer_kernels_filename,
                          "enum " + self.top_header_name + "_library_e",
                          "};",
                          "TIVX_LIBRARY_" + self.module.upper() + "_BASE",
                          "};",
                          "};",
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
            "enum tivx_kernel_" + self.module + "_e {" + "\n" +
            r"    /*! \\brief The " + self.kernel.name_lower + " kernel" + "\n" +
            "     * \\see group_vision_function_" + self.module + "\n" +
            "     */" + "\n" +
            "    " + self.kernel.enum_str_prefix + self.kernel.name_upper + " = VX_KERNEL_BASE(VX_ID_DEFAULT, TIVX_LIBRARY_" + self.module.upper() + "_BASE) + 0," + "\n" +
            "    " + self.kernel.enum_str_prefix + self.module.upper() + "_MAX_1_0, /*!< \internal Used for bounds checking in the conformance test. */" + "\n" +
            "};" + "\n\n/*! \n")
        CodeModify().block_insert(self.include_customer_kernels_filename,
                          "enum " + self.top_header_name + "_library_e",
                          " Used for the Application to load the",
                          "enum tivx_kernel_" + self.module + "_e {",
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
        self.prev_enum_value = CodeModify().block_search(self.include_customer_kernels_filename,
                                                  "enum tivx_kernel_" + self.module + "_e {",
                                                  self.kernel.enum_str_prefix + self.module.upper() + "_MAX_1_0",
                                                  " (\d)")
        self.insert = (r"/*! \\brief The " + self.kernel.name_lower + " kernel\n" +
                        "     * \\see group_vision_function_" + self.module + "\n" +
                        "     */\n" +
                        "    " + self.kernel.enum_str_prefix + self.kernel.name_upper +
                        " = VX_KERNEL_BASE(VX_ID_DEFAULT, TIVX_LIBRARY_" + self.module.upper() +
                        "_BASE) + " + "%d,\n    " % (int(self.prev_enum_value)+1))
        CodeModify().block_insert(self.include_customer_kernels_filename,
                          "enum tivx_kernel_" + self.module + "_e {",
                          self.kernel.enum_str_prefix + self.module.upper() + "_MAX_1_0",
                          " " +self.kernel.enum_str_prefix + self.kernel.name_upper + " = ",
                          self.kernel.enum_str_prefix + self.module.upper() + "_MAX_1_0",
                          self.kernel.enum_str_prefix + self.module.upper() + "_MAX_1_0",
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
        self.insert += (r" * \\see <tt>" + self.kernel.enum_str_prefix + self.kernel.name_upper + "</tt>" + "\n")
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
            self.insert += ("    vx_node node = tivxCreateNodeByKernelEnum(graph,\n")
            self.insert += ("%-42s %s,\n" % ("", self.kernel.enum_str_prefix + self.kernel.name_upper))
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
        CodeModify().block_insert(self.target_kernels_filename,
                          "void tivxAddTargetKernel",
                          "void tivxRemoveTargetKernel",
                          "void tivxAddTargetKernel" + self.kernel.name_camel + "(void);",
                          "\nvoid tivxRemoveTargetKernel",
                          "\nvoid tivxRemoveTargetKernel",
                          "void tivxAddTargetKernel" + self.kernel.name_camel + "(void);\n")

        CodeModify().block_insert(self.target_kernels_filename,
                          "void tivxRemoveTargetKernel",
                          "Tivx_Target_Kernel_List",
                          "void tivxRemoveTargetKernel" + self.kernel.name_camel + "(void);",
                          "\nstatic Tivx_Target_Kernel_List",
                          "\nstatic Tivx_Target_Kernel_List",
                          "void tivxRemoveTargetKernel" + self.kernel.name_camel + "(void);\n")

        CodeModify().block_insert(self.target_kernels_filename,
                          "Tivx_Target_Kernel_List",
                          "};",
                          "    {&tivxAddTargetKernel" + self.kernel.name_camel + ", &tivxRemoveTargetKernel" + self.kernel.name_camel + "}",
                          "};",
                          "};",
                          "    {&tivxAddTargetKernel" + self.kernel.name_camel + ", &tivxRemoveTargetKernel" + self.kernel.name_camel + "},\n")

    def todo(self) :
        self.todo_filename = self.workarea + "/DEVELOPER_TODO.txt"
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
        self.all_files = [y for x in os.walk(self.workarea) for y in glob(os.path.join(x[0], '*.*'))]
        for file in self.all_files :
            with open(file) as f:
                for num, line in enumerate(f, 1):
                    if 'DEVELOPER_TODO' in line:
                        if '>' in line:
                            self.state = False
                        else:
                            self.state = True
                        self.modLine = re.sub("^.*?DEVELOPER_TODO:","", line)
                        self.modLine = re.sub("^\s+","", self.modLine)
                        self.modLine = re.sub("\*\/","", self.modLine)
                        self.modLine = re.sub("\>","", self.modLine)
                        if self.fileName != file :
                            self.todo_code.write_line("\n" + file, new_line=False)
                        self.todo_code.write_line("\n    " + str(num) + ": " + self.modLine, new_line=False)
                        self.lineNum = num
                        self.fileName = file
                    elif self.state :
                        if '>' in line :
                            self.state = False
                        self.modLine = re.sub("^.*?DEVELOPER_TODO:","", line)
                        self.modLine = re.sub("#","", self.modLine)
                        self.modLine = re.sub("\/\/","", self.modLine)
                        self.modLine = re.sub("\/\*","", self.modLine)
                        self.modLine = re.sub("\*\/","", self.modLine)
                        self.modLine = re.sub("^\s+","", self.modLine)
                        self.modLine = re.sub("\>","", self.modLine)
                        self.todo_code.write_line("    " + str(num) + ": " + self.modLine, new_line=False)
                        self.lineNum = num
        self.todo_code.close()


    def export(self, kernel) :
        self.kernel = kernel
        self.h_filename = "tivx_kernel_" + kernel.name_lower + ".h";
        self.host_c_filename = "vx_" + kernel.name_lower + "_host.c";
        self.target_c_filename = "vx_" + kernel.name_lower + "_target.c";
        self.bam_target_c_filename = "vx_bam_" + kernel.name_lower + "_target.c";

        self.target_uses_dsp = False
        for target in self.kernel.targets :
            if target == Target.DSP1 or target == Target.DSP2 :
                self.target_uses_dsp = True

        print ('Generating C code for OpenVX kernel ...')
        print ()
        print ('Creating new directories ...')
        self.create_all_directories()
        print ('Creating new makefiles ...')
        self.generate_make_files()
        print ('Creating new headers ...')
        self.generate_headers()
        print ('Creating new module-level sources ...')
        self.generate_sources()

        self.modify_files()

        print ('Creating new kernel-specific files ...')
        self.generate_h_file_code()
        self.generate_host_c_file_code()
        self.generate_target_c_file_code()
        if self.target_uses_dsp :
            self.generate_bam_target_c_file_code()

        print ()
        print (self.kernel)
        print ('Generating C code for OpenVX kernel ... DONE !!!')
        self.todo()

