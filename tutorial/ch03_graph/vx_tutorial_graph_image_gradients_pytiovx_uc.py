#
# Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
# ALL RIGHTS RESERVED
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

    ## Create grad_x image object.
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

    ## Create and node to graph for Sobel3x3
    graph.add ( NodeSobel3x3(in_image, grad_x, grad_y, target=Target.DSP1) )

    ## Create and node to graph for Magnitude
    graph.add ( NodeMagnitude(grad_x, grad_y, magnitude, target=Target.DSP2) )

    ## Create and node to graph for Phase
    graph.add ( NodePhase(grad_x, grad_y, phase, target=Target.DSP1) )

    ## Create and node to graph for ConvertDepth to convert magnitude to 8b grayscale image
    graph.add ( NodeConvertDepth(magnitude, magnitude_img, Policy.SATURATE, shift, target=Target.DSP2) )

    ## Create and node to graph for ConvertDepth to convert grad_x to 8b grayscale image
    graph.add ( NodeConvertDepth(grad_x, grad_x_img, Policy.SATURATE, shift, target=Target.DSP2) )

    ## Create and node to graph for ConvertDepth to convert grad_y to 8b grayscale image
    graph.add ( NodeConvertDepth(grad_y, grad_y_img, Policy.SATURATE, shift, target=Target.DSP1) )

    ## Add graph to context
    context.add ( graph )

    ## Generate .jpg using 'dot' tool to generate visual representation of graphs in this context
    ExportImage(context).export()
    ## Generate C code to create, run, delete graphs in this context
    ExportCode(context).export()

make_my_graph()