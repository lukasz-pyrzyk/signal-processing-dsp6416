#include "DSK6416_AIC23.h"	        //codec support
Uint32 fs=DSK6416_AIC23_FREQ_8KHZ;	//set sampling rate
#define DSK6416_AIC23_INPUT_MIC 0x0015
#define DSK6416_AIC23_INPUT_LINE 0x0011
Uint16 inputsource=DSK6416_AIC23_INPUT_LINE; // select input
#define BUFSIZE 512

#define MAX_VALUE 6969
#define MIN_VALUE -6969

int buffer[BUFSIZE];
int buf_ptr = 0;
int max_Value = 0;
int min_Value = 0;

interrupt void c_int11()
{
  int data_left, data_right;
  float percentage_left = 0.0f;
  
  data_left = input_left_sample();
  data_right = input_right_sample();

	percentage_left = (float)(data_left - MIN_VALUE) / (float) (MAX_VALUE - MIN_VALUE);

	if(percentage_left > 0.75f)
	{
	// zapal 4

	DSK6416_LED_toggle(3);
	}
	else if(percentage_left > 0.5f)
	{
	// zapal 3

	DSK6416_LED_toggle(2);
	}
	else if(percentage_left > 0.25f)
	{
	// zapal 2

	DSK6416_LED_toggle(1);
	}
	else if(percentage_left > 0.0f)
	{
	// zapal 1

	DSK6416_LED_toggle(0);
	}


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
  DSK6416_LED_init();
  	comm_intr();
	while(1);
}
