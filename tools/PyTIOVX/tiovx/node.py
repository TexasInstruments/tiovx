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

## Node object (OpenVX equivalent = vx_node)
#
# This object is base class for specific nodes which inherit from this class.
# Some basic checks like parameter matching is done by this class.
# Inherited node classes can do additional parameter checking.
#
# Node is created by using below syntax.
#
# \code
#
# from tiovx import *
#
# my_node1 = Node<Name>(<data object param>, <data object param>, ..., name="<string>", target=Target.<Target Name>)
#
# # node needs to be added to a graph
# my_graph.add(my_node1)
#
# # a more compact way to the same is
# my_graph.add( Node<Name>(<data object param>, <data object param>, ..., name="<string>", target=Target.<Target Name>))
# \endcode
#
# Each node can take optional parameter of 'name' and 'target' as input. \n
# <b>'name'</b> is used to given a user readable name to the node which can be seen later in the generated code, image \n
# <b>'target'</b> is used to specify the target on which the node will run when executed. See tiovx.enums.Target for list of supported targets.
#
# \par Example Usage: Create nodes and add to graph
#
# \code
#
# from tiovx import *
#
# # create a node Sobel3x3 and add to graph object.
# # target is specified as DSP1
# graph.add ( NodeSobel3x3(in_image, grad_x, grad_y, target=Target.DSP1) )
#
# # create a node Phase and add to graph object.
# # target is specified as DSP2
# graph.add ( NodePhase(grad_x, grad_y, phase, target=Target.DSP2) )
#
# # create a custom node, as defined in next example and add to graph
# graph.add ( NodePhaseRgb(phase, phase_rgb, target=Target.DSP1) )
# \endcode
#
# Users can inherit from Node class to define their own custom nodes. \n
# <b>NOTE: </b>Users need not modify this file to inherit from Node class. \n
# Below code snippet shows one such example
#
# \par Example Usage: Create custom node class from base class
#
# \code
#
# from tiovx import *
#
# # inherit custom node class from tiobx.node.Node
# class NodePhaseRgb (Node) :
#     # implement constructor
#     # first list all input and output parameter data objects
#     # next provide optional parameters of name and target
#     def __init__(self, image_in, image_out, name="default", target=Target.DEFAULT) :
#         # call base class constructor with string of kernel name and list of parameters.
#         # This string of kernel name is later used to create the node
#         Node.__init__(self, "vx_tutorial_graph.phase_rgb", image_in, image_out)
#         # Tell base how many of the parameters are input and how many are output
#         # Also tell the data object type of the parameter
#         # This is used by base class to do type checking
#         self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
#         # Call base class API to set user provided target
#         self.setTarget(target)
#         # Tell base class that this is a user kernel and not a OpenVX specified kernel
#         self.setKernelEnumName("VX_USER_KERNEL")
#
#     # implement function to do parameter checking
#     # if not implemented then base class will do basic parameter checking like type checking
#     def checkParams(self, *param_type_args) :
#         # first call base class parameter checker
#         Node.checkParams(self, *param_type_args)
#         # Now add additional error conditions over the base class ones
#         assert ( self.ref[0].width    == self.ref[1].width ), "Input and Output width MUST match"
#         assert ( self.ref[0].height   == self.ref[1].height ), "Input and Output height MUST match"
#         assert ( self.ref[0].df_image == DfImage.U8 ), "Input data format must be U8"
#         assert ( self.ref[1].df_image == DfImage.RGB ), "Output data format must be RGB"
#
# \endcode
#
# Given below is the table of built-in kernels within PyTIOVX.
#
#    <TABLE frame="box" rules="all" cellspacing="0" width="50%" border="1" cellpadding="3">
#        <TR bgcolor="lightgrey">
#            <TD> Node class name </TD>
#            <TD> Parameter data object types (listed in order in which they need to be passed to the Node class constructor)</TD>        </TR>
#        </TR>
#        <TR>
#            <TD> NodeAbsDiff </TD>
#            <TD> [in] IMAGE  \n [in] IMAGE  \n [out] IMAGE \n</TD>
#        </TR>
#    </TABLE>
#
# \ingroup FRAMEWORK
class Node (Reference) :
    ## Constructor for base class
    #
    #  \param kernel [in] kernel name of type string. Used to create OpenVX node.
    #  \param args [in] Variable number of args list of parameter data objects
    def __init__(self, kernel, *args) :
        Reference.__init__(self, Type.NODE, "default")
        self.kernel = kernel
        self.ref = []
        self.param_dir = []
        self.target = Target.DEFAULT
        for arg in args :
            self.ref.append(arg)
        self.num_in = 0
        self.num_out = 0
        self.vx_kernel_enum = "VX_KERNEL_"

    ## Parameter checking function
    #
    #  Checks if number of parameters passed is correct
    #  Checks if data type of parameters passed is correct
    #
    #  \param param_type_args [in] Variable number of args list of type tiovx.enums.Type to specify data object types.s
    def checkParams(self, *param_type_args) :
        assert (len(param_type_args) == (self.num_in + self.num_out)), 'Expected %d arguments but %d provided' % (len(param_type_args), (self.num_in + self.num_out))
        for i in range(0, len(param_type_args)) :
            assert (self.ref[i].type == param_type_args[i]), 'Parameter %d: Expected %s but %s is provided' % (i, param_type_args[i], self.ref[i].type)

    ## Specify number of input/output parameters and data object type for each
    #
    # Assumes input parameters are followed by output parameters.
    # It is recommended user kernels follow this convention.
    #
    # \param num_in [in] Number of inputs
    # \param num_out [in] Number of outputs
    # \param param_type_args [in] Variable number of args list of type tiovx.enums.Type to specify data object types. \n
    #                             Number of arguments MUST match num_in+num_out
    def setParams(self, num_in, num_out, *param_type_args) :
        self.num_in = num_in
        self.num_out = num_out
        for i in range(0, self.num_in) :
            self.param_dir.append(Direction.INPUT)
        for i in range(0, self.num_out) :
            self.param_dir.append(Direction.OUTPUT)
        self.checkParams(*param_type_args)

    ## Specify target on which to run this node
    #
    # \param target [in] Object of type tiovx.enums.Target
    def setTarget(self, target):
        self.target = target

    def __str__(self):
        print_str = Reference.__str__(self) + ' [ ' + self.kernel + ' ] '
        idx = 0
        for ref in self.ref :
            print_str = print_str + '\n' + str(idx) + ': ' + str(ref)
            idx = idx + 1
        return print_str

    ## Specify kernel enum name to use
    #
    # Use "VX_USER_KERNEL" as 'kernel_enum_name' for custom/user kernels
    # \param kernel_enum_name [in] Type string.
    def setKernelEnumName(self, kernel_enum_name) :
        self.vx_kernel_enum = kernel_enum_name

    def get_vx_kernel_enum(self) :
        return self.vx_kernel_enum

