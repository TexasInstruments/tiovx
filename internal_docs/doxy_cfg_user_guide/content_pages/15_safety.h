/*!
    \page TIOVX_SAFETY TIOVX Safety Manual

    This section of the user guide describes relevant safety recommendations required for the ASIL-B certification
    of the TI OpenVX framework.  This activity is planned to be finalized in July 2024.

    Please reference the TI software safety qualification approach for more information about what all collateral
    shall be provided for final certification of the SW module.

    - \subpage TIOVX_SAFETY_SCOPE - Details the scope of what is considered for safety within TIOVX.
    - \subpage TIOVX_SAFETY_RECOMMENDATIONS - Various TIOVX safety recommendations when designing a SW system.
    - \subpage TIOVX_SAFETY_FEATURES - Description of features within TIOVX which can be useful when designing a SW system.
    - \subpage TIOVX_SAFETY_TOOLING - Description of provided tooling for helping enable a safety application using TIOVX.
 */

/*!
    \page TIOVX_SAFETY_SCOPE TIOVX Safety Scope

    \tableofcontents

     \section TIOVX_SAFETY_SCOPE_OVERALL What is and is not in scope for TIOVX safety

    What is in scope for TIOVX safety:
    - TI's implementation of the OpenVX framework (source/framework) with few exceptions listed in the "out of scope" section below.
    - The OS platform layer (source/platform)
    - The dependent utils of TIOVX within app_utils.  The specific libraries within this component are listed below:
        - utils/ipc
        - utils/mem
        - utils/misc
        - utils/rtos
        - utils/timer

    Please note the following which are out of scope for safety of TIOVX:
    - OpenVX standard kernels
    - Supernode extension (Note: this is not yet enabled on J7 platforms, but there are still references to it in the framework.
      These are thus commented out and disabled.)
    - VXU functions
    - Debug logging functionality
    - RT logging functionality
    - Export to dot file functionality

    Important note to how application shall be written using TIOVX that are used for safety: the "vx-" or "tivx-" functions shall only be used
    within a safety application, not the "own-" prefixed functions.  These functions shall only be called within the context of the framework itself,
    not applications.

     \section TIOVX_SAFETY_SCOPE_OS_SUPPORT TIOVX OS Support

     The aspects of TIOVX which are considered under the safety qualification effort are the framework, platform layer and associated utils layers.  For the
     remote cores supported via TIOVX safety, the safety OS which is supported is SafeRTOS.  For the host side components, given that the Linux and QNX OS
     layers do not support safety, a safety OS would need to be used in place of Linux and QNX.  Please note though that the generic POSIX platform layer will
     have safety qualification collateral (e.g., MISRA-C, Code Coverage, Requirement documentation, etc).
 */

