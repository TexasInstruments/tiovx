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
#        <TR>
#            <TD>  NodeAccumulateImage </TD>
#            <TD> [in] IMAGE \n [in] IMAGE \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeAccumulateSquareImage \n NodeAccumulateWeightedImage </TD>
#            <TD> [in] IMAGE \n [in] SCALAR \n [in] IMAGE \n [out] IMAGE</TD>
#        </TR>
#        <TR>
#            <TD> NodeAdd \n NodeSubtract</TD>
#            <TD> [in] IMAGE \n [in] IMAGE \n [in] SCALAR \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeAnd \n NodeXor \n NodeOr </TD>
#            <TD> [in] IMAGE \n [in] IMAGE \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeNot  </TD>
#            <TD> [in] IMAGE \n [out] IMAGE\n</TD>
#        </TR>
#        <TR>
#            <TD> NodeBox3x3 \n NodeDilate3x3 \n NodeErode3x3 \n NodeGaussian3x3 \n NodeMedian3x3</TD>
#            <TD> [in] IMAGE \n [out] IMAGE\n</TD>
#        </TR>
#        <TR>
#            <TD> NodeCannyEdgeDetector  </TD>
#            <TD> [in] IMAGE \n [in] THRESHOLD \n [in] SCALAR \n [in] SCALAR \n [in] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeChannelCombine </TD>
#            <TD> [in] IMAGE \n [in] IMAGE \n [in] IMAGE \n [in] IMAGE \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeChannelExtract  </TD>
#            <TD> [in] IMAGE \n [in] SCALAR \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD>  NodeColorConvert </TD>
#            <TD> [in] IMAGE \n [out] IMAGE\n</TD>
#        </TR>
#        <TR>
#            <TD>  NodeConvertDepth </TD>
#            <TD> [in] IMAGE \n [out] IMAGE\n [in] ENUM \n [in] SCALAR</TD>
#        </TR>
#        <TR>
#            <TD>  NodeConvolve </TD>
#            <TD> [in] IMAGE \n [in] CONVOLUTION \n [out] IMAGE </TD>
#        </TR>
#        <TR>
#            <TD> NodeEqualizeHist  </TD>
#            <TD> [in] IMAGE \n [in] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeFastCorners  </TD>
#            <TD> [in] IMAGE \n [in] SCALAR \n [in] SCALAR \n [out] ARRAY \n [out] SCALAR \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeNonLinearFilter  </TD>
#            <TD> [in] SCALAR \n [in] IMAGE \n [in] MATRIX \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeHarrisCorners  </TD>
#            <TD> [in] IMAGE \n [in] SCALAR \n [in] SCALAR \n [in] SCALAR \n [in] SCALAR \n [in] SCALAR \n [out] ARRAY \n [out] SCALAR \n </TD>
#        </TR>
#        <TR>
#            <TD> NodeHistogram  </TD>
#            <TD> [in] IMAGE \n [out] DISTRIBUTION \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeGaussianPyramid  </TD>
#            <TD> [in] IMAGE \n [out] PYRAMID \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeLaplacianPyramid  </TD>
#            <TD> [in] IMAGE \n [out] PYRAMID \n [out] IMAGE</TD>
#        </TR>
#        <TR>
#            <TD> NodeLaplacianReconstruct  </TD>
#            <TD> [in] PYRAMID \n [in] IMAGE \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD>  NodeIntegralImage </TD>
#            <TD> [in] IMAGE \n [out] IMAGE\n</TD>
#        </TR>
#        <TR>
#            <TD> NodeMagnitude \n NodePhase </TD>
#            <TD> [in] IMAGE \n [in] IMAGE \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeMeanStdDev  </TD>
#            <TD> [in] IMAGE \n [out] SCALAR \n [out] SCALAR \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeMinMaxLoc  </TD>
#            <TD> [in] IMAGE \n [out] SCALAR \n [out] SCALAR \n [out] ARRAY \n [out] ARRAY \n [out] SCALAR \n [out] SCALAR \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeOpticalFlowPyrLK  </TD>
#            <TD> [in] PYRAMID \n [in] PYRAMID \n [in] ARRAY \n [in] ARRAY \n [in] SCALAR \n [in] SCALAR \n [in] SCALAR \n [in] SCALAR \n [in] SCALAR \n [out] ARRAY \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeMultiply  </TD>
#            <TD> [in] IMAGE \n [in] IMAGE \n [in] SCALAR \n [in] SCALAR \n [in] SCALAR \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeRemap  </TD>
#            <TD> [in] IMAGE \n [in] REMAP \n [in] SCALAR \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeScaleImage \n NodeHalfScaleGaussian </TD>
#            <TD> [in] IMAGE \n [out] IMAGE \n [in] SCALAR \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeSobel3x3  </TD>
#            <TD> [in] IMAGE \n [out] IMAGE \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeTableLookup  </TD>
#            <TD> [in] IMAGE \n [in] LUT \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeThreshold  </TD>
#            <TD> [in] IMAGE \n [in] THRESHOLD \n [out] IMAGE \n</TD>
#        </TR>
#        <TR>
#            <TD> NodeWarpAffine \n NodeWarpPerspective  </TD>
#            <TD> [in] IMAGE \n [in] MATRIX \n [in] SCALAR \n [out] IMAGE</TD>
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
    #  \param param_type_args [in] Variable number of args list of type tiovx.enums.Type to specify data object types
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

