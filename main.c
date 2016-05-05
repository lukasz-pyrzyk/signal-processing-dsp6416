#include "DSK6416_AIC23.h"	        //codec support
Uint32 fs=DSK6416_AIC23_FREQ_8KHZ;	//set sampling rate
#define DSK6416_AIC23_INPUT_MIC 0x0015
#define DSK6416_AIC23_INPUT_LINE 0x0011
Uint16 inputsource=DSK6416_AIC23_INPUT_LINE; // select input
#define BUFSIZE 512

int buffer[BUFSIZE];
int buf_ptr = 0;
int max_Value = 0;
int min_Value = 0;

interrupt void c_int11()
{
  int data_left, data_right;
  
  data_left = input_left_sample();
  data_right = input_right_sample();

	if(data_left > max_Value)
	{
		max_Value = data_left;
	}

	if(data_left < min_Value)
	{
		min_Value = data_left;
	}

  output_left_sample(data_left);
  output_right_sample(data_right);
  return;
}

void main()
{
  	comm_intr();
	while(1);
}
