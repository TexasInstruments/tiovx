/*
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "TI/tivx.h"
#include "VX/vx.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_nf_bilateral.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vpac_nf_priv.h"
#include "vx_kernels_hwa_target.h"
#include <TI_BilateralFilter_cn.h>
#include <math.h>

#define LUT_ROWS 5

typedef struct
{
    /* Pointers to src and dest */
    uint16_t *src16;
    uint16_t *dst16;
    uint32_t buffer_size;
    uint16_t lut[LUT_ROWS*256];

    bilateralFilter_mmr_t mmr;
    bilateralFilter_debug_t debug;

} tivxVpacNfBilateralParams;

/* Given that for J7AHP, there are multiple VPAC's, there needs to be separate
 * target kernels in the PC emulation mode kernel file given how this is
 * registered */
static tivx_target_kernel vx_vpac_nf_bilateral_target_kernel[2] = {NULL};

static vx_status VX_CALLBACK tivxVpacNfBilateralProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfBilateralCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfBilateralDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static void tivxVpacNfBilateralFreeMem(tivxVpacNfBilateralParams *prms);
static void generate_LUT(
    uint8_t subRangeBits,
    double *sigma_s,
    double *sigma_r,
    uint16_t *i_lut,
    int16_t *mmrCenterPixelWeight
);
static uint32_t bilateral_coefsLUTGen(
    uint8_t mode,
    uint8_t inp_bitw,
    uint8_t filtSize,
    double sigma_s,
    double sigma_r,
    double *f_wt_lut,
    uint8_t out_bitw,
    uint16_t *i_wt_lut_spatial,
    uint16_t *i_wt_lut_full,
    uint16_t *mmrCenterPixelWeight);
static void interleaveTables(uint16_t **i_lut, uint8_t numTables, uint32_t rangeLutEntries);
static int getSubRangeBits(int i);

#ifndef VLAB_HWA

/* Static hardware configuration from J7 */
static bilateralFilter_algParams_t    algParams[] =
{
    {
        8,          /* rangeLutQuantSz */
        8,          /* lutValueBitWidth */
        24,         /* recipValueBitWidth */
        0,          /* interpolation */
        4,          /* recipLutQuantSz */
        512         /* recipLutQuantCutoff */
    }
};

#endif

