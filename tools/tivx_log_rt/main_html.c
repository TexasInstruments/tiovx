
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
#include <math.h>

#include <tivx_log_rt_if.h>

typedef struct {
    char in_file_name[256];
    char out_file_name[256];
} tivx_log_rt_args;

typedef struct {
    
    uint32_t min;
    uint32_t max;
    uint32_t avg;
    uint32_t std_dev;
    
} tivx_log_rt_global_stats_t;

typedef struct 
{
    uint64_t start_time;
    uint32_t frame_period;
    
} tivx_log_rt_node_frame_period_t;

typedef struct 
{
    tivx_log_rt_args arguments;
    tivx_log_rt_index_t index[TIVX_LOG_RT_INDEX_MAX]; /* event index's which map event ID to event name */
    uint32_t num_events[TIVX_LOG_RT_INDEX_MAX]; /* number of events for each event index */
    tivx_log_rt_entry_t *events; /* list of events */
    uint32_t *event_2_index_table; /* table mappging each event in 'events' to event index within 'index' */
    uint32_t total_events; /* total number of events, i.e number of entries in 'events', 'event_2_index_table' */
    uint32_t num_index; /* total number of event index's, i.e number of valid entries in 'index', 'num_events' */
    
    uint64_t start_time; /* start time of the event log */
    uint64_t end_time; /* end time of event log */
    
    tivx_log_rt_node_frame_period_t *frame_period[TIVX_LOG_RT_INDEX_MAX]; /* array of frame period's of each node vs start time of each frame */
    
    tivx_log_rt_global_stats_t global_stats[TIVX_LOG_RT_INDEX_MAX]; /* global statistics of event ID's */
   
} tivx_log_rt_obj_t;


const char *argp_program_version = "v1.0.0 " __DATE__ " " __TIME__;
const char *argp_program_bug_address = "e2e.ti.com";

static char cmd_doc[] = "Utility tool to convert .bin generated via tivxLogRtTraceExportToFile() on EVM to a HTML format.";
static char args_doc[] = "";

static struct argp_option options[] = { 
    { "input", 'i', "file", 0, "Input file generated via tivxLogRtTraceExportToFile()."},
    { "output_html", 'o', "file", 0, "HTML file. Open in web browser to view"},
    { 0 } 
};

