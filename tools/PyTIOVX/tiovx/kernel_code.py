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

class KernelExportCode :
    def __init__(self, kernel) :
        self.kernel = kernel
        self.h_filename = "tivx_kernel_" + kernel.name_lower + ".h";
        self.host_c_filename = "vx_" + kernel.name_lower + "_host.c";
        self.target_c_filename = "vx_" + kernel.name_lower + "_target.c";
        self.bam_target_c_filename = "vx_bam_" + kernel.name_lower + "_target.c";

        self.h_code = CodeGenerate(self.h_filename)
        self.host_c_code = CodeGenerate(self.host_c_filename)
        self.target_c_code = CodeGenerate(self.target_c_filename)
        target_uses_dsp = False
        for target in self.kernel.targets :
            if target == Target.DSP1 or target == Target.DSP2 :
                target_uses_dsp = True
        if target_uses_dsp :
            self.bam_target_c_code = CodeGenerate(self.bam_target_c_filename)

    def generate_h_file_code(self):
        self.h_code.open()
        self.h_code.write_ifndef_define("_TIVX_KERNEL_" + self.kernel.name_upper + "_")
        self.h_code.write_line("#ifdef __cplusplus")
        self.h_code.write_line("extern \"C\" {")
        self.h_code.write_line("#endif")
        self.h_code.write_newline();
        for prm in self.kernel.params :
            self.h_code.write_line("#define TIVX_KERNEL_%s_%s_IDX (%dU)" % (self.kernel.name_upper, prm.name_upper, prm.index))
        self.h_code.write_newline();
        self.h_code.write_line("#define TIVX_KERNEL_%s_MAX_PARAMS (%dU)" % (self.kernel.name_upper, len(self.kernel.params)))
        self.h_code.write_newline();
        self.h_code.write_line("#ifdef __cplusplus")
        self.h_code.write_line("}")
        self.h_code.write_line("#endif")
        self.h_code.write_newline()
        self.h_code.write_endif("_TIVX_KERNEL_" + self.kernel.name_upper + "_")
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
        self.host_c_code.write_line("            \"%s%s\"," % (self.kernel.name_str_prefix, self.kernel.name_lower))
        self.host_c_code.write_line("            %s%s," % (self.kernel.enum_str_prefix, self.kernel.name_upper))
        self.host_c_code.write_line("            NULL,")
        self.host_c_code.write_line("            TIVX_KERNEL_%s_MAX_PARAMS," % (self.kernel.name_upper) )
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
        self.host_c_code.write_line("vx_image img[%sU] = {NULL};" % self.kernel.getNumImages())
        if self.kernel.getNumScalars() != 0 :
            self.host_c_code.write_line("vx_scalar scalar[%sU] = {NULL};" % self.kernel.getNumScalars())
            num_scalar = 0
            for prm in self.kernel.params :
                if Type.is_scalar_type(prm.type) is True :
                    self.host_c_code.write_line("%s %s_%s = {NULL};" % (Type.get_vx_name(prm.type), prm.type.name.lower(), num_scalar))
                    num_scalar += 1
        self.host_c_code.write_line("vx_df_image fmt[%sU] = {NULL};" % self.kernel.getNumImages())
        self.host_c_code.write_line("/* Developer TODO: Change out_fmt to the correct output format */")
        self.host_c_code.write_line("vx_df_image out_fmt = VX_DF_IMAGE_U8;")
        #TODO write types other than just images and scalars
        self.host_c_code.write_line("vx_uint32 i, w[%sU], h[%sU], out_w, out_h;" % (self.kernel.getNumImages(), self.kernel.getNumImages()))
        self.host_c_code.write_newline()
        self.host_c_code.write_line("status = tivxKernelValidateParametersNotNull(parameters, TIVX_KERNEL_%s_MAX_PARAMS);" % self.kernel.name_upper)
        self.host_c_code.write_newline()

        # Query all types here
        self.host_c_code.write_line("if (VX_SUCCESS == status)")
        self.host_c_code.write_open_brace()
        # find code from target for here
        # assigned descriptors to local variables
        # TODO support other types than just images and scalars
        num_image = 0
        num_scalar = 0
        for prm in self.kernel.params :
            if Type.IMAGE == prm.type :
                self.host_c_code.write_line("img[%sU] = (%s)parameters[TIVX_KERNEL_%s_%s_IDX];" %
                    (num_image, Type.get_vx_name(prm.type), self.kernel.name_upper, prm.name_upper) )
                num_image+=1
            if Type.is_scalar_type(prm.type) is True :
                self.host_c_code.write_line("scalar[%sU] = (vx_scalar)parameters[TIVX_KERNEL_%s_%s_IDX];" %
                    (num_scalar, self.kernel.name_upper, prm.name_upper) )
                num_scalar+=1
        self.host_c_code.write_newline()
        self.host_c_code.write_close_brace()

        # for loop writing each query here around if statements checking the status
        # TODO support other types than just images and scalars
        num_image = 0
        num_scalar = 0
        for prm in self.kernel.params :
            self.host_c_code.write_line("if (VX_SUCCESS == status)")
            self.host_c_code.write_open_brace()
            if Type.IMAGE == prm.type :
                self.host_c_code.write_line("/* Get the image width/height and format */")
                self.host_c_code.write_line("status = vxQueryImage(img[%sU], VX_IMAGE_FORMAT, &fmt[%sU]," % (num_image, num_image))
                self.host_c_code.write_line("    sizeof(fmt[%sU]));" % num_image)
                self.host_c_code.write_line("status |= vxQueryImage(img[%sU], VX_IMAGE_WIDTH, &w[%sU], sizeof(w[%sU]));" % (num_image, num_image, num_image))
                self.host_c_code.write_line("status |= vxQueryImage(img[%sU], VX_IMAGE_HEIGHT, &h[%sU], sizeof(h[%sU]));" % (num_image, num_image, num_image))
                num_image+=1
            if Type.is_scalar_type(prm.type) is True :
                self.host_c_code.write_line("status = vxQueryScalar(scalar[%sU], VX_SCALAR_TYPE, &%s_%s, sizeof(%s_%s));" % (num_scalar, prm.type.name.lower(), num_scalar, prm.type.name.lower(), num_scalar))
                num_scalar+=1
            self.host_c_code.write_close_brace()
            self.host_c_code.write_newline()

        # If # of input images is = 2, validate that two input sizes are equal
        if self.kernel.getNumInputImages() == 2 :
            self.host_c_code.write_line("if (VX_SUCCESS == status)")
            self.host_c_code.write_open_brace()
            self.host_c_code.write_line("status = tivxKernelValidateInputSize(w[0U], w[1U], h[0U], h[1U]);")
            self.host_c_code.write_close_brace()
            self.host_c_code.write_newline()

        # Validate possible formats
        num_image = 0
        for prm in self.kernel.params :
            if Type.IMAGE == prm.type and Direction.INPUT == prm.direction :
                self.host_c_code.write_line("/* Check possible input image formats */")
                self.host_c_code.write_line("#if 0")
                self.host_c_code.write_line("if (VX_SUCCESS == status)")
                self.host_c_code.write_open_brace()
                self.host_c_code.write_line("status = tivxKernelValidatePossibleFormat(fmt[%sU], VX_DF_IMAGE_<possible_format>) &" % num_image)
                self.host_c_code.write_line("         tivxKernelValidatePossibleFormat(fmt[%sU], VX_DF_IMAGE_<possible_format>);" % num_image)
                num_image+=1
                self.host_c_code.write_close_brace()
                self.host_c_code.write_line("#endif")
                self.host_c_code.write_newline()

        # If there is at least 1 input image and 1 output image, validates each output image size
        # Checks if output size is equal to the input size
        if self.kernel.getNumInputImages() >= 1 and self.kernel.getNumOutputImages() >= 1 :
            for x in range(0, self.kernel.getNumOutputImages()) :
                self.host_c_code.write_line("if (VX_SUCCESS == status)")
                self.host_c_code.write_open_brace()
                temp = self.kernel.getNumOutputImages() - x
                self.host_c_code.write_line("status = tivxKernelValidateOutputSize(w[0U], w[%sU], h[0U], h[%sU], img[%sU]);" % (self.kernel.getNumImages()-temp, self.kernel.getNumImages()-temp, self.kernel.getNumImages()-temp) )
                self.host_c_code.write_close_brace()
                self.host_c_code.write_newline()

        num_scalar = 0
        for prm in self.kernel.params :
            if Type.is_scalar_type(prm.type) is True :
                self.host_c_code.write_line("if (VX_SUCCESS == status)")
                self.host_c_code.write_open_brace()
                self.host_c_code.write_line("status = tivxKernelValidateScalarType(%s_%s, %s);" % (prm.type.name.lower(), num_scalar, Type.get_vx_enum_name(prm.type)))
                num_scalar+=1
                self.host_c_code.write_close_brace()
                self.host_c_code.write_newline()

        # setting metas
        self.host_c_code.write_line("if (VX_SUCCESS == status)")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("tivxKernelSetMetas(metas, TIVX_KERNEL_%s_MAX_PARAMS, out_fmt, w[0U], h[0U]);" % self.kernel.name_upper)
        self.host_c_code.write_close_brace()

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
        self.host_c_code.write_line("tivxKernelValidRectParams prms;")
        self.host_c_code.write_newline()

        # Check number of parameters
        self.host_c_code.write_line("if (num_params != TIVX_KERNEL_%s_MAX_PARAMS)" % self.kernel.name_upper)
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("status = VX_ERROR_INVALID_PARAMETERS;")
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

        # Check if null params
        self.host_c_code.write_line("if (VX_SUCCESS == status)")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("status = tivxKernelValidateParametersNotNull(parameters, TIVX_KERNEL_%s_MAX_PARAMS);" % self.kernel.name_upper)
        self.host_c_code.write_close_brace()
        self.host_c_code.write_newline()

        # Config valid rectangle
        self.host_c_code.write_line("if (VX_SUCCESS == status)")
        self.host_c_code.write_open_brace()
        self.host_c_code.write_line("tivxKernelValidRectParams_init(&prms);")
        self.host_c_code.write_newline()

        # Set images
        num_input_image = 0
        num_output_image = 0
        for prm in self.kernel.params :
            if Type.IMAGE == prm.type and Direction.INPUT == prm.direction:
                self.host_c_code.write_line("prms.in_img[%sU] = (vx_image)parameters[TIVX_KERNEL_%s_%s_IDX];" %
                    (num_input_image, self.kernel.name_upper, prm.name_upper) )
                num_input_image+=1
            if Type.IMAGE == prm.type and Direction.OUTPUT == prm.direction:
                self.host_c_code.write_line("prms.out_img[%sU] = (vx_image)parameters[TIVX_KERNEL_%s_%s_IDX];" %
                    (num_output_image, self.kernel.name_upper, prm.name_upper) )
                num_output_image+=1

        self.host_c_code.write_newline()
        self.host_c_code.write_line("prms.num_input_images = %s;" % self.kernel.getNumInputImages())
        self.host_c_code.write_line("prms.num_output_images = %s;" % self.kernel.getNumOutputImages())
        self.host_c_code.write_newline()
        self.host_c_code.write_line("/* Developer TODO: Set padding values based on valid region*/")
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
        self.host_c_code.open()
        self.host_c_code.write_include("TI/tivx.h")
        self.host_c_code.write_include("VX/vx_types.h")
        self.host_c_code.write_include(self.h_filename)
        self.host_c_code.write_newline()
        self.host_c_code.write_line("static vx_kernel vx_%s_kernel = NULL;" % (self.kernel.name_lower))
        self.host_c_code.write_newline()
        self.generate_host_c_validate_func_code()
        self.generate_host_c_initialize_func_code()
        self.generate_host_c_add_func_code()
        self.generate_host_c_remove_func_code()
        self.host_c_code.close()

    def generate_target_c_add_func_code(self):
        self.target_c_code.write_line("void tivxAddTargetKernel%s()" % self.kernel.name_camel)
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
        self.target_c_code.write_line("                    VX_KERNEL_%s," % self.kernel.name_upper)
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
        self.target_c_code.write_line("void tivxRemoveTargetKernel%s()" % self.kernel.name_camel)
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
        self.target_c_code.write_line("vx_status VX_CALLBACK tivx%sCreate(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_delete_func_code(self):
        self.target_c_code.write_line("vx_status VX_CALLBACK tivx%sDelete(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_control_func_code(self):
        self.target_c_code.write_line("vx_status VX_CALLBACK tivx%sControl(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()
        self.target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_process_func_code(self):
        # define function name, and parameters
        self.target_c_code.write_line("vx_status VX_CALLBACK tivx%sProcess(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()

        # define status variables and obj descriptor variable
        self.target_c_code.write_line("vx_status status = VX_SUCCESS;")
        need_plane_idx_var = False
        for prm in self.kernel.params :
            if prm.do_map or prm.do_unmap :
                if prm.do_map_unmap_all_planes :
                    need_plane_idx_var = True
            self.target_c_code.write_line("%s *%s_desc;" % (Type.get_obj_desc_name(prm.type), prm.name_lower) )
        if need_plane_idx_var is True :
            self.target_c_code.write_line("uint16_t plane_idx;")
        self.target_c_code.write_newline()

        # checks function parameters
        self.target_c_code.write_line("if ( num_params != TIVX_KERNEL_%s_MAX_PARAMS" % self.kernel.name_upper )
        for prm in self.kernel.params :
            if prm.state is ParamState.REQUIRED :
                self.target_c_code.write_line("    || (NULL == obj_desc[TIVX_KERNEL_%s_%s_IDX])" % (self.kernel.name_upper, prm.name_upper))
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
            self.target_c_code.write_line("%s_desc = (%s *)obj_desc[TIVX_KERNEL_%s_%s_IDX];" %
                (prm.name_lower, Type.get_obj_desc_name(prm.type), self.kernel.name_upper, prm.name_upper) )
        self.target_c_code.write_newline()

        # convert descriptors pointer to target pointers
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if Type.is_scalar_type(prm.type) is False :
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_line("if( %s != NULL)" % desc)
                    self.target_c_code.write_open_brace()
                if prm.do_map or prm.do_unmap :
                    if prm.do_map_unmap_all_planes :
                        self.target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                        self.target_c_code.write_open_brace()
                        self.target_c_code.write_line("%s->mem_ptr[plane_idx].target_ptr = tivxMemShared2TargetPtr(" % desc )
                        self.target_c_code.write_line("  %s->mem_ptr[plane_idx].shared_ptr, %s->mem_ptr[plane_idx].mem_type);" % (desc, desc))
                        self.target_c_code.write_close_brace()
                    else:
                        self.target_c_code.write_line("%s->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(" % desc )
                        self.target_c_code.write_line("  %s->mem_ptr[0].shared_ptr, %s->mem_ptr[0].mem_type);" % (desc, desc))
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

        # map descriptors pointer
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if prm.do_map :
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_line("if( %s != NULL)" % desc)
                    self.target_c_code.write_open_brace()
                if prm.do_map_unmap_all_planes :
                    self.target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                    self.target_c_code.write_open_brace()
                    self.target_c_code.write_line("tivxMemBufferMap(%s->mem_ptr[plane_idx].target_ptr," % desc )
                    self.target_c_code.write_line("   %s->mem_size[plane_idx], %s->mem_ptr[plane_idx].mem_type," % (desc, desc))
                    self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                    self.target_c_code.write_close_brace()
                else:
                    self.target_c_code.write_line("tivxMemBufferMap(%s->mem_ptr[0].target_ptr," % desc )
                    self.target_c_code.write_line("   %s->mem_size[0], %s->mem_ptr[0].mem_type," % (desc, desc))
                    self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_close_brace()
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
        self.target_c_code.write_newline()
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
                if prm.do_map_unmap_all_planes :
                    self.target_c_code.write_line("for(plane_idx=0; plane_idx<%s->planes; plane_idx++)" % desc )
                    self.target_c_code.write_open_brace()
                    self.target_c_code.write_line("tivxMemBufferUnmap(%s->mem_ptr[plane_idx].target_ptr," % desc )
                    self.target_c_code.write_line("   %s->mem_size[plane_idx], %s->mem_ptr[plane_idx].mem_type," % (desc, desc))
                    self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                    self.target_c_code.write_close_brace()
                else:
                    self.target_c_code.write_line("tivxMemBufferUnmap(%s->mem_ptr[0].target_ptr," % desc )
                    self.target_c_code.write_line("   %s->mem_size[0], %s->mem_ptr[0].mem_type," % (desc, desc))
                    self.target_c_code.write_line("    %s);" % Direction.get_access_type(prm.direction))
                if prm.state is ParamState.OPTIONAL:
                    self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

        # set scalar values from local variables for output type scalars
        for prm in self.kernel.params :
            desc = prm.name_lower + "_desc"
            if (Type.is_scalar_type(prm.type) is True) and prm.direction != Direction.INPUT :
                self.target_c_code.write_line("%s->%s = %s_value" % (desc, Type.get_scalar_obj_desc_data_name(prm.type), prm.name_lower))
        self.target_c_code.write_newline()

        self.target_c_code.write_close_brace()

        self.target_c_code.write_newline()
        self.target_c_code.write_line("return status;")
        self.target_c_code.write_close_brace()
        self.target_c_code.write_newline()

    def generate_target_c_file_code(self):
        self.target_c_code.open()
        self.target_c_code.write_include("TI/tivx.h")
        self.target_c_code.write_include("VX/vx.h")
        self.target_c_code.write_include("tivx_openvx_core_kernels.h")
        self.target_c_code.write_include(self.h_filename)
        self.target_c_code.write_include("TI/tivx_target_kernel.h")
        self.target_c_code.write_include("ti/vxlib/vxlib.h")
        self.target_c_code.write_include("tivx_kernel_utils.h")
        self.target_c_code.write_newline()
        self.target_c_code.write_line("static tivx_target_kernel vx_%s_target_kernel = NULL;" % (self.kernel.name_lower))
        self.target_c_code.write_newline()
        self.generate_target_c_process_func_code()
        self.generate_target_c_create_func_code()
        self.generate_target_c_delete_func_code()
        self.generate_target_c_control_func_code()
        self.generate_target_c_add_func_code()
        self.generate_target_c_remove_func_code()
        self.target_c_code.close()

    def generate_bam_target_c_add_func_code(self):
        self.bam_target_c_code.write_line("void tivxAddTargetKernelBam%s()" % self.kernel.name_camel)
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
        self.bam_target_c_code.write_line("                    VX_KERNEL_%s," % self.kernel.name_upper)
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
        self.bam_target_c_code.write_line("void tivxRemoveTargetKernelBam%s()" % self.kernel.name_camel)
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
        self.bam_target_c_code.write_line("vx_status VX_CALLBACK tivx%sProcess(" % self.kernel.name_camel)
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
        self.bam_target_c_code.write_line("status = ownCheckNullParams(obj_desc, num_params,")
        self.bam_target_c_code.write_line("        TIVX_KERNEL_%s_MAX_PARAMS);" % self.kernel.name_upper)
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
            self.bam_target_c_code.write_line("%s_desc = (%s *)obj_desc[TIVX_KERNEL_%s_%s_IDX];" %
                (prm.name_lower, Type.get_obj_desc_name(prm.type), self.kernel.name_upper, prm.name_upper) )
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
                        self.bam_target_c_code.write_line("%s->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(" % desc )
                        self.bam_target_c_code.write_line("  %s->mem_ptr[0].shared_ptr, %s->mem_ptr[0].mem_type);" % (desc, desc))
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
                    self.bam_target_c_code.write_line("tivxMemBufferMap(%s->mem_ptr[0].target_ptr," % desc )
                    self.bam_target_c_code.write_line("   %s->mem_size[0], %s->mem_ptr[0].mem_type," % (desc, desc))
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
                    self.bam_target_c_code.write_line("tivxMemBufferUnmap(%s->mem_ptr[0].target_ptr," % desc )
                    self.bam_target_c_code.write_line("   %s->mem_size[0], %s->mem_ptr[0].mem_type," % (desc, desc))
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
        self.bam_target_c_code.write_line("vx_status VX_CALLBACK tivx%sCreate(" % self.kernel.name_camel)
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
        self.bam_target_c_code.write_line("status = ownCheckNullParams(obj_desc, num_params,")
        self.bam_target_c_code.write_line("        TIVX_KERNEL_%s_MAX_PARAMS);" % self.kernel.name_upper)
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
            self.bam_target_c_code.write_line("%s_desc = (%s *)obj_desc[TIVX_KERNEL_%s_%s_IDX];" %
                (prm.name_lower, Type.get_obj_desc_name(prm.type), self.kernel.name_upper, prm.name_upper) )
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
        self.bam_target_c_code.write_line("vx_status VX_CALLBACK tivx%sDelete(" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.bam_target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.bam_target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.bam_target_c_code.write_line("uint32_t size;")
        self.bam_target_c_code.write_line("tivx%sParams *prms = NULL;" % self.kernel.name_camel)
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("/* Check number of buffers and NULL pointers */")
        self.bam_target_c_code.write_line("status = ownCheckNullParams(obj_desc, num_params,")
        self.bam_target_c_code.write_line("        TIVX_KERNEL_%s_MAX_PARAMS);" % self.kernel.name_upper)
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
        self.bam_target_c_code.write_line("vx_status VX_CALLBACK tivx%sControl(" % self.kernel.name_camel)
        self.bam_target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.bam_target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.bam_target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.bam_target_c_code.write_open_brace()
        self.bam_target_c_code.write_line("vx_status status = VX_SUCCESS;")
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("return status;")
        self.bam_target_c_code.write_close_brace()
        self.bam_target_c_code.write_newline()

    def generate_bam_target_c_file_code(self):
        self.bam_target_c_code.open()
        self.bam_target_c_code.write_include("TI/tivx.h")
        self.bam_target_c_code.write_include("VX/vx.h")
        self.bam_target_c_code.write_include("tivx_openvx_core_kernels.h")
        self.bam_target_c_code.write_include(self.h_filename)
        self.bam_target_c_code.write_include("TI/tivx_target_kernel.h")
        self.bam_target_c_code.write_include("ti/vxlib/vxlib.h")
        self.bam_target_c_code.write_include("tivx_target_kernels_utils.h")
        self.bam_target_c_code.write_include("<vx_bam_kernel_wrapper.h")
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("typedef struct")
        self.bam_target_c_code.write_line("{")
        self.bam_target_c_code.write_line("    tivx_bam_graph_handle graph_handle;")
        self.bam_target_c_code.write_line("} tivx%sParams;" % (self.kernel.name_camel))
        self.bam_target_c_code.write_newline()
        self.bam_target_c_code.write_line("static tivx_target_kernel vx_%s_target_kernel = NULL;" % (self.kernel.name_lower))
        self.bam_target_c_code.write_newline()
        #add in bam process and fix create/delete
        self.generate_bam_target_c_process_func_code()
        self.generate_bam_target_c_create_func_code()
        self.generate_bam_target_c_delete_func_code()
        self.generate_bam_target_c_control_func_code()
        self.generate_bam_target_c_add_func_code()
        self.generate_bam_target_c_remove_func_code()
        self.bam_target_c_code.close()

    def export(self) :
        print ('Generating C code for OpenVX kernel ...')
        target_uses_dsp = False
        for target in self.kernel.targets :
            if target == Target.DSP1 or target == Target.DSP2 :
                target_uses_dsp = True
        if target_uses_dsp :
            print ('Files [%s] [%s] [%s] [%s]' % (self.h_filename, self.host_c_filename, self.target_c_filename, self.bam_target_c_filename))
        else :
            print ('Files [%s] [%s] [%s]' % (self.h_filename, self.host_c_filename, self.target_c_filename))
        print ()
        print (self.kernel)
        self.generate_h_file_code()
        self.generate_host_c_file_code()
        self.generate_target_c_file_code()
        if target_uses_dsp :
            self.generate_bam_target_c_file_code()
        print ('Generating C code for OpenVX kernel ... DONE !!!')

