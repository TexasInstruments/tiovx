#include "test_engine/test.h"
#include "test_tiovx.h"

#include <TI/tivx_event.h>
#include <TI/tivx_task.h>
#include <TI/tivx_test_kernels.h>
#include <TI/tivx_capture.h>

#define TIVX_TEST_INVALID_TIMEOUT   (0U)
#define TIVX_TEST_FAIL_CLEANUP(x) {(x) = 1; goto cleanup;}
#define TIVX_TEST_UPDATE_STATUS(x) {if ((x)) CT_RecordFailure();}

#define TIVX_TEST_END_WAIT_DELAY    (20)
#if 1
#define TIVX_TEST_WAIT_BEFORE_EXIT() tivxTaskWaitMsecs(TIVX_TEST_END_WAIT_DELAY)
#else
#define TIVX_TEST_WAIT_BEFORE_EXIT()
#endif

/* Test error codes. */
#define TIVX_TEST_SUCCESS                           (0)
#define TIVX_TEST_ERROR_GRAPH_CREATE_FAILED         (-1)
#define TIVX_TEST_ERROR_NODE_CREATE_FAILED          (-2)
#define TIVX_TEST_ERROR_CFG_OBJ_CREATE_FAILED       (-3)
#define TIVX_TEST_ERROR_SCALAR_INPUT_CREATE_FAILED  (-4)
#define TIVX_TEST_ERROR_SCALAR_OUTPUT_CREATE_FAILED (-5)
#define TIVX_TEST_ERROR_GRAPH_DELETE_FAILED         (-6)
#define TIVX_TEST_ERROR_NODE_DELETE_FAILED          (-7)
#define TIVX_TEST_ERROR_CFG_OBJ_DELETE_FAILED       (-8)
#define TIVX_TEST_ERROR_SCALAR_INPUT_DELETE_FAILED  (-9)
#define TIVX_TEST_ERROR_SCALAR_OUTPUT_DELETE_FAILED (-10)
#define TIVX_TEST_ERROR_NODE_SET_TARGET_FAILED      (-11)

TESTCASE(tivxCmdTimeout, CT_VXContext, ct_setup_vx_context, 0)

typedef struct
{
    vx_context                  vxContext;
    vx_graph                    vxGraph;
    vx_node                     vxScalarSrcNode;
    vx_node                     vxCmdTestNode;
    vx_user_data_object         vxCfg;
    vx_scalar                   vxInScalar;
    vx_scalar                   vxOutScalar;

    tivx_cmd_timeout_params_t   cfgParams;

} TestObjContext;

typedef struct
{
    const char* name;
    const char *tgt;
} TestArg;

#if defined(SOC_AM62A)
#define TEST_PARAMS \
    CT_GENERATE_PARAMETERS("TIVX_TARGET_A72_0", ARG, TIVX_TARGET_A72_0), \
    CT_GENERATE_PARAMETERS("TIVX_TARGET_MCU1_0", ARG, TIVX_TARGET_MCU1_0), \

#else
#define TEST_PARAMS \
    CT_GENERATE_PARAMETERS("TIVX_TARGET_A72_0", ARG, TIVX_TARGET_A72_0), \
    CT_GENERATE_PARAMETERS("TIVX_TARGET_MCU2_0", ARG, TIVX_TARGET_MCU2_0), \
    CT_GENERATE_PARAMETERS("TIVX_TARGET_MCU2_1", ARG, TIVX_TARGET_MCU2_1), \

#endif

