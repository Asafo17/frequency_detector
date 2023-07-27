/*
 *  ======== util.c ========
 */

#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gtz.h"

int tdiff,tdiff_final;

int sample, gtz_out[8];
int flag = 0;

short coef[8] =
			{ 0x6D02, 0x68AD, 0x63FC, 0x5EE7, 0x4A70, 0x4090, 0x3290, 0x23CE }; // goertzel coefficients
int frequency[8] = {697, 770, 852, 941, 1209, 1336, 1477, 1633};
int frequency_sequence[8][2] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};
void task1_dtmfDetect();
void task2_dtmfGenerate(char* keys);
extern short* buffer;

void task1_dtmfDetect() {

//	Define variables
	int i, n, k;
	char pad[4][4] = {{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
	char result[8];

//	Outer loop cycles through each tone
	for(n=0; n<8; n++) {

	    int max_1 = -1;
	    int max_2 = -1;
	    int index_1 = 0;
	    int index_2 = 0;

		while (!flag) Task_sleep(210);

//		Inner loop cycles through each goertzel output and finds index of frequencies
		for(i=0; i<8; i++) {
			if ((gtz_out[i] > max_1) && (i <= 3)) {
				max_1 = gtz_out[i];
				index_1 = i;
			}
			else if ((gtz_out[i] > max_2) && (i >= 4)) {
				max_2 = gtz_out[i];
				index_2 = i;
			}
		}

//		Print each frequency detected and resulting DTMF digit
		printf("\nGoertzel Detected at %d Hz: %d", frequency[index_1], gtz_out[index_1]);
		printf("\nGoertzel Detected at %d Hz: %d", frequency[index_2], gtz_out[index_2]);

//		Store frequencies for each digit in array
		frequency_sequence[n][0] = frequency[index_1];
		frequency_sequence[n][1] = frequency[index_2];
		result[n] = pad[index_1][index_2 - 4];
		printf("\nDigit: %c\n", result[n]);

		flag = 0;
	}


//	Print sequence of digits detected by looping through results array
	printf("\nDigit Sequence: \n");
	for (k=0; k<8; k++){
		printf("%c	", result[k]);
	}

	printf("\n\nDetection finished\n");
	printf("Generating audio\n");
	task2_dtmfGenerate(result);
	printf("Finished\n");
}


void task2_dtmfGenerate(char* keys)
{
	int fs = 10000;
	float tone_length = 0.5;
	int n_tones = 8;
	int samples_per_tone = (int) (tone_length * fs);
	int samples_total = samples_per_tone * n_tones;
	int i, n;
	char digit;
	for(n=0;n<n_tones;n++) {
		digit = keys[n];
		/* TODO 4. Complete the DTMF algorithm to generate audio signal based on the digits */
		/* ========================= */

		/* buffer[..] = ... */
		/* ========================= */
	}

	/* Writing the data to a wav file */
	FILE* fp = fopen("../answer.wav", "wb");
	int datasize = samples_total*2;
	int filesize = 36+datasize;
	int headersize = 16;
	int datatype = 1;
	int nchannel = 1;
	int byterate = fs*2;
	int align = 2;
	int bitpersample = 16;

	fwrite("RIFF", 1, 4, fp);
	fwrite(&filesize, 4, 1, fp);
	fwrite("WAVE", 1, 4, fp);
	fwrite("fmt ", 1, 4, fp);
	fwrite(&headersize, 4, 1, fp);
	fwrite(&datatype, 2, 1, fp);
	fwrite(&nchannel, 2, 1, fp);
	fwrite(&fs, 4, 1, fp);
	fwrite(&byterate, 4, 1, fp);
	fwrite(&align, 2, 1, fp);
	fwrite(&bitpersample, 2, 1, fp);
	fwrite("data", 1, 4, fp);
	fwrite(&datasize, 4, 1, fp);
	fwrite(buffer, 2, samples_total, fp);
	fclose(fp);
}
