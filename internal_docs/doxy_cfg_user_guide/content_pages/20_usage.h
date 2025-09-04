/*!
    \page TIOVX_USAGE TIOVX Usage

    This section of the user guide includes various pages with notes, tips, and detailed guides
    for expected usage of TIOVX that goes beyond the details of the OpenVX spec.

    As a reference to sections below, the Khronos standard specs implemented by TIOVX are listed here on the \ref RESOURCES page.

    - \subpage TIOVX_USAGE_REC - Various usage recommendations, guidelines, and performance considerations.
    - \subpage TIOVX_SPEC_INTERPRETATIONS - The OpenVX spec sometimes, intentionally or unintentionally, leaves things open
      to interpretation.  In some places, it also explicitly says "implementation specific".  This section attempts to define
      how TIOVX implements those "implementation specific" and otherwise "undefined" details.
    - \subpage TIOVX_TARGET_KERNEL - The OpenVX spec defines host-executable user kernels, but does not specify how to add
      user target kernels.  This section describes TI's User Target Kernel exension.
    - \subpage TIOVX_ADD_TARGET - This section describes the concept of targets in TIOVX and explains how to add new targets.
    - \subpage TIOVX_TARGET_EXECUTION - This section describes the Target Execution Model, including TIDL Network Preemption Support in TIOVX.
    - \subpage TIOVX_DEBUG - This section describes debug tools for TIOVX.
    - \subpage TIOVX_MEMORY_MANAGEMENT - This section describes memory management in TIOVX.
    - \subpage TIOVX_PIPELINING - This section describes performance considerations and appropriate usage of pipelining in TIOVX.
    - \subpage TIOVX_IMPORT_EXPORT - This section describes the usage model of the import reference and export reference API within TIOVX.
 */

/*!
    \page TIOVX_USAGE_REC TIOVX Usage Recommendations

    \tableofcontents

     \section TIOVX_USAGE_REC_SET_NODE_TARGET vxSetNodeTarget()
       -  vxSetNodeTarget() should only be called on nodes of a graph before calling \ref vxVerifyGraph() for that graph.
       -  If a kernel that has only one available target is instantiated, the vxSetNodeTarget() API does not have to be called
          for that instantiated kernel.  It will default to the only available target.
       -  If multiple targets are available for a given kernel and the vxSetNodeTarget() is not called, the framework will default
          to the first target added to the kernel via the tivxAddKernelTarget() API, which is called when adding the target kernel
          to the context.
       -  As per a requirement of the conformance tests, the vxSetNodeTarget API accepts string target values of "any", "aNy" and "ANY"
          then the behaviour is the same as if the VX_TARGET_ANY enum was passed.  If a legitimate TI-target is required for a given
          node, that target value must be provided to the API.
     \section TIOVX_USAGE_REC_REMAP vxGetRemapPoint() and vxSetRemapPoint()
       -  The vxGetRemapPoint() and vxSetRemapPoint() API's must always get or set the (0,0) element first prior to any other get or set.  Additionally,
          the element at (remap_width, remap_height) must be set or gotten last.  This is because getting or setting the (0,0) element performs a map of the remap
          object and getting or setting the (remap_width, remap_height) element performs an unmap of the remap object.
     \section TIOVX_USAGE_STRIDE Stride Alignment for Images and Raw Images
       -  TIOVX automatically aligns the stride of images and raw images to the value of \ref TIVX_DEFAULT_STRIDE_Y_ALIGN.  This is a requirement of certain
          hardware accelerators (such as LDC) and should not be modified.
     \section TIOVX_USAGE_REC_GRAPH_PERF Graph Performance API's with pipelining
       - The graph performance API's (\ref VX_GRAPH_PERFORMANCE) give the overall time to execute a graph.
       - However, when using the pipelining extension, this may not be as useful in the case that one may need to calculate the frame rate of the graph.
       - In this case, it is recommended to use the API's provided in vision_apps for calculating the frame rate and other performance information
         <a href="https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/vision_apps/docs/user_guide/group__group__vision__apps__utils__perf__stats.html" target="_blank">here</a>
     \section TIOVX_USAGE_REC_WORKER_THREAD Enabling a node within the graph to run at lower FPS than rest of graph
       - In some situations, it may be desirable to have a node within the graph run at a lower FPS than the rest of the graph.
       - In these scenarios, the node itself can be customized using the steps below to accomplish this.
           - A thread can be created from the create callback of the custom node.
           - Pass message/control commands from this thread in process callback.
           - Close thread from destroy callback
       - This is done in the \ref tivxHwaVpacVissNode to pass information to the tivxAewbNode and can be referenced for custom node implementations.
     \section TIOVX_USAGE_VIRTUAL_OBJECTS Virtual Object Support in TIOVX
       - While the OpenVX standard specification provides a provision for implementations to optimize memory footprint via virtual objects, the TIOVX framework
         does not provide any added optimization from using virtual objects.
       - Certain nodes, such as the DSS M2M node and the Mosaic node, do not support virtual objects.  In these cases, the standard objects should be used instead.
     \section TIOVX_USAGE_MAP_COPY_API Map and Copy API Usage in TIOVX
       - Each OpenVX data object has an associated vxMap<Data_Object> and vxCopy<Data_Object> API.
       - In general, all memory allocation within TIOVX occurs during the \ref vxVerifyGraph call, in order to provide a single return regarding whether or not
         a system has sufficient memory to run applications.  Therefore, even if the data object is created via the vxCreate<Data_Object> API, the memory will only
         be allocated at the call to \ref vxVerifyGraph.
       - There are a few exceptions to this rule:
           - The vxMap<Data_Object> and vxCopy<Data_Object> API's are an exception to this rule about memory allocation occurring at a single point.  Given that these API's
             return a pointer to the object data buffer, the memory for these data objects are allocated in the call to vxMap<Data_Object> and vxCopy<Data_Object>.  When the
             call to \ref vxVerifyGraph occurs, it will have already allocated this memory prior to executing the graph verification.
           - An additional exception to this rule is for the \ref vx_user_data_object API \ref vxCreateUserDataObject.  This API allows the developer to provide a data structure
             instance to populate the memory at creation of the \ref vx_user_data_object itself.  Therefore, in this case, the memory for the \ref vx_user_data_object will be
             allocated.
           - Similarly, the memory for a \ref vx_scalar will be allocated when calling \ref vxCreateScalar in the case that a scalar value is passed at create time.
     \section TIOVX_USAGE_IMAGE_API TIOVX Implementation of Image API's
       - The \ref vx_image API's below allow an application to use an allocated handle to create or swap into an OpenVX image object
           - \ref vxCreateImageFromHandle
           - \ref vxSwapImageHandle
       - An important note about these API's is that the handles used by these API's must be allocated using \ref tivxMemAlloc rather than by using malloc or other such memory.
     \section TIOVX_USAGE_SUBIMAGE_API TIOVX Implementation of Subimages
       - The OpenVX specification does not define the depth of the number of the subimages which a \ref vx_image may contain
       - Therefore, the TIOVX implementation has added a max value of "2" for the number of subimage levels, as well as a max value of "16" for the number of subimages per level.
       - These are configurable and can be updated in the max value configuration file.
     \section TIOVX_USAGE_REC_SEND_NODE_COMMAND tivxNodeSendCommand()
       -  The intention behind the \ref tivxNodeSendCommand() is to send non-real time data to a node and/or to query for information from the node
       -  Particularly if using pipelining, this API shall not be used for sending/receiving data from a node for every execution of the node.  If that is the requirement of a
          given use case, the node API should be changed to encapsulate this parameter.
     \section TIOVX_USAGE_SUPPL_DATA Supplementary Data with Composite Objects from Exemplar
       -  For the specific scenario of creating a composite object (i.e., a vx_pyramid or a vx_object_array) from exemplar when the composite object used as the exemplar contains
          supplementary data, the children objects of the resulting composite object will contain the same supplementary data as the base object (i.e., the vx_pyramid or vx_object_array).
       -  As an example, in the case that a vx_object_array "ABC" is created with supplementary data "XYZ", if an exemplar of object ABC is created, then the resulting object array
          elements will contain supplementary data "XYZ" as well.
     \section TIOVX_USAGE_TASK TIOVX Task API on MPU Cores
       -  When used on MPU cores running an HLOS (i.e., Linux or QNX), the priority parameter of the tivx_task API is ignored.  This is left to maintain compatibility with RTOS.
       -  If a thread needs to be created with a priority, please instead use the native pthread API
     \section TIOVX_USAGE_CACHE_IMPLICATIONS Using VX_MEMORY_TYPE_HOST vs TIVX_MEMORY_TYPE_DMA for map/unmap APIs
       -  Use \ref VX_MEMORY_TYPE_HOST for map/unmap operations when you want the calling CPU to access the buffer contents.  When doing this, the map/unmap operation will take
          care to also call the associated cache maintenance operations on the region specified (if required for the platform and CPU) so that the user doesn't have to.
       -  Use \ref TIVX_MEMORY_TYPE_DMA for map/unmap operations when you DO NOT intend for the calling CPU to access the buffer contents (perhaps you just need the pointer to pass
          to a hardware accelerator or DMA engine to access the contents).  When doing this, the map/unmap operation will skip the associated cache maintenance operations since it is
          not required in this situation, thus performing more optimally.
       -  Given that using \ref VX_MEMORY_TYPE_HOST may come at an additional cost of cache maintenance operations over the buffer, the application may consider the following performance optimization in certain cases:
            -  One such case is if the user needs access to a buffer to perform some sparse accesses across the region, then the user may not want to waste the time to do cache maintenance on the whole region.
               -  In this case, the user can decide to map/unmap the buffer using \ref TIVX_MEMORY_TYPE_DMA to skip the cache maintenance, and then take care of performing cache maintenance operations on just the specific subset
                  of the buffer before and after access.
            -  The map/unmap functions of the TIOVX implementation assumes that the buffers are allocated in cacheable write-back memory regions.  If the configuration is changed such that the map/unmap functions are called on a
               non-cached region, or a cached write-through region, then this assumption no longer applies and the following can be done to skip the unnecessary cache maintenance operation(s):
               -  Either the \ref tivxMemBufferMap or \ref tivxMemBufferUnmap functions should be altered, or
               -  The applciation can call one or both the map/unmap functions using memory type \ref TIVX_MEMORY_TYPE_DMA
 */