int32_t CreateGraph(TestObjContext *objCntxt, const char *tgt)
{
    vx_status   vxStatus = (vx_status)VX_SUCCESS;
    int32_t     status = TIVX_TEST_SUCCESS;

    objCntxt->vxGraph         = NULL;
    objCntxt->vxScalarSrcNode = NULL;
    objCntxt->vxCmdTestNode   = NULL;
    objCntxt->vxCfg           = NULL;
    objCntxt->vxInScalar      = NULL;
    objCntxt->vxOutScalar     = NULL;

    tivx_clr_debug_zone(VX_ZONE_INFO);

    /* Load the kernels. */
    tivxTestKernelsLoadKernels(objCntxt->vxContext);

    /* Create a graph. */
    objCntxt->vxGraph = vxCreateGraph(objCntxt->vxContext);

    if (objCntxt->vxGraph == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCreateGraph() failed.\n");
        status = TIVX_TEST_ERROR_GRAPH_CREATE_FAILED;
    }
    else
    {
        vxSetReferenceName((vx_reference)objCntxt->vxGraph,
                           "Command Timeout Test Graph");
    }

    if (status == TIVX_TEST_SUCCESS)
    {
        /* Create a config object. */
        objCntxt->vxCfg =
            vxCreateUserDataObject(objCntxt->vxContext,
                                   "tivx_cmd_timeout_params_t",
                                   sizeof(tivx_cmd_timeout_params_t),
                                   &objCntxt->cfgParams);

        if (objCntxt->vxCfg == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreateUserDataObject() failed.\n");
            status = TIVX_TEST_ERROR_CFG_OBJ_CREATE_FAILED;
        }
        else
        {
            vxSetReferenceName((vx_reference)objCntxt->vxCfg,
                               "Command Timeout Test Config");
        }
    }

    if (status == TIVX_TEST_SUCCESS)
    {
        /* Create the scalar input object. */
        objCntxt->vxInScalar = vxCreateScalar(objCntxt->vxContext,
                                              VX_TYPE_UINT8,
                                              NULL);

        if (objCntxt->vxInScalar == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreateScalar() for input failed.\n");
            status = TIVX_TEST_ERROR_SCALAR_INPUT_CREATE_FAILED;
        }
        else
        {
            vxSetReferenceName((vx_reference)objCntxt->vxInScalar,
                               "Input Scalar");
        }
    }

    if (status == TIVX_TEST_SUCCESS)
    {
        /* Create the scalar output object. */
        objCntxt->vxOutScalar = vxCreateScalar(objCntxt->vxContext,
                                               VX_TYPE_UINT8,
                                               NULL);

        if (objCntxt->vxOutScalar == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxCreateScalar() for output failed.\n");
            status = TIVX_TEST_ERROR_SCALAR_OUTPUT_CREATE_FAILED;
        }
        else
        {
            vxSetReferenceName((vx_reference)objCntxt->vxOutScalar,
                               "Output Scalar");
        }
    }

    if (status == TIVX_TEST_SUCCESS)
    {
        /* Create a timeout test node. */
        objCntxt->vxScalarSrcNode = tivxScalarSourceNode(objCntxt->vxGraph,
                                                         objCntxt->vxInScalar);

        if (objCntxt->vxScalarSrcNode == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxScalarSourceNode() failed.\n");
            status = TIVX_TEST_ERROR_NODE_CREATE_FAILED;
        }
        else
        {
            #if defined(SOC_AM62A)
            /* Set the node target to TIVX_TARGET_MCU1_0. */
            vxStatus = vxSetNodeTarget(objCntxt->vxScalarSrcNode,
                                       VX_TARGET_STRING,
                                       TIVX_TARGET_MCU1_0);
            #else
            /* Set the node target to TIVX_TARGET_MCU2_0. */
            vxStatus = vxSetNodeTarget(objCntxt->vxScalarSrcNode,
                                       VX_TARGET_STRING,
                                       TIVX_TARGET_MCU2_0);
            #endif

            if (vxStatus != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "vxSetNodeTarget(%s) failed for ScalarNode.\n", tgt);
                status = TIVX_TEST_ERROR_NODE_SET_TARGET_FAILED;
            }
            else
            {
                vxSetReferenceName((vx_reference)objCntxt->vxScalarSrcNode,
                                   "Scalar Source Node");
            }
        }
    }

    if (status == TIVX_TEST_SUCCESS)
    {
        /* Create a timeout test node. */
        objCntxt->vxCmdTestNode = tivxCmdTimeoutTestNode(objCntxt->vxGraph,
                                                         objCntxt->vxCfg,
                                                         objCntxt->vxInScalar,
                                                         objCntxt->vxOutScalar);

        if (objCntxt->vxCmdTestNode == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxCmdTimeoutTestNode() failed.\n");
            status = TIVX_TEST_ERROR_NODE_CREATE_FAILED;
        }
        else
        {
            /* Set the node target. */
            vxStatus = vxSetNodeTarget(objCntxt->vxCmdTestNode,
                                       VX_TARGET_STRING,
                                       tgt);

            if (vxStatus != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "vxSetNodeTarget(%s) failed for CmdTestNode.\n", tgt);
                status = TIVX_TEST_ERROR_NODE_SET_TARGET_FAILED;
            }
            else
            {
                vxSetReferenceName((vx_reference)objCntxt->vxCmdTestNode,
                                   "Command Timeout Test Node");
            }
        }
    }

    return status;

} /* CreateGraph */

