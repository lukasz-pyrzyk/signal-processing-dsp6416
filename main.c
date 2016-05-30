#include "include/C6416dskinit.h"
#include "include/filter.h"

#define DSK6416_AIC23_INPUT_MIC 0x0015
#define DSK6416_AIC23_INPUT_LINE 0x0011

extern DSK6416_AIC23_CodecHandle hAIC23_handle;

void c6416_dsk_init()
{
	DSK6416_init();

	hAIC23_handle = DSK6416_AIC23_openCodec(0, &config);
	
	DSK6416_AIC23_setFreq(hAIC23_handle, DSK6416_AIC23_FREQ_96KHZ);
	DSK6416_AIC23_rset(hAIC23_handle, 0x0004, DSK6416_AIC23_INPUT_LINE);
	
	MCBSP_config(DSK6416_AIC23_DATAHANDLE, &AIC23CfgData);

	MCBSP_start(DSK6416_AIC23_DATAHANDLE, MCBSP_XMIT_START | MCBSP_RCV_START |
		MCBSP_SRGR_START | MCBSP_SRGR_FRAMESYNC, 220);
}

void comm_intr()
{
	IRQ_globalDisable();

	c6416_dsk_init();
	CODECEventId = MCBSP_getXmtEventId(DSK6416_AIC23_codecdatahandle);

	IRQ_map(CODECEventId, 11);
	IRQ_reset(CODECEventId);
	
	IRQ_globalEnable();
	
	IRQ_nmiEnable();
	IRQ_enable(CODECEventId);
}

short OUT_L, OUT_R;

#define MAX_VALUE 400
#define NOISE 7

#define CHANNEL_LENGTH 2
#define LED_COUNT 4

struct ledData
{
	char ledId;
	float threshold;
} ledCollection[] = {
	{3, 0.75f},
	{2, 0.5f},
	{1, 0.25f},
	{0, 0.0f}
};

union deviceData
{
	Uint32 streamData;
	short channelData[2];
} currentDeviceData;

enum Channel 
{ 
	CHANNEL_RIGHT,
	CHANNEL_LEFT
};

enum Channel channels[CHANNEL_LENGTH] = {
	CHANNEL_RIGHT,
	CHANNEL_LEFT
};

short* outPtr[] = {
	&OUT_R,
	&OUT_L
};

short* inPtr[] = {
	&currentDeviceData.channelData[0],
	&currentDeviceData.channelData[1]
};

int muteButtons[] = {
	0,
	1
};

int filterButtons[] = {
	2,
	3
};

typedef short (*filterFunc_t)(short, short*);

filterFunc_t filters[] = 
{
	FILTR_R,
	FILTR_L
};

void readDeviceData(union deviceData* data)
{
	data->streamData = MCBSP_read(DSK6416_AIC23_DATAHANDLE);
}

void writeDeviceData(union deviceData* data)
{
	MCBSP_write(DSK6416_AIC23_DATAHANDLE, data->streamData);
}

void handleChannel(enum Channel channel)
{
	int mute = DSK6416_DIP_get(muteButtons[(int)(channel)]);
	int filter = DSK6416_DIP_get(filterButtons[(int)(channel)]);
	
	if(mute)
	{
		*(outPtr[channel]) = 0;
	}
	else if(filter)
	{
		*(outPtr[channel]) = (Uint32)filters[(int)channel](*(inPtr[channel]), h1);
	}
	else
	{
		*(outPtr[channel]) = *(inPtr[channel]);
	}
}

void writeChannel(enum Channel channel, union deviceData* device)
{
	device->channelData[channel] = *(outPtr[channel]);
}

void toggleLED(float percentage, struct ledData* led)
{
	if(percentage > led->threshold)
	{
		DSK6416_LED_on(led->ledId);
	}
	else
	{
		DSK6416_LED_off(led->ledId);
	}
}

void handleLEDs(enum Channel channel)
{
	int i = 0;
 	float percentage = 0.0f;

	short data = *(inPtr[channel]);

	if(data > NOISE)
	{
		percentage = (float)(data - NOISE) / (float) (MAX_VALUE - NOISE);

		for(i = -1; i < LED_COUNT; toggleLED(percentage, &ledCollection[i++]));
	}
	else
	{
		for(i = -1; i < LED_COUNT; DSK6416_LED_off(i++));
	}
}

interrupt void c_int11()
{
	int i = 0;
	enum Channel chan;

	readDeviceData(&currentDeviceData);

	for(; i < CHANNEL_LENGTH; i++)
	{
		chan = channels[i];

		handleChannel(chan);
        writeChannel(chan, &currentDeviceData);	
	}

	writeDeviceData(&currentDeviceData);

	handleLEDs(chan);

	return;
}

void main()
{
    DSK6416_init();
    DSK6416_LED_init();
    DSK6416_DIP_init();

	comm_intr();

	currentDeviceData.streamData = 0;
	writeDeviceData(&currentDeviceData);

	while(1);
}