/*!
    \page TIOVX_SPEC_INTERPRETATIONS OpenVX Standard Specification Interpretations

    \tableofcontents

     \section TIOVX_SPEC_INTERPRETATIONS_OVX1_1_SPEC OpenVX 1.1 Specification: Vendor-specific implementation details broken down by section
       - Section 2.10.2: User Kernels Naming Conventions
           - TI's vendor ID is VX_ID_TI
           - TI user kernel strings begin with "com.ti"
           - TI user kernel enums begin with TIVX_KERNEL_
           - The ':' character is reserved in TI's implementation, so a user kernel shall not use a ':' as part of the kernel name string.
       - Section 2.12: Targets
           - TI's targets can be found at <TIOVX_PATH>/include/TI/tivx.h
       - Section 2.18.1: Extending Attributes
           - TI's vendor extension ID is VX_ID_TI
       - Section 2.18.2: Vendor Custom Kernels
           - Further documentation is provided for any custom kernel extension
       - Section 2.18.3: Vendor Custom extensions
           - TI does not currently support any custom extensions of the form mentioned in this section.  See ti extensions as part of this API documentation.
       - Section 2.18.4: Hinting
           - TI does not currently support any custom hints
     \section TIVX_NAMESPACE TIVX Namespace Naming convention
       - TI's naming convention append "tivx" to the front of a given object. For example, a TI extension of the df_image_e is called "tivx_df_image_e".
       - TI's enumeration values append "TIVX" to the front of an enum. For example, a TI-specific target host is called TIVX_TARGET_HOST.
     \section TIOVX_SPEC_INTERPRETATIONS_CONTEXT_CREATION Context Creation
       - The TI Implementation of OpenVX supports the creation of a single context per process running on the host CPU. Additional calls to vxCreateContext beyond the initial call
         from that process will simply return a reference to the initially created context.
       - Furthermore, the TIVX_CONTEXT_MAX_OBJECTS does not take effect. It merely provides the value of the amount of contexts that are allowed per process, namely 1.
     \section TIOVX_SPEC_INTERPRETATIONS_VALID_REGION Valid Region
       - The valid regions for each kernel are set prior to graph verification and restrictions to valid regions are propagated to downstream kernels during the initialize callbacks of each kernel.
       - After graph verification, the valid regions for each kernel have been set.
       - Therefore, following graph verification, changes to the valid region using vxSetImageValidRectangle must not be made.
     \section TIOVX_SPEC_INTERPRETATIONS_OBJ_DATA_MAPPING Data Object Mapping
       - The OpenVX data objects are opaque:
           - Access to memory is explicit and temporary
           - No permanent pointers to internal memory/data
       - One way this is done is with Map/Unmap APIs
           - Every mapping must be explicitly unmapped after use
       - The following functions are used for mapping of specific data objects:
           - \ref vxMapArrayRange
           - \ref vxMapImagePatch
           - \ref vxMapDistribution
           - \ref vxMapLUT
           <!-- This Khronos page is still an asciidoc page so html hyperlinking is used -->
           - <a href="https://registry.khronos.org/OpenVX/extensions/vx_khr_user_data_object/1.1/vx_khr_user_data_object_1_1.html#_vxmapuserdataobjectgit">vxMapUserDataObject</a>
           - \ref tivxMapTensorPatch
           - \ref tivxMapRawImagePatch
       - The types of mapping supported for these functions are the following:
           - VX_READ_ONLY
           - VX_READ_AND_WRITE
           - VX_WRITE_ONLY
       - The framework allows multiple mappings of the same region of a given data object.
           - Therefore, the user must be aware of the regions that have been mapped and the read/write privileges that each mapping has been granted.
     \section TIOVX_SPEC_INTERPRETATIONS_TENSOR Tensor Object
       - Each tensor is mapped to a single contiguous buffer, of size = stride[number_of_dimensions - 1U] * dimensions[number_of_dimensions - 1U]
     \section TIOVX_SPEC_INTERPRETATIONS_USER_DATA_OBJECT User Data Object
       - The user data objects included as a part of the OpenVX User Data Object Extension allow you to create an OpenVX data object from a custom data structure.
         If the custom data structure were to include OpenVX data objects itself, the framework would not be able to recognize this fact and thus would not allocate
         the data buffers corresponding to that data object and would not assign object descriptors to the references included in the structure.  Also, these OpenVX
         data object handles are not recognized on remote targets due to the framework not performing memory pointer translation, as well as missing host APIs on
         remote cores.  For expert users, this allocation and assignment could be done manually within the application, but it is not recommended.
     \section TIOVX_SPEC_INTERPRETATIONS_REQUIRED_PARAMETERS Required Node Parameters
       - The OpenVX 1.3 specification allows for required parameters to be NULL during create time as per the last paragraph in the below portion of the specification:
           - https://www.khronos.org/registry/OpenVX/specs/1.3/html/OpenVX_Specification_1_3.html#sec_graph_verification
       - However, given that TI OpenVX supports the 1.1 specification, the required parameters must be non-NULL during create time.
     \section VXREMOVEKERNEL vxRemoveKernel
       - The OpenVX 1.1 specification for \ref vxRemoveKernel states that it returns a VX_FAILURE "If the application has not released all
         references to the kernel object OR if the application has not released all references to a node that is using this kernel
         OR if the application has not released all references to a graph which has nodes that is using this kernel."
       - The TI implementation is more relaxed than this and does not return \ref VX_FAILURE due to any of these conditions.  Instead,
         we make use of reference counting such that it is allowed to call this function prior to the application or other objects still
         retaining references.  Calling \ref vxRemoveKernel will mark the kernel to be removed when all references are released, either at the time
         of calling \ref vxRemoveKernel, or at a later time in the application teardown process.
       - This removes a burden of order of releasing references from the application, and is carried by the framework, which is consistent with how
         other OpenVX objects are defined to work.
     \section VXGETCONTEXT vxGetContext
       - The OpenVX 1.1 specification for \ref vxGetContext does not state the return value in the case that a \ref vx_context is provided to this function
       - For TI's implementation of OpenVX 1.1, the \ref vxGetContext will return the same \ref vx_context supplied as a parameter to this function.
     \section TIOVX_SPEC_INTERPRETATIONS_CREATE_OBJ_ARR Object Array and Object Array Elements
       - After creating a vx_object_array using \ref vxCreateObjectArray and obtaining a vx_object_array element using \ref vxGetObjectArrayItem, both
         the vx_object_array element and the vx_object_array itself must be released in order to avoid having a dangling reference when releasing the
         OpenVX context.
       - The order in which the vx_object_array and the vx_object_array elements are to be released is not mandated by the framework
       - A \ref vx_object_array cannot have a \ref vx_object_array as an exemplar.  If this is attempted, the creation of the object array will return an error
     \section TIOVX_VIRTUAL_OBJECTS Virtual Objects
       - For TI's implementation of OpenVX 1.1, virtual objects are treated as regular objects in most cases. For example, creating a \ref vx_image
         object by calling into \ref vxCreateVirtualImage and then passing it to \ref vxMapImagePatch would typically return an error as noted in
         the OpenVX specification. However, this behavior is not enforced in TIOVX. Using a virtual image with \ref vxMapImagePatch functions the same
         as an image created via \ref vxCreateImage.
     \subsection OBJARRAY_PYRAMID_GET vxGetObjectArrayItem and vxGetPyramidLevel on Virtual Objects
       - Similarly, objects created through calls to \ref vxCreateVirtualObjectArray and \ref vxCreateVirtualPyramid are accessible by the user
         directly. Calling \ref vxGetObjectArrayItem and \ref vxGetPyramidLevel on these virtual objects will return a valid reference
         regardless of whether they are virtual or not.
 */

/*!
    \page TIOVX_TARGET_KERNEL User Kernels and User Target Kernels

    \tableofcontents

    \section USER_KERNELS User Kernels

    The OpenVX specification describes how users can plugin their own kernels (which are only executed on the host core)
    into OpenVX.  TIOVX supports this feature, so for more details on this topic, please refer to the following section
    from the spec directly: \ref group_user_kernels

    \section USER_TARGET_KERNELS User Target Kernels

    Since the OpenVX specification only supports host-side user kernels, TI has created its own vendor extension which adds
    support for users to add kernels on other targets in the system.  The API for this extension is: \ref group_tivx_target_kernel

    This extension tried to preserve the pattern from the OpenVX spec as much as possible.  In fact, on the host cpu, the same
    apis are used to add a user kernel, except the kernel processing callback should be set to NULL (since the callback
    is not located on the host core).

    Then, on the target side, the \ref tivxAddTargetKernelByName function is called to register the target-side callbacks:
    - __process_func__: Main processing function which is called each time the graph is executed.
    - __create_func__: Called during graph verification, to perform any local memory setup or one-time configuration.
    - __delete_func__: Called during graph release, to release local memory or tear-down any local setup.
    - __control_func__: Can optionally be called asyncronously via \ref tivxNodeSendCommand from the application.

    The following call sequence shows the relative interaction between the host application and the target kernel callbacks:
       - \mscfile cdf_verification.msc Call sequence of a Graph Verify and Release with User Target Kernels.

    \subsection USER_TARGET_KERNEL_CALLBACK_GUIDELINES Callback Implementation Guidelines

    When a framework includes user callbacks, there are usually assumptions that the framework makes about how those callbacks
    are implemented.  TIOVX is no exception.  The following sections contain guidelines and assumptions that User Target Kernel callback
    implementers should follow for proper usage when using both the default behavior of the TIOVX framework as well as some additional
    considerations when using graph or node level timeouts.

    \subsubsection USER_TARGET_KERNEL_CALLBACK_GUIDELINES_STANDARD Standard Callback Implementation Guidelines

    - __create_func__:
      - __Thread/blocking Implications__: The \ref vxVerifyGraph function is a blocking function which runs to completion before returning.  It calls the create_func
        callback for each node one by one (sequentially) and then doesn't return until all node create function callbacks return.  Therefore, the
        following guidelines should be followed:
        - Do not assume some dependency on another node's create function since it may not have executed yet in the sequence of calls to each node.
        - Do not assume some dependency on some action that the application does after returning from \ref vxVerifyGraph.  For example,
          a blocking call called from within the create_func will result in blocking the full \ref vxVerifyGraph, potentially causing a deadlock
          if the create_func is waiting for further action from the same thread in the application which called \ref vxVerifyGraph, or from another
          node's create function.
      - __Memory Implications__: If there is some context which needs to be accessed for the other target-side callbacks, it should be created in the create_func
        since memory allocations are not allowed in any callback except the create_func callback.
        - The context pointer can be allocated using \ref tivxMemAlloc.  The following is an example of the allocation of the
          `tivxCannyParams` data structure context:
     ~~~
          tivxCannyParams prms = tivxMemAlloc(sizeof(tivxCannyParams), TIVX_MEM_EXTERNAL);
     ~~~
        - The allocated context needs to be added to the kernel instance using the \ref tivxSetTargetKernelInstanceContext function (so the other callbacks
          can retrieve it):
     ~~~
          tivxSetTargetKernelInstanceContext(kernel, prms, sizeof(tivxCannyParams));
     ~~~
        - If the node instance needs additional scratch or persistent memory, memory can be "allocated" in the create_func callback using
          the \ref tivxMemAlloc function. Then the corresponding pointers and sizes can be added to the context structure.  For persistent memory allocations, allocation can be made out of
          \ref TIVX_MEM_EXTERNAL, for scratch memory, it can made out of \ref TIVX_MEM_EXTERNAL_SCRATCH.
          - If allocating out of scratch memory, the following function should be called for each node before any scratch allocations are made:
     ~~~
          status = tivxMemResetScratchHeap(TIVX_MEM_EXTERNAL_SCRATCH);
     ~~~

    - __process_func__:
      - __Thread/blocking Implications__: The process_func callback is called for each node in order of graph dependency, and each target is executing in its own thread.  Therefore, for each target,
        the node process functions are queued up in order of arrival at each target, and are executed to completion before starting the next node process function
        queued up for that specific target.  Nodes running on different targets can execute in parallel.
        Therefore, the following guidelines should be followed:
        - If no target is assigned by the application, the framework statically assigns it to a target in which it is implemented for. The framework
          does not automatically optimize node target assignments in the graph to run in parallel on another target. This optimization is left to the application writer.  For example, if
          a kernel is implemented on the C66 DSP, and the platform support 2 C66 DSP targets, then the framework will assign both of them to run on
          \ref TIVX_TARGET_DSP1 sequentially.  If the graph is written is such a way that these can be run in parallel or pipelined across both DSPs, then
          the application can call \ref vxSetNodeTarget for each of the nodes to run on a different target.
      - __Memory Implications__: No memory allocations should happen in the process_func callback.  Any memory allocations should have been created in the
        create_func (see above).
        - If there is some context which needs to be accessed/updated from the create_func callback, it can be retrieved from the kernel instance using the
         \ref tivxGetTargetKernelInstanceContext function:
     ~~~
          status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);
     ~~~

    - __control_func__:
      - __Data Object Verification Implications__: Since the objects being used with the control callback are not necessarily node parameters, the parameters
        are not subject to the validate callback checks being done during the call to \ref vxVerifyGraph.  Therefore, if an object is to be used within a control
        callback, it is necessary for the control callback (or some other mechanism within the application, etc) to perform validation of the parameters being
        used within the callback.
      - __Thread/blocking Implications__: The control_func callback (if implemented) is triggered from the application by calling \ref tivxNodeSendCommand.
        The call to \ref tivxNodeSendCommand is blocked until the target can complete execution of the corresponding control_func callback.  Note that
        each target has a pending command queue.  If, during the processing of a graph, a target has multiple process_func and control_func commands in flight
        at the same time, they are serialized on a FIFO (first in first out) order to each target's command queue, and each one is executed in order.  This guarantees
        the thread safe behavior of process_func and control_func commands within each target, but has implications on how long the corresponding
        call to \ref tivxNodeSendCommand may be blocked.
        Therefore, the following guidelines shall be followed:
        - The calling thread will be blocked until the control_func can be executed on the command queue for the target, and returns, so consider
          this in the implementation of the callback.
        - Since the call is made asynchronous to the process_func, there is no guarantee on the order or exact time the command will get executed (i.e. it could
          get executed a few frames after it was called depending on the instantaneous depth of the command queue for this target).
      - __Memory Implications__: No memory allocations should happen in the control_func callback.  Any memory allocations should have been created in the
        create_func (see above).
        - If there is some context which needs to be accessed/updated from the create_func callback, it can be retrieved from the kernel instance using the
         \ref tivxGetTargetKernelInstanceContext function:
     ~~~
          status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);
     ~~~
        - The control_func callback (if implemented) should only be called after \ref vxVerifyGraph and before \ref vxReleaseGraph, since it may need to
          access the kernel instance context, which only exists in the time between these two calls.

    - __delete_func__:
      - __Thread/blocking Implications__: The \ref vxReleaseGraph function is a blocking function which runs to completion before returning.  It calls the delete_func
        callback for each node one by one (sequentially) and then doesn't return until all node delete function callbacks return.  Therefore, the
        following guidelines shall be followed:
        - Do not assume some dependency on another node's delete function since it may not have executed yet in the sequence of calls to each node.
        - Do not assume some dependency on some action that the application does after returning from \ref vxReleaseGraph.  For example,
          a blocking call called from within the delete_func will result in blocking the full \ref vxReleaseGraph, potentially causing a deadlock
          if the delete_func is waiting for further action from the same thread in the application which called \ref vxReleaseGraph, or from another
          node's create function.
      - __Memory Implications__: All memory buffers allocated during the create_func should be freed in the destroy_func
        - If there is some context which was allocated in the create_func callback, it can be retrieved from the kernel instance using the
         \ref tivxGetTargetKernelInstanceContext function:
     ~~~
          status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);
     ~~~
        - If the node instance context included pointers/sizes to additional scratch or persistent memory allocated in the create_func callback,
          should be freed in the destroy_func callback using the \ref tivxMemFree function.
        - Finally, the allocated context needs to be freed from the kernel instance using the \ref tivxMemFree function:
     ~~~
          tivxMemFree(prms, sizeof(tivxCannyParams), TIVX_MEM_EXTERNAL);
     ~~~

    \subsubsection USER_TARGET_KERNEL_CALLBACK_GUIDELINES_TIMEOUT Callback Implementation Guidelines with Timeouts

    While the above defines the default behavior of TIOVX with respect to the node callbacks, TIOVX has provided an extension to enable timeouts to avoid
    scenarios in which a remote core goes down and cannot be communicated with (amongst other things).  These timeouts can be set at the \ref vx_graph level
    with the \ref vxSetGraphAttribute API and the \ref VX_GRAPH_TIMEOUT attribute or at the \ref vx_node level with the \ref vxSetNodeAttribute API and
    the \ref TIVX_NODE_TIMEOUT attribute.

    When designing target kernels using the callback structure noted above, a particular scenario can occur due to the timeout of communication with remote cores
    which should be considered.  For instance, consider a timeout which occurs when iterating through the create callbacks of the nodes included within the OpenVX graph.
    This timeout may occur on a remote core, and thus the host CPU may not know the state in which the remote core was in prior to timing out.  The TIOVX framework will
    not know the state of the remote core after the timeout has been received.  The framework will loop through the nodes which were created, calling each of the
    corresponding delete callbacks.  For the node that timed out, the framework will still attempt to send a message to the remote core to call the delete callback.
    It is likely that this may time out as well.  However, this is at least attempted in the case that the remote core itself was not unresponsive and rather it was
    some hardware that it was communicating with that was unresponsive.  In that case, the remote core via the delete callback should free all the allocated resources
    prior to communicating with the hardware which was unresponsive.  Therefore, delete callbacks should be designed with this possible scenario in mind.

    \subsection KERNEL_MODULE_INTEGRATION Kernel Module Integration Guidelines

    In order to integrate a kernel as a node into an OpenVX graph, the callbacks of the kernel must be registered with the framework. The OpenVX spec defines how
    user kernels are to be registered, specifically using the vxLoadKernels() API.  This registration of kernels comes as a registration of an entire module of kernels
    that have a common function.  For instance, the OpenVX standard kernels are contained within a single kernel module.  OpenVX modules can be enabled or disabled
    depending on whether or not the kernels within that module need to be included in an application.  By excluding the modules that are not used, this saves code
    size and memory.

    In the case of a user target kernel, an additional registration must be performed on the target rather than simply registering the kernel module on the host.
    This is due to the fact that target kernels contain two sets of callbacks, one set on the host and one set on the remote target core as described in the section
    above.  Therefore, each of these kernel registrations must occur on the core where the callbacks will be invoked.

    As mentioned, these invocations will occur in two places.  First, the host callbacks may be invoked directly from the application.  For instance, when
    one of the Hardware Accelerator (HWA) kernels are to be used in an application, the \ref tivxHwaLoadKernels() must be called prior to the instantiation of
    any HWA nodes in an application. \ref tivxHwaUnLoadKernels() must also be called during application deinitialization.  These API's are wrapper API's for the
    vxLoadKernels() and vxUnloadKernels() API's.  Similarly, any new kernel module must have an equivalent kernel module loading and unloading done from the application.

    Second, the target side callbacks must also be registered on the appropriate core that the kernel module in question is intended to be run on.  If integrating a kernel
    module in vision_apps, this kernel registration can be done as a part of the appRegisterOpenVXTargetKernels() in the file vision_apps/apps/basic_demos/app_rtos/common/app_init.c.
    This init code is common to all cores, so it is recommended to add build macros around the kernel registration in order to only allow it to register on the intended cores.
    As an example, the TIDL kernel module running on the C7x is registered by adding the \ref tivxRegisterTIDLTargetKernels() to the appRegisterOpenVXTargetKernels()
    with build macros around the call to guarantee that it only registers on the C7x.

    The PyTIOVX tool can be used by a kernel developer to generate much of this boilerplate kernel registration code with a simple Python script.  For more information
    about the PyTIOVX tool, please see: \ref PYTIOVX.

 */