int32_t DeleteGraph(TestObjContext *objCntxt)
{
    vx_status   vxStatus  = (vx_status)VX_SUCCESS;
    int32_t     status = TIVX_TEST_SUCCESS;

    /* Release the command test node. */
    if (objCntxt->vxCmdTestNode)
    {
        vxStatus = vxReleaseNode(&objCntxt->vxCmdTestNode);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxReleaseNode() failed.\n");
            status = TIVX_TEST_ERROR_NODE_DELETE_FAILED;
        }
    }

    /* Release the scalar source node. */
    if (objCntxt->vxScalarSrcNode)
    {
        vxStatus = vxReleaseNode(&objCntxt->vxScalarSrcNode);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxReleaseNode() failed.\n");
            status = TIVX_TEST_ERROR_NODE_DELETE_FAILED;
        }
    }

    /* Release the input scalar object. */
    if (objCntxt->vxInScalar)
    {
        vxStatus = vxReleaseScalar(&objCntxt->vxInScalar);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxReleaseScalar() for input failed.\n");

            if (status == TIVX_TEST_SUCCESS)
            {
                status = TIVX_TEST_ERROR_SCALAR_INPUT_DELETE_FAILED;
            }
        }
    }

    /* Release the output scalar object. */
    if (objCntxt->vxOutScalar)
    {
        vxStatus = vxReleaseScalar(&objCntxt->vxOutScalar);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxReleaseScalar() for output failed.\n");

            if (status == TIVX_TEST_SUCCESS)
            {
                status = TIVX_TEST_ERROR_SCALAR_OUTPUT_DELETE_FAILED;
            }
        }
    }

    /* Release the user data object. */
    if (objCntxt->vxCfg)
    {
        vxStatus = vxReleaseUserDataObject(&objCntxt->vxCfg);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxReleaseGraph() failed.\n");

            if (status == TIVX_TEST_SUCCESS)
            {
                status = TIVX_TEST_ERROR_CFG_OBJ_DELETE_FAILED;
            }
        }
    }

    /* Release the graph. */
    if (objCntxt->vxGraph)
    {
        vxStatus = vxReleaseGraph(&objCntxt->vxGraph);

        if (vxStatus != (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "vxReleaseGraph() failed.\n");

            if (vxStatus != (vx_status)TIVX_ERROR_EVENT_TIMEOUT)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "vxReleaseGraph() failed. Expected an error code [%d] but "
                         "received [%d]\n", TIVX_ERROR_EVENT_TIMEOUT, vxStatus);
            }

            if (status == TIVX_TEST_SUCCESS)
            {
                status = TIVX_TEST_ERROR_GRAPH_DELETE_FAILED;
            }
        }
    }

    /* Unload the kernels. */
    tivxTestKernelsUnLoadKernels(objCntxt->vxContext);

    tivx_clr_debug_zone(VX_ZONE_INFO);

    return status;

} /* DeleteGraph */

/* This test checks the following:
 * - Creates a graph with single node which would have default timeout value of
 *   TIVX_EVENT_TIMEOUT_WAIT_FOREVER.
 * - Queries the timeout values from the graph and node contexts to verify that
 *   they are equal to TIVX_EVENT_TIMEOUT_WAIT_FOREVER
 */
