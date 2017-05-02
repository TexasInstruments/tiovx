/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_tutorial.h>

static char menu[] = {
    "\n"
    "\n ================"
    "\n Tutorial : Image"
    "\n ================"
    "\n"
    "\n 1: Load and save images"
    "\n 2: Query images and print image attributes"
    "\n 3: Advanced image create APIs"
    "\n 4: Image manipulation using VXU APIs"
    "\n 5: Image manipulation using graph and VX node APIs"
    "\n 6: Image manipulation using graph and generic node create APIs"
    "\n"
    "\n a: Run All"
    "\n"
    "\n x: Exit Menu"
    "\n"
    "\n Enter Choice: "
};

void vx_tutorial_image_run_all()
{
    vx_tutorial_image_load_save();
    vx_tutorial_image_query();
    vx_tutorial_image_crop_roi();
    vx_tutorial_image_extract_channel();
    vx_tutorial_image_color_convert();
    vx_tutorial_image_histogram();
}

void vx_tutorial_image_run_interactive()
{
    char ch;
    vx_bool done = vx_false_e;

    while(!done)
    {
        printf(menu);
        ch = vx_tutorial_get_char();
        printf("\n");

        switch(ch)
        {
            case '1':
                vx_tutorial_image_load_save();
                break;
            case '2':
                vx_tutorial_image_query();
                break;
            case '3':
                vx_tutorial_image_crop_roi();
                break;
            case '4':
                vx_tutorial_image_extract_channel();
                break;
            case '5':
                vx_tutorial_image_color_convert();
                break;
            case '6':
                vx_tutorial_image_histogram();
                break;
            case 'a':
                vx_tutorial_image_run_all();
                break;
            case 'x':
                done = vx_true_e;
                break;
            case '\n':
                break;
            default:
                printf("\n Invalid option !!!");
                break;
        }
    }
}