/*!
    \page TIOVX_ADD_TARGET Adding New Targets to TIOVX

    \tableofcontents

    \section TIOVX_TARGET_EXPLANATION Explanation of Targets in TIOVX

    A good place to start in understanding the concept of a "target" in TIOVX is to compare and contrast it with the concepts of "CPU" and "core" on a TI SoC.
    TI's SoC's have a number of heterogenous cores, such as ARM's, DSP's and HWA's.  However, only the cores that run an operating system, such as FREERTOS, are
    considered CPU's.  When comparing these concepts to the definition of a "target" in TIOVX, a target is simply a thread running on a given CPU.

    TIOVX provides one or more target threads for each of the given CPU's.  Additionally, for each of the HWA's, TIOVX has a thread running on the R5F (MCU2_0) to
    call into the VPAC HWA's.  In the graph based model of TIOVX, a given node is assigned to a target thread.  When the graph is processed, if multiple nodes are
    assigned to the same target, each node processing call will run to completion on the given target before the next node on that target becomes available.
    Additionally, in the case of HWA targets, a target becomes unblocked once the R5F initiates the processing on the HWA because it is pending on the completion
    of processing on the HWA.  For additional details, please refer to \ref TIOVX_TARGET_EXECUTION

    For a list of default targets that TI provides as part of TIOVX for this SDK, refer to \ref group_tivx_ext_targets

    \section TIOVX_CORE_TARGETS Available Targets on Each Core

    The available targets on each core vary depending on the architecture of its SoC. They are listed as follows:
*/
#if SOC == J721E
/*!
    \page TIOVX_ADD_TARGET Adding New Targets to TIOVX

    \subsection J721E_R5F_CORES R5F Cores
    |             MCU2_0              |         MCU2_1           |        MCU3_0         |        MCU3_1         |
    |:-------------------------------:|:------------------------:|:---------------------:|:---------------------:|
    |      TIVX_TARGET_ID_MCU2_0      |  TIVX_TARGET_ID_MCU2_1   | TIVX_TARGET_ID_MCU3_0 | TIVX_TARGET_ID_MCU3_1 |
    |     TIVX_TARGET_ID_VPAC_NF      | TIVX_TARGET_ID_DMPAC_SDE |           -           |           -           |
    |    TIVX_TARGET_ID_VPAC_LDC1     | TIVX_TARGET_ID_DMPAC_DOF |           -           |           -           |
    | TIVX_TARGET_ID_VPAC_MSC[1 - 2]  |            -             |           -           |           -           |
    |    TIVX_TARGET_ID_VPAC_VISS1    |            -             |           -           |           -           |
    |  TIVX_TARGET_ID_CAPTURE[1 - 8]  |            -             |           -           |           -           |
    |  TIVX_TARGET_ID_DISPLAY[1 - 2]  |            -             |           -           |           -           |
    |      TIVX_TARGET_ID_CSITX       |            -             |           -           |           -           |
    | TIVX_TARGET_ID_DISPLAY_M2M[1-4] |            -             |           -           |           -           |

    \subsection J721E_A72_C66_C7X_CORES A72, C66x, and C7x Cores
    |         A72 MPU          |              C66x_1                 |              C66x_2                 |               C7x_1                 |
    |:------------------------:|:-----------------------------------:|:-----------------------------------:|:-----------------------------------:|
    | TIVX_TARGET_ID_MPU_[0-3] |         TIVX_TARGET_ID_DSP1         |         TIVX_TARGET_ID_DSP2         | TIVX_TARGET_ID_DSP_C7_1\n(TIVX_TARGET_ID_DSP_C7_1_PRI_1) |
    |            -             |                  -                  |                  -                  | TIVX_TARGET_ID_DSP_C7_1_PRI_[2 - 8] |

*/
#endif
#if SOC == J784S4
/*!
    \page TIOVX_ADD_TARGET Adding New Targets to TIOVX

    \subsection J784S4_R5F_CORES R5F Cores
    |             MCU2_0              |         MCU2_1           |        MCU3_0         |        MCU3_1         |            MCU4_0             |        MCU4_1         |
    |:-------------------------------:|:------------------------:|:---------------------:|:---------------------:|:-----------------------------:|:---------------------:|
    |      TIVX_TARGET_ID_MCU2_0      |  TIVX_TARGET_ID_MCU2_1   | TIVX_TARGET_ID_MCU3_0 | TIVX_TARGET_ID_MCU3_1 |     TIVX_TARGET_ID_MCU4_0     | TIVX_TARGET_ID_MCU4_1 |
    |     TIVX_TARGET_ID_VPAC_NF      | TIVX_TARGET_ID_DMPAC_SDE |           -           |           -           |    TIVX_TARGET_ID_VPAC2_NF    |           -           |
    |    TIVX_TARGET_ID_VPAC_LDC1     | TIVX_TARGET_ID_DMPAC_DOF |           -           |           -           |   TIVX_TARGET_ID_VPAC2_LDC1   |           -           |
    | TIVX_TARGET_ID_VPAC_MSC[1 - 2]  |            -             |           -           |           -           | TIVX_TARGET_ID_VPAC2_MSC[1-2] |           -           |
    |    TIVX_TARGET_ID_VPAC_VISS1    |            -             |           -           |           -           |  TIVX_TARGET_ID_VPAC2_VISS1   |           -           |
    | TIVX_TARGET_ID_CAPTURE[1 - 12]  |            -             |           -           |           -           |               -               |           -           |
    |  TIVX_TARGET_ID_DISPLAY[1 - 2]  |            -             |           -           |           -           |               -               |           -           |
    |     TIVX_TARGET_ID_CSITX[2]     |            -             |           -           |           -           |               -               |           -           |
    | TIVX_TARGET_ID_DISPLAY_M2M[1-4] |            -             |           -           |           -           |               -               |           -           |

    \subsection J784S4_A72_C7X_CORES A72 and C7x Cores
    |         A72 MPU          |               C7x_1                 |               C7x_2                 |               C7x_3                 |               C7x_4                 |
    |:------------------------:|:-----------------------------------:|:-----------------------------------:|:-----------------------------------:|:-----------------------------------:|
    | TIVX_TARGET_ID_MPU_[0-3] |       TIVX_TARGET_ID_DSP_C7_1\n(TIVX_TARGET_ID_DSP_C7_1_PRI_1) | TIVX_TARGET_ID_DSP_C7_2\n(TIVX_TARGET_ID_DSP1)\n(TIVX_TARGET_ID_DSP_C7_2_PRI_1) | TIVX_TARGET_ID_DSP_C7_3\n(TIVX_TARGET_ID_DSP_C7_3_PRI_1) | TIVX_TARGET_ID_DSP_C7_4\n(TIVX_TARGET_ID_DSP_C7_4_PRI_1) |
    |            -             | TIVX_TARGET_ID_DSP_C7_1_PRI_[2 - 8] | TIVX_TARGET_ID_DSP_C7_2_PRI_[2 - 8] | TIVX_TARGET_ID_DSP_C7_3_PRI_[2 - 8] | TIVX_TARGET_ID_DSP_C7_4_PRI_[2 - 8] |
*/
#endif
#if SOC == J742S2
/*!
    \page TIOVX_ADD_TARGET Adding New Targets to TIOVX

    \subsection J742S2_R5F_CORES R5F Cores
    |             MCU2_0              |         MCU2_1           |        MCU3_0         |        MCU3_1         |            MCU4_0             |        MCU4_1         |
    |:-------------------------------:|:------------------------:|:---------------------:|:---------------------:|:-----------------------------:|:---------------------:|
    |      TIVX_TARGET_ID_MCU2_0      |  TIVX_TARGET_ID_MCU2_1   | TIVX_TARGET_ID_MCU3_0 | TIVX_TARGET_ID_MCU3_1 |     TIVX_TARGET_ID_MCU4_0     | TIVX_TARGET_ID_MCU4_1 |
    |     TIVX_TARGET_ID_VPAC_NF      | TIVX_TARGET_ID_DMPAC_SDE |           -           |           -           |    TIVX_TARGET_ID_VPAC2_NF    |           -           |
    |    TIVX_TARGET_ID_VPAC_LDC1     | TIVX_TARGET_ID_DMPAC_DOF |           -           |           -           |   TIVX_TARGET_ID_VPAC2_LDC1   |           -           |
    | TIVX_TARGET_ID_VPAC_MSC[1 - 2]  |            -             |           -           |           -           | TIVX_TARGET_ID_VPAC2_MSC[1-2] |           -           |
    |    TIVX_TARGET_ID_VPAC_VISS1    |            -             |           -           |           -           |  TIVX_TARGET_ID_VPAC2_VISS1   |           -           |
    | TIVX_TARGET_ID_CAPTURE[1 - 12]  |            -             |           -           |           -           |               -               |           -           |
    |  TIVX_TARGET_ID_DISPLAY[1 - 2]  |            -             |           -           |           -           |               -               |           -           |
    |     TIVX_TARGET_ID_CSITX[2]     |            -             |           -           |           -           |               -               |           -           |
    | TIVX_TARGET_ID_DISPLAY_M2M[1-4] |            -             |           -           |           -           |               -               |           -           |

    \subsection J742S2_A72_C7X_CORES A72 and C7x Cores
    |         A72 MPU          |               C7x_1                 |               C7x_2                 |               C7x_3                 |
    |:------------------------:|:-----------------------------------:|:-----------------------------------:|:-----------------------------------:|
    | TIVX_TARGET_ID_MPU_[0-3] |       TIVX_TARGET_ID_DSP_C7_1\n(TIVX_TARGET_ID_DSP_C7_1_PRI_1) | TIVX_TARGET_ID_DSP_C7_2\n(TIVX_TARGET_ID_DSP1)\n(TIVX_TARGET_ID_DSP_C7_2_PRI_1) | TIVX_TARGET_ID_DSP_C7_3\n(TIVX_TARGET_ID_DSP_C7_3_PRI_1) |
    |            -             | TIVX_TARGET_ID_DSP_C7_1_PRI_[2 - 8] | TIVX_TARGET_ID_DSP_C7_2_PRI_[2 - 8] | TIVX_TARGET_ID_DSP_C7_3_PRI_[2 - 8] |
*/
#endif
#if SOC == J721S2
/*!
    \page TIOVX_ADD_TARGET Adding New Targets to TIOVX

    \subsection J721S2_R5F_CORES R5F Cores
    |             MCU2_0              |         MCU2_1           |        MCU3_0         |        MCU3_1         |
    |:-------------------------------:|:------------------------:|:---------------------:|:---------------------:|
    |      TIVX_TARGET_ID_MCU2_0      |  TIVX_TARGET_ID_MCU2_1   | TIVX_TARGET_ID_MCU3_0 | TIVX_TARGET_ID_MCU3_1 |
    |     TIVX_TARGET_ID_VPAC_NF      | TIVX_TARGET_ID_DMPAC_SDE |           -           |           -           |
    |    TIVX_TARGET_ID_VPAC_LDC1     | TIVX_TARGET_ID_DMPAC_DOF |           -           |           -           |
    | TIVX_TARGET_ID_VPAC_MSC[1 - 2]  |            -             |           -           |           -           |
    |    TIVX_TARGET_ID_VPAC_VISS1    |            -             |           -           |           -           |
    |  TIVX_TARGET_ID_CAPTURE[1 - 8]  |            -             |           -           |           -           |
    |  TIVX_TARGET_ID_DISPLAY[1 - 2]  |            -             |           -           |           -           |
    |     TIVX_TARGET_ID_CSITX[2]     |            -             |           -           |           -           |
    | TIVX_TARGET_ID_DISPLAY_M2M[1-4] |            -             |           -           |           -           |

    \subsection J721S2_A72_C7X_CORES A72 and C7x Cores
    |         A72 MPU          |               C7x_1                 |               C7x_2                 |
    |:------------------------:|:-----------------------------------:|:-----------------------------------:|
    | TIVX_TARGET_ID_MPU_[0-3] | TIVX_TARGET_ID_DSP_C7_1\n(TIVX_TARGET_ID_DSP_C7_1_PRI_1) | TIVX_TARGET_ID_DSP1 |
    |            -             | TIVX_TARGET_ID_DSP_C7_1_PRI_[2 - 8] |                  -                  |
*/
#endif
#if SOC == J722S
/*!
    \page TIOVX_ADD_TARGET Adding New Targets to TIOVX

    \subsection J722S_R5F_CORES R5F Cores
    |             MCU2_0              |         MCU1_0           |
    |:-------------------------------:|:------------------------:|
    |      TIVX_TARGET_ID_MCU2_0      |  TIVX_TARGET_ID_MCU1_0   |
    |    TIVX_TARGET_ID_VPAC_LDC1     |            -             |
    | TIVX_TARGET_ID_VPAC_MSC[1 - 2]  |            -             |
    |    TIVX_TARGET_ID_VPAC_VISS1    |            -             |
    |  TIVX_TARGET_ID_CAPTURE[1 - 4]  |            -             |
    |  TIVX_TARGET_ID_DISPLAY[1 - 2]  |            -             |
    |     TIVX_TARGET_ID_CSITX[2]     |            -             |
    | TIVX_TARGET_ID_DISPLAY_M2M[1-4] |            -             |
    |    TIVX_TARGET_ID_DMPAC_SDE     |            -             |
    |    TIVX_TARGET_ID_DMPAC_DOF     |            -             |

    \subsection J722S_A53_C7X_CORES A53 and C7x Cores
    |         A53 MPU          |               C7x_1                 |               C7x_2                 |
    |:------------------------:|:-----------------------------------:|:-----------------------------------:|
    | TIVX_TARGET_ID_MPU_[0-3] | TIVX_TARGET_ID_DSP_C7_1\n(TIVX_TARGET_ID_DSP_C7_1_PRI_1) | TIVX_TARGET_ID_DSP_C7_1\n(TIVX_TARGET_ID_DSP1)\n(TIVX_TARGET_ID_DSP_C7_1_PRI_1) |
    |            -             | TIVX_TARGET_ID_DSP_C7_1_PRI_[2 - 8] | TIVX_TARGET_ID_DSP_C7_2_PRI_[2 - 8] |

*/
#endif
#if SOC == AM62A
/*!
    \page TIOVX_ADD_TARGET Adding New Targets to TIOVX

    \subsection AM62A_R5F_CORES R5F Cores
    |             MCU1_0              |
    |:-------------------------------:|
    |      TIVX_TARGET_ID_MCU1_0      |
    |    TIVX_TARGET_ID_VPAC_LDC1     |
    | TIVX_TARGET_ID_VPAC_MSC[1 - 2]  |
    |    TIVX_TARGET_ID_VPAC_VISS1    |

    \subsection AM62A_A53_C7X_CORES A53 and C7x Cores
    |         A53 MPU          |               C7x_1                 |
    |:------------------------:|:-----------------------------------:|
    | TIVX_TARGET_ID_MPU_[0-3] | TIVX_TARGET_ID_DSP_C7_1\n(TIVX_TARGET_ID_DSP1)\n(TIVX_TARGET_ID_DSP_C7_1_PRI_1) |
    |            -             | TIVX_TARGET_ID_DSP_C7_1_PRI_[2 - 8] |
    */
