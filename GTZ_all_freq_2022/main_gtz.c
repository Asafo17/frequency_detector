/*
 *  ======== gtz.c ========
 */

#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gtz.h"

void clk_SWI_Read_Data(UArg arg0);
void clk_SWI_GTZ_All_Freq(UArg arg0);

extern void task0_dtmfGen(void);
extern void task1_dtmfDetect(void);

extern int sample, tdiff, tdiff_final, gtz_out[8];
extern short coef[8];
extern int flag;
short data[NO_OF_SAMPLES];
short *buffer;

/*
 *  ======== main ========
 */
int main() {
	System_printf("\n System Start\n");
	System_flush();

	/* Read binary data file */
	FILE* fp = fopen("../data.bin", "rb");
	if(fp==0) {
		System_printf("Error: Data file not found\n");
		System_flush();
		return 1;
	}
	fread(data, 2, NO_OF_SAMPLES, fp);
	buffer = (short*)malloc(2*8*10000);

	/* Create a Clock Instance */
	Clock_Params clkParams;

	/* Initialise Clock Instance with time period and timeout (system ticks) */
	Clock_Params_init(&clkParams);
	clkParams.period = 1;
	clkParams.startFlag = TRUE;

	/* Instantiate ISR for tone generation  */
	Clock_create(clk_SWI_Read_Data, TIMEOUT, &clkParams, NULL);

	/* Instantiate 8 parallel ISRs for each of the eight Goertzel coefficients */
	Clock_create(clk_SWI_GTZ_All_Freq, TIMEOUT, &clkParams, NULL);

	/* Start SYS_BIOS */
	BIOS_start();
}

/*
 *  ====== clk_SWI_Generate_DTMF =====
 *  Dual-Tone Generation
 *  ==================================
 */
void clk_SWI_Read_Data(UArg arg0) {
	static int tick;
	tick = Clock_getTicks();
	sample = data[tick%NO_OF_SAMPLES];
}

/*
 *  ====== clk_SWI_GTZ =====
 *  gtz sweep
 *  ========================
 */
void clk_SWI_GTZ_All_Freq(UArg arg0) {
    static int N = 0;
    static int Goertzel_Value[8] = {0,0,0,0,0,0,0,0};
    static short delay[8] = {0,0,0,0,0,0,0,0};
    static short delay_1[8] = {0,0,0,0,0,0,0,0};
    static short delay_2[8] = {0,0,0,0,0,0,0,0};
    int i;
    short input;

    for(i=0; i<8; i++) {
        input = ((short) sample) >> 4;

        // Delay calcs.
        delay[i] = input - delay_2[i] + ((coef[i] * delay_1[i]) >> 14);
        delay_2[i] = delay_1[i];
        delay_1[i] = delay[i];
        N++;

        if (N == 206)
        {
            // Value calc.
            Goertzel_Value[i] = (delay_1[i]*delay_1[i])+(delay_2[i]*delay_2[i])-(((coef[i]*delay_1[i])>>14)*delay_2[i]);
            Goertzel_Value[i] <<= 4;      // Scaled up.
			N = delay_1[i] = delay_2[i] = 0; // Reset.
        }
        gtz_out[i] = Goertzel_Value[i];
        flag++;
    }
}