class NodeAbsDiff (Node) :
    def __init__(self, image_in1, image_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.absdiff", image_in1, image_in2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_ABSDIFF");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Inputs MUST have same image data format"
        assert ( self.ref[1].df_image == self.ref[2].df_image ), "Inputs and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 or self.ref[0].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"

#TODO BIDI
class NodeAccumulateImage (Node) :
    def __init__(self, image_in1, image_inout2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.accumulate", image_in1, image_inout2, image_inout2)
        self.setParams(2, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_ACCUMULATE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[1].df_image == DfImage.S16 ), "In/out image data format must be S16"

#TODO BIDI
class NodeAccumulateSquareImage (Node) :
    def __init__(self, image_in1, shift_in2, image_inout3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.accumulate_square", image_in1, shift_in2, image_inout3, image_inout3)
        self.setParams(3, 1, Type.IMAGE, Type.SCALAR, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_ACCUMULATE_SQUARE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[2].df_image == DfImage.S16 ), "In/out image data format must be S16"

#TODO BIDI
class NodeAccumulateWeightedImage (Node) :
    def __init__(self, image_in1, alpha_in2, image_inout3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.accumulate_weighted", image_in1, alpha_in2, image_inout3, image_inout3)
        self.setParams(3, 1, Type.IMAGE, Type.SCALAR, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_ACCUMULATE_WEIGHTED");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[2].df_image == DfImage.U8 ), "In/out image data format must be U8"

class NodeAdd (Node) :
    def __init__(self, image_in1, image_in2, policy3, image_out4, name="default", target=Target.DEFAULT) :
        scalar = Scalar(Type.ENUM, policy3)
        Node.__init__(self, "org.khronos.openvx.add", image_in1, image_in2, scalar, image_out4)
        self.setParams(3, 1, Type.IMAGE, Type.IMAGE, Type.SCALAR, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_ADD");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 or self.ref[0].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"
        assert ( self.ref[1].df_image == DfImage.U8 or self.ref[1].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"
        assert ( self.ref[3].df_image == DfImage.U8 or self.ref[3].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"
        assert ( not((self.ref[0].df_image == DfImage.S16 or self.ref[1].df_image == DfImage.S16) and self.ref[3].df_image == DfImage.U8) ), "Output must be S16 if either input is S16"

class NodeSubtract (Node) :
    def __init__(self, image_in1, image_in2, policy, image_out3, name="default", target=Target.DEFAULT) :
        scalar = Scalar(Type.ENUM, policy)
        Node.__init__(self, "org.khronos.openvx.subtract", image_in1, image_in2, scalar, image_out3)
        self.setParams(3, 1, Type.IMAGE, Type.IMAGE, Type.SCALAR, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_SUBTRACT");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 or self.ref[0].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"
        assert ( self.ref[1].df_image == DfImage.U8 or self.ref[1].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"
        assert ( self.ref[3].df_image == DfImage.U8 or self.ref[3].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"
        assert ( not((self.ref[0].df_image == DfImage.S16 or self.ref[1].df_image == DfImage.S16) and self.ref[3].df_image == DfImage.U8) ), "Output must be S16 if either input is S16"

class NodeAnd (Node) :
    def __init__(self, image_in1, image_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.and", image_in1, image_in2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_AND");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Inputs MUST have same image data format"
        assert ( self.ref[1].df_image == self.ref[2].df_image ), "Inputs and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeXor (Node) :
    def __init__(self, image_in1, image_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.xor", image_in1, image_in2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_XOR");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Inputs MUST have same image data format"
        assert ( self.ref[1].df_image == self.ref[2].df_image ), "Inputs and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeOr (Node) :
    def __init__(self, image_in1, image_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.or", image_in1, image_in2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_OR");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Inputs MUST have same image data format"
        assert ( self.ref[1].df_image == self.ref[2].df_image ), "Inputs and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeNot (Node) :
    def __init__(self, image_in1, image_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.not", image_in1, image_out2)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_NOT");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Inputs and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeBox3x3 (Node) :
    def __init__(self, image_in1, image_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.box_3x3", image_in1, image_out2)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_BOX_3x3");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Inputs and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeCannyEdgeDetector (Node) :
    def __init__(self, image_in1, hyst_in2, grad_size_in3, norm_type_in4, image_out5, name="default", target=Target.DEFAULT) :
        scalar3 = Scalar(Type.INT32, grad_size_in3)
        scalar4 = Scalar(Type.ENUM, norm_type_in4)
        Node.__init__(self, "org.khronos.openvx.canny_edge_detector", image_in1, hyst_in2, scalar3, scalar4, image_out5)
        self.setParams(4, 1, Type.IMAGE, Type.THRESHOLD, Type.SCALAR, Type.SCALAR, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_CANNY_EDGE_DETECTOR");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[4].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

#TODO how to handle optional parameters?
class NodeChannelCombine (Node) :
    def __init__(self, image_in1, image_in2, image_in3, image_in4, image_out5, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.channel_combine", image_in1, image_in2, image_in3, image_in4, image_out5)
        self.setParams(4, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_CHANNEL_COMBINE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"
        assert ( self.ref[1].df_image == DfImage.U8 ), "Image data format must be U8"
        assert ( self.ref[2].df_image == DfImage.U8 ), "Image data format must be U8"
        assert ( self.ref[3].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeChannelExtract (Node) :
    def __init__(self, image_in1, channel2, image_out3, name="default", target=Target.DEFAULT) :
        scalar = Scalar(Type.ENUM, channel2)
        Node.__init__(self, "org.khronos.openvx.channel_extract", image_in1, scalar, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.SCALAR, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_CHANNEL_EXTRACT");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[2].df_image == DfImage.U8 ), "Output format must be U8"

class NodeColorConvert (Node) :
    def __init__(self, image_in1, image_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.color_convert", image_in1, image_out2)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_COLOR_CONVERT");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones

#TODO Order of params
class NodeConvertDepth (Node) :
    def __init__(self, image_in1, image_out2, policy3, shift4, name="default", target=Target.DEFAULT) :
        scalar3 = Scalar(Type.ENUM, policy3)
        Node.__init__(self, "org.khronos.openvx.convertdepth", image_in1, image_out2, scalar3, shift4)
        self.setParams(3, 1, Type.IMAGE, Type.IMAGE, Type.SCALAR, Type.SCALAR)
        self.param_dir[0] = Direction.INPUT;
        self.param_dir[1] = Direction.OUTPUT;
        self.param_dir[2] = Direction.INPUT;
        self.param_dir[3] = Direction.INPUT;
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_CONVERTDEPTH");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones

class NodeConvolve (Node) :
    def __init__(self, image_in1, conv2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.custom_convolution", image_in1, conv2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.CONVOLUTION, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_CUSTOM_CONVOLUTION");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input format must be U8"
        assert ( self.ref[2].df_image == DfImage.S16 ), "Output format must be S16"

class NodeDilate3x3 (Node) :
    def __init__(self, image_in1, image_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.dilate_3x3", image_in1, image_out2)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_DILATE_3x3");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeEqualizeHist (Node) :
    def __init__(self, image_in1, image_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.equalize_histogram", image_in1, image_out2)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_EQUALIZE_HISTOGRAM");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeErode3x3 (Node) :
    def __init__(self, image_in1, image_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.erode_3x3", image_in1, image_out2)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_ERODE_3x3");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

#TODO, this one also has an optional parameter (corners5)
class NodeFastCorners (Node) :
    def __init__(self, image_in1, strengh_thresh2, nonmax3, arr_out4, corners5, name="default", target=Target.DEFAULT) :
        scalar3 = Scalar(Type.ENUM, nonmax3)
        Node.__init__(self, "org.khronos.openvx.fast_corners", image_in1, strengh_thresh2, scalar3, arr_out4, corners5)
        self.setParams(3, 2, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.SCALAR)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_FAST_CORNERS");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[1].data_type == Type.FLOAT32 ), "Strength scalar format must be F32"

class NodeGaussian3x3 (Node) :
    def __init__(self, image_in1, image_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.gaussian_3x3", image_in1, image_out2)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_GAUSSIAN_3x3");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeNonLinearFilter (Node) :
    def __init__(self, function1, image_in2, matrix_in3, image_out4, name="default", target=Target.DEFAULT) :
        scalar1 = Scalar(Type.ENUM, function1)
        Node.__init__(self, "org.khronos.openvx.non_linear_filter", scalar1, image_in2, matrix_in3, image_out4)
        self.setParams(3, 1, Type.SCALAR, Type.IMAGE, Type.MATRIX, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_NON_LINEAR_FILTER");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[1].df_image == DfImage.U8 ), "Image data format must be U8"
        assert ( self.ref[3].df_image == DfImage.U8 ), "Image data format must be U8"

#TODO, this one also has an optional parameter (num_corners8)
class NodeHarrisCorners (Node) :
    def __init__(self, image_in1, strengh_thresh2, dist3, sensitivity4, gradient_size5, block_size6, arr_out7, num_corners8, name="default", target=Target.DEFAULT) :
        scalar5 = Scalar(Type.ENUM, gradient_size5)
        scalar6 = Scalar(Type.ENUM, block_size6)
        Node.__init__(self, "org.khronos.openvx.harris_corners", image_in1, strengh_thresh2, dist3, sensitivity4, scalar5, scalar6, arr_out7, num_corners8)
        self.setParams(6, 2, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.SCALAR, )
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_HARRIS_CORNERS");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[1].data_type == Type.FLOAT32 ), "Strength scalar format must be F32"
        assert ( self.ref[2].data_type == Type.FLOAT32 ), "Distance scalar format must be F32"
        assert ( self.ref[3].data_type == Type.FLOAT32 ), "Sensitivity scalar format must be F32"

class NodeHistogram (Node) :
    def __init__(self, image_in1, dist2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.histogram", image_in1, dist2)
        self.setParams(1, 1, Type.IMAGE, Type.DISTRIBUTION)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_HISTOGRAM");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"

class NodeGaussianPyramid (Node) :
    def __init__(self, image_in1, pyr_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.gaussian_pyramid", image_in1, pyr_out2)
        self.setParams(1, 1, Type.IMAGE, Type.PYRAMID)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_GAUSSIAN_PYRAMID");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].format ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeLaplacianPyramid (Node) :
    def __init__(self, image_in1, pyr_out2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.laplacian_pyramid", image_in1, pyr_out2, image_out3)
        self.setParams(1, 2, Type.IMAGE, Type.PYRAMID, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_LAPLACIAN_PYRAMID");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[1].format == DfImage.S16 ), "Output pyramid image data format must be S16"
        assert ( self.ref[2].df_image == DfImage.S16 ), "Output image data format must be S16"

class NodeLaplacianReconstruct (Node) :
    def __init__(self, pyr_in1, image_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.laplacian_reconstruct", pyr_in1, image_in2, image_out3)
        self.setParams(2, 1, Type.PYRAMID, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_LAPLACIAN_RECONSTRUCT");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].format == DfImage.S16 ), "Input pyramid image data format must be S16"
        assert ( self.ref[1].df_image == DfImage.S16 ), "Input image data format must be S16"
        assert ( self.ref[2].df_image == DfImage.U8 ), "Output image data format must be U8"

class NodeIntegralImage (Node) :
    def __init__(self, image_in1, image_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.integral_image", image_in1, image_out2)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_INTEGRAL_IMAGE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[1].df_image == DfImage.U32 ), "Output image data format must be U32"

class NodeMagnitude (Node) :
    def __init__(self, image_in1, image_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.magnitude", image_in1, image_in2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_MAGNITUDE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Inputs MUST have same image data format"
        assert ( self.ref[1].df_image == self.ref[2].df_image ), "Inputs and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.S16 ), "Image data format must be S16"

class NodeMeanStdDev (Node) :
    def __init__(self, image_in1, mean2, stddev3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.mean_stddev", image_in1, mean2, stddev3)
        self.setParams(1, 2, Type.IMAGE, Type.SCALAR, Type.SCALAR)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_MEAN_STDDEV");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[1].data_type == Type.FLOAT32 ), "Mean scalar format must be F32"
        assert ( self.ref[2].data_type == Type.FLOAT32 ), "Standard Deviation scalar format must be F32"

class NodeMedian3x3 (Node) :
    def __init__(self, image_in1, image_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.median_3x3", image_in1, image_out2)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_MEDIAN_3x3");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

#TODO Optional Parameters
class NodeMinMaxLoc (Node) :
    def __init__(self, image_in1, min2, max3, array_out4, array_out5, minCnt6, maxCnt7, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.minmaxloc", image_in1, min2, max3, array_out4, array_out5, minCnt6, maxCnt7)
        self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.ARRAY, Type.SCALAR, Type.SCALAR)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_MINMAXLOC");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 or self.ref[0].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"

#TODO Order of parameters
class NodeOpticalFlowPyrLK (Node) :
    def __init__(self, pyr_in1, pyr_in2, array_in3, array_in4, array_in5, termination6, epsilon7, num_iters8, use_initial_estimate9, window_dim10, name="default", target=Target.DEFAULT) :
        scalar6 = Scalar(Type.ENUM, termination6)
        scalar10 = Scalar(Type.ENUM, window_dim10)
        Node.__init__(self, "org.khronos.openvx.optical_flow_pyr_lk", pyr_in1, pyr_in2, array_in3, array_in4, scalar6, epsilon7, num_iters8, use_initial_estimate9, scalar10, array_in5)
        self.setParams(9, 1, Type.PYRAMID, Type.PYRAMID, Type.ARRAY, Type.ARRAY, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.ARRAY)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_OPTICAL_FLOW_PYR_LK");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].format == DfImage.U8 ), "Input pyramid image data format must be U8"
        assert ( self.ref[1].format == DfImage.U8 ), "Input pyramid image data format must be U8"
        assert ( self.ref[5].data_type == Type.FLOAT32 ), "Epsilon scalar format must be F32"

class NodePhase (Node) :
    def __init__(self, image_in1, image_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.phase", image_in1, image_in2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_PHASE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Inputs MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.S16 ), "Input image data format must be S16"
        assert ( self.ref[2].df_image == DfImage.U8 ), "Output image data format must be U8"

class NodeMultiply (Node) :
    def __init__(self, image_in1, image_in2, scale3, overflow4, rounding5, image_out6, name="default", target=Target.DEFAULT) :
        scalar4 = Scalar(Type.ENUM, overflow4)
        scalar5 = Scalar(Type.ENUM, rounding5)
        Node.__init__(self, "org.khronos.openvx.multiply", image_in1, image_in2, scale3, scalar4, scalar5, image_out6)
        self.setParams(5, 1, Type.IMAGE, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_MULTIPLY");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 or self.ref[0].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"
        assert ( self.ref[1].df_image == DfImage.U8 or self.ref[1].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"
        assert ( self.ref[5].df_image == DfImage.U8 or self.ref[5].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"
        assert ( not((self.ref[0].df_image == DfImage.S16 or self.ref[1].df_image == DfImage.S16) and self.ref[5].df_image == DfImage.U8) ), "Output must be S16 if either input is S16"

class NodeRemap (Node) :
    def __init__(self, image_in1, table_in2, policy3, image_out4, name="default", target=Target.DEFAULT) :
        scalar3 = Scalar(Type.ENUM, policy3)
        Node.__init__(self, "org.khronos.openvx.remap", image_in1, table_in2, scalar3, image_out4)
        self.setParams(3, 1, Type.IMAGE, Type.REMAP, Type.SCALAR, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_REMAP");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[3].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

#TODO Order of params
class NodeScaleImage (Node) :
    def __init__(self, image_in1, image_out2, interp3, name="default", target=Target.DEFAULT) :
        scalar = Scalar(Type.ENUM, interp3)
        Node.__init__(self, "org.khronos.openvx.scale_image", image_in1, scalar, image_out2)
        self.setParams(2, 1, Type.IMAGE, Type.SCALAR, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_SCALE_IMAGE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[2].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

#TODO Order of params
class NodeHalfScaleGaussian (Node) :
    def __init__(self, image_in1, image_out2, kernel_size, name="default", target=Target.DEFAULT) :
        scalar = Scalar(Type.ENUM, kernel_size)
        Node.__init__(self, "org.khronos.openvx.halfscale_gaussian", image_in1, scalar, image_out2)
        self.setParams(2, 1, Type.IMAGE, Type.SCALAR, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_HALFSCALE_GAUSSIAN");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[2].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

class NodeSobel3x3 (Node) :
    def __init__(self, image_in1, image_out2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.sobel_3x3", image_in1, image_out2, image_out3)
        self.setParams(1, 2, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_SOBEL_3x3");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[1].df_image == self.ref[2].df_image ), "Output images MUST have same image data format"
        assert ( self.ref[1].df_image == DfImage.S16 ), "Output image data format must be S16"

class NodeTableLookup (Node) :
    def __init__(self, image_in1, lut_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.table_lookup", image_in1, lut_in2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.LUT, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_TABLE_LOOKUP");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[2].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 or self.ref[0].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"

class NodeThreshold (Node) :
    def __init__(self, image_in1, thresh_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.threshold", image_in1, thresh_in2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.THRESHOLD, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_THRESHOLD");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"

class NodeWarpAffine (Node) :
    def __init__(self, image_in1, matrix_in2, interp3, image_out4, name="default", target=Target.DEFAULT) :
        scalar = Scalar(Type.ENUM, interp3)
        Node.__init__(self, "org.khronos.openvx.warp_affine", image_in1, matrix_in2, scalar, image_out4)
        self.setParams(3, 1, Type.IMAGE, Type.MATRIX, Type.SCALAR, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_WARP_AFFINE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[3].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"
        assert ( self.ref[1].data_type == Type.FLOAT32 ), "Matrix data format must be F32"

class NodeWarpPerspective (Node) :
    def __init__(self, image_in1, matrix_in2, interp3, image_out4, name="default", target=Target.DEFAULT) :
        scalar = Scalar(Type.ENUM, interp3)
        Node.__init__(self, "org.khronos.openvx.warp_perspective", image_in1, matrix_in2, scalar, image_out4)
        self.setParams(3, 1, Type.IMAGE, Type.MATRIX, Type.SCALAR, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_WARP_PERSPECTIVE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[3].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"
        assert ( self.ref[1].data_type == Type.FLOAT32 ), "Matrix data format must be F32"