#endif
/*!
    \page TIOVX_ADD_TARGET Adding New Targets to TIOVX

    \section TIOVX_TARGET_CODE_CHANGES Code Changes to Enable More Targets

    In certain situations, multiple target threads may need to be added to a given CPU beyond the single target provided by TIOVX.  (For exapmle, in the case of
    pipelining applications and two algorithms must run in parallel on the same core.)  In this case, the framework requires a set of changes to enable the new target.
    These changes are described in the steps below.

    \subsection STEP1 Step 1

    File to change: include/TI/tivx.h

    Required Changes: A new #define needs to be added for the new target name.  The #define follows the pattern of TIVX_TARGET_<New target name>.  The #define
    should be a unique string identifier.

    \subsection STEP2 Step 2

    File to change: source/platform/targets/soc/tivx_target_config_<soc>.h

    Required Changes: The TIVX_PLATFORM_MAX_TARGETS must be increased by the number of new targets added.

    \subsection STEP3 Step 3

    File to change: source/platform/targets/soc/tivx_target_config_<soc>.h

    Required Changes: The new enumeration value for the new target must be added to the tivx_target_id_e enumeration in the format TIVX_TARGET_ID_<New target name>.
    This must be set equal to TIVX_MAKE_TARGET_ID with the first argument being the CPU ID that this new target is to be run on and the second argument being the 1 plus the existing
    number of threads on that target.

    \subsection STEP4 Step 4

    File to change: source/platform/targets/soc/tivx_target_config_<soc>.h

    Required Changes: The #define from \ref STEP1 and the enumeration from \ref STEP3 must be added to the TIVX_TARGET_INFO structure.  For example, the DSP1 target is added
    to this structure with the line {\ref TIVX_TARGET_DSP1, (vx_enum)TIVX_TARGET_ID_DSP1}.

    \subsection STEP5 Step 5

    File to change: Dependent on chosen CPU.  Available options:
    - MPU Targets: source/platform/targets/tivx_target_config_mpu1_0.c
    - R5F Targets: source/platform/targets/r5f/tivx_target_config_r5f_<soc>.c
    - C66X Targets: source/platform/targets/tivx_target_config_c66.c
    - C7X Targets: source/platform/targets/tivx_target_config_c7.c

    Required Changes: The target thread must created in the appropriate file depending on the CPU that this new thread must be run on.  The API to create the new thread
    is tivxPlatformCreateTargetId().  Each of these files are slightly different in the way that they call this API.  Regardless of the file, the tivxPlatformCreateTargetId() must be called
    with the first argument being the target ID enumeration created in \ref STEP3.  The second parameter will be dependent on the desired parameters of the developer.

    \section TIOVX_TARGET_PRIORITY Disclaimer about "priority" argument for TIOVX targets

    One of the parameters to the \ref tivxPlatformCreateTargetId API is "priority".  The value of the priority set for this argument is based on the OS priorities of which the core is running.
    Therefore, when adding a new target to a given core, the developer should be aware of which OS is being used on that core.  This is of particular interest because the priority ordering is
    different in Linux vs in QNX/FreeRTOS/SafeRTOS.  A few references for this information can be found for <a href="https://stackoverflow.com/questions/3649281/how-to-increase-thread-priority-in-pthreads" target="_blank">Linux</a>,
    <a href="https://www.qnx.com/developers/docs/7.1/#com.qnx.doc.neutrino.prog/topic/overview_Priority_range.html" target="_blank">QNX</a>, and <a href="https://www.freertos.org/RTOS-task-priority.html" target="_blank">FreeRTOS</a>.

    TIOVX supports Linux/QNX on the A72 and FreeRTOS/SafeRTOS on the R5F/C66/C7X and thus the priorities set for the tasks running on each of these cores is taken into account and optimized for
    the use cases of interest for the TIOVX framework.  For instance, the capture target is running at the highest priority level for the R5F OS in order to prioritze the receiving of capture
    frames within a given system.
 */



