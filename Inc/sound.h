#ifndef SOUND_H_
#define SOUND_H_

#include "player.h"
#include "ff.h"
#include "ffconf.h"
#include "fatfs.h"
#include "string.h"
#include "math.h"
#include "stdbool.h"


extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim6;

FIL myFILE;
UINT numberofbytes = 2;
const int wave_size =4095;
uint16_t wav_buffer1[wave_size]={0x7};


int player(char *fname) {  // main function  , execute once 
	FRESULT res;
	
	
	res = f_open(&myFILE, fname, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK ) {
		// go to data part
		char locate[4];
		const char word[4] = "data";
		int count = 0;
		while (strncmp(locate, word, 4)) {
			count++;
			res = f_lseek(&myFILE, count);
			f_read(&myFILE, locate, 4, &numberofbytes);
		}
	} else {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
	
	return 1;} // unsuccess

		pullData();  // read data first
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)wav_buffer1, wave_size, DAC_ALIGN_12B_R);
		HAL_TIM_Base_Start(&htim6);
		pullData();

	return 0;
}

void data_shift_right_4 (){	
	for(int i=0;i<wave_size;i++){
		wav_buffer1[i] = wav_buffer1[i]>>4;    // shift right by 4
		if(wav_buffer1[i]<0x800){ wav_buffer1[i]=0x800+wav_buffer1[i];}// zero is at 2048 (0x800),for any value smaller than 0x800 need to be 2048-x
		else{wav_buffer1[i]=wav_buffer1[i]-0x800;}     // Signed int , 0xFFF is 1, so consider smaller than 
	}
}

void pullData(void) {

	f_read(&myFILE,wav_buffer1, wave_size*2, &numberofbytes);
	data_shift_right_4();
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
	HAL_TIM_Base_Stop(&htim6);
	if (!f_eof(&myFILE)) {
		HAL_DAC_Start_DMA(hdac, DAC_CHANNEL_1, (uint32_t*)wav_buffer1, wave_size, DAC_ALIGN_12B_R); 
		HAL_TIM_Base_Start_IT(&htim6);
		pullData();		// get data for next one
	} else {
		HAL_DAC_Stop_DMA(hdac, DAC_CHANNEL_1);
		f_close(&myFILE);
	}
	
}
#endif /* SOUND_H_ */
