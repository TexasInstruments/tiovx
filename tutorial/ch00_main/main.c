/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */
#include <TI/tivx.h>
#include <vx_tutorial.h>

static void show_usage(int argc, char* argv[])
{
    printf(" \n");
    printf(" OpenVX Tutorials, \n");
    printf(" (c) Texas Instruments 2017 - www.ti.com\n");
    printf(" \n");
    printf(" Usage: %s [options] \n", argv[0]);
    printf(" \n");
    printf(" Options:\n");
    printf("  -h, --help             Show this message\n");
    printf("  -i, --run_interactive  Run tutorials using console based interactive UI\n");
    printf("  --run_all              Run all tutorials\n");
    printf(" \n");
}

int main(int argc, char* argv[])
{
    tivxInit();

    if(argc <= 1)
    {
        show_usage(argc, argv);
        vx_tutorial_run_interactive();
    }
    else
    {
        uint32_t i;

        for(i=1; i < argc; i++)
        {
            if(strcmp(argv[i], "--run_all")==0)
            {
                vx_tutorial_run_all();
                break;
            }
            if(strcmp(argv[i], "--run_interactive")==0
                ||
               strcmp(argv[i], "-i")==0
                )
            {
                vx_tutorial_run_interactive();
                break;
            }
            if(strcmp(argv[i], "--help")==0
                ||
               strcmp(argv[i], "-h")==0
                )
            {
                show_usage(argc, argv);
                break;
            }
        }
    }
    tivxDeInit();
}

char vx_tutorial_get_char()
{
    return getchar();
}
