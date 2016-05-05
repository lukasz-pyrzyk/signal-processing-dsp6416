#include "DSK6416_AIC23.h"	        //codec support
Uint32 fs=DSK6416_AIC23_FREQ_8KHZ;	//set sampling rate
#define DSK6416_AIC23_INPUT_MIC 0x0015
#define DSK6416_AIC23_INPUT_LINE 0x0011
Uint16 inputsource=DSK6416_AIC23_INPUT_LINE; // select input

#define MAX_VALUE 6969
#define MIN_VALUE -6969
#define NOISE 1500

interrupt void c_int11()
{
 	int i = 0;
 	int data_left, data_right;
 	float percentage_left = 0.0f;
  
	data_left = input_left_sample();
	data_right = input_right_sample();

	if(data_left > NOISE)
	{
		percentage_left = (float)(data_left - NOISE) / (float) (MAX_VALUE - NOISE);

		if(percentage_left > 0.75f)
		{
			DSK6416_LED_on(3);
		}
		else
		{
			DSK6416_LED_off(3);
		}


		if(percentage_left > 0.5f)
		{
			DSK6416_LED_on(2);
		}
		else
		{
			DSK6416_LED_off(2);
		}


		if(percentage_left > 0.25f)
		{
			DSK6416_LED_on(1);
		}
		else
		{
			DSK6416_LED_off(1);
		}


		if(percentage_left > 0.0f)
		{
			DSK6416_LED_on(0);
		}
		else
		{
			DSK6416_LED_off(0);
		}

	}
	else
	{
		for(i = 0; i < 4; i++)
		{
			DSK6416_LED_off(i);
		}
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