## Node object used to generate an absdiff node
#
# \par Example Usage: Creating absdiff node
#
# \ingroup NODE
#
class NodeAbsDiff (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] First input image
    # \param image_in2      [in] Second input image
    # \param image_out3     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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
## Node object used to generate an accumulate node
#
# \par Example Usage: Creating accumulate node
#
# \ingroup NODE
#
class NodeAccumulateImage (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_inout2   [in] In/out accumulate image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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
## Node object used to generate an accumulate square node
#
# \par Example Usage: Creating accumulate square node
#
# \ingroup NODE
#
class NodeAccumulateSquareImage (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param shift_in2      [in] Scalar input
    # \param image_inout3   [in] In/out accumulate image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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
## Node object used to generate an accumulate weighted node
#
# \par Example Usage: Creating accumulate weighted node
#
# \ingroup NODE
#
class NodeAccumulateWeightedImage (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param alpha_in2      [in] Alpha scalar input
    # \param image_inout3   [in] In/out accumulate image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate an add node
#
# \par Example Usage: Creating add node
#
# \ingroup NODE
#
class NodeAdd (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] First input image
    # \param image_in2      [in] Second input image
    # \param policy3        [in] Addition policy
    # \param image_out4     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a subtract node
#
# \par Example Usage: Creating subtract node
#
# \ingroup NODE
#
class NodeSubtract (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] First input image
    # \param image_in2      [in] Second input image
    # \param policy3        [in] Subtraction policy
    # \param image_out4     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate an AND node
#
# \par Example Usage: Creating AND node
#
# \ingroup NODE
#
class NodeAnd (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] First input image
    # \param image_in2      [in] Second input image
    # \param image_out3     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate an XOR node
#
# \par Example Usage: Creating XOR node
#
# \ingroup NODE
#
class NodeXor (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] First input image
    # \param image_in2      [in] Second input image
    # \param image_out3     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate an OR node
#
# \par Example Usage: Creating OR node
#
# \ingroup NODE
#
class NodeOr (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] First input image
    # \param image_in2      [in] Second input image
    # \param image_out3     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a NOT node
#
# \par Example Usage: Creating NOT node
#
# \ingroup NODE
#
class NodeNot (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Box3x3 node
#
# \par Example Usage: Creating Box3x3 node
#
# \ingroup NODE
#
class NodeBox3x3 (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Canny Edge Detector node
#
# \par Example Usage: Creating Canny Edge Detector node
#
# \ingroup NODE
#
class NodeCannyEdgeDetector (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param hyst_in2       [in] Threshold input
    # \param grad_size_in3  [in] Gradient scalar input
    # \param norm_type_in4  [in] Norm type scalar image
    # \param image_out5     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Channel Combine node
#
# \par Example Usage: Creating Channel Combine node
#
# \ingroup NODE
#
class NodeChannelCombine (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] First input image
    # \param image_in2      [in] Second input image
    # \param image_in3      [in] Third input image; optional from a node point of view
    # \param image_in4      [in] Fourth input image; optional from a node point of view
    # \param image_out5     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, image_in1, image_in2, image_in3, image_in4, image_out5, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.channel_combine", image_in1, image_in2, image_in3, image_in4, image_out5)
        # modify based on null type?
        if image_in4.type == Type.NULL :
            if image_in3.type == Type.NULL :
                self.setParams(4, 1, Type.IMAGE, Type.IMAGE, Type.NULL, Type.NULL, Type.IMAGE)
            else :
                self.setParams(4, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE, Type.NULL, Type.IMAGE)
        else :
            self.setParams(4, 1, Type.IMAGE, Type.IMAGE, Type.IMAGE, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_CHANNEL_COMBINE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"
        assert ( self.ref[1].df_image == DfImage.U8 ), "Image data format must be U8"
        if param_type_args[2] == Type.IMAGE :
            assert ( self.ref[2].df_image == DfImage.U8 ), "Image data format must be U8"
        if param_type_args[3] == Type.IMAGE :
            assert ( self.ref[3].df_image == DfImage.U8 ), "Image data format must be U8"
        assert ( self.ref[4].df_image != DfImage.U8 ), "Image data format must not be U8"

## Node object used to generate a Channel Extract node
#
# \par Example Usage: Creating Channel Extract node
#
# \ingroup NODE
#
class NodeChannelExtract (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] First input image
    # \param channel2       [in] Channel to extract
    # \param image_out3     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Color Convert node
#
# \par Example Usage: Creating Color Convert node
#
# \ingroup NODE
#
class NodeColorConvert (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, image_in1, image_out2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.color_convert", image_in1, image_out2)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_COLOR_CONVERT");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones

#TODO Order of params
## Node object used to generate a Convert Depth node
#
# \par Example Usage: Creating Convert Depth node
#
# \ingroup NODE
#
class NodeConvertDepth (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param policy3        [in] Depth policy
    # \param shift4         [in] Scalar shift value
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Custom Convolution node
#
# \par Example Usage: Creating Custom Convolution node
#
# \ingroup NODE
#
class NodeConvolve (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param conv2          [in] Convolution input
    # \param image_out3     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, image_in1, conv2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.custom_convolution", image_in1, conv2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.CONVOLUTION, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_CUSTOM_CONVOLUTION");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input format must be U8"
        assert ( self.ref[2].df_image == DfImage.U8 or self.ref[2].df_image == DfImage.S16 ), "Output format must be either U8 or S16"

## Node object used to generate a Dilate3x3 node
#
# \par Example Usage: Creating Dilate3x3 node
#
# \ingroup NODE
#
class NodeDilate3x3 (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate an Equalize Histogram node
#
# \par Example Usage: Creating Equalize Histogram node
#
# \ingroup NODE
#
class NodeEqualizeHist (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate an Erode3x3 node
#
# \par Example Usage: Creating Erode3x3 node
#
# \ingroup NODE
#
class NodeErode3x3 (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Fast Corners node
#
# \par Example Usage: Creating Fast Corners node
#
# \ingroup NODE
#
class NodeFastCorners (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1        [in] Input image
    # \param strength_thresh2 [in] Strength threshold scalar input
    # \param nonmax3          [in] Nonmax suppression boolean input
    # \param arr_out4         [in] Output array
    # \param corners5         [in] Number of corners scalar output; optional from a node point of view
    # \param name             [in] [optional] Name of the node; Default="default"
    # \param target           [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, image_in1, strength_thresh2, nonmax3, arr_out4, corners5, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.fast_corners", image_in1, strength_thresh2, nonmax3, arr_out4, corners5)
        if corners5.type == Type.NULL :
            self.setParams(3, 2, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.NULL)
        else :
            self.setParams(3, 2, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.SCALAR)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_FAST_CORNERS");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[1].data_type == Type.FLOAT32 ), "Strength scalar format must be F32"

## Node object used to generate a Gaussian3x3 node
#
# \par Example Usage: Creating Gaussian3x3 node
#
# \ingroup NODE
#
class NodeGaussian3x3 (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Nonlinear Filter node
#
# \par Example Usage: Creating Nonlinear Filter node
#
# \ingroup NODE
#
class NodeNonLinearFilter (Node) :
    ## Constructor used to create this object
    #
    # \param function1       [in] Scalar input of nonlinear function
    # \param image_in2       [in] Input image
    # \param matrix_in3      [in] Matrix mask input
    # \param image_out4      [in] Output array
    # \param name            [in] [optional] Name of the node; Default="default"
    # \param target          [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Harris Corners node
#
# \par Example Usage: Creating Harris Corners node
#
# \ingroup NODE
#
class NodeHarrisCorners (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1        [in] Input image
    # \param strengh_thresh2  [in] Strength threshold scalar input
    # \param dist3            [in] Min distance scalar input
    # \param sensitivity4     [in] Sensitivity scalar input
    # \param gradient_size5   [in] Gradient size scalar input
    # \param block_size6      [in] Block window size scalar input
    # \param arr_out7         [in] Output corners array
    # \param num_corners8     [in] Output number of corners scalar; optional from a node point of view
    # \param name             [in] [optional] Name of the node; Default="default"
    # \param target           [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, image_in1, strength_thresh2, dist3, sensitivity4, gradient_size5, block_size6, arr_out7, num_corners8, name="default", target=Target.DEFAULT) :
        scalar5 = Scalar(Type.INT32, gradient_size5)
        scalar6 = Scalar(Type.INT32, block_size6)
        Node.__init__(self, "org.khronos.openvx.harris_corners", image_in1, strength_thresh2, dist3, sensitivity4, scalar5, scalar6, arr_out7, num_corners8)
        if num_corners8.type == Type.NULL :
            self.setParams(6, 2, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.NULL)
        else :
            self.setParams(6, 2, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.SCALAR)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_HARRIS_CORNERS");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"
        assert ( self.ref[1].data_type == Type.FLOAT32 ), "Strength scalar format must be F32"
        assert ( self.ref[2].data_type == Type.FLOAT32 ), "Distance scalar format must be F32"
        assert ( self.ref[3].data_type == Type.FLOAT32 ), "Sensitivity scalar format must be F32"

## Node object used to generate a Histogram node
#
# \par Example Usage: Creating Histogram node
#
# \ingroup NODE
#
class NodeHistogram (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1   [in] Input image
    # \param dist2       [in] Output histogram distribution
    # \param name        [in] [optional] Name of the node; Default="default"
    # \param target      [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, image_in1, dist2, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.histogram", image_in1, dist2)
        self.setParams(1, 1, Type.IMAGE, Type.DISTRIBUTION)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_HISTOGRAM");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"

## Node object used to generate a Gaussian Pyramid node
#
# \par Example Usage: Creating Gaussian Pyramid node
#
# \ingroup NODE
#
class NodeGaussianPyramid (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1   [in] Input image
    # \param dist2       [in] Output histogram distribution
    # \param name        [in] [optional] Name of the node; Default="default"
    # \param target      [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Laplacian Pyramid node
#
# \par Example Usage: Creating Laplacian Pyramid node
#
# \ingroup NODE
#
class NodeLaplacianPyramid (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1   [in] Input image
    # \param pyr_out2    [in] Output pyramid
    # \param image_in1   [in] Output image
    # \param name        [in] [optional] Name of the node; Default="default"
    # \param target      [in] [optional] Default core to run on; Default="Target.DEFAULT"
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
        assert ( self.ref[2].df_image == DfImage.S16 or self.ref[2].df_image == DfImage.U8 ), "Output image data format must be either U8 or S16"

## Node object used to generate a Laplacian Reconstruct node
#
# \par Example Usage: Creating Laplacian Reconstruct node
#
# \ingroup NODE
#
class NodeLaplacianReconstruct (Node) :
    ## Constructor used to create this object
    #
    # \param pyr_in1     [in] Input pyramid
    # \param image_in2   [in] Input image
    # \param image_out3  [in] Output image
    # \param name        [in] [optional] Name of the node; Default="default"
    # \param target      [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, pyr_in1, image_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.laplacian_reconstruct", pyr_in1, image_in2, image_out3)
        self.setParams(2, 1, Type.PYRAMID, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_LAPLACIAN_RECONSTRUCT");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].format == DfImage.S16 ), "Input pyramid image data format must be S16"
        assert ( self.ref[1].df_image == DfImage.S16 or self.ref[1].df_image == DfImage.U8), "Input image data format must be either U8 or S16"
        assert ( self.ref[2].df_image == DfImage.U8 ), "Output image data format must be U8"

## Node object used to generate an Integral Image node
#
# \par Example Usage: Creating Integral Image node
#
# \ingroup NODE
#
class NodeIntegralImage (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Magnitude node
#
# \par Example Usage: Creating Magnitude node
#
# \ingroup NODE
#
class NodeMagnitude (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] First input image
    # \param image_in2      [in] Second input image
    # \param image_out3     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Mean Standard Deviation node
#
# \par Example Usage: Creating Mean Standard Deviation node
#
# \ingroup NODE
#
class NodeMeanStdDev (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1   [in] Input image
    # \param mean2       [in] Mean scalar output
    # \param stddev3     [in] Standard Deviation scalar output
    # \param name        [in] [optional] Name of the node; Default="default"
    # \param target      [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Median3x3 node
#
# \par Example Usage: Creating Median3x3 node
#
# \ingroup NODE
#
class NodeMedian3x3 (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Min Max Location node
#
# \par Example Usage: Creating Min Max Location node
#
# \ingroup NODE
#
class NodeMinMaxLoc (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1    [in] Input image
    # \param min2         [in] Min output scalar
    # \param max3         [in] Max output scalar
    # \param array_out4   [in] Min loc output array
    # \param array_out5   [in] Max loc output array
    # \param minCnt6      [in] Min count output scalar; optional from a node point of view
    # \param maxCnt7      [in] Max count output scalar; optional from a node point of view
    # \param name         [in] [optional] Name of the node; Default="default"
    # \param target       [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, image_in1, min2, max3, array_out4, array_out5, minCnt6, maxCnt7, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.minmaxloc", image_in1, min2, max3, array_out4, array_out5, minCnt6, maxCnt7)

        # 0000
        if array_out4.type == Type.NULL and array_out5.type == Type.NULL and \
           minCnt6.type == Type.NULL and maxCnt7.type == Type.NULL :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.NULL, Type.NULL, Type.NULL, Type.NULL)
        #0001
        elif array_out4.type == Type.ARRAY and array_out5.type == Type.NULL and \
           minCnt6.type == Type.NULL and maxCnt7.type == Type.NULL :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.NULL, Type.NULL, Type.NULL)
        #0010
        elif array_out4.type == Type.NULL and array_out5.type == Type.ARRAY and \
           minCnt6.type == Type.NULL and maxCnt7.type == Type.NULL :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.NULL, Type.ARRAY, Type.NULL, Type.NULL)
        #0011
        elif array_out4.type == Type.ARRAY and array_out5.type == Type.ARRAY and \
           minCnt6.type == Type.NULL and maxCnt7.type == Type.NULL :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.ARRAY, Type.NULL, Type.NULL)
        #0100
        elif array_out4.type == Type.NULL and array_out5.type == Type.NULL and \
           minCnt6.type == Type.SCALAR and maxCnt7.type == Type.NULL :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.NULL, Type.NULL, Type.SCALAR, Type.NULL)
        #0101
        elif array_out4.type == Type.ARRAY and array_out5.type == Type.NULL and \
           minCnt6.type == Type.SCALAR and maxCnt7.type == Type.NULL :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.ARRAY, Type.NULL, Type.NULL, Type.SCALAR, Type.NULL)
        #0110
        elif array_out4.type == Type.NULL and array_out5.type == Type.ARRAY and \
           minCnt6.type == Type.SCALAR and maxCnt7.type == Type.NULL :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.NULL, Type.ARRAY, Type.NULL, Type.SCALAR, Type.NULL)
        #0111
        elif array_out4.type == Type.ARRAY and array_out5.type == Type.ARRAY and \
           minCnt6.type == Type.SCALAR and maxCnt7.type == Type.NULL :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.ARRAY, Type.ARRAY, Type.NULL, Type.SCALAR, Type.NULL)
        #1000
        elif array_out4.type == Type.NULL and array_out5.type == Type.NULL and \
           minCnt6.type == Type.NULL and maxCnt7.type == Type.SCALAR :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.NULL, Type.NULL, Type.NULL, Type.SCALAR)
        #1001
        elif array_out4.type == Type.ARRAY and array_out5.type == Type.NULL and \
           minCnt6.type == Type.NULL and maxCnt7.type == Type.SCALAR :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.NULL, Type.NULL, Type.SCALAR)
        #1010
        elif array_out4.type == Type.NULL and array_out5.type == Type.ARRAY and \
           minCnt6.type == Type.NULL and maxCnt7.type == Type.SCALAR :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.NULL, Type.ARRAY, Type.NULL, Type.SCALAR)
        #1011
        elif array_out4.type == Type.ARRAY and array_out5.type == Type.ARRAY and \
           minCnt6.type == Type.NULL and maxCnt7.type == Type.SCALAR :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.ARRAY, Type.NULL, Type.SCALAR)
        #1100
        elif array_out4.type == Type.NULL and array_out5.type == Type.NULL and \
           minCnt6.type == Type.SCALAR and maxCnt7.type == Type.SCALAR :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.NULL, Type.NULL, Type.SCALAR, Type.SCALAR)
        #1101
        elif array_out4.type == Type.ARRAY and array_out5.type == Type.NULL and \
           minCnt6.type == Type.SCALAR and maxCnt7.type == Type.SCALAR :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.NULL, Type.SCALAR, Type.SCALAR)
        #1110
        elif array_out4.type == Type.NULL and array_out5.type == Type.ARRAY and \
           minCnt6.type == Type.SCALAR and maxCnt7.type == Type.SCALAR :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.NULL, Type.ARRAY, Type.SCALAR, Type.SCALAR)
        #1111
        else :
            self.setParams(1, 6, Type.IMAGE, Type.SCALAR, Type.SCALAR, Type.ARRAY, Type.ARRAY, Type.SCALAR, Type.SCALAR)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_MINMAXLOC");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 or self.ref[0].df_image == DfImage.S16 ), "Image data format must be either U8 or S16"

#TODO Order of parameters
## Node object used to generate an Optical Flow node
#
# \par Example Usage: Creating Optical Flow node
#
# \ingroup NODE
#
class NodeOpticalFlowPyrLK (Node) :
    ## Constructor used to create this object
    #
    # \param pyr_in1                 [in] First input pyramid
    # \param pyr_in2                 [in] Second input pyramid
    # \param array_in3               [in] First input points array
    # \param array_in4               [in] Second input points array
    # \param array_out5              [in] Output points array
    # \param termination6            [in] Input scalar termination value
    # \param epsilon7                [in] Input scalar epsilon value
    # \param num_iters8              [in] Number of iterations scalar input
    # \param use_initial_estimate9   [in] Boolean input
    # \param window_dim10            [in] Window size scalar input
    # \param name                    [in] [optional] Name of the node; Default="default"
    # \param target                  [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, pyr_in1, pyr_in2, array_in3, array_in4, array_out5, termination6, epsilon7, num_iters8, use_initial_estimate9, window_dim10, name="default", target=Target.DEFAULT) :
        scalar6 = Scalar(Type.ENUM, termination6)
        epsilon7 = Scalar(Type.FLOAT32, epsilon7)
        num_iters8 = Scalar(Type.UINT32, num_iters8)
        use_initial_estimate9 = Scalar(Type.BOOL, use_initial_estimate9)
        scalar10 = Scalar(Type.SIZE, window_dim10)
        Node.__init__(self, "org.khronos.openvx.optical_flow_pyr_lk", pyr_in1, pyr_in2, array_in3, array_in4, array_in5, scalar6, epsilon7, num_iters8, use_initial_estimate9, scalar10)
        self.setParams(9, 1, Type.PYRAMID, Type.PYRAMID, Type.ARRAY, Type.ARRAY, Type.ARRAY, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.SCALAR, Type.SCALAR)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_OPTICAL_FLOW_PYR_LK");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].format == DfImage.U8 ), "Input pyramid image data format must be U8"
        assert ( self.ref[1].format == DfImage.U8 ), "Input pyramid image data format must be U8"
        assert ( self.ref[6].data_type == Type.FLOAT32 ), "Epsilon scalar format must be F32"

## Node object used to generate a Phase node
#
# \par Example Usage: Creating Phase node
#
# \ingroup NODE
#
class NodePhase (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1       [in] First input image
    # \param image_in2       [in] Second input image
    # \param image_out3      [in] Output image
    # \param name            [in] [optional] Name of the node; Default="default"
    # \param target          [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Multiply node
#
# \par Example Usage: Creating Multiply node
#
# \ingroup NODE
#
class NodeMultiply (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1       [in] First input image
    # \param image_in2       [in] Second input image
    # \param scale3          [in] Input scalar
    # \param overflow4       [in] Input scalar overflow policy
    # \param rounding5       [in] Input scalar rounding policy
    # \param image_out6      [in] Output image
    # \param name            [in] [optional] Name of the node; Default="default"
    # \param target          [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Remap node
#
# \par Example Usage: Creating Remap node
#
# \ingroup NODE
#
class NodeRemap (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1       [in] First input image
    # \param table_in2       [in] Input remap table
    # \param policy3         [in] Interpolation policy input scalar
    # \param image_out4      [in] Output image
    # \param name            [in] [optional] Name of the node; Default="default"
    # \param target          [in] [optional] Default core to run on; Default="Target.DEFAULT"
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
## Node object used to generate a Scale node
#
# \par Example Usage: Creating Scale node
#
# \ingroup NODE
#
class NodeScaleImage (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1       [in] First input image
    # \param image_out2      [in] Input remap table
    # \param interp3         [in] Interpolation policy input scalar
    # \param image_out4      [in] Output image
    # \param name            [in] [optional] Name of the node; Default="default"
    # \param target          [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, image_in1, image_out2, interp3, name="default", target=Target.DEFAULT) :
        scalar = Scalar(Type.ENUM, interp3)
        Node.__init__(self, "org.khronos.openvx.scale_image", image_in1, image_out2, scalar)
        self.setParams(2, 1, Type.IMAGE, Type.IMAGE, Type.SCALAR)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_SCALE_IMAGE");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == self.ref[1].df_image ), "Input and Output MUST have same image data format"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Image data format must be U8"

#TODO Order of params
## Node object used to generate a Half Scale Gaussian node
#
# \par Example Usage: Creating Half Scale Gaussian node
#
# \ingroup NODE
#
class NodeHalfScaleGaussian (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] Output image
    # \param kernel_size    [in] Scalar kernel size
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Sobel3x3 node
#
# \par Example Usage: Creating Sobel3x3 node
#
# \ingroup NODE
#
class NodeSobel3x3 (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1      [in] Input image
    # \param image_out2     [in] First output image
    # \param image_out3     [in] Second output image
    # \param name           [in] [optional] Name of the node; Default="default"
    # \param target         [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Table Lookup node
#
# \par Example Usage: Creating Table Lookup node
#
# \ingroup NODE
#
class NodeTableLookup (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1       [in] First input image
    # \param lut_in2         [in] Input lookup table
    # \param image_out3      [in] Output image
    # \param name            [in] [optional] Name of the node; Default="default"
    # \param target          [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Threshold node
#
# \par Example Usage: Creating Threshold node
#
# \ingroup NODE
#
class NodeThreshold (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1       [in] First input image
    # \param thresh_in2      [in] Input threshold
    # \param image_out3      [in] Interpolation policy input scalar
    # \param name            [in] [optional] Name of the node; Default="default"
    # \param target          [in] [optional] Default core to run on; Default="Target.DEFAULT"
    def __init__(self, image_in1, thresh_in2, image_out3, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "org.khronos.openvx.threshold", image_in1, thresh_in2, image_out3)
        self.setParams(2, 1, Type.IMAGE, Type.THRESHOLD, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_KERNEL_THRESHOLD");

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input image data format must be U8"

## Node object used to generate a Warp Affine node
#
# \par Example Usage: Creating Warp Affine node
#
# \ingroup NODE
#
class NodeWarpAffine (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1       [in] Input image
    # \param matrix_in2      [in] Input affine matrix
    # \param interp3         [in] Interpolation type input scalar
    # \param image_out4      [in] Output image
    # \param name            [in] [optional] Name of the node; Default="default"
    # \param target          [in] [optional] Default core to run on; Default="Target.DEFAULT"
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

## Node object used to generate a Warp Perspective node
#
# \par Example Usage: Creating Warp Perspective node
#
# \ingroup NODE
#
class NodeWarpPerspective (Node) :
    ## Constructor used to create this object
    #
    # \param image_in1       [in] Input image
    # \param matrix_in2      [in] Input perspective matrix
    # \param interp3         [in] Interpolation type input scalar
    # \param image_out4      [in] Output image
    # \param name            [in] [optional] Name of the node; Default="default"
    # \param target          [in] [optional] Default core to run on; Default="Target.DEFAULT"
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