static vx_status VX_CALLBACK tivxVpacNfBilateralProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    tivxVpacNfBilateralParams *prms = NULL;
    tivx_obj_desc_image_t *input_desc;
    tivx_obj_desc_user_data_object_t *sigmas_desc;
    tivx_obj_desc_user_data_object_t *configuration_desc;
    tivx_obj_desc_image_t *output_desc;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        void *configuration_target_ptr;
        void *input_target_ptr;
        void *sigmas_target_ptr;
        void *output_target_ptr;

        configuration_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_CONFIGURATION_IDX];
        input_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_INPUT_IDX];
        sigmas_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_SIGMAS_IDX];
        output_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxVpacNfBilateralParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            configuration_target_ptr = tivxMemShared2TargetPtr(&configuration_desc->mem_ptr);
            input_target_ptr = tivxMemShared2TargetPtr(&input_desc->mem_ptr[0]);
            sigmas_target_ptr = tivxMemShared2TargetPtr(&sigmas_desc->mem_ptr);
            output_target_ptr = tivxMemShared2TargetPtr(&output_desc->mem_ptr[0]);

            tivxCheckStatus(&status, tivxMemBufferMap(configuration_target_ptr,
               configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferMap(input_target_ptr,
               input_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferMap(sigmas_target_ptr,
               sigmas_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferMap(output_target_ptr,
               output_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));

            /* C-model supports only 12-bit in uint16_t container
             * So we may need to translate.  In HW, NF_LSE does this
             */
            lse_reformat_in(input_desc, input_target_ptr, prms->src16, 0, 0);

#ifdef VLAB_HWA

            status = vlab_hwa_process(VPAC_NF_BASE_ADDRESS, "VPAC_NF_BILATERAL", sizeof(bilateralFilter_mmr_t), &prms->mmr);

#else
            status = bilateral_hw(&algParams[0], &prms->mmr, &prms->debug);
            if (0 != status)
            {
                status = (vx_status)VX_FAILURE;
            }
#endif

        }
        if ((vx_status)VX_SUCCESS == status)
        {

            lse_reformat_out(input_desc, output_desc, output_target_ptr, prms->dst16, 12, 0);

            tivxCheckStatus(&status, tivxMemBufferUnmap(configuration_target_ptr,
               configuration_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferUnmap(input_target_ptr,
               input_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferUnmap(sigmas_target_ptr,
               sigmas_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_READ_ONLY));
            tivxCheckStatus(&status, tivxMemBufferUnmap(output_target_ptr,
               output_desc->mem_size[0], (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfBilateralCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        tivx_obj_desc_image_t *src;
        tivxVpacNfBilateralParams *prms = NULL;

        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_VPAC_NF_BILATERAL_INPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxVpacNfBilateralParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxVpacNfBilateralParams));

            prms->buffer_size = src->imagepatch_addr[0].dim_x *
                                src->imagepatch_addr[0].dim_y * 2;

            prms->src16 = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            if (NULL == prms->src16)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->dst16 = tivxMemAlloc(prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
                if (NULL == prms->dst16)
                {
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                tivx_vpac_nf_bilateral_params_t *params;
                tivx_vpac_nf_bilateral_sigmas_t *sigmas;

                tivx_obj_desc_user_data_object_t *params_array;
                tivx_obj_desc_user_data_object_t *sigmas_array;

                void *params_array_target_ptr;
                void *sigmas_array_target_ptr;

                params_array = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_CONFIGURATION_IDX];
                sigmas_array = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_SIGMAS_IDX];

                params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);
                sigmas_array_target_ptr = tivxMemShared2TargetPtr(&sigmas_array->mem_ptr);

                tivxCheckStatus(&status, tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
                tivxCheckStatus(&status, tivxMemBufferMap(sigmas_array_target_ptr, sigmas_array->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                params = (tivx_vpac_nf_bilateral_params_t *)params_array_target_ptr;
                sigmas = (tivx_vpac_nf_bilateral_sigmas_t *)sigmas_array_target_ptr;

                prms->mmr.filterMode = FILTERMODE_BILATERAL;
                prms->mmr.width = src->imagepatch_addr[0].dim_x;
                prms->mmr.height = src->imagepatch_addr[0].dim_y;
                prms->mmr.input = prms->src16;
                prms->mmr.output = prms->dst16;

                prms->mmr.inputPlaneInterleaving = params->params.input_interleaved;
                prms->mmr.shift = params->params.output_downshift;
                prms->mmr.offset = params->params.output_offset;
                prms->mmr.subRangeBits = getSubRangeBits(sigmas->num_sigmas);
                prms->mmr.adaptiveMode = params->adaptive_mode;
                prms->mmr.subTableSelect = params->sub_table_select;
                prms->mmr.outputPixelSkip = params->params.output_pixel_skip;
                prms->mmr.outputPixelSkipOdd = params->params.output_pixel_skip_odd;
                prms->mmr.kern_ln_offset = params->params.kern_ln_offset;
                prms->mmr.kern_sz_height = params->params.kern_sz_height;
                prms->mmr.src_ln_inc_2 = params->params.src_ln_inc_2;
                prms->mmr.lut = prms->lut;

                prms->debug.fileTapName = NULL;
                prms->debug.tapMask = 0;
                prms->debug.recipFileName = NULL;
                prms->debug.replicateH_off = 0;
                prms->debug.replicateV_off = 0;

                generate_LUT(prms->mmr.subRangeBits, sigmas->sigma_space, sigmas->sigma_range,
                             (uint16_t*)&prms->lut, &prms->mmr.centerPixelWeight);

                tivxCheckStatus(&status, tivxMemBufferUnmap(params_array_target_ptr, params_array->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxVpacNfBilateralParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxVpacNfBilateralFreeMem(prms);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfBilateralDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        uint32_t size;
        tivxVpacNfBilateralParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxVpacNfBilateralParams) == size))
        {
            tivxVpacNfBilateralFreeMem(prms);
        }
    }

    return status;
}

void tivxAddTargetKernelVpacNfBilateral(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC_NF, TIVX_TARGET_MAX_NAME);
        vx_vpac_nf_bilateral_target_kernel[0] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_NF_BILATERAL_NAME,
                            target_name,
                            tivxVpacNfBilateralProcess,
                            tivxVpacNfBilateralCreate,
                            tivxVpacNfBilateralDelete,
                            NULL,
                            NULL);
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_NF, TIVX_TARGET_MAX_NAME);
        vx_vpac_nf_bilateral_target_kernel[1] = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_NF_BILATERAL_NAME,
                            target_name,
                            tivxVpacNfBilateralProcess,
                            tivxVpacNfBilateralCreate,
                            tivxVpacNfBilateralDelete,
                            NULL,
                            NULL);
    }
    #endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
    }
}

