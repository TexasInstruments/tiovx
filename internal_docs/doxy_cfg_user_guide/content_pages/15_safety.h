/*!
    \page TIOVX_SAFETY TIOVX Safety Manual

    This section of the user guide describes relevant safety considerations required for the ASIL-B certification
    of the TI OpenVX framework.  This activity is planned to be finalized in July 2024.

    In scope for this safety project is TI's implementation of the OpenVX framework (source/framework), the OS platform
    layer (source/platform) and the dependent utils of TIOVX (app_utils).

    Please reference the TI software safety qualification approach for more information about what all collateral
    shall be provided for final certification of the SW module.

    - \subpage TIOVX_SAFETY_CONSIDERATIONS - Various TIOVX safety considerations when designing a SW system.
    - \subpage TIOVX_SAFETY_FEATURES - Description of features within TIOVX which can be useful when designing a SW system.
    - \subpage TIOVX_SAFETY_TOOLING - Description of provided tooling for helping enable a safety application using TIOVX.
 */

/*!
    \page TIOVX_SAFETY_CONSIDERATIONS TIOVX Safety Considerations

    \tableofcontents

     \section TIOVX_SAFETY_USAGE TIOVX Usage Recommendations for Safety

     When utilizing TIOVX within a safety system, there are a few things to note regarding the implementation
     of various API's.

     \subsection TIOVX_SAFETY_USAGE_DELAY Delay Object Support

     The \ref vx_delay objects have limited support within TIOVX, particularly when using them within pipelining.
     Thus, it is not recommended to use \ref vx_delay objects with pipelining.  The section \ref TIOVX_PIPELINING_PERFORMANCE_OPTIMIZATION_DELAY_PARAMETERS
     provides more details on this limitation and alternative means by which to implement similar functionality.

     \subsection TIOVX_SAFETY_USAGE_COMPOSITE_OBJECTS Composite Object Pipelining Support

     Similarly, composite objects such as \ref vx_object_array and \ref vx_pyramid have certain limitations with pipelining.  The section
     \ref TIOVX_PIPELINING_USAGE_CONSIDERATIONS_COMPOSITE_OBJECTS provides more details on this limitation and alternative means by which
     to implement similar functionality.

     \subsection TIOVX_SAFETY_USAGE_MAP_COPY Safety Implications of TIOVX Map and Copy API Implementation

     Per the section \ref TIOVX_USAGE_MAP_COPY_API, there are some safety implications for the map and copy API's implementations within TIOVX.
     Given that these API's allocate memory prior to graph verification, the application developer must by cognizant of this fact when creating
     the TIOVX-based application.  In particular, if using data objects as exemplars or other such elements disconnected from the graph, it is
     possible that the available memory of the system can be reached prior to the call to \ref vxVerifyGraph and thus should be taken into account
     when designing the system.

     \subsection TIOVX_SAFETY_USAGE_FURTHER_INFO Further information

     For full details, please reference the TIOVX usage sections at the location \ref TIOVX_USAGE

     \section TIOVX_SAFETY_MEMORY_MANAGEMENT TIOVX Memory Management for Safety

     A critical component of safety SW systems is the memory management scheme.  Please reference the section \ref TIOVX_MEMORY_MANAGEMENT_OPTIMIZATIONS
     for details on how memory management is handled in TIOVX and how it facilitates safety.

     \section TIOVX_SAFETY_RESOURCE_TEARDOWN TIOVX Resource Teardown

     For applications created using TIOVX as the middleware, the resource teardown shall be considered in the development of the applications.
     While TIOVX provides API's to release references that have previously been created, this logic must be called in the case that an event is received
     at the application level which causes an abort.  Signal handler logic should handle the teardown of OpenVX data objects that had previously been created.
     The implementation of this logic will depend on the OS used by the OpenVX host core.

     In order to understand how this shall be implemented within an OpenVX-based application with the OpenVX host running on a POSIX-based OS, please reference
     the "File Descriptor Exchange across Processes" application found within the vision apps package of the PSDK RTOS.  This application registers a signal
     handler which is executed upon a Ctrl-C signal.  This is done using the code snippet below:

     \code
     signal(SIGINT, App_intSigHandler);
     \endcode

     Within the App_intSigHandler, the application calls App_deInit, which calls the teardown logic associated with all of the OpenVX objects contained
     within the application.

     For information about ensuring that all resources have been freed appropriately, please reference \ref TIOVX_SAFETY_TOOLING.

     \section TIOVX_SAFETY_IMPORT_REFERENCE TIOVX Import Reference

     The \ref tivxReferenceImportHandle API has several important restrictions in how it is to be used within TIOVX.  The API guide gives details as to
     the requirements of the imported handle, which must be adhered to.  In particular, there are a few important aspects of the imported handles that need
     to be reviewed below:

     - OpenVX data type requirements.  The only provided list of data types are valid.
     - Memory region requirements.  An error is thrown if the memory is not created from the region specified
     - Memory alignment requirements.  While there is not a check for this given that it is simply a memory address, the API is required to be used for memory
       allocation will automatically align the memory to the required alignment
     - There is an error thrown if the corresponding number of entries doesn't match a set of number of valid addresses.  If the total number of memory pointers
       are not equal to the number of poitners required for the reference, then an error will be thrown.

     For more information about how to use this API, please refer to the Producer/Consumer application within vision_apps as well as the test cases
     found at tiovx/conformance_tests/test_tiovx/test_tivxMem.c

     \section TIOVX_SAFETY_OS_SUPPORT TIOVX OS Support

     The aspects of TIOVX which are considered under the safety qualification effort are the framework, platform layer and associated utils layers.  For the
     remote cores supported via TIOVX safety, the safety OS which is supported is SafeRTOS.  For the host side components, given that the Linux and QNX OS
     layers do not support safety, a safety OS would need to be used in place of Linux and QNX.  Please note though that the generic POSIX platform layer will
     have safety qualification collateral (e.g., MISRA-C, Code Coverage, Requirement documentation, etc).

 */

