/*
*
* Copyright (c) 2020 Texas Instruments Incorporated
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
#include <TI/tivx.h>
#include <tivx_utils_tidl_trace.h>
#include <tivx_tidl_trace.h>
#include <stdio.h>

vx_status tivx_utils_tidl_trace_write(vx_user_data_object traceData, char *prefix)
{
    vx_status status = (vx_status)VX_SUCCESS;

    void *trace_buffer = NULL;
    vx_map_id map_id;
    vx_size capacity;

    status = vxGetStatus((vx_reference)traceData);

    if((vx_status)VX_SUCCESS == status)
    {
        status = vxQueryUserDataObject(traceData, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &capacity, sizeof(capacity));

        if((vx_status)VX_SUCCESS == status)
        {
            status = vxMapUserDataObject(traceData, 0, capacity, &map_id,
                (void **)&trace_buffer, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            tivxTIDLTraceDataManager mgr;
            tivxTIDLTraceHeader *header;
            uint64_t offset;

            tivxTIDLTraceDataInit(&mgr, (uint8_t *)trace_buffer, (uint64_t)capacity);

            offset = 0;
            header = (tivxTIDLTraceHeader *)tivxTIDLTraceGetData(&mgr, offset, (uint64_t)sizeof(tivxTIDLTraceHeader));

            if (header == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR,"TIDL Trace Buffer Capacity too small for header!\n");
                status = (vx_status)VX_FAILURE;
            }
            else if ((header->size == 0) || (header->offset == 0))
            {
                VX_PRINT(VX_ZONE_ERROR,"TIDL Trace Buffer empty\n");
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                offset += sizeof(tivxTIDLTraceHeader);
                while(strncmp(header->fileName, "EOB", 3) != 0)
                {
                    FILE *fp;
                    uint8_t *data_ptr;
                    char file_name[TIVX_TIDL_TRACE_FILE_NAME_SIZE+1];

                    data_ptr = tivxTIDLTraceGetData(&mgr, header->offset, header->size);
                    offset += header->size;

                    snprintf(file_name, TIVX_TIDL_TRACE_FILE_NAME_SIZE, "%s%s", prefix, header->fileName);

                    fp = fopen(file_name, "wb");

                    if(fp == NULL)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Unable to open file %s !\n", file_name);
                        status = (vx_status)VX_FAILURE;
                        break;
                    }
                    VX_PRINT(VX_ZONE_INFO,"Writing %s of size %d bytes... ", file_name, header->size);

                    fwrite(data_ptr, header->size, sizeof(uint8_t), fp);
                    fflush(fp);
                    fclose(fp);

                    VX_PRINT(VX_ZONE_INFO, "Done! \n");

                    header = (tivxTIDLTraceHeader *)tivxTIDLTraceGetData(&mgr, offset, sizeof(tivxTIDLTraceHeader));
                    offset += sizeof(tivxTIDLTraceHeader);
                }
            }

            vxUnmapUserDataObject(traceData, map_id);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"Unable to map trace_buffer!\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return(status);
}
