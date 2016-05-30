#include "C6416dskinit.h"

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

short L_in[101]; 		/* PRÓBKI WEJSIOWE L_in[0] NAJNOWSZA PRÓBKA LEWA,L_in[101] NAJSTARSZA PRÓBKA LEWA*/
short R_in[101];        /* PRÓBKI WEJSIOWE P_in[0] NAJNOWSZA PRÓBKA PRAWA,P_in[101] NAJSTARSZA PRÓBKA Prawa*/

/* WSPÓ£CZYNNIKI WYLICZONE Z CZÊSTOTLIWOŒCI¥ PRÓBKOWANIA 96kHz I ODCIÊCIE DLA 1kHz */
short h1[]={
        0,      0,      0,      0,      0,      1,      2,      4,      6,
        9,     12,     17,     23,     29,     37,     46,     56,     68,
       81,     95,    111,    128,    146,    166,    186,    208,    231,
      255,    279,    304,    330,    356,    382,    408,    434,    459,
      484,    508,    531,    553,    574,    594,    612,    628,    642,
      654,    664,    672,    678,    682,    683,    682,    678,    672,
      664,    654,    642,    628,    612,    594,    574,    553,    531,
      508,    484,    459,    434,    408,    382,    356,    330,    304,
      279,    255,    231,    208,    186,    166,    146,    128,    111,
       95,     81,     68,     56,     46,     37,     29,     23,     17,
       12,      9,      6,      4,      2,      1,      0,      0,      0,
        0,      0
};

short FILTR_L (short, short*);
short FILTR_R (short, short*);

short OUT_L, OUT_R;

#define MAX_VALUE 400
#define NOISE 5

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

short FILTR_L (short input, short *h)
{
	int i;
	short output;
	int acc=0;
	int prod;
	L_in[0] = input;         	/* ODŒWIE¯A NAJNOWSZA PRÓBKÊ */

	acc = 0;                      	/* ZERUJE AKUMULATOR */
	for (i=0; i<101; i++)         	/* PETLA WYKONA SIE 101 RAZY */
	{   
		prod = (h[i]*L_in[i]);  /* MNO¯ENIE Q.15 */
		acc = acc + prod;       /* ODŒWIE¯ANIE 32-bit AKUMULATORA */
	}                             	
	output = (short) (acc>>15);    	/* PRZERABIANIE WYJSCIA DO WARTOSCI 16-bits. */

	for (i=100; i>0; i--)         	/* PRZESUNIÊCIE PRÓBEK */
		L_in[i]=L_in[i-1];

	return output;			/* ZWRACA PRZEFILTROWANA PRÓBKÊ */
}

short FILTR_R (short input, short *h)
{
	int i;
	short output;
	int acc=0;
	int prod;
	R_in[0] = input;         	/* ODŒWIE¯A NAJNOWSZA PRÓBKÊ */

	acc = 0;                      	/* ZERUJE AKUMULATOR */
	for (i=0; i<101; i++)         	/* PETLA WYKONA SIE 101 RAZY */
	{   
		prod = (h[i]*R_in[i]);  /* MNO¯ENIE Q.15 */
		acc = acc + prod;       /* ODŒWIE¯ANIE 32-bit AKUMULATORA */
	}                             	
	output = (short) (acc>>15);    	/* PRZERABIANIE WYJSCIA DO WARTOSCI 16-bits. */

	for (i=100; i>0; i--)         	/* PRZESUNIÊCIE PRÓBEK */
		R_in[i]=R_in[i-1];

	return output;			/* ZWRACA PRZEFILTROWANA PRÓBKÊ */
}
