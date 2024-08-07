##
#    \page PYTIOVX PyTIOVX User Guide
#
#    \tableofcontents
#
#    \section Overview
#    The PyTIOVX Tool allows users to generate an OpenVX extension kernel wrapper by specifying
#    parameters of the kernel and setting relationships between these parameters.
#    <BR><BR>
#    'tiovx' is the python module that is used to specify OpenVX graph in a compact manner using
#    predefined classes and objects. After specification of OpenVX graph using PyTIOVX APIs,
#    it can be exported in various formats including C code, JPG image
#
#    \section INSTALLATION Installation
#    - Install Python 3.5.2 or later (https://www.python.org/downloads/)
#      - To confirm "python" and "pip" are in your install path, type below:
#        \code
#        python3 --version
#        pip3 --version
#        \endcode
#    - Install 'dot' tool provided as part of 'graphviz' (http://www.graphviz.org/download/). \n
#      'dot' is required to generated .JPG file for a OpenVX graph specification.
#      - To confirm "dot" tool is in your install path, type:
#        \code
#        dot -V
#        \endcode
#    - Install 'tiovx' python module by executing below command at folder
#      "<tiovx install path>/tools/PyTIOVX".
#      \code
#      sudo pip3 install -e .
#      \endcode
#      Expected output:
#      \code
#      Obtaining file:///<tiovx install path>/tiovx/tools/PyTIOVX
#      Installing collected packages: tiovx
#        Running setup.py develop for tiovx
#      Successfully installed tiovx-0.1
#      \endcode
#    - Now, the tiovx python module can be imported and used in your python script
#      by doing below:
#      \code
#      from tiovx import *
#      \endcode
#
#    \section USAGE Usage
#    - Describe your application graph or kernel wrapper using PyTIOVX APIs in a .py file.
#      Execute the .py file by invoking python:
#      \code
#      python my_kernel_wrapper.py
#      \endcode
#      Note: if the following error is observed, please run using the command "python3.5 my_use_case.py".
#      \code
#      Traceback (most recent call last):
#          File "kernel_generate_example.py", line 6, in <module>
#              from tiovx import *
#          File "tiovx/__init__.py", line 63, in <module>
#              from .enums import *
#          File "tiovx/tools/PyTIOVX/tiovx/enums.py", line 62, in <module>
#              from enum import Enum
#      ImportError: No module named enum
#      \endcode
#    - See also examples in TIOVX tutorial
#      <a href="CH03_GRAPH.html" target="_blank">[HTML]</a>
#      for making OpenVX usecases using PyTIOVX tool.
#    - See also sample python scripts (*.py) at
#      "<tiovx install path>/tools/sample_use_cases" for additional examples.
#
#    \section KERNEL_SCRIPT Kernel Generation
#    \par Example Kernel Generation Python Script
#
#    - Below is an example of a Python script used for generating a custom Channel Extract kernel:
#      \code
#      from tiovx import *
#
#      code = KernelExportCode(Module.IMAGING, Core.C66, "CUSTOM_APPLICATION_PATH")
#
#      code.setCoreDirectory("c66")
#
#      kernel = Kernel("channel_extract")
#
#      kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.REQUIRED, "IN", ['VX_DF_IMAGE_U8'])
#      kernel.setParameter(Type.ENUM, Direction.INPUT, ParamState.REQUIRED, "CHANNEL")
#      kernel.setParameter(Type.IMAGE, Direction.OUTPUT, ParamState.REQUIRED, "OUT", ['VX_DF_IMAGE_U8'])
#      kernel.setParameterRelationship(["IN", "OUT"], [Attribute.Image.WIDTH, Attribute.Image.HEIGHT])
#      kernel.allocateLocalMemory("img_scratch_mem", ["width*height"], "IN")
#
#      kernel.setTarget(Target.DSP1)
#      kernel.setTarget(Target.DSP2)
#
#      code.export(kernel)
#      \endcode
#
#    - The constructor for the KernelExportCode API contains arguments that serve as the directory paths of the generated code.
#    - \code
#      code = KernelExportCode(Module.IMAGING, Core.C66, "CUSTOM_APPLICATION_PATH")
#      \endcode
#          - The first argument, Module.IMAGING, is the intended module of the kernel. A full list of modules are contained with the module.py file of PyTIOVX.
#            If the name of the module needed is not listed, either the module can be added to the module.py file or the module name can be passed as a string to the constructor.
#          - The second argument, Core.C66 is the specific core that this kernel will run on. Similar to the module name, a list of core values are listed in core.py.
#            If the name of the core needed is not listed, either the core can be added to the core.py file or the core name can be passed as a string to the constructor.
#          - The third argument "CUSTOM_APPLICATION_PATH" is the path where the output files will be exported to.
#              - Note: this path must be set as an environment variable by using "export" or else an error will be thrown.
#          - The constructor has two optional arguments, include_subpath and include_filename.
#              - The include_subpath should be set to the string value of the company name of the developer and defaults to "TI".
#              - The include_filename contains the path name to the include file. By default, this value is set an empty string which results in the filepath being set to "<lowercase(include_subpath)>vx_<module>". However, the filepath can be overwritten
#                by filling in this optional parameter.
#
#    - \code
#    code.setCoreDirectory("c66")
#    \endcode
#       - Several dedicated API's are provided in the case that certain parameters of KernelExportCode must be changed during execution of the script. This example shows one such API,
#      setCoreDirectory. This method can change the intended core by passing in a particular core as its only argument.
#    - \code
#    kernel = Kernel("channel_extract")
#    \endcode
#       - The constructor for the Kernel class contains as its only argument the name of the kernel to be created.
#    - \code
#    kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.REQUIRED, "IN", ['VX_DF_IMAGE_U8'])
#    kernel.setParameter(Type.ENUM, Direction.INPUT, ParamState.REQUIRED, "CHANNEL")
#    kernel.setParameter(Type.IMAGE, Direction.OUTPUT, ParamState.REQUIRED, "OUT", ['VX_DF_IMAGE_U8'])
#    \endcode
#       - Parameters of a kernel are set via the setParameter API as in the example above. Here, 3 parameters are set: an input image, an input scalar enum, and an output image.
#       - The following are arguments of the setParameter method:
#          - The first argument is the OpenVX data type of the parameter. Allowed values can be found in enums.py.
#          - The second argument is the direction of the parameter. Allowed values are Direction.INPUT or Direction.OUTPUT.
#          - The third argument determines whether the parameter is required or optional. Allowed values are ParamState.REQUIRED or ParamState.OPTIONAL.
#          - The fourth argument allows you to provide a name for the new parameter.  This argument accepts a string value as the input.  For instance, if this object is an input image,
#            you may want to provide the name argument "IN_IMAGE".
#          - The fifth argument is optional and defines a set of allowed data types of the parameter. These are given as OpenVX data types as defined in the OpenVX 1.1 specification and
#            are given as a comma separated list.
#          - For further information, please refer to the API guide here \ref PYTIOVX_API.
#    - \code
#    kernel.setParameterRelationship(["IN", "OUT"], [Attribute.Image.WIDTH, Attribute.Image.HEIGHT])
#    \endcode
#       - After setting all parameters via the setParameter method, relationships among these parameters can be set via the setParameterRelationship method. This method is designed to ouput
#       code in the validation callback to verify equality between certain attributes of parameters of the kernel. This validation of parameters is important in order no run-time errors of
#       the kernel.
#       - In this case, a relationship is set between the width and height of the "IN" and "OUT" parameters. This will produce code in the kernel validate callback that
#       first queries the "IN" and "OUT" parameters for the width and height then checks for equality between the width of the "IN" parameter against the width of the "OUT" parameter
#       in addition to checking for the equality between the height of the "IN" parameter against the height of the "OUT" parameter.
#    - \code
#    kernel.allocateLocalMemory("img_scratch_mem", ["width*height"], "IN")
#    \endcode
#       - The allocateLocalMemory method is used to generate OpenVX code for allocating and free-ing local memory based on characteristics of existing data objects or on constant values.
#       In this example, the allocateLocalMemory method is used to create a buffer with the name "img_scratch_mem" and is equal to the size of the width * height of the image, "IN".
#       For further information, please refer to the API guide.
#    - \code
#    kernel.setTarget(Target.DSP1)
#    kernel.setTarget(Target.DSP2)
#    \endcode
#       - The list of targets for the kernel to run on can be set via the setTarget API. All possible targets that the kernel could run on should be given as arguments to this API. In this example, the target kernel is
#       can be run on either DSP1 or DSP2 and therefore the setTarget API is called twice with each argument given. Allowed target values are given in enums.py.
#    - \code
#    code.export(kernel)
#    \endcode
#       - After setting up all parameters and targets of the kernel, the kernel wrappers can be generated by running the export method.
#    - Further documentation of the API's used for kernel code generation can be found <a href="modules.html" target="_blank">here.</a>
#
#    \par Generated Kernel Code Overview
#
#    - This section describes the generated files based on the example given above.
#    - The kernel files are generated in different locations depending on the address specified by the code generation path.
#    - The files that are only generated the first time are noted. Here, the generation path was set to "custom_application_path".
#      \image html custom_application_path.png
#      \code
#      <CUSTOM_APPLICATION_PATH>/DEVELOPER_TODO.txt (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/concerto_inc.mak
#      <CUSTOM_APPLICATION_PATH>/custom_tools_path.mak
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/include/TI/tivx_imaging.h (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/include/TI/tivx_imaging_kernels.h (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/include/TI/tivx_imaging_nodes.h (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/c66/concerto.mak (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/c66/vx_channel_extract_target.c
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/c66/vx_kernels_imaging_target.c (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/host/concerto.mak (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/tivx_imaging_node_api.c (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/host/vx_channel_extract_host.c
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/host/vx_kernels_imaging_host.c (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/include/tivx_imaging_kernels.h (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/include/tivx_kernel_channel_extract.h
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/test/concerto.mak (generated first time only)
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/test/test_main.h (generated first time only)
#      \endcode
#    - If the final argument of the KernelExportCode function was provided as VISION_APPS_PATH, the following files are produced:
#      \image html vision_apps_path.png
#      \code
#      <VISION_APPS_PATH>/kernels/DEVELOPER_TODO.txt (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/include/TI/tivx_imaging.h (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/include/TI/tivx_imaging_kernels.h (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/include/TI/tivx_imaging_nodes.h (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/c66/concerto.mak (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/c66/vx_channel_extract_target.c
#      <VISION_APPS_PATH>/kernels/imaging/c66/vx_kernels_imaging_target.c (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/host/concerto.mak (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/tivx_imaging_node_api.c (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/host/vx_channel_extract_host.c
#      <VISION_APPS_PATH>/kernels/imaging/host/vx_kernels_imaging_host.c (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/include/tivx_imaging_kernels.h (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/include/tivx_kernel_channel_extract.h
#      <VISION_APPS_PATH>/kernels/imaging/test/concerto.mak (generated first time only)
#      <VISION_APPS_PATH>/kernels/imaging/test/test_main.h (generated first time only)
#      \endcode
#    - As noted, the majority of the generated files are only generated once per kernel module. These files contain boilerplate code needed for registering/unregistering the new kernels and node creation.
#    - In the case that the module already exists, new code will simply be appended to the existing files. Previously generated files will not be duplicated.
#
#    \par Overview of Necessary Kernel Code Modifications
#
#    - In addition to the code generated by the python script, a text file called "DEVELOPER_TODO.txt" is generated that provides information regarding the changes to be made to the generated files.
#    - The two files wherein most modifications will occur are the host and target kernel files. The files for the given example are listed below:
#      \code
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/c66/vx_channel_extract_target.c
#      <CUSTOM_APPLICATION_PATH>/kernels/imaging/host/vx_channel_extract_host.c
#      \endcode
#    - The host kernel file must be modified in the following ways:
#        - Validate callback: Additional custom parameter checking may need to be added to the validate callback. The script will generate parameter checking for attributes of parameters that are equal. Additional checking
#          beyond the scope of the script must be entered in manually by the user. For example, in the case of a kernel with a scaled output, the validate callback should verify that the output
#          is dimensions are scaled properly from the input. This type of check will most likely first require querying a given kernel parameter as specified by the OpenVX API designated
#          for querying the given parameter.  At this point the value can be verified against a known quantity. This step of proper validation catches errors during the verification stage and
#          is important within the context of OpenVX to ensure no errors during execution of the graph.
#        - Initialize callback: Appropriate valid region padding values must be added to the initialize callback to support the UNDEFINED border mode in the case that the data type in question is an image. These values 
#          will be determined by whether or not the generated kernel contains a region-based operation. For example, in the case that the kernel in question is a 3x3 filter, the padding for the top, bottom, left and right 
#          will all be a value of '1'. In the case that no region-based operations are performed, these values can be set to '0'.
#    - The target kernel file must be modified in the following ways:
#        - Create callback: The create callback is run once upon verification of the graph and should include allocation of any required local memory buffers as well as any one time initialization required for the kernel.
#        - Delete callback: The delete callback is run upon release of the graph and should include the freeing of local memory buffers that were allocated within the create callback as well as any deinitialization of
#          parameters that were initialized in the create callback.
#        - Process callback: The process callback is run upon each execution of the graph and should include a call to the kernel process function. For example, in the case of OpenVX core kernels, the process callback
#          contains a call to the optimized VXLIB function that performs the operation of the kernel.
#        - Control callback: The control callback is run upon a user command to the kernel at run-time. It should include the processing of this user command and the operation necessary to modify the operation of the
#          kernel.
#    - Within the generated test folder, the test_main.h and concerto allows for simple integration into the TIOVX test framework.  However, the test case itself must be
#      developed.  For test case references, please refer to tiovx/conformance_tests/
#
#    \par Integrating User Kernels and OpenVX Graphs
#    - Further documentation on creating user kernel nodes can be found <a href="TIOVX_TARGET_KERNEL.html#KERNEL_MODULE_INTEGRATION" target="_blank">here.</a>
#
#    \if APP_GEN_TOOL
#
#    \section USE_CASE_SCRIPT Use Case Generation
#    \par Example Use Case Generation Python Script
#
#    - In addition to generating kernel wrappers, the Python script also provides the ability to generate full use cases consisting of existing nodes. The Python script will generate all of the necessary OpenVX-related function calls at a fraction of the amount of code.
#    - In addition to generating OpenVX code, the Python tool also has the ability to generate an image capturing the nodes and connections within the OpenVX graph. This allows the developer to better visualize the functionality of the graph.
#    - Several examples are given as a part of the TIOVX release at the location tiovx/tools/sample_use_cases. Particularly, the file "uc_sample_07.py" gives an in-depth look at the extent of the tool by providing a superset of the data types as well as showing how a good portion of the nodes are called.
#    - An example is given below of how to generate OpenVX code using the Python tool. This example consists of two nodes, a table lookup node and a convolution node. The next section will describe the 
#      generated OpenVX code from this example.
#      \code
#      from tiovx import *
#
#      context = Context("uc_sample_07")
#      graph = Graph()
#
#      img_in = Image(640, 480, DfImage.U8)
#      img_out_lut = Image(640, 480, DfImage.U8)
#      lut = Lut(Type.UINT8, 255)
#      node_lut = NodeTableLookup(img_in, lut, img_out_lut);
#
#      img_out_conv = Image(640, 480, DfImage.S16)
#
#      conv = Convolution(3, 3)
#      node_conv = NodeConvolve(img_out_lut, conv, img_out_conv, target=Target.DSP2);
#
#      graph.add ( node_lut )
#      graph.add ( node_conv )
#
#      context.add ( graph )
#
#      ExportImage(context).export()
#      ExportCode(context, "VISION_APPS_PATH").export()
#      \endcode
#    - Further documentation of the API's used for use case generation can be found <a href="modules.html" target="_blank">here</a>
#
#    \par Generated Use Case Code Overview
#
#    - For the above Python script, a set of use case-specific files get generated. In the default case, the exported code gets generated at the "VISION_APPS_PATH". The intent of generating use cases along this path is to build the use case as a part of the vision_apps project. A list of the generated files can be seen below:
#      \image html vapps_path2.png
#      \code
#      <VISION_APPS_PATH>/apps/uc_sample_07/DEVELOPER_TODO.txt
#      <VISION_APPS_PATH>/apps/uc_sample_07/concerto_inc.mak
#      <VISION_APPS_PATH>/apps/uc_sample_07/app_uc_sample_07/concerto.mak
#      <VISION_APPS_PATH>/apps/uc_sample_07/app_uc_sample_07/uc_sample_07.c
#      <VISION_APPS_PATH>/apps/uc_sample_07/app_uc_sample_07/uc_sample_07.h
#      \endcode
#    - The other supported export path is the CUSTOM_APPLICATION_PATH. The intent of generating use cases along this path is to build the use case as a part of the tiovx project. A list of generated files using this path with the above script can be seen below. Note: a few additional files are generated in this case in order for it to be included in the tiovx build.
#      \image html custom_application_path_use_case.png
#      \code
#      <CUSTOM_APPLICATION_PATH>/DEVELOPER_TODO.txt
#      <CUSTOM_APPLICATION_PATH>/concerto_inc.mak
#      <CUSTOM_APPLICATION_PATH>/custom_tools_path.mak
#      <CUSTOM_APPLICATION_PATH>/kernels/custom_app_kernel_library_tests.h
#      <CUSTOM_APPLICATION_PATH>/app_uc_sample_07/concerto.mak
#      <CUSTOM_APPLICATION_PATH>/app_uc_sample_07/uc_sample_07.c
#      <CUSTOM_APPLICATION_PATH>/app_uc_sample_07/uc_sample_07.h
#      \endcode
#    - The top-level function prototypes generated by the use case generation script can be seen below:
#      \code
#      vx_status uc_sample_07_create(uc_sample_07 usecase);
#      vx_status uc_sample_07_delete(uc_sample_07 usecase);
#      vx_status uc_sample_07_verify(uc_sample_07 usecase);
#      vx_status uc_sample_07_run(uc_sample_07 usecase);
#      \endcode
#    - The functions listed above can be called from a main function in sequential order upon successful return in order to run the OpenVX graph.
#        - The create function simply allocates all memory and data objects for the graph and performs any necessary initialization.
#        - The verify function calls the vxVerifyGraph API to ensure that all parameters of the graph are correct.
#        - The run function simply executes the graph once upon each call.
#        - The delete function frees all memory and performs deinitialization.
#    - The tutorial provided at tiovx/tutorial provides sample code of how to link these functions to a main function.
#    - The following code snippet shows the creation of the data used in the use case generation script.
#      \code
#      vx_status uc_sample_07_data_create(uc_sample_07 usecase)
#      {
#          vx_status status = VX_SUCCESS;
#
#          vx_context context = usecase->context;
#
#          if (status == VX_SUCCESS)
#          {
#              usecase->image_1 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
#              if (usecase->image_1 == NULL)
#              {
#                  status = VX_ERROR_NO_RESOURCES;
#              }
#              vxSetReferenceName( (vx_reference)usecase->image_1, "image_1");
#          }
#          if (status == VX_SUCCESS)
#          {
#              usecase->lut_3 = vxCreateLUT(context, VX_TYPE_UINT8, 255);
#              if (usecase->lut_3 == NULL)
#              {
#                  status = VX_ERROR_NO_RESOURCES;
#              }
#          }
#          if (status == VX_SUCCESS)
#          {
#              usecase->image_2 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8);
#              if (usecase->image_2 == NULL)
#              {
#                  status = VX_ERROR_NO_RESOURCES;
#              }
#              vxSetReferenceName( (vx_reference)usecase->image_2, "image_2");
#          }
#          if (status == VX_SUCCESS)
#          {
#              usecase->convolution_6 = vxCreateConvolution(context, 3, 3);
#              if (usecase->convolution_6 == NULL)
#              {
#                  status = VX_ERROR_NO_RESOURCES;
#              }
#          }
#          if (status == VX_SUCCESS)
#          {
#              usecase->image_5 = vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16);
#              if (usecase->image_5 == NULL)
#              {
#                  status = VX_ERROR_NO_RESOURCES;
#              }
#              vxSetReferenceName( (vx_reference)usecase->image_5, "image_5");
#          }
#
#          return status;
#      }
#      \endcode
#    - As the above code snippet indicates, the PyTIOVX use case generation tool encapsulates a large amount of OpenVX code in very few lines of Python code.
#      The OpenVX generated code also provides error checking in the event that the data objects were unable to be created.
#    - The following code snippet shows the functions used for creating the nodes corresponding to the use case generation script.
#      \code
#      static vx_node usecase_node_create_node_4 (
#        vx_graph graph ,
#        vx_image image_0 ,
#        vx_lut lut_1 ,
#        vx_image image_2 
#        )
#      {
#          vx_node node = NULL;
#          vx_reference params[] =
#          {
#                (vx_reference)image_0 ,
#                (vx_reference)lut_1 ,
#                (vx_reference)image_2 
#          };
#          node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_TABLE_LOOKUP, params, 3);
#
#          return node;
#      }
#
#      static vx_node usecase_node_create_node_7 (
#        vx_graph graph ,
#        vx_image image_0 ,
#        vx_convolution convolution_1 ,
#        vx_image image_2 
#        )
#      {
#          vx_node node = NULL;
#          vx_reference params[] =
#          {
#                (vx_reference)image_0 ,
#                (vx_reference)convolution_1 ,
#                (vx_reference)image_2 
#          };
#          node = tivxCreateNodeByKernelEnum(graph, VX_KERNEL_CUSTOM_CONVOLUTION, params, 3);
#
#          return node;
#      }
#      \endcode
#    - As the above code snippet shows, the nodes for the use case are created by first passing the corresponding data objects to a function the creating the node
#      by enum. This process is how any given node is created using the script.
#    - The following code snippet shows the generated OpenVX code for the verification and run steps.
#      \code
#      vx_status uc_sample_07_graph_0_verify(uc_sample_07 usecase)
#      {
#          vx_status status = VX_SUCCESS;
#
#          vx_graph graph = usecase->graph_0;
#
#          if (status == VX_SUCCESS)
#          {
#              status = vxVerifyGraph(graph);
#          }
#
#          return status;
#      }
#
#      vx_status uc_sample_07_graph_0_run(uc_sample_07 usecase)
#      {
#          vx_status status = VX_SUCCESS;
#
#          vx_graph graph = usecase->graph_0;
#
#          if (status == VX_SUCCESS)
#          {
#              status = vxProcessGraph(graph);
#          }
#
#          return status;
#      }
#      \endcode
#    - The code snippet above is generated for every use case.
#        - The first function simply performs verification on the graph and returns the status of verification.
#        - The second function runs the graph by executing vxProcessGraph. In the case that the use case requires streaming, this function can be executed in an infinite loop.
#    - For further information on this use case, the entire file can be found at tiovx/tools/sample_use_cases/uc_sample_07.c.
#
#    \par Using PyTIOVX to create a use case with a user kernel
#
#    - The PyTIOVX tool can also be used to generate use cases with user kernels as nodes of the graph. The following example describes the necessary steps in this
#      process.
#    - The following PyTIOVX script produces a simple user kernel wrapper with a single input image and a single output image. These images have equivalent widths
#      heights and are set to run on both DSP1 and DSP2. In this example, the functionality of the kernel will be a simple copy from input to output.
#      \code
#      from tiovx import *
#
#      code = KernelExportCode("TI", "presentation", "presentation_module", "c66x", "CUSTOM_KERNEL_PATH")
#
#      kernel = Kernel("presentation_example")
#
#      kernel.setParameter(Type.IMAGE, Direction.INPUT, ParamState.REQUIRED, "IN", ['VX_DF_IMAGE_U8'])
#      kernel.setParameter(Type.IMAGE, Direction.OUTPUT, ParamState.REQUIRED, "OUT", ['VX_DF_IMAGE_U8'])
#      kernel.setParameterRelationship(["IN", "OUT"], [Attribute.Image.WIDTH, Attribute.Image.HEIGHT])
#
#      kernel.setTarget(Target.DSP1)
#      kernel.setTarget(Target.DSP2)
#
#      code.export(kernel)
#      \endcode
#    - After running the previous script, the next step is to create a use case generation PyTIOVX script that includes the newly created user kernel. An example
#      of this can be found at tiovx/tutorial/ch03_graph/vx_tutorial_graph_user_kernel_pytiovx_uc.py.
#    - Minimal modifications to this script are needed to incorporate the previously discussed user kernel.
#        - The first modification to be made is to change the name of the node to match the name given to the user kernel. In this case, the node name is NodePresentationExample.
#        - The next simple modification to be made is to replace the existing kernel string with the kernel string produced by the kernel generation script. In this case, the kernel string is "com.ti.presentation_module.presentation_example".
#        - Finally, slight modifications must be made to the checkParams method of the Node class based on the kernel parameters' relationships.
#    - The resulting PyTIOVX script can be seen below:
#      \code
#      from tiovx import *
#
#      class NodePresentationExample (Node) :
#          def __init__(self, image_in, image_out, name="default", target=Target.DEFAULT) :
#              Node.__init__(self, "com.ti.presentation_module.presentation_example", image_in, image_out)
#              self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
#              self.setTarget(target)
#              self.setKernelEnumName("VX_USER_KERNEL")
#
#          def checkParams(self, *param_type_args) :
#              Node.checkParams(self, *param_type_args)
#              assert ( self.ref[0].width    == self.ref[1].width ), "Input and Output width MUST match"
#              assert ( self.ref[0].height   == self.ref[1].height ), "Input and Output height MUST match"
#              assert ( self.ref[0].df_image == self.ref[1].df_image ), "Input data format and output data format must match"
#              assert ( self.ref[1].df_image == DfImage.U8 ), "Data format must be U8"
#
#      def make_my_graph() :
#          context = Context("uc_sample_08")
#
#          graph = Graph()
#
#          width = 640
#          height = 480
#
#          in_image = Image(width, height, DfImage.U8, name="in")
#
#          out_image = Image(width, height, DfImage.U8, name="out")
#
#          graph.add ( NodePresentationExample(in_image, out_image, target=Target.DSP1) )
#
#          context.add ( graph )
#
#          ExportImage(context).export()
#          ExportCode(context).export()
#
#      make_my_graph()
#      \endcode
#
#
#    <BR> <HR>
#
#    \endif
#
#