/*!
    \page TIOVX_SAFETY_RECOMMENDATIONS TIOVX Safety Recommendations

    \tableofcontents

     \section TIOVX_SAFETY_INITIALIZATION TIOVX Initialization in Safety Systems

     TI provides sample host side application code along with target side sample firmware within the "vision_apps" project (included with the PSDK RTOS).
     This serves as an example for how the initializations should be done within a system integrating TIOVX.  There are several modules within the "app_utils"
     project which require initialization in order for TIOVX to work properly, including such items as shared memory, IPC, Sciclient, etc.

     For host side initialization of TIOVX, the appInit function within vision_apps/utils/app_init shall be called.  This functions calls the appCommonInit,
     \ref tivxInit and \ref tivxHostInit calls.  The appCommonInit call performs the necessary host side initializations outside of TIOVX, including the
     set up of shared memory, IPC, timers as well as some optional initializations such as logging, performance measurement, etc.  This function is supported on
     both Linux and QNX, with the only difference across the two is the resource table usage on Linux (not supported on QNX).  The API also ensures that
     these initializations are only performed once per system, in the case of multi-threaded or multi-process designs.

     For target side initialization of TIOVX, the example initialization of firmware occur within the vision_apps/platform/<soc>/rtos/common/app_init.c.
     This file is common across all remote cores of the system and has modules enabled/disabled based on the settings in vision_apps/platform/<soc>/rtos/common/app_cfg*.h.
     In order for TIOVX to be properly enabled, the ENABLE_TIOVX, ENABLE_IPC and ENABLE_SCICLIENT must be set.  Additionally, depending on the kernels
     enabled in the system, certain memory sections may also be required if they are depended on by the respective kernels.

     Within the ENABLE_TIOVX macro, the key initialization call in order for TIOVX to be enabled properly is \ref tivxInit.

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

     A critical component of safety SW systems is the memory management scheme.  Please reference the section \ref TIOVX_MEMORY_MANAGEMENT
     for details on how memory management is handled in TIOVX and how it facilitates safety.

     \subsection TIOVX_SAFETY_MEMORY_MANAGEMENT_APPLICATION Requirements for Application Usage of OpenVX data objects

     OpenVX has a clear “initialization” phase, “run-time” phase, and “deinit” phase for each graph.  The initialization phase for a graph
     ends when the call to \ref vxVerifyGraph returns.  With one specific case exception listed below, all of the resources are initialized, and
     at no point in the run-time phase the framework or kernels allocate memory. (Note that it is possible that the application itself may still
     allocate memory as in the case of the control callback objects discussed below.)

     The exception to this case is if the application wants to call control commands which require it to create additional OpenVX references which are
     not already node parameters and thus allocated as a part of the verify graph call.  Due to this fact, an application should ideally create, map and
     unmap these data references prior to the call to \ref vxVerifyGraph.  This allows the data references to be allocated and thus the full system memory
     will be considered when returning a status from the graph verification.

     If an application initializes all of the graphs it expects to run prior to running, it is guaranteed that it has all the
     memory resources reserved and will not run into an out of memory condition or memory fragmentation, which is the primary rationale for this rule.
     In the case of having multiple graphs spanning multiple threads or processes, the create and verification of all graphs in a system shall be called
     prior to the process phase of any graph in the system.  This is the recommended approach when using OpenVX.

     In TIOVX, all framework and data objects “created” in the init phase reserve statically allocated slots in a specific global memory array of objects,
     and max values that define the length of this array are defined statically at build time using the respective SoC tivx_config.h.  During development,
     if a use case exceeds the max allocation from these lists, then a run-time terminal error print which says which value to increase in this file, and
     the user can rebuild and run again.  In order to ensure no memory is wasted, we have a feature where the user can initialize their application to be
     used in production and run a function which creates an updated version of this file which sets all the max values to the peak usage at the time that
     the function was called.  This way the user can recompile the framework using this new header file and only allocate the memory required for this application
     use case.

     For data buffers, there is a specific contiguous shared memory carveout that data buffers get allocated out of during the \ref vxVerifyGraph function call.
     Since these are all done during initialization time, then once the vxVerifyGraph function returns, that graph will never experience an out of memory or memory
     fragmentation issue.

     When designing applications, the application shall not selectively delete graphs or memory associated with OpenVX objects.  Rather, it should persist throughout
     the duration of the application.  The reason for this is that selective deletion and re-creation of various OpenVX objects can lead to memory fragmentation.

     \subsection TIOVX_SAFETY_MEMORY_MANAGEMENT_OBJ_DESCRIPTOR_TABLE Requirement for object descriptor table

     As mentioned above and in the \ref TIOVX_MEMORY_MANAGEMENT documentation, TIOVX uses a table of object descriptors in non-cached memory which are exchanged
     across nodes in order to access data buffers.  Upon firmware boot, this table is reset by the remote core firmware and not on the host side.  Furthermore,
     this table is modified by the framework when new object descriptors are populated or removed from the table.

     From the application side, it is necessary to avoid applications writing into this memory, as this will corrupt the object descriptors and therefore could
     result in invalid reads or writes from remote cores.

     \subsection TIOVX_SAFETY_MEMORY_MANAGEMENT_PHYS_ADDR Note about Physical Addresses

     As a note to application developers, physical addresses are used in multiple places within user space (for instance, with the API \ref tivxMemTranslateVirtAddr).
     This is important to note, as a misbehaving application could corrupt this value and cause crashes on remote cores.  Care should be taken to avoid corrupting
     these values within the application.

     \subsection TIOVX_SAFETY_EXTERNALLY_ALLOCATED_MEMORY Requirements for Memory Allocated Outside of TIOVX Framework

     Certain API's allow an OpenVX data object to be associated with memory allocated from outside of the framework.  There are several important constraints
     for such memory.  There are a few API's in question which are explained further below.

     \subsubsection TIOVX_SAFETY_EXTERNALLY_ALLOCATED_MEMORY_IMAGE Requirements of vxCreateImageFromHandle and vxSwapImageHandle

     Both the \ref vxCreateImageFromHandle and \ref vxSwapImageHandle API allow for the importing of memory which may or may not have been allocated using the TIOVX
     framework to a \ref vx_image object.  In order to avoid errors, the memory which is being imported to these objects are required to be allocated using the
     \ref tivxMemAlloc API even though there are no explicit error checks in the framework for this requirement.

     \subsubsection TIOVX_SAFETY_EXTERNALLY_ALLOCATED_MEMORY_IMPORT Requirements of tivxReferenceImportHandle

     The \ref tivxReferenceImportHandle API has several important restrictions in how it is to be used within TIOVX.  The API guide gives details as to
     the requirements of the imported handle, which must be adhered to.  In particular, there are a few important aspects of the imported handles that need
     to be reviewed below:

     - OpenVX data type requirements.  The only provided list of data types are valid.
     - Memory region requirements.  An error is thrown if the memory is not created from the region specified
     - Memory alignment requirements.  While there is not a check for this given that it is simply a memory address, the API is required to be used for memory
       allocation will automatically align the memory to the required alignment
     - There is an error thrown if the corresponding number of entries doesn't match a set of number of valid addresses.  If the total number of memory pointers
       are not equal to the number of pointers required for the reference, then an error will be thrown.
     - Subimages of a given image object will not be imported to the subsequent imported image object.

     For more information about how to use this API, please refer to the Producer/Consumer application within vision_apps as well as the test cases
     found at tiovx/conformance_tests/test_tiovx/test_tivxMem.c

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

     \section TIOVX_SAFETY_SPINLOCK_USAGE TIOVX Spinlock Usage and Recommendations

     There are a few different scenarios in which a spinlock is required to be used by TIOVX in order to provide exclusive access amongst the multiple cores
     which may require access to a given piece of information.  The 3 scenarios are listed below along with the spinlock ID which is used for that scenario:

     - Run time event logger: \ref TIVX_PLATFORM_LOCK_LOG_RT_HW_SPIN_LOCK_ID
     - Object descriptor table: \ref TIVX_PLATFORM_LOCK_OBJ_DESC_TABLE_HW_SPIN_LOCK_ID
     - Data reference queue: \ref TIVX_PLATFORM_LOCK_DATA_REF_QUEUE_HW_SPIN_LOCK_ID

     There is no resource manager for spinlocks within the SDK.  Therefore, it is important for an application developer to guarantee that no other piece of
     software assumes access to these locks.  If other software components are using these locks, it will cause significant delays in execution of TIOVX.
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

     \section TIOVX_SAFETY_FEATURES_EVENT TIOVX Event API

     TIOVX supports the event handling API which is included in the OpenVX Pipelining and Streaming Extension (link found in \ref RESOURCES).

     This event handling API can be useful for detecting node level errors by using VX_EVENT_NODE_ERROR within the \ref vx_event_type_e
     enumeration.  This allows an application to use the \ref vxRegisterEvent API to know when an error has occurred within the process
     callback of a node.

     One limitation of this approach is that the exact error code is not provided, only an event which signals that an error has occurred.
     At present, the suggested approach for determining any further information is to additionally register a control callback within the
     node which can be queried by the application if an error has occurred.

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
