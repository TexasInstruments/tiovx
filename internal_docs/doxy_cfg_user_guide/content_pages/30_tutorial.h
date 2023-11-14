/*!
    \page TUTORIALS TIOVX Tutorials

    For a detailed OpenVX tutorial, complete with videos, Khronos has a 
    great tutorial, as well as other resources listed on the \ref RESOURCES page.

    The \ref TUTORIALS section teaches how to use standard OpenVX APIs
    and TI OpenVX extension APIs. The tutorials are arranged in
    different chapters with increasing complexity. It is recommended
    to learn the tutorials in order listed below.

    The below chapters are included in this tutorial
    - \subpage CH01_GETTING_STARTED
    - \subpage CH02_IMAGE
    - \subpage CH03_GRAPH
    - \subpage CH04_GRAPH_PIPELINE
    \if DOCS_J6
    - \subpage CH05_TIDL
    \endif


 */

/**
 \page CH01_GETTING_STARTED Chapter 1: Getting Started

 The TIOVX User Guide \ref TUTORIALS provides the documentation and references necessary to begin
 development on TI's platforms using TIOVX.<p>

 Follow steps given in the \ref BUILD_INSTRUCTIONS page in the user guide to build and run the tutorials.

*/

/**
 \page CH02_IMAGE Chapter 2: Understanding image data object

 In OpenVX, image data object is the most commonly used data object. In this
 tutorial chapter, we understand how to manipulate image data objects.

    <TABLE>
        <TR bgcolor="lightgrey">
            <TH> Tutorial file </TH>
            <TH> Purpose </TH>
        </TR>
        <TR>
            <TD> \ref ch02_image/vx_tutorial_image_load_save.c </TD>
            <TD> Load and save images </TD>
        </TR>
        <TR>
            <TD> \ref ch02_image/vx_tutorial_image_query.c </TD>
            <TD> Query image objects and print image attributes </TD>
        </TR>
        <TR>
            <TD> \ref ch02_image/vx_tutorial_image_crop_roi.c </TD>
            <TD> Advanced image create APIs </TD>
        </TR>
        <TR>
            <TD> \ref ch02_image/vx_tutorial_image_extract_channel.c </TD>
            <TD> Image manipulation using VXU APIs </TD>
        </TR>
        <TR>
            <TD> \ref ch02_image/vx_tutorial_image_color_convert.c </TD>
            <TD> Image manipulation using graph and VX node APIs </TD>
        </TR>
        <TR>
            <TD> \ref ch02_image/vx_tutorial_image_histogram.c </TD>
            <TD> Image manipulation using graph and generic node create APIs </TD>
        </TR>
   </TABLE>

 */

/**
 \page CH03_GRAPH Chapter 3: Understanding OpenVX graphs

 In OpenVX, graph is the means by which computer vision functions are
 executed. These tutorials show how to create different types of graph and
 how to automate generation of graph application code.

    <TABLE>
        <TR bgcolor="lightgrey">
            <TH> Tutorial file </TH>
            <TH> Purpose </TH>
        </TR>
        <TR>
            <TD> \ref ch03_graph/vx_tutorial_graph_image_gradients.c </TD>
            <TD> Graph with multiple targets </TD>
        </TR>
        <TR>
            <TD> \ref ch03_graph/vx_tutorial_graph_user_kernel.c </TD>
            <TD> Graph with user kernels</TD>
        </TR>
        <TR>
            <TD> \ref ch03_graph/vx_tutorial_graph_user_kernel_pytiovx.c </TD>
            <TD> Graph with user kernels and generated with PyTIOVX tool </TD>
        </TR>
        <TR>
            <TD> \ref ch03_graph/vx_tutorial_graph_user_kernel.c </TD>
            <TD> Graph with target kernels </TD>
        </TR>
        <TR>
            <TD> \ref ch03_graph/vx_tutorial_graph_user_kernel_pytiovx.c </TD>
            <TD> Graph with target kernels and generated with PyTIOVX tool </TD>
        </TR>
   </TABLE>

 */

/**
 \page CH04_GRAPH_PIPELINE Chapter 4: Understanding graph pipelining

 In OpenVX, graph pipelining allows multiple instance of a graph to execute simulataneously
 each operating of different input and outout data. In this
 tutorial chapter, we understand how this is done using the graph pipelining API.

    <TABLE>
        <TR bgcolor="lightgrey">
            <TH> Tutorial file </TH>
            <TH> Purpose </TH>
        </TR>
        <TR>
            <TD> \ref ch04_graph_pipeline/vx_tutorial_graph_pipeline_two_nodes.c </TD>
            <TD> Graph pipelining of nodes running on two different CPU targets </TD>
        </TR>
   </TABLE>

 */

 /**
 \if DOCS_J6
 \page CH05_TIDL Chapter 5: Understanding TIDL node

 In TIOVX, TI has created a node API for calling invoking an entire Neural Network from
 within a node within the context of an graph. This enables users to dispatch not only
 the network, but also and preprocessing and parallel processing from one or more OpenVX
 graphs.  In this tutorial chapter, we understand how this is done using the TIDL Node API.

    <TABLE>
        <TR bgcolor="lightgrey">
            <TH> Tutorial file </TH>
            <TH> Purpose </TH>
        </TR>
        <TR>
            <TD> \ref ch05_tidl/vx_tutorial_tidl.c </TD>
            <TD> TIDL Node Example </TD>
        </TR>
   </TABLE>
\endif
 */

