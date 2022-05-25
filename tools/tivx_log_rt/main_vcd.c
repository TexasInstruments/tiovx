
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <tivx_log_rt_if.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (((byte & 0x08U) != 0U) ? '1' : '0'), \
  (((byte & 0x04U) != 0U) ? '1' : '0'), \
  (((byte & 0x02U) != 0U) ? '1' : '0'), \
  (((byte & 0x01U) != 0U) ? '1' : '0')

#define FILENAME_MAX_LENGTH   (256u)

typedef struct {
    char in_file_name[FILENAME_MAX_LENGTH];
    char out_file_name[FILENAME_MAX_LENGTH];
    uint32_t start_offset;
    uint32_t duration;

} tivx_log_rt_args;

const char *argp_program_version = "v1.0.0 " __DATE__ " " __TIME__;
const char *argp_program_bug_address = "e2e.ti.com";

static char cmd_doc[] = "Utility tool to convert .bin generated via tivxLogRtTraceExportToFile() on EVM to a VCD (Value Change Dump) format.";
static char args_doc[] = "";

static struct argp_option options[] = {
    { "input", 'i', "file", 0, "Input file generated via tivxLogRtTraceExportToFile()."},
    { "output_vcd", 'o', "file", 0, "VCD (Value Change Dump) Output file. Use 'gtkwave' to visualize the output."},
    { "start", 's', "time in msecs", 0, "Start offset for clip in units of ms"},
    { "duration", 'd', "time in msecs", 0, "Duration for clip in units of ms"},
    { 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    tivx_log_rt_args *arguments = state->input;

    switch (key)
    {
        case 'i': strncpy(arguments->in_file_name, arg, FILENAME_MAX_LENGTH); break;
        case 'o': strncpy(arguments->out_file_name, arg, FILENAME_MAX_LENGTH); break;
        case 's':
            arguments->start_offset = atoi(arg);
            break;
        case 'd':
            arguments->duration = atoi(arg);
            break;
        case ARGP_KEY_ARG: return 0;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, cmd_doc, 0, 0, 0 };

typedef struct
{
    tivx_log_rt_args arguments;
    tivx_log_rt_index_t index[TIVX_LOG_RT_INDEX_MAX];

} tivx_log_rt_obj_t;

tivx_log_rt_obj_t g_tivx_log_rt_obj;

static void set_default(tivx_log_rt_args *arguments)
{
    strncpy(arguments->in_file_name, "in.bin", FILENAME_MAX_LENGTH);
    strncpy(arguments->out_file_name, "out.vcd", FILENAME_MAX_LENGTH);
    arguments->start_offset = 0;
    arguments->duration = 0xFFFFFFFF;
}

int main(int argc, char *argv[])
{
    tivx_log_rt_obj_t *obj = &g_tivx_log_rt_obj;
    size_t bytes_read;
    uint32_t i, event_cnt;
    tivx_log_rt_entry_t event;
    uint64_t start_time, global_start_time;

    set_default(&obj->arguments);
    argp_parse(&argp, argc, argv, 0, 0, &obj->arguments);

    FILE *out_fp = fopen(obj->arguments.out_file_name, "w");
    int in_fd  = open(obj->arguments.in_file_name, O_RDONLY);

    if(in_fd < 0)
    {
        printf("ERROR: Input file [%s] NOT found !!!\n", obj->arguments.in_file_name);
    }
    if(out_fp==NULL)
    {
        printf("ERROR: Output file [%s] could not be created !!!\n", obj->arguments.out_file_name);
    }

    fprintf(out_fp, "$timescale\n");
    fprintf(out_fp, "1 us\n");
    fprintf(out_fp, "$end\n");

    /* read index and define variables */
    bytes_read = read(in_fd, &obj->index[0], TIVX_LOG_RT_INDEX_MAX*sizeof(tivx_log_rt_index_t));
    if(bytes_read!=TIVX_LOG_RT_INDEX_MAX*sizeof(tivx_log_rt_index_t))
    {
        printf("ERROR: Unable to read index information from input file [%s]\n", obj->arguments.in_file_name);
        exit(0);
    }

    for(i=0; i<TIVX_LOG_RT_INDEX_MAX; i++)
    {
        tivx_log_rt_index_t *index;

        index = &obj->index[i];

        switch(index->event_class)
        {
            case TIVX_LOG_RT_EVENT_CLASS_KERNEL_INSTANCE:
                fprintf(out_fp, "$var reg 1 k_%" PRIuPTR " %s $end\n",
                    (uintptr_t)index->event_id,
                    index->event_name);
                break;
            case TIVX_LOG_RT_EVENT_CLASS_NODE:
                fprintf(out_fp, "$var reg 4 n_%" PRIuPTR " %s $end\n",
                    (uintptr_t)index->event_id,
                    index->event_name);
                break;
            #if 0
            case TIVX_LOG_RT_EVENT_CLASS_GRAPH:
                fprintf(out_fp, "$var reg 4 g_%" PRIuPTR " %s $end\n",
                    (uintptr_t)index->event_id,
                    index->event_name);
                break;
            #endif
            case TIVX_LOG_RT_EVENT_CLASS_TARGET:
                fprintf(out_fp, "$var reg 1 t_%" PRIuPTR " %s $end\n",
                    (uintptr_t)index->event_id,
                    index->event_name);
                break;
        }
    }
    fprintf(out_fp, "$enddefinitions $end\n");

    i = 0;
    event_cnt = 0;
    /* read events */
    do {
        bytes_read = read(in_fd, &event, sizeof(tivx_log_rt_entry_t));

        if(bytes_read != sizeof(tivx_log_rt_entry_t))
        {
            /* done, exit */
            break;
        }

        if(i==0)
        {
            global_start_time = event.timestamp;
        }
        i++;

        if(event.timestamp >= (global_start_time + (uint64_t)obj->arguments.start_offset*1000)
            && event.timestamp <= (global_start_time + (uint64_t)(obj->arguments.start_offset + obj->arguments.duration)*1000)
            )
        {
            if(event_cnt  == 0)
            {
                start_time = event.timestamp;
            }
            event_cnt++;

            switch(event.event_class)
            {
                case TIVX_LOG_RT_EVENT_CLASS_KERNEL_INSTANCE:

                    if(event.event_type==TIVX_LOG_RT_EVENT_TYPE_START)
                    {
                        fprintf(out_fp, "#%" PRIu64 "\n" "b1 k_%" PRIuPTR "\n",
                            event.timestamp  - start_time,
                            (uintptr_t)event.event_id);
                    }
                    else
                    if(event.event_type==TIVX_LOG_RT_EVENT_TYPE_END)
                    {
                        fprintf(out_fp, "#%" PRIu64 "\n" "b0 k_%" PRIuPTR "\n",
                            event.timestamp  - start_time,
                            (uintptr_t)event.event_id);
                    }
                    break;
                case TIVX_LOG_RT_EVENT_CLASS_NODE:

                    if(event.event_type==TIVX_LOG_RT_EVENT_TYPE_START)
                    {
                        fprintf(out_fp, "#%" PRIu64 "\n" "b"BYTE_TO_BINARY_PATTERN" n_%" PRIuPTR "\n",
                            event.timestamp - start_time,
                            BYTE_TO_BINARY(event.event_value),
                            (uintptr_t)event.event_id);
                    }
                    else
                    if(event.event_type==TIVX_LOG_RT_EVENT_TYPE_END)
                    {
                        fprintf(out_fp, "#%" PRIu64 "\n" "bZ n_%" PRIuPTR "\n",
                            event.timestamp - start_time,
                            (uintptr_t)event.event_id);
                    }
                    break;
                #if 0 /* not supported as of now */
                case TIVX_LOG_RT_EVENT_CLASS_GRAPH:
                    if(event.event_type==TIVX_LOG_RT_EVENT_TYPE_START)
                    {
                        fprintf(out_fp, "#%" PRIu64 "\n" "b"BYTE_TO_BINARY_PATTERN" g_%" PRIuPTR "\n",
                            event.timestamp  - start_time,
                            BYTE_TO_BINARY(event.event_value),
                            (uintptr_t)event.event_id);
                    }
                    else
                    if(event.event_type==TIVX_LOG_RT_EVENT_TYPE_END)
                    {
                        fprintf(out_fp, "#%" PRIu64 "\n" "bZ g_%" PRIuPTR "\n",
                            event.timestamp  - start_time,
                            (uintptr_t)event.event_id);
                    }
                    break;
                #endif
                case TIVX_LOG_RT_EVENT_CLASS_TARGET:
                    if(event.event_type==TIVX_LOG_RT_EVENT_TYPE_START)
                    {
                        fprintf(out_fp, "#%" PRIu64 "\n" "b1 t_%" PRIuPTR "\n",
                            event.timestamp  - start_time,
                            (uintptr_t)event.event_id);
                    }
                    else
                    if(event.event_type==TIVX_LOG_RT_EVENT_TYPE_END)
                    {
                        fprintf(out_fp, "#%" PRIu64 "\n" "b0 t_%" PRIuPTR "\n",
                            event.timestamp  - start_time,
                            (uintptr_t)event.event_id);
                    }
                    break;
            }
        }

    } while(1);

    if(in_fd>0)
        close(in_fd);
    if(out_fp!=NULL)
        fclose(out_fp);

    return 0;
}