void tivxRemoveTargetKernelVpacNfBilateral(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
    {
        status = tivxRemoveTargetKernel(vx_vpac_nf_bilateral_target_kernel[0]);
        if (status == (vx_status)VX_SUCCESS)
        {
            vx_vpac_nf_bilateral_target_kernel[0] = NULL;
        }
    }
    #if defined(SOC_J784S4)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
    {
        status = tivxRemoveTargetKernel(vx_vpac_nf_bilateral_target_kernel[1]);
        if (status == (vx_status)VX_SUCCESS)
        {
            vx_vpac_nf_bilateral_target_kernel[1] = NULL;
        }
    }
    #endif
}

static void tivxVpacNfBilateralFreeMem(tivxVpacNfBilateralParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->src16)
        {
            tivxMemFree(prms->src16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->src16 = NULL;
        }
        if (NULL != prms->dst16)
        {
            tivxMemFree(prms->dst16, prms->buffer_size, (vx_enum)TIVX_MEM_EXTERNAL);
            prms->dst16 = NULL;
        }

        tivxMemFree(prms, sizeof(tivxVpacNfBilateralParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}


/*---------------------------------------------------------------------*/
/* Generate LUT                                                        */
/*---------------------------------------------------------------------*/
static void generate_LUT
(
    uint8_t subRangeBits,
    double *sigma_s,
    double *sigma_r,
    uint16_t *i_lut,
    int16_t *mmrCenterPixelWeight
)
{
    uint32_t numTables = 1 << subRangeBits;
    uint8_t tableNum;
    uint32_t rangeLutEntries = 256 >> subRangeBits;
    double   f_lut[LUT_ROWS * 256];

    for (tableNum = 0; tableNum < numTables; tableNum++)
    {
        double s_sigma = sigma_s[tableNum];
        double r_sigma = sigma_r[tableNum];

        /*-----------------------------------------------------------------*/
        /* Generate fixed point LUT values, with index to LUT being        */
        /* pixel differences.                                              */
        /*-----------------------------------------------------------------*/
        bilateral_coefsLUTGen
            (
            0,
            8 - subRangeBits,
            5,
            s_sigma,
            r_sigma,
            f_lut,
            8,
            NULL,
            &i_lut[tableNum * LUT_ROWS * rangeLutEntries],
            (uint16_t*)mmrCenterPixelWeight
            );
    }

    if (numTables > 1)
    {
        interleaveTables(&i_lut, numTables, rangeLutEntries);
    }
}

static uint32_t bilateral_coefsLUTGen
(
    uint8_t mode,
    uint8_t inp_bitw,
    uint8_t filtSize,
    double sigma_s,
    double sigma_r,
    double *f_wt_lut,
    uint8_t out_bitw,
    uint16_t *i_wt_lut_spatial,
    uint16_t *i_wt_lut_full,
    uint16_t *mmrCenterPixelWeight
)
{
    int row, col;
    int lut_w, lut_h;
    int lutSize = 1 << inp_bitw;
    int referenceSize = 12;
    uint8_t numspatialDistances;

    double wt_s;
    double wt_r;
    double wt_sum;
    double max = 0;

    /* Translate filter size (5x5, 3x3, 1x1) into spatialDistances */
    if (filtSize == 3)
    {
        numspatialDistances = 2;
    }
    else if (filtSize == 1)
    {
        numspatialDistances = 0;
    }
    else
    {
        numspatialDistances = 5;
    }

    /*---------------------------------------------------------------------*/
    /* Compute LUT width and height.                                       */
    /*---------------------------------------------------------------------*/
    memset(f_wt_lut, 0, LUT_ROWS * lutSize*sizeof(double));

    lut_w = (1 << inp_bitw);
    lut_h = (numspatialDistances);

    /*---------------------------------------------------------------------*/
    /* Actual doubleing pt. LUT creation for Bilateral filter.              */
    /*---------------------------------------------------------------------*/

    /* If the space sigma is 0, then table should remain all zeros */
    if (sigma_s)
    {
        /* Index of the space distance from the center pixel, 0-4 */
        const uint8_t spaceWeightIndex[25] = {  4, 3, 2, 3, 4,
                                                3, 1, 0, 1, 3,
                                                2, 0, 0, 0, 2,
                                                3, 1, 0, 1, 3,
                                                4, 3, 2, 3, 4  };

        /* This distance-squared mapping of each index (above) in a 5x5 to the center pixel */
        const double distanceValuesSquared[] =
        {
            1.0, // 0: 1^2 + 0^2
            2.0, // 1: 1^2 + 1^2
            4.0, // 2: 2^2 + 0^2
            5.0, // 3: 2^2 + 1^2
            8.0  // 4: 2^2 + 2^2
        };

        /* If the range sigma is 0, bilateral doesn't make sense, so instead generic mode weights should be generated */
        if (sigma_r == 0)
        {
            /* Generate generic mode weights using gaussian curve from sigma_s */
            for (row = 0; row < 25; row++)
            {
                if (spaceWeightIndex[row] < numspatialDistances)
                {   // gaussian_s = exp(-(x^2 / (2*sigma_s^2))
                    wt_s = distanceValuesSquared[spaceWeightIndex[row]] / (2 * sigma_s * sigma_s);
                    f_wt_lut[row] = exp(-wt_s);
                }
            }
            for (row = 13; row < 25; row++)
            {
                if (spaceWeightIndex[row] < numspatialDistances)
                {   // gaussian_s = exp(-(x^2 / (2*sigma_s^2))
                    wt_s = distanceValuesSquared[spaceWeightIndex[row]] / (2 * sigma_s * sigma_s);
                    f_wt_lut[row-1] = exp(-wt_s);
                }
            }
            /* Overwrite center pixel weight to be 1 */
            max = 1.0;
            f_wt_lut[12] = 1.0;
        }
        else {
            if (mode == 0) /* Bilateral Filter Weights */
            {
                for (col = 0; col < lut_w; col++)
                {
                    // Only generate the lut values across the range that are needed
                    int col_mod = col*(1 << (referenceSize - inp_bitw));
                    // gaussian_r = exp(-(x^2 / (2*sigma_r^2))
                    wt_r = (col_mod * col_mod) / (2 * sigma_r * sigma_r);

                    for (row = 0; row < lut_h; row++)
                    {
                        // gaussian_s = exp(-(x^2 / (2*sigma_s^2))
                        wt_s = distanceValuesSquared[row] / (2 * sigma_s * sigma_s);
                        wt_sum = wt_s + wt_r;
                        f_wt_lut[(row * lut_w) + col] = exp(-wt_sum);
                        //printf("%f\n", f_wt_lut[(row * lut_w) + col]);
                    }
                }
                max = 1.0;
            }
            else if ((mode == 1) || (mode == 2)) /* Passthrough Weights */
            {
                f_wt_lut[0] = 1.0;
                max = 1.0;
            }
            else if (mode == 3) /* Highest Value Weights */
            {
                for (col = 0; col < lut_w; col++)
                {
                    for (row = 0; row < lut_h; row++)
                    {
                        f_wt_lut[(row * lut_w) + col] = 1.0;
                    }
                }
                max = 1.0;
            }
            else
            {
                return 1;
            }
        }
        *mmrCenterPixelWeight = (uint16_t)max;
    }

    /*---------------------------------------------------------------------*/
    /* Fixed point LUT creation here.                                      */
    /*---------------------------------------------------------------------*/
    if (i_wt_lut_full){
        int i;
        memset(i_wt_lut_full, 0, LUT_ROWS * lutSize*sizeof(uint16_t));
        if (mode == 2) {
            *mmrCenterPixelWeight = 1;
        }
        else {
            int32_t one_lsl_out_bitw = (1 << out_bitw);
            double temp0 = ((max * (double)one_lsl_out_bitw) - 1.0f);
            for (i = 0; i < (LUT_ROWS * lutSize); i++) {
                int32_t one_lsl_out_bitw_minus_one = (one_lsl_out_bitw - 1);
                double temp1 = (((double)(f_wt_lut[i] / max) * (double)one_lsl_out_bitw_minus_one) + 0.5f);
                i_wt_lut_full[i] = (uint16_t)temp1;
            }
            *mmrCenterPixelWeight = (uint16_t)temp0;
        }
    }

    /* In case spatial lut needs to be generated separatly */
    if (i_wt_lut_spatial){
        memset(i_wt_lut_spatial, 0, LUT_ROWS * sizeof(i_wt_lut_spatial[0]));
        for (row = 0; row < lut_h; row++)
        {
            i_wt_lut_spatial[row] = i_wt_lut_full[row * lut_w];
        }
    }
    return 0;
}

static void interleaveTables(uint16_t **i_lut, uint8_t numTables, uint32_t rangeLutEntries)
{
    uint16_t *oldLut = *i_lut;
    uint16_t newLut[LUT_ROWS*256];
    uint32_t i, j;

    for (j = 0; j < numTables; j++)
    {
        for (i = 0; i < (LUT_ROWS * rangeLutEntries); i++)
        {
            newLut[(numTables*i) + j] = oldLut[((j * LUT_ROWS) * rangeLutEntries) + i];
        }
    }

    memcpy(oldLut, newLut, LUT_ROWS*256*sizeof(uint16_t));
}

static int getSubRangeBits(int i)
{
    int out = 0;
    int j = i >> 1;
    while(j != 0)
    {
        out++;
        j = j >> 1;
    }
    return out;
}
