/*!
 * \defgroup group_tivx_c_api TIOVX APIs
 * \brief TI OpenVX API definitions.
 */

/*!
 * \defgroup group_tivx_api 1: TIOVX Public APIs
 * \brief TI OpenVX API definitions that are called by the application or user target kernels.
 * \ingroup group_tivx_c_api
 */

/*!
 * \defgroup group_vx_internal 2: TIOVX Private APIs
 * \brief TI internal function definitions used only within the OpenVX framework and are not exposed on external
 *        header files.  Most users will not need to see these unless you are trying to navigate
 *        or modify the framework internals.
 * \ingroup group_tivx_c_api
 */

/*!
 * \defgroup group_khronos Khronos OpenVX Standard APIs
 * \brief Links to public Khronos OpenVX Specifications and Extensions
 * \ingroup group_tivx_api
 */

/*!
 * \defgroup group_tivx_ext_top TI OpenVX Extension APIs
 * \brief APIs TI has added in addition to Khronos APIs
 * \ingroup group_tivx_api
 */

/*!
 * \defgroup group_tivx_ext_targets a: TI Targets
 * \brief TI Targets (for \ref vxSetNodeTarget)
 *
 * The following list of defines are the list of targets supported
 * for the call to \ref vxSetNodeTarget
 * \ingroup group_tivx_ext_top
 */

/*!
 * \defgroup group_vision_function_hwa b: Kernels
 * \brief TI Vendor Extension Kernels
 * \details The Khronos specification includes a list of supported kernels.  This section
 * lists the additional kernels TI has added.
 * \ingroup group_tivx_ext_top
 */

/*!
 * \defgroup group_tivx_ext_host c: Application Interface APIs
 * \brief APIs for application interface accessible only on host
 * \ingroup group_tivx_ext_top
 */

/*!
 * \defgroup group_tivx_ext_target d: Target Kernel Interface APIs
 * \brief APIs for target kernel interface accessible only on target
 * \ingroup group_tivx_ext_top
 */

/*!
 * \defgroup group_tivx_ext_common e: Common APIs
 * \brief APIs accessible on both host and target
 * \ingroup group_tivx_ext_top
 */

/*!
 * \defgroup group_tivx_log_resource f. Resource Logging APIs
 * \brief APIs for logging of statically allocated framework data structures
 * \ingroup group_tivx_ext_top
 */

/*!
 * \defgroup group_vision_function_tidl TIDL (TI Deep Learning) Kernel APIs
 * \brief TIDL Network Kernels
 * \ingroup group_vision_function_hwa
 */

/*!
 * \defgroup group_vision_function_obj_array_split Object Array Split Kernel APIs
 * \brief Object Array Split Kernels
 * \ingroup group_vision_function_hwa
 */

/*!
 * \defgroup group_tivx_ext_common_kernel Common Kernel API's
 * \brief APIs accessible on both host and target kernels
 * \ingroup group_tivx_ext_common
 */

/*!
 * \defgroup group_vx_framework_config TIOVX Configuration Parameters
 * \brief Parameters used for static configuration of data structures
 * \ingroup group_tivx_ext_common
 */

/*!
 * \defgroup group_tivx_target_kernel Target Kernel APIs
 * \brief APIs for kernel operations on the target
 * \ingroup group_tivx_ext_target
 */

/*!
 * \defgroup group_tivx_ext_bam BAM Wrapper APIs
 * \brief APIs for adding BAM support to DSP target kernel plugins
 * \ingroup group_tivx_ext_target
 */

/*!
 * \defgroup group_tivx_obj_desc Object Descriptor APIs
 * \brief APIs for object descriptor manipulation
 * \ingroup group_tivx_ext_target
 */

/*!
 * \defgroup group_tivx_target_utils Target Utility APIs
 * \brief APIs for common target kernel operations
 * \ingroup group_tivx_ext_target
 */

/*!
 * \defgroup group_tivx_ext_bam_supernode BAM Supernode Support APIs
 * \brief APIs for adding BAM supernode support to DSP target kernel plugins
 * \ingroup group_tivx_ext_bam
 */


/*!
 * \defgroup group_tivx_obj_desc_priv Object Descriptor APIs
 * \brief Internal APIs for object descriptor operations
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_tivx_ext_host_kernel Kernel Helper APIs
 * \brief Helper APIs used used for host side kernel validation
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_vx_context_cfg Context Configuration
 * \brief Context static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_graph_cfg Graph Configuration
 * \brief Graph static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
  * \defgroup group_vx_node_cfg Node Configuration
  * \brief Node static configuration values
  * \ingroup group_vx_framework_config
  */

/*!
 * \defgroup group_vx_array_cfg Data Object: Array Configuration
 * \brief Array static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_convolution_cfg Data Object: Convolution Configuration
 * \brief Convolution static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_distribution_cfg Data Object: Distribution Configuration
 * \brief Distribution static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_delay_cfg Data Object: Delay Configuration
 * \brief Delay static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_image_cfg Data Object: Image Configuration
 * \brief Image static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_lut_cfg Data Object: LUT Configuration
 * \brief LUT static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_matrix_cfg Data Object: Matrix Configuration
 * \brief Matrix static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_object_array_cfg Data Object: Object Array Configuration
 * \brief Object array static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_pyramid_cfg Data Object: Pyramid Configuration
 * \brief Pyramid static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_tivx_raw_image_cfg Data Object: Raw Image Configuration
 * \brief Raw image static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_remap_cfg Data Object: Remap Configuration
 * \brief Remap static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_scalar_cfg Data Object: Scalar Configuration
 * \brief Scalar static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_tensor_cfg Data Object: Tensor Configuration
 * \brief Tensor static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_threshold_cfg Data Object: Threshold Configuration
 * \brief Threshold static configuration values
 * \ingroup group_vx_framework_config
 */
 
