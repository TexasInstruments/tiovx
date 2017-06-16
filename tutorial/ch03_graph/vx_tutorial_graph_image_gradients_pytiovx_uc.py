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


## \file vx_tutorial_graph_image_gradients_pytiovx_uc.py This file show example of using basic PyTIOVX APIs to
#       describe an OpenVX graph.
#
#        To import PyTIOVX module so that the APIs can be called in this file add below line
#        \code
#        from tiovx import *
#        \endcode
#
#        The basic steps one follows to describe the graph for PyTIOVX tool are,
#        - Create a context object.
#          - Pass as input a string which is used as a perfix to name the generated code and function.
#        - Create a graph object object
#        - Create data objects.
#          - TIP: specify a name for data objects via name="xyz" to be able to identify it in the generated
#            C code and image file
#        - Create node objects and link it to required data objects
#          - TIP: Add nodes to graph as you create them using graph.add()
#          - TIP: Use target=Target.<TargetName> to specify the target on which this node runs.
#        - Add graph to context using context.add()
#        - Generate C code for the objects in a context using
#          \code
#          ExportCode(context).export()
#          \endcode
#        - Generate image file for the objects in a context using
#          \code
#          ExportImage(context).export()
#          \endcode
#        - Run the PyTIOVX on this file using
#          \code
#          python <.py file>
#          \endcode
#        - In case of any error reported by the PYTIOVX tool, correct the input .py file
#        - Verify the generated image file to confirm the graph is as expected.
#        - Write the remaining C code to invoke the generated APIs
#
# See source code of function vx_tutorial_graph_image_gradients_pytiovx_uc.make_my_graph() for detailed API usage for this example.

from tiovx import *

## Function to describe a graph and generate code, image using PyTIOVX tool
def make_my_graph() :
    ## Create a context object.
    context = Context("vx_tutorial_graph_image_gradients_pytiovx_uc")

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

    ## Create magnitude image object.
    magnitude = Image(width, height, DfImage.S16, name="magnitude")

    ## Create phase image object.
    phase = Image(width, height, DfImage.U8, name="phase")

    ## Create image object to represent grad_x as grayscale image
    grad_x_img = Image(width, height, DfImage.U8, name="grad_x_img")

    ## Create image object to represent grad_y as grayscale image
    grad_y_img = Image(width, height, DfImage.U8, name="grad_y_img")

    ## Create image object to represent magnitude as grayscale image
    magnitude_img = Image(width, height, DfImage.U8, name="magnitude_img")

    ## Create a scalar object to represent 'shift' for ConvertDepth
    shift = Scalar(Type.INT32, 0, name="shift")

    ## Create and add node to graph for Sobel3x3
    graph.add ( NodeSobel3x3(in_image, grad_x, grad_y, target=Target.DSP1) )

    ## Create and add node to graph for Magnitude
    graph.add ( NodeMagnitude(grad_x, grad_y, magnitude, target=Target.DSP2) )

    ## Create and add node to graph for Phase
    graph.add ( NodePhase(grad_x, grad_y, phase, target=Target.DSP1) )

    ## Create and add node to graph for ConvertDepth to convert magnitude to 8b grayscale image
    graph.add ( NodeConvertDepth(magnitude, magnitude_img, Policy.SATURATE, shift, target=Target.DSP2) )

    ## Create and add node to graph for ConvertDepth to convert grad_x to 8b grayscale image
    graph.add ( NodeConvertDepth(grad_x, grad_x_img, Policy.SATURATE, shift, target=Target.DSP2) )

    ## Create and add node to graph for ConvertDepth to convert grad_y to 8b grayscale image
    graph.add ( NodeConvertDepth(grad_y, grad_y_img, Policy.SATURATE, shift, target=Target.DSP1) )

    ## Add graph to context
    context.add ( graph )

    ## Generate .jpg using 'dot' tool to generate visual representation of graphs in this context
    ExportImage(context).export()
    ## Generate C code to create, run, delete graphs in this context
    ExportCode(context).export()

make_my_graph()