/*!
    \page TIOVX_TARGET_EXECUTION Target Execution Model

    \tableofcontents

    This section gives some insight into the implementation of the TIOVX framework with respect to how multiple workloads are dispatched to
    and executed on remote cores and targets.

    \section TIOVX_TARGET_EXECUTION_SINGLE FIFO run to completion per Target

    All of the targets execute workloads in FIFO order to completion.  Each CPU has a dedicated IPC_RX task which forwards workloads
    to the appropriate TARGET task/thread to be executed.  Please refer to the diagram and description below:

    \image html single_thread.png width=60%

    - **IPC_RX Task:**
      - The OpenVX framework creates a IPC_RX task on each Core in the system.  This IPC_RX task is run at highest priority, so that any OVX IPC
    workload request message can be immediately routed to the appropriate OpenVX target.
    - **OpenVX Targets:**
      - OpenVX has notion of "Targets", which are essentially named processing threads/tasks on a core.  For more details, see \ref TIOVX_TARGET_EXPLANATION
      - Each target will process requests to completion in FIFO order. A core can have N number of targets.
      - There are two reasons to have multiple targets on a core:
        - If a core is managing multiple compute resources or HWAs, then a separate target can be created for each resource (e.g. VPAC VISS target, VPAC
        LDC target, etc). This architecture ensures proper resource management across requesters.
        - If the application wants priority-based access to a resource, then different targets can be used at different task priorities.

    \section TIOVX_TARGET_EXECUTION_MULTI Priority-Based Preemption of C7X Targets

    In order to support the situation where multiple TIDL networks are needed to be run at different rates with different priorities, TIOVX
    supports multiple priority-based TARGETS on the C7X DSP(s).  Additionally, TIDL has been instrumented to lock and unlock interrupts such
    that priority preemption can happen at well-defined points in the network processing to provide optimal context save/restore points and avoid
    otherwise costly preemption of the highly pipelined hardware. Please refer to the diagram and description below:

    \image html preemption.png width=70%

    - **Assumption**
      - RTOS supports and is configured to have tasks running at different priorities
    - **OpenVX Targets**
      - The application can assign each OVX DNN node to a specific target at initialization time.
      - To support multiple priorities of DNN execution with preemption, we just create multiple OVX targets on the DSP with different task priorities so
      that the OS can preempt and manage the priority scheme for us.
      - User just has to assign nodes to targets based on priority
        - e.g. \ref vxSetNodeTarget (node, \ref TIVX_TARGET_DSP_C7_1_PRI_1 );
    - **Critical Sections**
      - Since we need to control when the DNNs can be preempted, the DNN execution will call critical section callbacks provided by the OpenVX DNN node wrapper,
      disabling/reenabling interrupts to prevent the OS from preemption at wrong time.
    - **Deactivate/Activate**
      - Global variables are used to keep track of ACTIVE DNN handle, so that each DNN task, upon entering its critical section, checks if the CURRENT handle
      matches the ACTIVE handle.  If not, it first deactivates ACTIVE handle and activates the CURRENT handle, and then updates the ACTIVE handle variable.

 */



/*!
    \page TIOVX_DEBUG Debug Tools for TIOVX

    This page itemizes the various debug utilities and methods recommended for TIOVX application development

    - \subpage DEBUG_PRINT - How to enable debug print statements.
    - \subpage GRAPH_NODE_DEBUG - How to set graph/node specific debug levels
    - \subpage JPEG_TOOL - How to use the graph JPEG generation tool
    - \subpage DEBUG_LOG_RT - How to enable run-time event start/stop logging and offline visualization
 */

 /*!
    \page DEBUG_PRINT Debug Print Statements

    The TIOVX framework contains informative debug logging that can be enabled and disabled. The logging print statements are enabled
    based on the level of logging that has been set. The logging levels can be found as a part of the enum \ref tivx_debug_zone_e.
    By default, the VX_ZONE_INFO and VX_ZONE_WARNING zones are globally enabled on each core. Logging levels can be globally enabled
    and disabled for each core by calling the \ref tivx_set_debug_zone and \ref tivx_clr_debug_zone APIs respectively. Through the
    \ref VX_PRINT API, additional information can be reported based on the global debug level state. Graph, node, and kernel-specific
    statements can be printed via the \ref VX_PRINT_GRAPH, \ref VX_PRINT_NODE, and \ref VX_PRINT_KERNEL APIs respectively. See
    \ref GRAPH_NODE_DEBUG for more details.

 */

  /*!
    \page GRAPH_NODE_DEBUG Maintaining Graph and Node Debug Levels

    Explicitly setting logging levels creates flexibility for debugging specific graphs and nodes within an application.
    Debug levels can be set for specific graphs and nodes using the \ref tivxSetGraphDebugZone and \ref tivxSetNodeDebugZone APIs
    respectively. Including a vx_true_e flag enables a specified zone, and passing a vx_false_e flag disables a specified zone.
    Changing the debug levels of a given graph will also update the levels of each node that belongs to it. When a graph is created,
    each of its logging levels will be enabled or disabled to match the status of the global debug state. The debug levels of each
    newly created node will match those of the graph it belongs to.

    Changes to the debug zones of graphs and nodes can be made even at runtime. This allows for more debug control over specific objects
    both before and after graph verification. To continue supporting the existing \ref VX_PRINT mechanism, a set of different APIs must
    be used to take advantage of graph and node debug levels. \ref VX_PRINT_GRAPH and \ref VX_PRINT_NODE can be used at the
    application/framework levels to print statements based on the debug levels of a given graph or node. VX_PRINT_NODE can also be used
    within User Extension Kernel callbacks on the host side when a vx_node object is accessible. \ref VX_PRINT_KERNEL can be used within
    User Target Kernel callbacks when a target_kernel_instance object is accessible. See the following examples for more information.

    \section GRAPH_DEBUG_PRINT Graph-Specific Debug Prints

    In order to print debug information for a specific graph, first enable/disable the desired debug zones using \ref tivxSetGraphDebugZone.
    \code
    tivxSetGraphDebugZone(VX_ZONE_INFO, graph, vx_true_e);
    tivxSetGraphDebugZone(VX_ZONE_WARNING, graph, vx_false_e);
    \endcode

    These calls will enable the VX_ZONE_INFO zone and disable the VX_ZONE_WARNING zone for that graph. To add logging for this graph, call
    the following:

    \code
    VX_PRINT_GRAPH(VX_ZONE_INFO, graph, "This is a graph-specific debug statement\n");
    \endcode

    Because the level VX_ZONE_INFO has been enabled for the graph, this print statement will execute wherever it exists in the
    application/framework.

    \section NODE_DEBUG_PRINT Node-Specific Debug Prints

    \code
    tivxSetNodeDebugZone(VX_ZONE_ERROR, node, vx_false_e)
    tivxSetNodeDebugZone(VX_ZONE_INFO, node, vx_true_e)
    \endcode

    For this node, VX_ZONE_ERROR will be disabled while VX_ZONE_INFO is activated.

    This print
    \code
    VX_PRINT_NODE(VX_ZONE_ERROR, node, "Node-specific error statement\n");
    \endcode
    will not be performed, but the following will:

    \code
    VX_PRINT_NODE(VX_ZONE_INFO, node, "Node-specific info statement\n");
    \endcode

    In addition to application and framework use cases, \ref VX_PRINT_NODE can be called within host-side user extension kernel
    callback functions. When these kernel callbacks are invoked within an application, the user kernel print statements will be
    executed assuming the required debug levels are enabled within the nodes that are instances of it. See the following example
    within the Not user kernel validate callback function:

    \code
    static vx_status VX_CALLBACK tivxAddKernelNotNotValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
    {

    ...

    if (VX_SUCCESS == status)
    {
        VX_PRINT_NODE(VX_ZONE_ERROR, node, "Status is Success\n");

    ...
    \endcode

    \section KERNEL_DEBUG_PRINT Kernel-Specific Debug Prints

    Kernel-specific debug prints can also be added to user target kernel callback functions. This is achieved in a similar way as
    the host-side user kernel callbacks, but the API \ref VX_PRINT_KERNEL is used instead. See the following example within the
    Not target kernel create callback function:

    \code
    static vx_status VX_CALLBACK tivxNotNotCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
    {

    ...

    if(status == VX_SUCCESS)
        {
            VX_PRINT_KERNEL(VX_ZONE_ERROR, kernel, "Not Not Kernel created\n");
    
    ...
    \endcode

    \section DEBUG_PRINT_FAQ Frequently Asked Questions

    \subsection LEGACY_PRINT Can I still use VX_PRINT?

    Yes - VX_PRINT still functions as usual. Separate print APIs were created to ensure backward compatibility is
    maintained and existing VX_PRINT calls continue to work properly.

    \subsection NEW_PRINT_VS_LEGACY When should I use VX_PRINT_GRAPH/NODE/KERNEL instead of VX_PRINT?

    The graph, node, and kernel prints can be used anywhere within an application, the framework, or a user extension kernel
    that a \ref vx_graph, \ref vx_node, or \ref tivx_target_kernel_instance object are accessible. VX_PRINT can also be used
    in these places, but the specific API selection ultimately depends on the collection of debug levels the print statement
    will be evaluated against: the global core debug state or a local object's debug state.

    \subsection DEBUG_SCENARIO_ANSWERS Which should be used in these scenarios?

    \subsubsection FRAMEWORK_EXTENSION_DEBUG Writing a new extension to the framework:

    The VX_PRINT API can be used anywhere throughout the framework for general debug print statements that are enabled as long
    as the global debug level is set appropriately. Extensions to the kernel that specifically access node and graph objects
    could potentially benefit from calling \ref VX_PRINT_GRAPH and \ref VX_PRINT_NODE. Debug statements would be enabled/disabled
    for each individual graph or node depending on each one's debug state.

    \subsubsection USER_KERNEL_DEBUG Developing a new user kernel:

    Callback functions for host-side user kernels have access to a vx_node object, so these functions can benefit from using
    the \ref VX_PRINT_NODE API. Some callback functions for the target user kernel have access to a tivx_target_kernel_instance
    object. In these locations, the \ref VX_PRINT_KERNEL API can be leveraged. In both circumstances, the debug states of nodes
    that are created as instances of these user kernels will be checked against the required debug zone to print kernel-specific
    debug information.

    \subsubsection APPLICATION_DEBUG Creating a new application:

    Similiarly to framework extensions, both the VX_PRINT and graph/node/kernel print APIs are available for use within applications.
    The specific use case will determine which to use as it depends on the availability of vx_graph and vx_node objects.

 */

 /*!
    \page JPEG_TOOL JPEG Visualization of Graph

    The \ref tivxExportGraphToDot API provides the ability to generate a JPEG diagramming the nodes in a graph. Below are a few notes on
    how to use this tool.
    - In PC emulation mode, this API will generate both a .txt file and a .jpg file located at the specified output path.  The graph diagram
      will not show the cores that each node runs on, it will simply show the interactions between nodes in the graph.
    - In target mode, this API will generate only a .txt file located at the specified output path. However, the .jpg file can be generated
      on a Linux machine using the dot command.  The command to generate the JPEG can be seen below:

      dot -Tjpg -o<output file path>/<jpg name>.jpg <text file name>

      In the case of a text file called graph.txt, the following command would be used to generate a JPEG called graph.jpg in the current directory:

      dot -Tjpg -o./graph.jpg graph.txt

    - In the case of a pipelined graph, this API outputs 4 separate files:
       - 1. The first file shows the interaction between all nodes and data objects in a graph.
       - 2. In the case of a pipelined graph, the second file shows the interactions between the nodes and data reference queues for every
            pipeline ID for the pipeline depth of a graph.
       - 3. In the case of a pipelined graph, the third file shows how each of the data reference queues are acquired and released by
            the framework.  (Note: this file requires a more in-depth knowledge of the framework and is mainly intended for debugging
            issues within the framework.)
       - 4. In the case of a pipelined graph, the fourth file shows the interactions between the nodes and data reference queues for the first
            pipeline ID of a graph.

 */

 /*!
    \page DEBUG_LOG_RT Run-time Event Logging and Visualization

    \tableofcontents

    \section BUILT_IN Built-in support

    The \ref tivxLogRtTraceEnable, \ref tivxLogRtTraceExportToFile, \ref tivxLogRtTraceDisable
    APIs provides the ability to log node start, node stop and other useful events to a shared memory.
    These events can then be written to a file and visualized to gain important insight into a graph execution.
    While events are being logged, the intrusion into node execution is minimal, hence logging these
    events does not change performance of the graph being monitored.

    When logging is enabled, all node and target start/end events are captured and ready to be visualized.  Furthermore, events from within
    specific built-in kernels (such as HWA and DMA start/stop events in VPAC_VISS) are also captured.

    \section CUSTOM_EVENTS Adding custom events within interior of user kernels

    In order to add custom even logging within user kernels, the \ref tivxLogRtTraceKernelInstanceAddEvent API can be called from within
    the initialization callback, and the \ref tivxLogRtTraceKernelInstanceRemoveEvent can be called from within the deinitialization
    callback on the host side of the user kernel.  Then, within the target side callbacks (create, destroy, process, control), the \ref
    tivxLogRtTraceKernelInstanceExeStart and \ref tivxLogRtTraceKernelInstanceExeEnd APIs can be called at the appropriate points to trigger
    the appropriate log event.

    \section RTLOGGING How to enable rt-logging and visualization

    To enable run-time event logging in your program do below,
    - After graph verify but before graph execution, call \ref tivxLogRtTraceEnable (graph) to enable event logging for that graph.
    - Execute the graph as usual
    - After graph executions have stopped, or periodically, call \ref tivxLogRtTraceExportToFile (filename) to save event data
      to a file.
    - After graph executions have stopped and before vxReleaseGraph, call \ref tivxLogRtTraceDisable (graph) to disable run-time logging for that graph

    To visualize the graph data do below,
    - **[ONE TIME ONLY]** Install 'gtkwave' on your linux PC
      \code
      sudo apt install gtkwave
      \endcode
    - **[ONE TIME ONLY]** Build the tools for format converiosn by doing below
      \code
      cd tiovx/tools/tivx_log_rt
      make
      \endcode
    - Copy the event log .bin file, saved on EVM via \ref tivxLogRtTraceExportToFile (filename), to a folder in your Linux PC.
    - Generate .vcd file (Value Change Dump) file to visualize the events in gtkwave as below
      \code
      tiovx/tools/tivx_log_rt/tivx_log_rt_2_vcd.out -i event_log.bin -o event_log.vcd
      \endcode
    - Generate .html file to visualize per node frame level statistics as below
      \code
      tiovx/tools/tivx_log_rt/tivx_log_rt_2_html.out -i event_log.bin -o event_log.html
      \endcode
    - Open the .vcd file in gtkwave
      \code
      gtkwave event_log.vcd
      \endcode
    - In gtkwave, drag and drop the 'signals' of interest from bottom left frame to center frame and visualize the 'signals' in the right frame.
    - Open the .html file in web browser to visualize per node frame level statistics.
      \note The html files uses dygraphs.js from http://dygraphs.com/ so make sure your internet connectivity works fine

    \section DEBUG_LOG_RT_SAMPLE Sample outputs from app_tidl_avp2 demo

    \subsection DEBUG_LOG_RT_SAMPLE_VCD VCD View

    1. There are two types of 'signals' in this view, openVX nodes and CPUs / HW accelerators. The nodes are typically
       represented as bus [3:0], appended by *Node and the CPU / HW accelerators are represented as single bit values
       (either HIGH or LOW).
    2. Nodes are ScalarNode[3:0], ODPreProcNode[3:0], ODTODLNode[3:0] etc. and CPU / HW accelerators are DISPLAY1,
       DSP-1, DSP-2, DSP_C7-1 etc.
    3. The value of the node 0, 1, 2, 3 .. depicts which frame is bring processed. This is a relative number just to
       get a better understanding that when a particular node is processing frame 3, which frame number is being
       processed by the other nodes.

    \image html vcd_view.png

    \subsection DEBUG_LOG_RT_SAMPLE_HTML HTML View

    1. The HTML view gives more of a statistical data.

    \image html html_view.png
 */

