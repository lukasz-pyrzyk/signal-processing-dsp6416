#include "DSK6416_AIC23.h"
Uint32 fs=DSK6416_AIC23_FREQ_8KHZ;
#define DSK6416_AIC23_INPUT_MIC 0x0015
#define DSK6416_AIC23_INPUT_LINE 0x0011
Uint16 inputsource=DSK6416_AIC23_INPUT_MIC;

void main()
{
  short sample_data;

  comm_poll();
  while(1)
  {
    sample_data = input_left_sample();
    output_left_sample(sample_data);
  }
}