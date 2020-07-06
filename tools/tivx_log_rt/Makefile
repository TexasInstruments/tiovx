

TIOVX_PATH=$(abspath ../..)

EXE_FILE_1=tivx_log_rt_2_vcd.out
EXE_FILE_2=tivx_log_rt_2_html.out

all: $(EXE_FILE_1) $(EXE_FILE_2)
clean:
	rm *.out

$(EXE_FILE_1): main_vcd.c $(TIOVX_PATH)/source/include/tivx_log_rt_if.h
	gcc -Wall -I$(TIOVX_PATH)/source/include -o $(EXE_FILE_1) main_vcd.c

$(EXE_FILE_2): main_html.c $(TIOVX_PATH)/source/include/tivx_log_rt_if.h
	gcc -Wall -I$(TIOVX_PATH)/source/include -o $(EXE_FILE_2) main_html.c -lm
