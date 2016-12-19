'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class KernelExportCode :
    def __init__(self, kernel) :
        self.kernel = kernel
        self.h_filename = "tivx_kernel_" + kernel.name_lower + ".h";
        self.host_c_filename = "vx_" + kernel.name_lower + "_host.c";
        self.target_c_filename = "vx_" + kernel.name_lower + "_target.c";

        self.h_code = CodeGenerate(self.h_filename)
        self.host_c_code = CodeGenerate(self.host_c_filename)
        self.target_c_code = CodeGenerate(self.target_c_filename)

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
        self.host_c_code.write_line("            NULL,")
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
        self.host_c_code.write_line("vx_status status = VX_SUCCESS;")
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
        self.target_c_code.write_line("                    tivx%s," % self.kernel.name_camel)
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
        self.target_c_code.write_line("vx_status tivx%sCreate(" % self.kernel.name_camel)
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
        self.target_c_code.write_line("vx_status tivx%sDelete(" % self.kernel.name_camel)
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
        self.target_c_code.write_line("vx_status tivx%sControl(" % self.kernel.name_camel)
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
        self.target_c_code.write_line("vx_status tivx%s(" % self.kernel.name_camel)
        self.target_c_code.write_line("       tivx_target_kernel_instance kernel,")
        self.target_c_code.write_line("       tivx_obj_desc_t *obj_desc[],")
        self.target_c_code.write_line("       uint16_t num_params, void *priv_arg)")
        self.target_c_code.write_open_brace()

        # define status variables and obj descriptor variable
        self.target_c_code.write_line("vx_status status = VX_SUCCESS;")
        need_plane_idx_var = False
        for prm in self.kernel.params :
            if prm.do_map_umnap_all_planes :
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
                if prm.do_map_umnap_all_planes :
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
                if prm.do_map_umnap_all_planes :
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
                if prm.do_map_umnap_all_planes :
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

    def export(self) :
        print ('Generating C code for OpenVX kernel ...')
        print ('Files [%s] [%s] [%s]' % (self.h_filename, self.host_c_filename, self.target_c_filename))
        print ()
        print (self.kernel)
        self.generate_h_file_code()
        self.generate_host_c_file_code()
        self.generate_target_c_file_code()
        print ('Generating C code for OpenVX kernel ... DONE !!!')