/*!
    \page TIOVX_MEMORY_MANAGEMENT Memory Management in TIOVX

    \tableofcontents

    \section TIOVX_MEMORY_MANAGEMENT_INTRODUCTION Introductory Concepts

    A given OpenVX graph consists of the states seen in the below diagram.  These 4 states are described below:
    - The "Create" phase involves the creation of all OpenVX objects.  These objects may be the OpenVX graphs, nodes, or data objects, such as images, tensors, etc.
      OpenVX provides simple API's for creating each OpenVX object.  For example, an OpenVX graph is created using the \ref vxCreateGraph() API and an image data object
      is create using the \ref vxCreateImage() API.
    - The "Verify" phase consists of a single API, \ref vxVerifyGraph().  The verify graph API returns a status to the application, describing whether or not
      this graph is valid and can be processed.  Many operations occur within this single API, which are vendor specific.  This section will detail the operations
      occurring from a memory management point of view.  For more information on other operations occurring during the verify phase, please see the
      \ref USER_TARGET_KERNELS section.
    - The "Execute" phase consists of the actual scheduling and processing of the OpenVX graph.  During this phase, the process callbacks of each of the nodes are
      called in a sorted sequence determined in the verify graph phase.  The verify graph phase performs a topological sort of the OpenVX nodes in order to determine
      any data dependencies among the nodes in the graph.  During the execute phase, each node is processed according to that order.  Once each node completes, any nodes
      that were dependent on the output data from that node is processed.  The below sections describe how this data is transferred across the cores of the SoC.
    - Similar to the "Create" phase, the "Destroy" phase involves the freeing of all OpenVX objects.  Each corresponding object in OpenVX has an associated API for
      freeing the object.  For example, a graph can be freed by calling the \ref vxReleaseGraph() API while an image can be freed by calling the \ref vxReleaseImage()
      API.

    \image html openvx_state_machine.png

    \section TIOVX_MEMORY_MANAGEMENT_CREATE TIOVX Create Phase

    At init time of an application using OpenVX, the API \ref tivxInit is called upon each core that is being used within an OpenVX application.  One of the purposes of
    the init API is to perform a static allocation of the handles used for OpenVX objects in a non-cached region of DDR.  The framework enables logging of the actual
    amount of these statically allocated structures being used in an application in the event that these values need to be modified.

    As mentioned previously, the "Create" phase calls the appropriate create API's for each of the OpenVX data objects within the application.  With respect to memory
    management, the create API's do not allocate the memory needed for the data objects; the memory is allocated in the next phase, the verify phase.  Instead, it
    simply returns an opaque handle to the OpenVX object. Therefore, the memory is not accessible directly within the application.  These handles point to object descriptors
    referred to early that reside in a non-cached region of DDR as well as a set of cached attributes, including the OpenVX object data buffers.

    This call sequence highlights the process from the application and framework perspectives when an object is created:
       - \mscfile data_object_allocation.msc Data Object Handle Acquisition Process

    As an example, the following illustrates this procedure when creating image and graph objects:
       - \mscfile image_graph_allocation.msc Example: Image and Graph Handle Acquisition

    In the event that the memory must be accessed within the application or initialized to a specific value, OpenVX provides certain standard API's for performing a map
    or copy of the data object.  For example, the image object has the map and unmap API's \ref vxMapImagePatch and \ref vxUnmapImagePatch as well as a copy API,
     \ref vxCopyImagePatch.  In the event that the memory must be accessed prior to the verify phase, the framework will allocate the buffer(s) associated with the
     object within shared memory so that it can be accessed.

    \section TIOVX_MEMORY_MANAGEMENT_VERIFY TIOVX Verify Phase

    The allocation and mapping of the OpenVX data objects occurs during the verification phase.  The memory for each of the data objects is allocated from a carved out
    region of DDR shared memory by using the \ref tivxMemBufferAlloc API internally within the framework. This allocation is done on the host CPU.  By performing this
     allocation within the vxVerifyGraph call, the framework can return an error at a single point within the application to notify the application that the graph is
     valid and can be scheduled.

    In addition to allocating memory for the OpenVX data objects, the local memory needed for each kernel instance is also allocated during the call to \ref vxVerifyGraph.
    In this context, local memory refers to memory regions accessible by the given core the target kernel is run on.  This local memory allocation occurs as the vxVerifyGraph
    call loops through all the nodes within the graph and calls the "create" callbacks of the given nodes.  The design intention of the framework is that all memory allocation
    occurs during the create callbacks of each kernel, thereby avoiding memory allocation at run-time inside the process callbacks.  For more information about the callbacks for each kernel see
     \ref vxAddUserKernel and \ref tivxAddTargetKernel.  For more information about the order in which each of these callbacks are called during vxVerifyGraph, see \ref USER_TARGET_KERNELS.

    Each kernel instance has its own context that allows the kernel instance to store context variables within a data structure using the API's
    \ref tivxSetTargetKernelInstanceContext and \ref tivxGetTargetKernelInstanceContext.  Therefore, local memory can be allocated and stored within the kernel structure.
    The simple API's are provided to allocate kernel memory are \ref tivxMemAlloc and \ref tivxMemFree.  These API's allow memory to be allocated from various memory
    regions given here \ref tivx_mem_heap_region_e.  For instance, if multiple algorithms are running consecutively within a kernel, intermediate data can be allocated within the kernel
    context.  Also, within the \ref tivx_mem_heap_region_e, there are attributes to create persistent memory within DDR or to create non-persistent, scratch memory within
    DDR.

    \section TIOVX_MEMORY_MANAGEMENT_PROCESS TIOVX Execute Phase

    During the scheduling and execution of the OpenVX graphs, the OpenVX data buffers reside in external shared memory and the pointers to each of these data buffers are passed
    along to subsequent nodes in the graph.  Inside each node process callbacks, the node may access the external shared memory via the pointers that have been passed from the
    previous node. The \ref tivxMemBufferMap and \ref tivxMemBufferUnmap encapsulate the mapping and cache maintenance operations necessary for mapping the shared memory to
    the target core.  After a node completes, the framework handles the triggering of nodes depending on the data from the output of the current node.

    \section TIOVX_MEMORY_MANAGEMENT_DELETE TIOVX Destroy Phase

    As mentioned previously, the "Delete" phase calls the appropriate delete API's for each of the OpenVX data objects within the application.  At this
    point, the data buffer(s) of the OpenVX data objects are freed from shared memory using the \ref tivxMemBufferFree API.  This freeing of memory occurs within the individual object's release
    API's, such as the \ref vxReleaseImage for the image object.

    \section TIOVX_MEMORY_MANAGEMENT_OPTIMIZATIONS TIOVX Memory Optimizations

    As mentioned above, the default behavior of OpenVX data buffer transfer is to write intermediate buffers to DDR prior to these buffers being read by subsequent
    nodes.  Below are a few recommendations and suggestions of how to optimize memory transfer:
    - As mentioned during the "Create" section, the amount of statically allocated structures in non-cached memory can be queried and modified. To do so, refer to the following:
      - The maximum values of statically allocated structures are defined in the files <TIOVX_PATH>/include/TI/\ref tivx_config.h and <TIOVX_PATH>/include/TI/tivx_config_<SOC>.h
      - These values were sized according to the applications using OpenVX within the vision_apps repo.  However, these values can be increased or decreased depending on the needs
        of a given application.
      - The following utility functions were developed to assist in optimizing these values:
        - \ref tivxPrintAllResourceStats prints the currently used value, maximum used value, and minimum required values
        - \ref tivxQueryResourceStats provides information about the parameter values of a specific resource whose name is passed as a parameter to the function
        - \ref tivxExportAllResourceMaxUsedValueToFile generates a new configuration file called "tivx_config_generated.h" at VX_TEST_DATA_PATH. This config file initializes each
          parameter to the maximum used value as determined by the previous runtime.
      - All of the parameter maximum values are documented in \ref group_vx_framework_config
      - All of these API's are documented further in \ref group_tivx_ext_host
    - In the event that multiple algorithms must be run consecutively on a single core, it is typically recommended to encapsulate this into a single OpenVX kernel.
      The reason for this is so that the intermediate data can be written to local memory, thereby improving memory access time.  This avoids the alternative of splitting
      the operations into two separate kernels and writing the intermediate data to DDR.  Note: this limits the flexibility of these algorithms being deployed separately,
      and thus must also be weighed in the decision to merge these two algorithms into a single kernel.
    - Another optimization technique that can be taken advantage of is the usage of DMA to parallelize the memory fetch with the compute of a given target kernel.
      By using DMA to fetch tiled portions of a given input, a kernel can operate on the input and generate an output in a block-based manner.  This can greatly
      improve the throughput of a given kernel.

 */