/*!
 * \defgroup group_vx_user_data_cfg Data Object: User Data Object Configuration
 * \brief User data object static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_misc_cfg Miscellaneous Objects Configuration
 * \brief Miscellaneous objects static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_ti_extensions_cfg TI Extensions
 * \brief TI Extensions static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_scalar_cfg Data Object: Scalar Configuration
 * \brief Scalar static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_vx_module_cfg Module Configuration Configuration
 * \brief Module static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_tivx_obj_cfg Object Configuration
 * \brief Object static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_tivx_obj_desc_cfg Object Descriptor Configuration
 * \brief Object Descriptor static configuration values
 * \ingroup group_vx_framework_config
 */

/*!
 * \defgroup group_tivx_mem Memory APIs
 * \brief APIs for memory mapping on host and target
 * \ingroup group_tivx_ext_common
 */

/*!
 * \defgroup group_tivx_ext_host_utils Utility APIs
 * \brief Utility APIs available for the application running on the host
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_tivx_osal Platform OSAL APIs
 * \brief Platform operating system abstraction layer on the host
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_tivx_queue Queue APIs
 * \brief APIs for queue operations on the host
 * \ingroup group_tivx_osal
 */

/*!
 * \defgroup group_tivx_mutex Mutex APIs
 * \brief APIs for mutex operations on the host
 * \ingroup group_tivx_osal
 */

/*!
 * \defgroup group_tivx_event Event APIs
 * \brief APIs for event operations on the host
 * \ingroup group_tivx_osal
 */

/*!
 * \defgroup group_tivx_task Task APIs
 * \brief APIs for task operations on the host
 * \ingroup group_tivx_osal
 */

/*!
 * \defgroup group_vx_debug Debug APIs
 * \brief APIs for debug operations on the host
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_vx_framework a: TIOVX Implementation Modules
 * \brief Internal APIs for framework implementation
 * \ingroup group_vx_internal
 */

/*!
 * \defgroup group_tivx_int_common_kernel Kernel Helper APIs
 * \brief Internal APIs for openVX standard 1.1 kernels
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_framework_object Framework Objects
 * \brief Internal APIs for framework object operations
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_framework_data_object Data Objects
 * \brief Internal APIs for data object operations
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_vx_context Context APIs
 * \brief Internal APIs for context operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_graph Graph APIs
 * \brief Internal APIs for graph operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_node Node APIs
 * \brief Internal APIs for node operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_kernel Kernel APIs
 * \brief Internal APIs for kernel operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_parameter Parameter APIs
 * \brief Internal APIs for parameter operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_reference Reference APIs
 * \brief Internal APIs for reference operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_tivx_target Target APIs
 * \brief Internal APIs for target operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_tivx_target_kernel_priv Target Kernel APIs
 * \brief Internal APIs for target kernel operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_tivx_target_kernel_instance Target Kernel Instance APIs
 * \brief Internal APIs for target kernel instance operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_tivx_obj Object APIs
 * \brief Internal APIs for object operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_module Module APIs
 * \brief Internal APIs for module operations
 * \ingroup group_vx_framework_object
 */

/*!
 * \defgroup group_vx_image Image APIs
 * \brief Internal APIs for image operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_scalar Scalar APIs
 * \brief Internal APIs for scalar operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_remap Remap APIs
 * \brief Internal APIs for remap operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_delay Delay APIs
 * \brief Internal APIs for delay operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_objarray Object Array APIs
 * \brief Internal APIs for object array operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_array Array APIs
 * \brief Internal APIs for array operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_convolution Convolution APIs
 * \brief Internal APIs for convolution operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_distribution Distribution APIs
 * \brief Internal APIs for distribution operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_lut LUT APIs
 * \brief Internal APIs for lut operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_matrix Matrix APIs
 * \brief Internal APIs for matrix operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_pyramid Pyramid APIs
 * \brief Internal APIs for pyramid operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_threshold Threshold APIs
 * \brief Internal APIs for threshold operations
 * \ingroup group_vx_framework_data_object
 */

/*!
 * \defgroup group_vx_framework_utils Utility and Debug Modules
 * \brief Internal APIs for utility and debug operations
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_tivx_log_rt_trace Real-time Trace APIs
 * \brief APIs for real-time node processing trace operations
 * \ingroup group_vx_framework
 */

/*!
 * \defgroup group_tivx_log_rt_trace_host Real-time Trace APIs
 * \brief APIs for real-time event trace operations on the host
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_tivx_ext_host_resource_log Resource Logging APIs
 * \brief APIs for resource logging on the host
 * \ingroup group_tivx_ext_host
 */

/*!
 * \defgroup group_tivx_log_rt_trace_target Real-time Trace APIs
 * \brief APIs for real-time event trace operations on the target
 * \ingroup group_tivx_ext_target
 */


/*!
 * \defgroup group_vx_utils Utility APIs
 * \brief Internal APIs for utility operations
 * \ingroup group_vx_framework_utils
 */

/*!
 * \defgroup group_vx_error Error APIs
 * \brief Internal APIs for error operations
 * \ingroup group_vx_framework_utils
 */

/*!
 * \defgroup group_vx_platform b: TIOVX Platform Modules
 * \brief Internal APIs for platform-specific operations
 * \ingroup group_vx_internal
 */


/*!
 * \defgroup group_tivx_ipc Inter-Processor Communication (IPC) APIs
 * \brief Internal APIs for IPC operations
 * \ingroup group_vx_platform
 */

/*!
 * \defgroup group_tivx_platform Platform APIs
 * \brief Internal APIs for platform operations
 * \ingroup group_vx_platform
 */