TEST_WITH_ARG(tivxCmdTimeout, testDefaultTimeout, TestArg, TEST_PARAMS)
{
    TestObjContext              objCntxt;
    tivx_cmd_timeout_params_t  *cfgParams;
    vx_uint32                   vxTimeoutVal;
    uint32_t                    testFail = 0;
    vx_status                   vxStatus;
    int32_t                     status;

    /* Initialize the timeout to 0 means do not use this parameter. */
    cfgParams                    = &objCntxt.cfgParams;
    objCntxt.vxContext           = context_->vx_context_;
    cfgParams->createCmdTimeout  = 0;
    cfgParams->deleteCmdTimeout  = 0;
    cfgParams->controlCmdTimeout = 0;
    cfgParams->processCmdTimeout = 0;

    /* Create objects. */
    status = CreateGraph(&objCntxt, arg_->tgt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "CreateGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Query the graph timeout and validate. */
    vxStatus = vxQueryGraph(objCntxt.vxGraph, TIVX_GRAPH_TIMEOUT,
                            &vxTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* The default timeout should be TIVX_EVENT_TIMEOUT_WAIT_FOREVER. */
    if (vxTimeoutVal != TIVX_EVENT_TIMEOUT_WAIT_FOREVER)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Expected default graph time out does not match");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Query the node timeout and validate. */
    vxStatus = vxQueryNode(objCntxt.vxCmdTestNode, TIVX_NODE_TIMEOUT,
                           &vxTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryNode() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* The default timeout should be TIVX_EVENT_TIMEOUT_WAIT_FOREVER. */
    if (vxTimeoutVal != TIVX_EVENT_TIMEOUT_WAIT_FOREVER)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Expected default node time out does not match");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Verify the graph. */
    vxStatus = vxVerifyGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxVerifyGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

cleanup:
    /* Release the objects. */
    status = DeleteGraph(&objCntxt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "DeleteGraph() failed.\n");
        testFail = 1;
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

/* This test checks the following:
 * - Creates a graph with single node which would have default timeout value of
 *   TIVX_EVENT_TIMEOUT_WAIT_FOREVER.
 * - Tries to set an invalid value value of 0 (zero) and checkes that the set
 *   attribute for graph and node gets rejected.
 * - The timeout values of the graph and node still remain to be
 *   TIVX_EVENT_TIMEOUT_WAIT_FOREVER
 */
TEST_WITH_ARG(tivxCmdTimeout, testInvalidTimeoutSet, TestArg, TEST_PARAMS)
{
    TestObjContext              objCntxt;
    tivx_cmd_timeout_params_t  *cfgParams;
    vx_uint32                   vxTimeoutVal;
    uint32_t                    testFail = 0;
    vx_status                   vxStatus;
    int32_t                     status;

    /* Initialize the timeout to 0 means do not use this parameter. */
    cfgParams                    = &objCntxt.cfgParams;
    objCntxt.vxContext           = context_->vx_context_;
    cfgParams->createCmdTimeout  = 0;
    cfgParams->deleteCmdTimeout  = 0;
    cfgParams->controlCmdTimeout = 0;
    cfgParams->processCmdTimeout = 0;

    /* Create objects. */
    status = CreateGraph(&objCntxt, arg_->tgt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "CreateGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Set an invalid timeout attribute. This API should fail and the default
     * attribute should not be modified.
     */
    vxTimeoutVal = TIVX_TEST_INVALID_TIMEOUT;
    vxStatus = vxSetGraphAttribute(objCntxt.vxGraph, TIVX_GRAPH_TIMEOUT,
                                   &vxTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_ERROR_INVALID_PARAMETERS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetGraphAttribute(TIVX_GRAPH_TIMEOUT) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Query the graph timeout and validate. */
    vxStatus = vxQueryGraph(objCntxt.vxGraph, TIVX_GRAPH_TIMEOUT,
                            &vxTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* The default timeout should still be TIVX_EVENT_TIMEOUT_WAIT_FOREVER. */
    if (vxTimeoutVal != TIVX_EVENT_TIMEOUT_WAIT_FOREVER)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Expected default graph time out does not match");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Set an invalid timeout attribute. This API should fail and the default
     * attribute should not be modified.
     */
    vxTimeoutVal = TIVX_TEST_INVALID_TIMEOUT;
    vxStatus = vxSetNodeAttribute(objCntxt.vxCmdTestNode, TIVX_NODE_TIMEOUT,
                                   &vxTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_ERROR_INVALID_PARAMETERS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetNodeAttribute(TIVX_NODE_TIMEOUT) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Query the node timeout and validate. */
    vxStatus = vxQueryNode(objCntxt.vxCmdTestNode, TIVX_NODE_TIMEOUT,
                           &vxTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryNode() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* The default timeout should still be TIVX_EVENT_TIMEOUT_WAIT_FOREVER. */
    if (vxTimeoutVal != TIVX_EVENT_TIMEOUT_WAIT_FOREVER)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Expected default node time out does not match");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Verify the graph. */
    vxStatus = vxVerifyGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxVerifyGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

cleanup:
    /* Release the objects. */
    status = DeleteGraph(&objCntxt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "DeleteGraph() failed.\n");
        testFail = 1;
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

/* This test checks the following:
 * - Creates a graph with single node which would have default timeout value of
 *   TIVX_EVENT_TIMEOUT_WAIT_FOREVER.
 * - Tries to set a value value of 10 mill-secs and checkes that the set
 *   attribute for graph and node gets through.
 * - The timeout values of the graph and node are queried to check that the
 *   new value has taken effect.
 */
TEST_WITH_ARG(tivxCmdTimeout, testValidTimeoutSet, TestArg, TEST_PARAMS)
{
    TestObjContext              objCntxt;
    tivx_cmd_timeout_params_t  *cfgParams;
    vx_uint32                   vxSetTimeoutVal;
    vx_uint32                   vxTimeoutVal;
    uint32_t                    testFail = 0;
    vx_status                   vxStatus;
    int32_t                     status;

    /* Set the graph and node timeout values to be 10 milli-seconds. */
    vxSetTimeoutVal = 10;

    /* Initialize the timeout to a valid timeout value less tha vxSetTimeoutVal. */
    cfgParams                    = &objCntxt.cfgParams;
    objCntxt.vxContext           = context_->vx_context_;
    cfgParams->createCmdTimeout  = 2;
    cfgParams->deleteCmdTimeout  = 7;
    cfgParams->controlCmdTimeout = 2;
    cfgParams->processCmdTimeout = 0;

    /* Create objects. */
    status = CreateGraph(&objCntxt, arg_->tgt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "CreateGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Set a valid timeout attribute. */
    vxStatus = vxSetGraphAttribute(objCntxt.vxGraph, TIVX_GRAPH_TIMEOUT,
                                   &vxSetTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetGraphAttribute(TIVX_GRAPH_TIMEOUT) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Query the graph timeout and validate. */
    vxStatus = vxQueryGraph(objCntxt.vxGraph, TIVX_GRAPH_TIMEOUT,
                            &vxTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* The timeout should be updated. */
    if (vxTimeoutVal != vxSetTimeoutVal)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Expected default graph time out does not match");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Set a valid timeout attribute.  */
    vxStatus = vxSetNodeAttribute(objCntxt.vxCmdTestNode, TIVX_NODE_TIMEOUT,
                                   &vxSetTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetNodeAttribute(TIVX_NODE_TIMEOUT) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Query the node timeout and validate. */
    vxStatus = vxQueryNode(objCntxt.vxCmdTestNode, TIVX_NODE_TIMEOUT,
                           &vxTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryNode() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* The timeout should be updated. */
    if (vxTimeoutVal != vxSetTimeoutVal)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Expected default node time out does not match");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Verify the graph. */
    vxStatus = vxVerifyGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxVerifyGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

cleanup:
    /* Release the objects. */
    status = DeleteGraph(&objCntxt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "DeleteGraph() failed.\n");
        testFail = 1;
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

/* This test checks the following:
 * - Creates a graph with single node which would have default timeout value of
 *   TIVX_EVENT_TIMEOUT_WAIT_FOREVER.
 * - Tries to set a value value of 1 mill-secs and checkes that the set
 *   attribute for node gets through.
 * - The node is configured to wait for 5 milli-sec on create which should
 *   lead to the graph verification failure.
 */
TEST_WITH_ARG(tivxCmdTimeout, testTimeoutCreateFail, TestArg, TEST_PARAMS)
{
    TestObjContext              objCntxt;
    tivx_cmd_timeout_params_t  *cfgParams;
    vx_uint32                   vxSetTimeoutVal;
    vx_uint32                   vxTimeoutVal;
    uint32_t                    testFail = 0;
    vx_status                   vxStatus;
    int32_t                     status;

    /* Initialize the timeout to a valid timeout value less tha vxSetTimeoutVal. */
    cfgParams                    = &objCntxt.cfgParams;
    objCntxt.vxContext           = context_->vx_context_;
    cfgParams->createCmdTimeout  = 2;
    cfgParams->deleteCmdTimeout  = 0;
    cfgParams->controlCmdTimeout = 0;
    cfgParams->processCmdTimeout = 0;

    /* Create objects. */
    status = CreateGraph(&objCntxt, arg_->tgt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "CreateGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Set the node timeout value. */
    vxSetTimeoutVal = 1;

    /* Set a valid timeout attribute.  */
    vxStatus = vxSetNodeAttribute(objCntxt.vxCmdTestNode, TIVX_NODE_TIMEOUT,
                                   &vxSetTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetNodeAttribute(TIVX_NODE_TIMEOUT) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Query the node timeout and validate. */
    vxStatus = vxQueryNode(objCntxt.vxCmdTestNode, TIVX_NODE_TIMEOUT,
                           &vxTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryNode() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* The timeout should be updated. */
    if (vxTimeoutVal != vxSetTimeoutVal)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Expected default node time out does not match");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Verify the graph. */
    vxStatus = vxVerifyGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)TIVX_ERROR_EVENT_TIMEOUT)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "vxVerifyGraph() failed. Expected an error code [%d] but "
                 "received [%d]\n", TIVX_ERROR_EVENT_TIMEOUT, vxStatus);
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Delay the exit. */
    TIVX_TEST_WAIT_BEFORE_EXIT();

cleanup:
    /* Release the objects. */
    status = DeleteGraph(&objCntxt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "DeleteGraph() failed.\n");
        testFail = 1;
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

/* This test checks the following:
 * - Creates a graph with single node which would have default timeout value of
 *   TIVX_EVENT_TIMEOUT_WAIT_FOREVER.
 * - Tries to set a value value of 1 mill-secs and checkes that the set
 *   attribute for graph gets through.
 * - The node is configured to wait for 5 milli-sec on delete which should
 *   lead to the graph deletion failure.
 */
TEST_WITH_ARG(tivxCmdTimeout, testTimeoutDeleteFail, TestArg, TEST_PARAMS)
{
    TestObjContext              objCntxt;
    tivx_cmd_timeout_params_t  *cfgParams;
    vx_uint32                   vxSetTimeoutVal;
    vx_uint32                   vxTimeoutVal;
    uint32_t                    testFail = 0;
    vx_status                   vxStatus;
    int32_t                     status;

    /* Initialize the timeout to a valid timeout value less tha vxSetTimeoutVal. */
    cfgParams                    = &objCntxt.cfgParams;
    objCntxt.vxContext           = context_->vx_context_;
    cfgParams->createCmdTimeout  = 0;
    cfgParams->deleteCmdTimeout  = 5;
    cfgParams->controlCmdTimeout = 0;
    cfgParams->processCmdTimeout = 0;

    /* Create objects. */
    status = CreateGraph(&objCntxt, arg_->tgt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "CreateGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Set the node timeout value. */
    vxSetTimeoutVal = 2;

    /* Set a valid timeout attribute.  */
    vxStatus = vxSetNodeAttribute(objCntxt.vxCmdTestNode, TIVX_NODE_TIMEOUT,
                                   &vxSetTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetNodeAttribute(TIVX_NODE_TIMEOUT) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Query the node timeout and validate. */
    vxStatus = vxQueryNode(objCntxt.vxCmdTestNode, TIVX_NODE_TIMEOUT,
                           &vxTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryNode() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* The timeout should be updated. */
    if (vxTimeoutVal != vxSetTimeoutVal)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Expected default node time out does not match");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Verify the graph. */
    vxStatus = vxVerifyGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxVerifyGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Delay the exit. */
    TIVX_TEST_WAIT_BEFORE_EXIT();

cleanup:
    /* Release the objects. */
    status = DeleteGraph(&objCntxt);

    if (status != TIVX_TEST_ERROR_GRAPH_DELETE_FAILED)
    {
        VX_PRINT(VX_ZONE_ERROR, "DeleteGraph() failed.\n");
        testFail = 1;
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

/* This test checks the following:
 * - Creates a graph with single node which would have default timeout value of
 *   TIVX_EVENT_TIMEOUT_WAIT_FOREVER.
 * - The node is configured to wait for 2 milli-sec on control command which should
 *   lead to the control command API to fail.
 */
TEST_WITH_ARG(tivxCmdTimeout, testTimeoutCtrlCmdFail, TestArg, TEST_PARAMS)
{
    TestObjContext              objCntxt;
    tivx_cmd_timeout_params_t  *cfgParams;
    vx_uint32                   vxTimeoutVal;
    uint32_t                    testFail = 0;
    vx_status                   vxStatus;
    int32_t                     status;

    /* Initialize the timeout to a valid timeout set. */
    cfgParams                    = &objCntxt.cfgParams;
    objCntxt.vxContext           = context_->vx_context_;
    cfgParams->createCmdTimeout  = 0;
    cfgParams->deleteCmdTimeout  = 0;
    cfgParams->controlCmdTimeout = 2;
    cfgParams->processCmdTimeout = 0;

    /* Create objects. */
    status = CreateGraph(&objCntxt, arg_->tgt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "CreateGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Verify the graph. */
    vxStatus = vxVerifyGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxVerifyGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Issue a control command. */
    /* Set a value greater than cfgParams->controlCmdTimeout. This should pass. */
    vxStatus = tivxNodeSendCommandTimed(objCntxt.vxCmdTestNode, 0, 0,
                                        (vx_reference*)&objCntxt.vxCfg, 1, 5);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxNodeSendCommandTimed() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Set a value less than cfgParams->controlCmdTimeout. This should fail. */
    vxStatus = tivxNodeSendCommandTimed(objCntxt.vxCmdTestNode, 0, 0,
                                        (vx_reference*)&objCntxt.vxCfg, 1, 1);

    if (vxStatus != (vx_status)TIVX_ERROR_EVENT_TIMEOUT)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "tivxNodeSendCommandTimed() failed. Expected an error code [%d] but "
                 "received [%d]\n", TIVX_ERROR_EVENT_TIMEOUT, vxStatus);
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Delay the exit. */
    TIVX_TEST_WAIT_BEFORE_EXIT();

cleanup:
    /* Release the objects. */
    status = DeleteGraph(&objCntxt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "DeleteGraph() failed.\n");
        testFail = 1;
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

/* This test checks the following:
 * - Creates a graph with single node which would have default timeout value of
 *   TIVX_EVENT_TIMEOUT_WAIT_FOREVER.
 * - The graph timeout attribute it updated to check the process function behavior.
 */
TEST_WITH_ARG(tivxCmdTimeout, testTimeoutGraph, TestArg, TEST_PARAMS)
{
    TestObjContext              objCntxt;
    tivx_cmd_timeout_params_t  *cfgParams;
    vx_uint32                   vxTimeoutVal;
    vx_uint32                   vxSetTimeoutVal;
    uint32_t                    testFail = 0;
    vx_status                   vxStatus;
    int32_t                     status;

    /* Initialize the timeout to a valid timeout set. */
    cfgParams                    = &objCntxt.cfgParams;
    objCntxt.vxContext           = context_->vx_context_;
    cfgParams->createCmdTimeout  = 0;
    cfgParams->deleteCmdTimeout  = 0;
    cfgParams->controlCmdTimeout = 0;
    cfgParams->processCmdTimeout = 0;

    /* Create objects. */
    status = CreateGraph(&objCntxt, arg_->tgt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "CreateGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Verify the graph. */
    vxStatus = vxVerifyGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxVerifyGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Schedule the graph. */
    vxStatus = vxScheduleGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxScheduleGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    vxStatus = vxWaitGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxWaitGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Change the timeout attribute of the graph. */
    vxSetTimeoutVal = 5;

    vxStatus = vxSetGraphAttribute(objCntxt.vxGraph, TIVX_GRAPH_TIMEOUT,
                                   &vxSetTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetGraphAttribute(TIVX_GRAPH_TIMEOUT) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Update the config parameters and update the vx object. */
    cfgParams->processCmdTimeout = 15;

    vxStatus = vxCopyUserDataObject(objCntxt.vxCfg,
                                    0,
                                    sizeof(tivx_cmd_timeout_params_t),
                                    cfgParams,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCopyUserDataObject() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Schedule the graph. */
    vxStatus = vxScheduleGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxScheduleGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    vxStatus = vxWaitGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)TIVX_ERROR_EVENT_TIMEOUT)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "vxWaitGraph() failed. Expected an error code [%d] but "
                 "received [%d]\n", TIVX_ERROR_EVENT_TIMEOUT, vxStatus);
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Delay the exit. */
    TIVX_TEST_WAIT_BEFORE_EXIT();

cleanup:
    /* Release the objects. */
    status = DeleteGraph(&objCntxt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "DeleteGraph() failed.\n");
        testFail = 1;
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

/* This test checks the following:
 * - Creates a graph with single node which would have default timeout value of
 *   TIVX_EVENT_TIMEOUT_WAIT_FOREVER.
 * - Set the graph in streaming mode
 * - The graph timeout attribute it updated to check the process function behavior.
 */
TEST_WITH_ARG(tivxCmdTimeout, testTimeoutGraphStream, TestArg, TEST_PARAMS)
{
    TestObjContext              objCntxt;
    tivx_cmd_timeout_params_t  *cfgParams;
    vx_uint32                   vxTimeoutVal;
    vx_uint32                   vxSetTimeoutVal;
    uint32_t                    testFail = 0;
    uint32_t                    numStreams;
    vx_status                   vxStatus;
    int32_t                     status;

    /* Initialize the timeout to a valid timeout set. */
    cfgParams                    = &objCntxt.cfgParams;
    objCntxt.vxContext           = context_->vx_context_;
    cfgParams->createCmdTimeout  = 0;
    cfgParams->deleteCmdTimeout  = 0;
    cfgParams->controlCmdTimeout = 0;
    cfgParams->processCmdTimeout = 0;

    /* Create objects. */
    status = CreateGraph(&objCntxt, arg_->tgt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "CreateGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Enable streaming. */
    vxStatus = vxEnableGraphStreaming(objCntxt.vxGraph, objCntxt.vxScalarSrcNode);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxStartGraphStreaming() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Verify the graph. */
    vxStatus = vxVerifyGraph(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxVerifyGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Start streaming. */
    vxStatus = vxStartGraphStreaming(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxStartGraphStreaming() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    tivxTaskWaitMsecs(10);

    /* Stop streaming. */
    vxStatus = vxStopGraphStreaming(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxStopGraphStreaming() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Validate if the streaming has been successful. */
    vxStatus = vxQueryGraph(objCntxt.vxGraph, TIVX_GRAPH_STREAM_EXECUTIONS,
                            &numStreams, sizeof(uint32_t));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    if (numStreams == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "Graph streaming operation failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Change the timeout attribute of the graph. */
    vxSetTimeoutVal = 5;

    vxStatus = vxSetGraphAttribute(objCntxt.vxGraph, TIVX_GRAPH_TIMEOUT,
                                   &vxSetTimeoutVal, sizeof(vx_uint32));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxSetGraphAttribute(TIVX_GRAPH_TIMEOUT) failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Update the config parameters and update the vx object. */
    cfgParams->processCmdTimeout = 15;

    vxStatus = vxCopyUserDataObject(objCntxt.vxCfg,
                                    0,
                                    sizeof(tivx_cmd_timeout_params_t),
                                    cfgParams,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxCopyUserDataObject() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Start streaming. */
    vxStatus = vxStartGraphStreaming(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxStartGraphStreaming() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    tivxTaskWaitMsecs(10);

    /* Stop streaming. */
    vxStatus = vxStopGraphStreaming(objCntxt.vxGraph);

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxStopGraphStreaming() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Validate if the streaming has been successful. */
    vxStatus = vxQueryGraph(objCntxt.vxGraph, TIVX_GRAPH_STREAM_EXECUTIONS,
                            &numStreams, sizeof(uint32_t));

    if (vxStatus != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "vxQueryGraph() failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    if (numStreams == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "Graph streaming operation failed.\n");
        TIVX_TEST_FAIL_CLEANUP(testFail);
    }

    /* Delay the exit. */
    TIVX_TEST_WAIT_BEFORE_EXIT();

cleanup:
    /* Release the objects. */
    status = DeleteGraph(&objCntxt);

    if (status != TIVX_TEST_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "DeleteGraph() failed.\n");
        testFail = 1;
    }

    TIVX_TEST_UPDATE_STATUS(testFail);
}

TESTCASE_TESTS(tivxCmdTimeout,
               testDefaultTimeout,
               testInvalidTimeoutSet,
               testValidTimeoutSet,
               testTimeoutCreateFail,
               testTimeoutDeleteFail,
               testTimeoutCtrlCmdFail,
               testTimeoutGraph,
               DISABLED_testTimeoutGraphStream)