/*!
    \page TIOVX_PIPELINING Graph Pipelining in TIOVX

    For general pipelining usage guidelines, please follow the latest version of the Khronos OpenVX pipelining extension <a href="https://registry.khronos.org/OpenVX/extensions/vx_khr_pipelining/2.0/vx_khr_pipelining_2_0_0/vx_khr_pipelining_2_0_0.html" target="_blank">here.</a>

    This updated version allows for a more robust pipelining implementation than before. Requirements for the pipelining/event handling
    mechanisms have been separated from those used for streaming, the concept of graph events and additional attributes have been introduced,
    and retrieval of graph/kernel parameter information has been simplified. Complete details of this migration can be found
    <a href="https://registry.khronos.org/OpenVX/extensions/vx_khr_pipelining/2.0/vx_khr_pipelining_2_0_0/vx_khr_pipelining_2_0_0.html#_summary_of_changes_from_version_1_1_1_of_the_extension" target="_blank">here.</a>

    Note:
    When it comes to graph parameters, a graph parameter's list of enqueueable references should match the type of the relevant node's output
    node parameter and should be registered by the application. When the type of the output parameter is a container object (\ref group_pyramid
    or \ref group_object_array), the graph parameter should be registered and enqueued from the container's first element rather than the
    container itself. While previously supported, attempting to enqueue a container object as a graph parameter will now accurately result in
    a runtime-error.

    \section NODE_GRAPH_PARAMETER_DEFINITION Node and Graph Parameter Definitions

    Pipelining in OpenVX requires an understanding of graph parameters and node parameters.  The below sections link to the explanation of these
    concepts in the OpenVX spec.

    \ref sub_graph_parameters : Graph parameters are used in the non-<a href="http://registry.khronos.org/OpenVX/extensions/vx_khr_pipelining/2.0/vx_khr_pipelining_2_0_0/vx_khr_pipelining_2_0_0.html#sec_streaming" target="_blank">streaming</a>,
    <a href="https://registry.khronos.org/OpenVX/extensions/vx_khr_pipelining/2.0/vx_khr_pipelining_2_0_0/vx_khr_pipelining_2_0_0.html#sec_pipelining" target="_blank">pipelining</a>
    mode to identify the parameters within a graph that are user-enqueueable.  By creating a graph parameter and explicitly
    enqueue-ing and dequeue-ing this parameter from the graph, the application has the ability to access the object data.
    Otherwise, when using <a href="https://registry.khronos.org/OpenVX/extensions/vx_khr_pipelining/2.0/vx_khr_pipelining_2_0_0/vx_khr_pipelining_2_0_0.html#sec_pipelining" target="_blank">pipelining</a>,
    non-graph parameters are not accessible during graph execution.

    \ref sub_node_parameters : In the TIOVX implementation, node parameters are used to identify where multiple buffers need
    to be created in the case of <a href="https://registry.khronos.org/OpenVX/extensions/vx_khr_pipelining/2.0/vx_khr_pipelining_2_0_0/vx_khr_pipelining_2_0_0.html#sec_pipelining" target="_blank">pipelining</a>.
    This is explained further in the \ref TIOVX_PIPELINING_PERFORMANCE_OPTIMIZATION_BUFFER_DEPTH section.  If buffer access from the application
    is needed, the parameter in question should be created as a graph parameter given that multiple buffers will be created at that parameter
    and there is not a way of accessing a particular buffer of this buffer queue.

    \section TIOVX_PIPELINING_PERFORMANCE_OPTIMIZATION Performance Considerations of Pipelining

    The following section details how to get the best performance and how to optimize the memory footprint of an OpenVX graph.  In order to
    do so, the concepts of graph pipeline depth and buffer depth must be understood in the context of the TIOVX framework.  The following two
    sections provide details about these concepts.

    As of Processor SDK 7.2, the TIOVX framework has been enhanced to enable automatic graph pipelining depth and buffer depth setting.
    This alleviates the burden of the application developer to set these values in order to get real time performance.  While the framework
    will set these values automatically, they can be overwritten by the respective API's used for setting these values.  Further explanation
    is provided in the sections below.

    \subsection TIOVX_PIPELINING_PERFORMANCE_OPTIMIZATION_PIPE_DEPTH Pipeline depth

    In order to for an OpenVX graph to get full utilization of TI's heterogeneous SoC's, the OpenVX graph must be pipelined.  In the TIOVX
    framework, the characteristic which describes the utilization of the heterogeneous cores is the pipeline depth.  Consider a simple 3 node
    graph seen in the image below.  This graph uses 3 cores on the SoC: an ISP, DSP and CPU.

    \image html basic_graph.png

    Without pipelining, a new graph execution would not be able to begin until the initial graph execution was completed.  However,
    since each of these cores can be running concurrently, this graph execution does not allow for the best hardware utilization.
    The TIOVX pipelining implementation allows for full hardware utilization by internally creating multiple instances of the graph
    based on the value of the pipeline depth.  Therefore, each of these graphs will execute simultaneously, such that each core is
    actively processing.  In this scenario, the optimal pipeline depth is 3.  With a pipeline depth of 3, the framework treats the
    graph as if there were 3 instances of the same graph processing simultaneously as shown below.  The image below shows the graph
    processing at time T=2, such that every processing unit was active.

    \image html basic_graph_pipelined.png

    By parsing the structure of the graph, the TIOVX framework can automatically set the pipeline depth and achieve real time performance.
    The automated pipe depth feature sets the pipe depth during the \ref vxVerifyGraph call based on the structure of the graph.  For instance,
    in the example above, the framework will set the pipeline depth to 3, thereby allowing each node in the graph to execute.

    The value used as the pipe depth is determined by doing a depth search to find the longest sequence of consecutive nodes within a graph.
    This methodology is intended to return a pipeline depth that provides the best performance by considering the worst case scenario of
    OpenVX node target assignment.  (For more information about TIOVX targets, see \ref TIOVX_TARGET_EXPLANATION.)  For instance, if in the
    example above, the OpenVX nodes were all assigned to run on the CPU, the TIOVX framework would still set a pipeline depth of 3.

    If the calculated pipe depth value exceeds the \ref TIVX_GRAPH_MAX_PIPELINE_DEPTH, the value will be set to the max allowable depth
    and a warning will be thrown.  This value can be increased if necessary by modifying the \ref TIVX_GRAPH_MAX_PIPELINE_DEPTH macro in the file
    tiovx/include/TI/tivx_config.h.

    In order to further fine tune and potentially optimize the OpenVX graph, the application developer has the option to set the pipeline depth
    directly via the \ref tivxSetGraphPipelineDepth API.  In this case, the framework will use the value provided by the application rather than
    the calculated value. However, if this value is less than the optimal value as calculated by the framework, a message will be printed to
    the console using the \ref VX_ZONE_WARNING logging level indicating to the user that this value may not be optimal.  By using the
    \ref tivxSetGraphPipelineDepth API, the application developer can potentially reduce the memory footprint of the graph in the case that
    real time performance can be achieved by using a smaller pipeline depth value than detected by the framework.  In order to know the value
    the framework is using for pipeline depth, the user can call the \ref vxQueryGraph API using the \ref VX_GRAPH_PIPELINE_DEPTH macro
    after calling \ref vxVerifyGraph.

    \subsection TIOVX_PIPELINING_PERFORMANCE_OPTIMIZATION_DELAY_PARAMETERS Pipelining with Delay Objects

    It is not recommended to use \ref vx_delay objects when using pipelining with TIOVX.  The delay objects will unintentionally create
    serializations in the graph and break the desired pipelining.

    Instead of using delay objects, graph parameters can be used to emulate a delay in the graph, thus allowing the delay to be handled
    in the application.

    \subsection TIOVX_PIPELINING_PERFORMANCE_OPTIMIZATION_BUFFER_DEPTH Buffer depth

    In order for a node producing a buffer to operate concurrently with a node consuming that buffer, multiple buffers must be used at the
    buffer parameter in order to avoid pipelining stalls.  This is illustrated in the diagram below.

    \image html buffer_queue.png

    As defined in the pipelining specification of OpenVX, a buffer can be created as a graph parameter with multiple
    buffers created within the application.  (For more information on this, see \ref NODE_GRAPH_PARAMETER_DEFINITION.)  In the case that
    graph parameters are not used, there is not a native OpenVX API to set multiple buffers at a given node parameter.

    In order to allow the OpenVX graph to run with full performance without changing the native implementation of OpenVX, the TIOVX framework
    will automatically allocate and set the multiple buffers during the \ref vxVerifyGraph call according to the node connections of the buffer.
    The value used as the buffer depth of a particular parameter is found by detecting the total number of nodes connected to a given buffer.
    For instance, let us consider the example from the previous section.  Since each buffer is connected to a single producer node and a
    single consumer node, the framework will set the buffer depth of each of these parameters to 2 as shown in the below diagram.

    \image html basic_graph_pipelined_buffering.png

    This feature excludes parameters that have already been enabled as graph parameters given that these parameters require the buffer to
    manually be enqueued and dequeued from the application, therefore requiring the buffer depth to be set in the application.

    In order to further fine tune and potentially optimize the OpenVX graph, the application developer has the option to set the buffer depth
    directly via the \ref tivxSetNodeParameterNumBufByIndex API.  (The parameter being set to use multiple buffers using this API must not already
    be created as a graph parameter.  Otherwise, an error will be thrown during \ref vxVerifyGraph.)  If the \ref tivxSetNodeParameterNumBufByIndex API
    is used, the framework will use the value provided by the application rather than the calculated value.  However, if this value is less
    than the optimal value as calculated by the framework, a message will be printed to the console using the \ref VX_ZONE_WARNING
    logging level indicating to the user that this value may not be optimal.  By using the \ref tivxSetNodeParameterNumBufByIndex API,
    the application developer can potentially reduce the memory footprint of the graph in the case that real time performance can be
    achieved by using a smaller buffer depth value than detected by the framework.  In order to know the value the framework is using
    for pipeline depth, the user can call the \ref tivxGetNodeParameterNumBufByIndex after calling \ref vxVerifyGraph.

    \if DOCS_J7

    \section TIOVX_PIPELINING_USAGE_CONSIDERATIONS Pipelining Usage Considerations

    \subsection TIOVX_PIPELINING_USAGE_CONSIDERATIONS_COMPOSITE_OBJECTS Pipelining with Composite Objects

    Composite objects are a special consideration when pipelining using TIOVX.  In this context, the definition of a composite object is an OpenVX object that serves
    as a container for other OpenVX objects.  A list of composite objects are given below along with a graph construction suggestion. An explanation for these situations is given
    in the section below.
    - Object Arrays (\ref group_object_array): Pipelining is supported using vxReplicateNode().
    - Pyramids (\ref group_pyramid): Pipelining is supported using vxReplicateNode().
    - Region of Interest (ROI) Images (vxCreateImageFromROI()): Pipelining is not supported in TIOVX(see: \ref ROI_NOTE).

    Object Arrays and Pyramids are treated similarly in TIOVX from a pipelining perspective. There are many common pipelining scenarios using these objects
    which arise when developing an application using TIOVX.  The following examples illustrate how a graph with object arrays MUST be constructed using TIOVX.  Even though
    the example uses object arrays, the same principles can be applied to the pyramid object with no difference in implementation.

    Object arrays are used prevalently in multi-sensor TIOVX applications, such as with the \ref tivxCaptureNode() which outputs an object array and has as its elements the
    individual sensor images.  These elements will often be processed by downstream nodes, such as the \ref tivxVpacVissNode() that takes as an input an individual element
    of the object array, rather than the full object array.  This interaction can be seen in the block diagram below.

    \image html Object_Array_Produced.png

    Similarly, the output objects of multiple nodes may consist of elements of object arrays which then are consumed by a single node in the form of the full object array.
    As an example, this situation occurs when creating a Surround View application with TIOVX.  The individual output images from multiple instances of the VISS node form a single
    object array, which is consumed by a node generating the Surround View output.  This interaction can be seen in the diagram below.

    \image html Object_Array_Consumed.png

    OpenVX natively supports two options for supporting the above interaction of sending separate elements of an object array to downstream nodes for processing.  The
    first option is to use the vxReplicateNode() feature.  By using this feature, the application developer avoids re-writing large portions of code by allowing the
    framework to instantiate as many instances of the node as there are elements in the object array while retaining the ability to customize specific parameters of the node.

    The second option OpenVX provides for accessing elements of object arrays is to use the vxGetObjectArrayItem() API.  This option is NOT natively supported with
    pipelining in TIOVX.  Therefore, the recommendation is to use the replicate node feature.  However, if the replicate node feature cannot be used within
    the application, a workaround is available when the object array is the output of the producing node and the object array elements are the input of the
    consumer node.  This workaround requires a slight modification of the kernel that is consuming the object array.

    Within the OpenVX graph, the vxGetObjectArrayItem() API is called to extract the 0th element of the object array output of the producer node.  This element is then passed
    to the input of the consumer node.  In order to extract the appropriate object array item within the kernel consuming the element, the
    tivxGetObjDescElement() API can be called with the arguments being the object descriptor of the object array element and the appropriate element
    ID of the object array.  This API will then return the object descriptor of the element given as the elem_idx argument. This elem_idx can be provided
    via a config parameter or sent via the tivxNodeSendCommand() API depending on the requirements of the kernel.  Internally, this API has logic to determine
    whether or not the input object descriptor is in fact an object array element.  If it is not an object array element, it will return the original object
    descriptor.  This provides flexibility to the kernel to handle both the case of if input element is an object array element or non-object array element.
    An example of this workaround can be found within the display node (video_io/kernels/video_io/display/vx_display_target.c).  Note: due to the limitation of
    the framework, the kernel itself will choose the object array element rather than the application.  Therefore, the element ID passed to the vxGetObjectArrayItem()
    API within the application is ultimately ignored and must be programmed within the kernel.

    ROI Images are supported in the non-pipelined implementation of TIOVX.  However, pipelining a graph containing ROI images is not supported in
    TIOVX.

    \anchor ROI_NOTE Note: If support for ROI images is required for your application, please request support from your local TI sales representative.

    \subsection TIOVX_PIPELINING_USAGE_CONSIDERATIONS_REPLICATE_NODE Pipelining with Replicate Node

    When using graph parameters with the \ref vxReplicateNode feature of OpenVX, there are a few unique scenarios as described above to appropriately
    pipeline the graph.

    Scenario 1: As defined in the OpenVX specification, a node that is replicated uses an element of a composite object (either an object array or a pyramid)
    with a depth of that object equal to the number of total replicated nodes.  Consider the following scenario when using vxReplicateNode() for pipelining.
    The node has input and output images that are elements of an object array.  In the case that the application needs to access this image data inside
    application, this parameter must be created as a graph parameter.  In this case, the first element of the object array is required to be created as
    the graph parameter, rather than the entire object array itself.

    \image html Replicate_Graph_Parameter_Scenario_1.png

    Scenario 2: Let us now consider another similar scenario.  In the situation below, the object array which is used as an input to the replicated node is also
    being provided as an input to another node.  Whereas the replicated node takes as an input parameter an element of the array, the other node takes
    as a parameter then entire object array itself.  In this scenario it is required to create the graph parameter from the entire object array rather
    than just the element of the object array as in Scenario 1.

    \image html Replicate_Graph_Parameter_Scenario_2.png

    \endif

    \subsection TIOVX_PIPELINING_USAGE_CONSIDERATIONS_GRAPH_PARAMETER_SETUP Graph Parameter Considerations

    When setting up data references to send as a parameter of the \ref vxSetGraphScheduleConfig API, all the data references in the list must contain the same meta data as all
    others in the list of data references.  There is presently no validation of the full list of data references done by the framework.  The \ref vxVerifyGraph performs checks based
    on the validate callback of each kernel, but this will only validate the first data reference of the list.  In the case that subsequent data references have different meta-data
    than the first, the framework will not validate this.  Therefore, all data references must consist of the same meta-data.  A validation check has been added to the
    \ref vxSetGraphScheduleConfig API to flag this scenario.

    Consider the graph below.  In the case that Ref 1 is to be made a graph parameter, only a single graph parameter should be made of this reference and it can be made from either
    node parameter.  Therefore, the reference will be dequeued once both the nodes that are consuming the reference are completed.  Note, this is only required when the given
    parameter is at the edge of the graph (i.e., the parameter is not an output of a preceding node.)  In the case that it is an internal buffer to a graph and needs to be made
    as a graph parameter, the graph parameter is created from the output node parameter.

    \image html graph_parameter.png

 */