/*!
    \page TIOVX_SAFETY_FEATURES TIOVX Safety Features

    \tableofcontents

     \section TIOVX_SAFETY_FEATURES_TIMEOUT TIOVX Timeout

     By default, certain API's within OpenVX allow users to specify timeout values for the API operation.  This can present an
     issue in the case of a heterogeneous system such as TI's SoC's in the event that a remote processor or OpenVX target
     experiences a fault and thus cannot receive new messages from the host CPU.  Given the plausibility of this event occurring,
     safety systems must have a mechanism by which to recover from this issue.

     If any of the blocking calls of API's from the following OpenVX objects fail due to a timeout issue, the return value will be
     \ref TIVX_ERROR_EVENT_TIMEOUT.  More information about how timeouts can be used with these objects is described below.

     \subsection TIOVX_SAFETY_FEATURES_TIMEOUT_NODE Node Timeouts

     For the \ref vx_node API, a timeout can be specified by calling the \ref vxSetNodeAttribute API with the attribute
     \ref TIVX_NODE_TIMEOUT.  The timeout value can also be queried using the \ref vxQueryNode API.

     In the case that a timeout occurs during the create or delete commands of a node, the error will get propogated
     through the \ref vxVerifyGraph or \ref vxReleaseGraph calls.

     The TI extension API \ref tivxNodeSendCommandTimed also allows for specifying a timeout value at which time the API
     will return the \ref TIVX_ERROR_EVENT_TIMEOUT value.

     \subsection TIOVX_SAFETY_FEATURES_TIMEOUT_GRAPH Graph Timeouts

     For the \ref vx_graph API, a timeout can be specified by calling the \ref vxSetGraphAttribute API with the attribute
     \ref TIVX_GRAPH_TIMEOUT.  The timeout value can also be queried using the \ref vxQueryGraph API.

     The following graph related API's may return the \ref TIVX_NODE_TIMEOUT error when enabling timeouts on the graph or
     nodes within the graph.
     - \ref vxCreateGraph
     - \ref vxVerifyGraph
     - \ref vxProcessGraph
     - \ref vxWaitGraph
     - \ref vxReleaseGraph

     The documentation section \ref TIOVX_TARGET_KERNEL provides some additional details about how the timeouts interact
     with the target kernel callback structure.

     \subsection TIOVX_SAFETY_FEATURES_TIMEOUT_CUSTOM Custom Timeout Configuration in Target Kernel

     For a custom target kernel, a timeout can be implemented at the callback level in order to return in the case
     that the timeout needs to be set at that granularity.

     The recommended approach to achieve this is to configure the node API to pass a \ref vx_user_data_object containing
     timeout values to the custom node.  The \ref tivxTaskWaitMsecs can then be used when setting the timeout.
     Please reference the target kernel implementation of the tivxCmdTimeoutTestNode for an example of how this can
     be achieved.
 */

/*!
    \page TIOVX_SAFETY_TOOLING TIOVX Safety Tooling

    \tableofcontents

     \section TIOVX_SAFETY_TOOLING_STRUCT_CHECKING TIOVX Safety Tooling for Static Structures

     Given the requirements of utilizing static memory within safety systems, TIOVX along with other lower level components
     utilize a static allocation of structures.

     \subsection TIOVX_SAFETY_TOOLING_STRUCT_CHECKING_HEAP Remote Core Heap Statistics Tool

     The SDK provides tooling around to identify the usage of the static structures within lower level components by
     querying the amount that are being used by an application along with how many are still available to use.  This
     can be utilized in the context of an application to identify the case that any teardown calls were accidentally
     excluded by calling this tool before and after an application is run.

     Please reference the "Remote Core Heap Statistics" utility application within the vision_apps component for more
     information.

     \subsection TIOVX_SAFETY_TOOLING_STRUCT_CHECKING_CONFIG TIOVX Configuration Tool

     TIOVX also provides a tool which can be used to identify how many elements of a given configuration value are
     used by a given application.  More information around this tool can be found at the link \ref TIOVX_MEMORY_MANAGEMENT_OPTIMIZATIONS.

*/
