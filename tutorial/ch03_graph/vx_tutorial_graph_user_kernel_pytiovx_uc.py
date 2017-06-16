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

## \file vx_tutorial_graph_user_kernel_pytiovx_uc.py This file show example of using basic PyTIOVX APIs to
#       describe user/target kernel class and use it within an OpenVX graph.
#
# See vx_tutorial_graph_image_gradients_pytiovx_uc.py for basic API usage.
#
# The basic steps to define user/target class and use it with PyTIOVX tool
#  - Define a class for user/target kernel dervied from tiovx::node::Node base class
#  - Overide the constructor to explicitly specific the parameters and types
#    - This allows the python to do basic syntax checking
#  - Overide the checkParams method to specify additional parameter checking if required
#  - Write the OpenVX use-case using PyTIOVX APIs
#    - Use the user/target class when adding the user/target kernel to the graph
#  - Export the graph as C code or .jpg file
#
# See source code of function vx_tutorial_graph_user_kernel_pytiovx_uc.make_my_graph() for detailed API usage for this example.
#

from tiovx import *

## Class to represent user/target kernel
#
# Dervied from tiovx::node::Node base class
class NodePhaseRgb (Node) :
    ## Constructor for user/target kernel class
    #
    #  \param image_in [in] Input image handle parameter for this function
    #  \param image_out [in] Output image handle parameter for this function
    #  \param name [in] user indetifiable name
    #  \param target [in] target CPU on which this kernel/node will run
    def __init__(self, image_in, image_out, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "vx_tutorial_graph.phase_rgb", image_in, image_out)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_USER_KERNEL")

    ## Parameter checking function
    #
    #  Checks if number of parameters passed is correct
    #  Checks if data type of parameters passed is correct
    #
    #  \param param_type_args [in] Variable number of args list of type tiovx.enums.Type to specify data object types
    def checkParams(self, *param_type_args) :
        # invoke base class checker function
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].width    == self.ref[1].width ), "Input and Output width MUST match"
        assert ( self.ref[0].height   == self.ref[1].height ), "Input and Output height MUST match"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input data format must be U8"
        assert ( self.ref[1].df_image == DfImage.RGB ), "Output data format must be RGB"

## Function to describe a graph and generate code, image using PyTIOVX tool
def make_my_graph() :
    ## Create a context object.
    context = Context("vx_tutorial_graph_user_kernel_pytiovx_uc")

    ## Create a graph object.
    graph = Graph()

    ## local variables to define the with and height of input image
    width = 640
    ## local variables to define the with and height of input image
    height = 480

    ## Create input image object.
    in_image = Image(width, height, DfImage.U8, name="input")

    ## Create grad_x image object.
    grad_x = Image(width, height, DfImage.S16, name="grad_x")

    ## Create grad_y image object.
    grad_y = Image(width, height, DfImage.S16, name="grad_y")

    ## Create phase image object.
    phase = Image(width, height, DfImage.U8, name="phase")

    ## Create 24b rgb object to represent phase.
    phase_rgb = Image(width, height, DfImage.RGB, name="phase_rgb")

    ## Create and add node to graph for Sobel3x3
    graph.add ( NodeSobel3x3(in_image, grad_x, grad_y, target=Target.DSP1) )

    ## Create and add node to graph for Phase
    graph.add ( NodePhase(grad_x, grad_y, phase, target=Target.DSP1) )

    ## Create and add node to graph for user/target kernel
    graph.add ( NodePhaseRgb(phase, phase_rgb, target=Target.DSP1) )

    ## Add graph to context
    context.add ( graph )

    ## Generate .jpg using 'dot' tool to generate visual representation of graphs in this context
    ExportImage(context).export()
    ## Generate C code to create, run, delete graphs in this context
    ExportCode(context).export()

make_my_graph()