/*!
    \page TIOVX_IMPORT_EXPORT Import Reference and Export Reference Usage in TIOVX

    In order to further explain the usage of the \ref tivxReferenceImportHandle and \ref tivxReferenceExportHandle API's, several use cases are provided below.
    In particular, each of these use cases note when each \ref vx_reference is in a safe state to release.  Please also note the legend on the left side of the
    images to fully understand the separate blocks within the diagrams.

    \section TIOVX_IMPORT_EXPORT_SCENARIO1 Use Case 1

    The first use case involves 2 \ref vx_reference 's, one which already has a memory buffer allocated for the \ref vx_reference and one which does not.
    (Note: please refer to \ref TIOVX_MEMORY_MANAGEMENT for more information on when memory buffer allocation occurs.)  These use case diagrams describe how
    to successfully import the memory buffer from one \ref vx_reference to the next without causing a memory leak.

    The below image describes the initial state of this use case, with ref1 pointing to buf1 while ref2 does not yet point to a memory buffer.

     \image html scenario1_t1.png width=60%

    The next step in this use case is to export the buf1 from ref1 first to the application, followed by an import of buf1 to ref2.  This sequence is shown in
    the below diagram.  Following this operation, ref2 will be pointing to buf1, while ref1 is also still pointing to buf1.  The important point about this
    portion of the sequence is that neither ref1 nor ref2 should be released at this time, as it will cause the reference that is not released to be pointing
    to a buffer which has been released.

     \image html scenario1_t2.png width=60%

    Now that the ref2 is pointing to buf1, the ref1 is no longer needed.  The proper way to then release ref1 is to first import a NULL pointer to ref1 to
    ensure it is no longer pointing to buf1.  Once this import is done, the ref2 can be used as required and both ref1 and ref2 are in a state that allows them
    to be released without a memory leak or invalid memory access.

     \image html scenario1_t3.png width=60%

    \section TIOVX_IMPORT_EXPORT_SCENARIO2 Use Case 2

    The next use case involves 2 \ref vx_reference 's, which both already have memory buffers allocated for each of the \ref vx_reference 's. (Note: please
    refer to \ref TIOVX_MEMORY_MANAGEMENT for more information on when memory buffer allocation occurs.)  These use case diagrams describe how to successfully
    import the memory buffer from one \ref vx_reference to the next without causing a memory leak.

    The below image describes the initial state of this use case, with ref1 pointing to buf1 and ref2 pointing to buf2.

     \image html scenario2_t1.png width=60%

    Since we are ultimately trying to import buf1 to ref2, we first export buf2 from ref2.  If instead we were to import another buffer directly to ref2 prior
    to this step of exporting buf2, the implementation will throw a \ref VX_ZONE_INFO message indicating that we are overriding the buffer.  In this case,
    this buffer will be lost and there will be a memory leak.  Therefore, this step is needed to avoid such a scenario.

     \image html scenario2_t2.png width=60%

    Similar to the previous use case, the next step is to export the buf1 from ref1 first to the application, followed by an import of buf1 to ref2 as shown
    in the below diagram.  And similarly to the use case above, it should be noted that neither ref1 nor ref2 should be released at this time, as it will cause
    the reference that is not released to be pointing to a buffer which has been released.

     \image html scenario2_t3.png width=60%

    The next step is to free buf2 obtained from the export from ref2 using the \ref tivxMemFree API.  This step can optionally be done before the preceding step
    without issue.  However, while ref1 and ref2 are both pointing to buf1, these still cannot be released.

     \image html scenario2_t4.png width=60%

    Finally, similarly to use case 1, in order to prepare ref1 for release, a NULL pointer can be imported to ref1.  Once this import is done, the ref2 can be
    used as required and both ref1 and ref2 are in a state that allows them to be released without a memory leak or invalid memory access.

     \image html scenario2_t5.png width=60%

    \section TIOVX_IMPORT_EXPORT_SCENARIO3 Use Case 3

    The next use case to consider is the case wherein you want to swap the buffers being pointed to by the references.

    The below image describes the initial state of this use case, with ref1 pointing to buf1 and ref2 pointing to buf2.

     \image html scenario3_t1.png width=60%

    The first step to achieve this swap is to initially export both buf1 from ref1 and buf2 from ref2.

     \image html scenario3_t2.png width=60%

    These buffers are now available to be imported to the different references.  In the diagram below, buf2 is imported into ref1, overriding buf1's association with
    ref1.  At this point in the call sequence, the references are not in a state which allows them to be released properly.

     \image html scenario3_t3.png width=60%

    Finally, the buf1 is imported into ref2, completing the swap.  Once this import is done, both ref1 and ref2 are in a state that allows them to be released without
    a memory leak or invalid memory access.

     \image html scenario3_t4.png width=60%

    \section TIOVX_IMPORT_EXPORT_SCENARIO4 Use Case 4

    The last use case considered here is the import of memory which has not been allocated via the framework directly via the data object constructors.  The memory
    must still be allocated using the \ref tivxMemAlloc API as per the \ref tivxReferenceImportHandle API documentation.

    In the diagram below, the situation is such that a \ref vx_reference has been created with no buffer yet allocated, while a separate buffer has been allocated using
    the \ref tivxMemAlloc API.

     \image html scenario4_t1.png width=60%

    In order to have the ref1 point to this new buf1, the \ref tivxReferenceImportHandle API can be called, creating the desired result of ref1 pointing to buf1.
    Now when the ref1 is released, the buf1 will also be freed.

     \image html scenario4_t2.png width=60%

    \section TIOVX_IMPORT_EXPORT_SCENARIO5 Use Case 5

    One high level use case of this API is sharing the exported buffers across processes.  The below set of diagrams describe how this can be achieved within a multi-process
    scenario.  At a high level, the semantic which must be adhered to in this scenario is that the process which originally allocated the buffer must finally free the buffer.

    First, let us consider two processes where we have created 2 OpenVX references where we ultimately want the OpenVX reference in each process to point to the same memory.
    In the first process, we have ensured the buffer has been allocated, while in the second we have an OpenVX reference pointing to a NULL buffer.  (Note: if the reference
    in P2 has already been allocated, it must first be released similarly to \ref TIOVX_IMPORT_EXPORT_SCENARIO2).

     \image html scenario5_t1.png width=60%

    Next, we would like to export the buffer buf1 from ref1 across the process boundary to ref2. (Note: There are utility API's to do aid in this sharing of buffers across
    process boundaries.  Please reference the "File Descriptor Exchange across Processes" application found within the vision apps package of the PSDK RTOS for an example on
    how to use these.)  Internally to these API's, the physical buffer has now been shared across the process boundary and has been mmapped to the process.  Depending on the
    application, there may now need to be further communication across the process boundary, particularly in the case that each of these references are graph parameters of
    graphs within the separate processes.

     \image html scenario5_t2.png width=60%

    Once the application has completed, the sequence below must be followed for releasing these references.  The consumer processs of the buffer (P2) can first release the
    reference "ref2".  This in turn munmaps the buf1 from this process.  Note that if this buf1 was shared across multiple references in P2, all but one of those references
    can have a NULL buffer imported to the reference and subsequently be released.

     \image html scenario5_t3.png width=60%

    Now that buf1 has been munmapped from P2, the same communication mechanism established from P1 <-> P2 must signal back to P1 that the buffer has been munmapped from P2
    and thus is ready to be ultimately freed from P1.

     \image html scenario5_t4.png width=60%

    Now that P1 has received the signal from P2 that the buf1 has been munmapped, the buf1 is ready to be released. This can be done by releasing ref1, thereby freeing buf1.

     \image html scenario5_t5.png width=60%

    It is important to note that within each process, the buffer handling and releasing adheres to the same semantic as described in the use cases above, namely that
    the actual release of the memory must be done a single time.  For other references which received the imported buffer within the same process, the suggested process
    for release is that the reference has imported a NULL reference prior to release.
 */