static void set_default(tivx_log_rt_args *arguments)
{
    strcpy(arguments->in_file_name, "in.bin");
    strcpy(arguments->out_file_name, "out.html");
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    tivx_log_rt_args *arguments = state->input;
    
    switch (key) 
    {
        case 'i': 
            strcpy(arguments->in_file_name, arg); 
            break;
        case 'o': 
            strcpy(arguments->out_file_name, arg); 
            break;
        case ARGP_KEY_ARG: 
            return 0;
        default: 
            return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, cmd_doc, 0, 0, 0 };

static uint32_t find_event_index(uint64_t event_id, tivx_log_rt_obj_t *obj)
{
    uint32_t i;
    
    for(i=0; i<obj->num_index; i++)
    {
        if(event_id == obj->index[i].event_id)
        {
            return i;
        }
    }
    return (uint32_t)-1;
}

static void create_time_series(tivx_log_rt_obj_t *obj)
{
    uint32_t i, k, num_events;
    uint64_t start, end;
    tivx_log_rt_node_frame_period_t *frame_period;
    
    for(i=0; i<obj->num_index; i++)
    {
        if(obj->num_events[i]==0)
            continue;
        
        obj->frame_period[i] = malloc(obj->num_events[i] * sizeof(tivx_log_rt_node_frame_period_t) );
                
        if(obj->frame_period[i]==NULL)
        {
            printf(" ERROR: Unable to allocate memory for time series\n");
            exit(0);
        }
        
        frame_period = obj->frame_period[i];
        
        num_events = 0;
        start = obj->start_time;
        end = obj->end_time;
        for(k=0; k<obj->total_events; k++)
        {
            tivx_log_rt_entry_t *event = &obj->events[k];
            uint32_t event_index = obj->event_2_index_table[k];
            
            if(event_index < obj->num_index && event_index == i)
            {
                if(event->event_type==TIVX_LOG_RT_EVENT_TYPE_START)
                {
                    start = event->timestamp - obj->start_time; 
                }
                else
                if(event->event_type==TIVX_LOG_RT_EVENT_TYPE_END)
                {
                    end = event->timestamp - obj->start_time; 
                    
                    if(num_events < obj->num_events[i])
                    {
                        
                        frame_period[num_events].start_time = start;
                        frame_period[num_events].frame_period = (uint32_t)(end - start);
                        
                        num_events++;
                    }
                   
                    start = end;        
                }
            }
        }
        obj->num_events[i] = num_events;
    }
    
    for(i=0; i<obj->num_index; i++)
    {
        uint32_t min, max;
        uint64_t sum;
        
        min = (uint32_t)-1;
        max = 0;
        sum = 0;
        frame_period = obj->frame_period[i];
        for(k=0; k<obj->num_events[i]; k++)
        {
            if(frame_period[k].frame_period <= min)
                min = frame_period[k].frame_period;
            if(frame_period[k].frame_period >= max)
                max = frame_period[k].frame_period;
            sum += frame_period[k].frame_period;    
        }
        obj->global_stats[i].min = min;
        obj->global_stats[i].max = max;
        obj->global_stats[i].avg = sum/obj->num_events[i];
        obj->global_stats[i].std_dev = 0;
        for(k=0; k<obj->num_events[i]; k++)
        {
            uint64_t diff;
            
            diff = (int32_t)frame_period[k].frame_period - (int32_t)obj->global_stats[i].avg;
            sum  += diff*diff;
        }
        sum = sum/obj->num_events[i];
        obj->global_stats[i].std_dev = sqrt(sum);        
    }
}

static void read_in_file(tivx_log_rt_obj_t *obj)
{
    int in_fd  = open(obj->arguments.in_file_name, O_RDONLY);
    struct stat st;
    size_t in_bytes_read, read_bytes, in_file_size;
    uint32_t cur_index, i;
    tivx_log_rt_index_t index[TIVX_LOG_RT_INDEX_MAX];
    
    if(in_fd < 0)
    {
        printf("ERROR: Input file [%s] NOT found !!!\n", obj->arguments.in_file_name);
        exit(0);
    }

    fstat(in_fd, &st);
    in_file_size = st.st_size;    
    
    read_bytes = sizeof(tivx_log_rt_index_t)*TIVX_LOG_RT_INDEX_MAX;
    
    if(in_file_size <= read_bytes)
    {
        printf("ERROR: Input file [%s] does not have enough bytes \n", obj->arguments.in_file_name);
        exit(0);
    }
    
    in_bytes_read = read(in_fd, &index[0], read_bytes);
    if(in_bytes_read != read_bytes)
    {
        printf("ERROR: Unable to read input file [%s]\n", obj->arguments.in_file_name);
        exit(0);
    }
    
    read_bytes = in_file_size - sizeof(tivx_log_rt_index_t)*TIVX_LOG_RT_INDEX_MAX;
    
    obj->events = malloc(read_bytes);
    if(obj->events==NULL)
    {
        printf(" ERROR: Unable to allocate event memory of size %ld bytes\n", read_bytes);
        exit(0);
    }
    
    in_bytes_read = read(in_fd, obj->events, read_bytes);
    if(in_bytes_read != read_bytes)
    {
        printf("ERROR: Unable to read input file [%s]\n", obj->arguments.in_file_name);
        exit(0);
    }

    obj->total_events = read_bytes / sizeof(tivx_log_rt_entry_t);    
    
    obj->event_2_index_table = malloc( obj->total_events * sizeof(uint32_t) );
    if(obj->event_2_index_table==NULL)
    {
        printf(" ERROR: Unable to allocate memory of size %ld bytes\n", obj->total_events * sizeof(uint32_t));
        exit(0);
    }
    
       
    /* count the valid event ID's and put in obj->index[] array */
    cur_index = 0;
    for(i=0; i<TIVX_LOG_RT_INDEX_MAX; i++)
    {
        switch(index[i].event_class)
        {
            /* we dont handle GRAPH class for now */
            case TIVX_LOG_RT_EVENT_CLASS_TARGET:
            case TIVX_LOG_RT_EVENT_CLASS_NODE:
                obj->index[cur_index] = index[i];
                cur_index++;
                break;
        }
    }
    obj->num_index = cur_index;

    for(i=0; i<TIVX_LOG_RT_INDEX_MAX; i++)
    {
        obj->num_events[i] = 0;
    }
    
    /* map event ID to event name via event_2_index_table */
    for(i=0; i<obj->total_events; i++)
    {
        tivx_log_rt_entry_t *event;
        uint32_t index;
        
        event = &obj->events[i];
        
        if(i==0)
        {
            obj->start_time = event->timestamp;
        }
        else
        if(i==(obj->total_events-1))
        {
            obj->end_time = event->timestamp;
        }
        index = find_event_index(event->event_id, obj);
        obj->event_2_index_table[i] = index;
        
        if(event->event_type == TIVX_LOG_RT_EVENT_TYPE_START && index < obj->num_index)  
        {
            obj->num_events[index]++;
        }
    }
    printf(" Input file, total duration = %ld usecs, %d events, %d event ID's\n",
        obj->end_time - obj->start_time, obj->total_events, obj->num_index
        );
}

static void create_output(tivx_log_rt_obj_t *obj)
{
    uint32_t i, event_index;
    tivx_log_rt_node_frame_period_t *frame_period;
    FILE *out_fp = fopen(obj->arguments.out_file_name, "w");
    
    const char header[] = 
        "<html>\n"
        "<head>\n"
        "  <script src=\"http://cdnjs.cloudflare.com/ajax/libs/dygraph/2.1.0/dygraph.min.js\"></script>\n"
        "  <link rel=\"stylesheet\" href=\"http://cdnjs.cloudflare.com/ajax/libs/dygraph/2.1.0/dygraph.min.css\" />\n"
        "</head>\n"
        "<body>\n"
        ;
        
        
    const char div_tags[] = 
        "  <div id=\"%s\"></div>\n"
        "  <p> <b>Global statistics : </b> min = %d, max = %d, avg = %d, std dev = %d, #frame = %d </p> \n"
        "  <hr> \n"
        ;
        
    const char script_header[] = 
        "  <script type=\"text/javascript\">"
        "\n"
        ;
        
    const char graph_header[] = 
        "   g%d = new Dygraph(\n"
        "           document.getElementById(\"%s\"),\n"    
        ;
        
    const char graph_options[] = 
        "          {\n"
        "               title: '%s',\n"
        "               legend: 'always',\n"
        "               showRangeSelector: true,\n"
        "               xlabel: 'Global time (usecs)',\n"
        "               ylabel: 'Frame time (usecs)',\n"
        "           }\n"
        "         );\n"
        "\n"
        ;

    const char footer[] = 
        "  </script>\n"
        "</body>\n"
        "</html>\n"
        "\n"
        ;
    
    if(out_fp==NULL)
    {
        printf("ERROR: Output file [%s] could not be created !!!\n", obj->arguments.out_file_name);
        exit(0);
    }

    fprintf(out_fp, header);
    for(event_index=0; event_index<obj->num_index; event_index++)
    {
        if(obj->index[event_index].event_class == TIVX_LOG_RT_EVENT_CLASS_NODE)
        {
            fprintf(out_fp, div_tags, 
                    obj->index[event_index].event_name,
                    obj->global_stats[event_index].min,
                    obj->global_stats[event_index].max,
                    obj->global_stats[event_index].avg,
                    obj->global_stats[event_index].std_dev,
                    obj->num_events[event_index]
                    );
        }
    }
    fprintf(out_fp, script_header);
    for(event_index=0; event_index<obj->num_index; event_index++)
    {   
        if(obj->index[event_index].event_class == TIVX_LOG_RT_EVENT_CLASS_NODE)
        { 
            fprintf(out_fp, graph_header, event_index, obj->index[event_index].event_name);
                
            frame_period  = obj->frame_period[event_index];
        
            fprintf(out_fp, "    \"Global Time, Frame Time\\n\" + \n");
            for(i=0;i<obj->num_events[event_index];i++)
            {
                if(i==obj->num_events[event_index]-1)
                {    
                    fprintf(out_fp, "    \"%ld, %d\\n\" , \n", frame_period[i].start_time, frame_period[i].frame_period);
                }
                else
                {
                    fprintf(out_fp, "    \"%ld, %d\\n\" + \n", frame_period[i].start_time, frame_period[i].frame_period);
                }
            }
        
            fprintf(out_fp, graph_options, obj->index[event_index].event_name);
        }
    }
    fprintf(out_fp, footer);
        
    if(out_fp!=NULL)
        fclose(out_fp);
    
}

int main(int argc, char *argv[])
{
    tivx_log_rt_obj_t tivx_log_rt_obj;
    tivx_log_rt_obj_t *obj = &tivx_log_rt_obj;
    
    set_default(&obj->arguments);
    argp_parse(&argp, argc, argv, 0, 0, &obj->arguments);
    read_in_file(obj);
    create_time_series(obj);
    create_output(obj);

    return 0;
}
