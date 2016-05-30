// C6416dskinit.h Include file for C6416DSKINIT.C */

#include "dsk6416.h"
#include "dsk6416_aic23.h"
#include <csl_mcbsp.h>

// Define AIC23 data structure unioning a 32 bit variable with two 16 bit variables
#define LEFT  1
#define RIGHT 0
union {
	Uint32 uint;
	short channel[2];
	} AIC_data; 

extern far void vectors();	// Declare an external function called vectors

static Uint32 CODECEventId, poll;	// Define a poll variable and an interrupt event ID for the AIC23

// This is needed to modify the BSL's data channel McBSP configuration
// See the changes below
   MCBSP_Config AIC23CfgData = {
        MCBSP_FMKS(SPCR, FREE, NO)              |
        MCBSP_FMKS(SPCR, SOFT, NO)              |
        MCBSP_FMKS(SPCR, FRST, YES)             |
        MCBSP_FMKS(SPCR, GRST, YES)             |
        MCBSP_FMKS(SPCR, XINTM, XRDY)           |
        MCBSP_FMKS(SPCR, XSYNCERR, NO)          |
        MCBSP_FMKS(SPCR, XRST, YES)             |
        MCBSP_FMKS(SPCR, DLB, OFF)              |
        MCBSP_FMKS(SPCR, RJUST, RZF)            |
        MCBSP_FMKS(SPCR, CLKSTP, DISABLE)       |
        MCBSP_FMKS(SPCR, DXENA, OFF)            |
        MCBSP_FMKS(SPCR, RINTM, RRDY)           |
        MCBSP_FMKS(SPCR, RSYNCERR, NO)          |
        MCBSP_FMKS(SPCR, RRST, YES),

        MCBSP_FMKS(RCR, RPHASE, SINGLE)         |
        MCBSP_FMKS(RCR, RFRLEN2, DEFAULT)       |
        MCBSP_FMKS(RCR, RWDLEN2, DEFAULT)       |
        MCBSP_FMKS(RCR, RCOMPAND, MSB)          |
        MCBSP_FMKS(RCR, RFIG, NO)               |
        MCBSP_FMKS(RCR, RDATDLY, 0BIT)          |
        MCBSP_FMKS(RCR, RFRLEN1, OF(0))         | // This changes to 1 FRAME
        MCBSP_FMKS(RCR, RWDLEN1, 32BIT)         | // This changes to 32 bits per frame
        MCBSP_FMKS(RCR, RWDREVRS, DISABLE),

        MCBSP_FMKS(XCR, XPHASE, SINGLE)         |
        MCBSP_FMKS(XCR, XFRLEN2, DEFAULT)       |
        MCBSP_FMKS(XCR, XWDLEN2, DEFAULT)       |
        MCBSP_FMKS(XCR, XCOMPAND, MSB)          |
        MCBSP_FMKS(XCR, XFIG, NO)               |
        MCBSP_FMKS(XCR, XDATDLY, 0BIT)          |
        MCBSP_FMKS(XCR, XFRLEN1, OF(0))         | // This changes to 1 FRAME
        MCBSP_FMKS(XCR, XWDLEN1, 32BIT)         | // This changes to 32 bits per frame
        MCBSP_FMKS(XCR, XWDREVRS, DISABLE),
        
        MCBSP_FMKS(SRGR, GSYNC, DEFAULT)        |
        MCBSP_FMKS(SRGR, CLKSP, DEFAULT)        |
        MCBSP_FMKS(SRGR, CLKSM, DEFAULT)        |
        MCBSP_FMKS(SRGR, FSGM, DEFAULT)         |
        MCBSP_FMKS(SRGR, FPER, DEFAULT)         |
        MCBSP_FMKS(SRGR, FWID, DEFAULT)         |
        MCBSP_FMKS(SRGR, CLKGDV, DEFAULT),

        MCBSP_MCR_DEFAULT,
        MCBSP_RCERE0_DEFAULT,						// These are added due to extra					
        MCBSP_RCERE1_DEFAULT,						// bits in the C6416 McBSP regs
        MCBSP_RCERE2_DEFAULT,
        MCBSP_RCERE3_DEFAULT,
        MCBSP_XCERE0_DEFAULT,
        MCBSP_XCERE1_DEFAULT,
        MCBSP_XCERE2_DEFAULT,
        MCBSP_XCERE3_DEFAULT,

        MCBSP_FMKS(PCR, XIOEN, SP)              |
        MCBSP_FMKS(PCR, RIOEN, SP)              |
        MCBSP_FMKS(PCR, FSXM, EXTERNAL)         |
        MCBSP_FMKS(PCR, FSRM, EXTERNAL)         |
        MCBSP_FMKS(PCR, CLKXM, INPUT)           |
        MCBSP_FMKS(PCR, CLKRM, INPUT)           |
        MCBSP_FMKS(PCR, CLKSSTAT, DEFAULT)      |
        MCBSP_FMKS(PCR, DXSTAT, DEFAULT)        |
        MCBSP_FMKS(PCR, FSXP, ACTIVEHIGH)       |
        MCBSP_FMKS(PCR, FSRP, ACTIVEHIGH)       |
        MCBSP_FMKS(PCR, CLKXP, FALLING)         |
        MCBSP_FMKS(PCR, CLKRP, RISING)
    };


DSK6416_AIC23_Config config = { // normal mode line in, mic off
    0x0017, // 0 DSK6416_AIC23_LEFTINVOL  Glosnosc wejscia lewego kanalu 
    0x0017, // 1 DSK6416_AIC23_RIGHTINVOL Glosnosc wejscia prawego kanalu
    0x00F9, // 2 DSK6416_AIC23_LEFTHPVOL  Glosnosc lewego kanalu sluchawek
    0x00F9, // 3 DSK6416_AIC23_RIGHTHPVOL Glosnosc prawego kanalu sluchawek
    0x0012, // 4 DSK6416_AIC23_ANAPATH    kontrola  analogowej sciezki audio 
    0x0000, // 5 DSK6416_AIC23_DIGPATH    kontrola cyfrowej sciezki audio 
    0x0002, // 6 DSK6416_AIC23_POWERDOWN  kontrola w³¹czonych urz¹dzeñ
    0x0043, // 7 DSK6416_AIC23_DIGIF      format interfejsu cyfrowego dzwieku
    0x0023, // 8 DSK6416_AIC23_SAMPLERATE kontrola czestotliwosci probkowania
    0x0001  // 9 DSK6416_AIC23_DIGACT     aktywacja cyfrowego interfejsu
};
DSK6416_AIC23_CodecHandle hAIC23_handle;

void c6416_dsk_init();
void comm_poll();
void comm_intr();
void output_sample(Uint32);
void output_left_sample(short);
void output_right_sample(short);
Uint32 input_sample();
short input_left_sample();
short input_right_sample();
