
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

typedef struct {
    char in_file_name[256];
    char out_file_name[256];
} tivx_log_rt_args;

const char *argp_program_version = "v1.0.0 " __DATE__ " " __TIME__;
const char *argp_program_bug_address = "e2e.ti.com";

static char cmd_doc[] = "Utility tool to convert .bin generated via tivxLogRtTraceExportToFile() on EVM to a VCD (Value Change Dump) format.";
static char args_doc[] = "";

static struct argp_option options[] = { 
    { "input", 'i', "file", 0, "Input file generated via tivxLogRtTraceExportToFile()."},
    { "output_vcd", 'o', "file", 0, "VCD (Value Change Dump) Output file. Use 'gtkwave' to visualize the output."},
    { 0 } 
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    tivx_log_rt_args *arguments = state->input;
    
    switch (key) 
    {
        case 'i': strcpy(arguments->in_file_name, arg); break;
        case 'o': strcpy(arguments->out_file_name, arg); break;
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
    strcpy(arguments->in_file_name, "in.bin");
    strcpy(arguments->out_file_name, "out.vcd");
}

int main(int argc, char *argv[])
{
    tivx_log_rt_obj_t *obj = &g_tivx_log_rt_obj;
    size_t bytes_read;
    int32_t i;
    tivx_log_rt_entry_t event;
    uint64_t start_time;
    
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
            start_time = event.timestamp;
        }
        i++;

        switch(event.event_class)
        {
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
        
    } while(1);
        
    if(in_fd>0)
        close(in_fd);
    if(out_fp!=NULL)
        fclose(out_fp);

    return 0;
}
