#include "player.h"
#include "ff.h"
#include "ffconf.h"
#include "fatfs.h"
#include "string.h"
#include "math.h"
//#include "malloc.h"

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim6;
//extern ADC_HandleTypeDef hadc1;

enum f_type type;

FIL pfile;
UINT br;

short int output[2][512];

//For wav
struct HEADER header;

int adc;

enum stat status = 0;
enum short_fmt s_fmt;
int bytes_read, bytes_finish, pdata;

size_t alt_read(void *ptr, size_t size, size_t nmemb, void *datasource) {
	if (!f_eof(&pfile)) {
		f_read(datasource, ptr, size * nmemb, &br);
		return br;
	} else
		return 0;
}
int alt_seek(void *datasource, ogg_int64_t offset, int whence) {
	FRESULT res;
	int curpos = 0;

	switch (whence) {
	case SEEK_SET:
		curpos = offset;
		break;
	case SEEK_CUR:
		curpos = f_tell(&pfile) + offset;
		break;
	case SEEK_END:
		curpos = f_size(&pfile) - offset;
		break;
	}

	res = f_lseek(datasource, curpos);
	return res;
}
int alt_close(void *datasource) {
	return f_close(datasource);
}
long alt_tell(void *datasource) {
	return f_tell(&pfile);
}

int player(char *fname) {
	checkExtension(fname);
	FRESULT res;
//	trace_printf("%s\n", fname);
	res = f_open(&pfile, fname, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_OK && type != 0) {
		pdata = 0;
		s_fmt = 0;
		bytes_finish = 0;
		if (type == wav) {
			pullHeader();
			if (checkHeader()) {
				TIM_reINIT(header.format.sample_rate);
				f_lseek(&pfile, header.data.pStart);
			} else {
				return 1;
			}
		} else {
			return 1;
		}
//		printINFO();
		TIM_reINIT(getRate());
		//HAL_ADC_Start_DMA(&hadc1, &adc, sizeof(adc));
		pullData();
		//trace_printf("lol");
		Start_DMA();
		pullData();
	} else {
		return 1;
	}
	return 0;
}

void checkExtension(char *fname) {
	char *ext = strrchr(fname, '.');
	if (strncmp(ext, ".wav", 4) == 0) {
		type = wav;
	}  else {
		type = unsupported;
	}
}

long getRate(void) {
	if (type == wav) {
		return (long) header.format.byterate;
	}
	return info->rate;
}

void printINFO(void) {
	if (type == wav) {
		//trace_printf("WAV\n");
	}
	//trace_printf("Sample rate:\t%d\n", getRate());
	switch (s_fmt) {
	case PCM_8_mono:
		trace_printf("8-bits Mono PCM\n");
		break;
	case PCM_8_stereo:
		trace_printf("8-bits Stereo PCM\n");
		break;
	case PCM_16_mono:
		trace_printf("16-bits Mono PCM\n");
		break;
	case PCM_16_stereo:
		trace_printf("16-bits Stereo PCM\n");
		break;
	}
}

void pullHeader(void) {
	f_read(&pfile, header.riff, sizeof(header.riff), &br);
	f_read(&pfile, &header.overall_size, sizeof(header.overall_size), &br);
	f_read(&pfile, header.wave, sizeof(header.wave), &br);
	header.fmt_done = false;
	header.data_done = false;
	while (!f_eof(&pfile)) {
		uint32_t temp;
		f_read(&pfile, &temp, sizeof(temp), &br);
		switch (temp) {
		case 0x20746d66:
			//aka "fmt " in little endian, big endian"66 6d 74 20"
			pullFormatChunk();
			break;
		case 0x61746164:
			//aka "data" in little endian, big endian"64 61 74 61"
			pullDataChunk();
			break;
		case 0x5453494c:
			//aka "List" in little endian, big endian"4c 49 53 54"
			pullListChunk();
			break;
		default:
			f_read(&pfile, &temp, sizeof(temp), &br);
			f_lseek(&pfile, f_tell(&pfile) + temp);
			break;
		}
		if (header.fmt_done && header.data_done && header.list_done) {
			break;
		}
	};
}

void pullFormatChunk(void) {
	f_lseek(&pfile, f_tell(&pfile) - 4);
	f_read(&pfile, header.format.label, sizeof(header.format.label), &br);
	f_read(&pfile, &header.format.size, sizeof(header.format.size), &br);
	f_read(&pfile, &header.format.type, sizeof(header.format.type), &br);
	f_read(&pfile, &header.format.channels, sizeof(header.format.channels),
			&br);
	f_read(&pfile, &header.format.sample_rate,
			sizeof(header.format.sample_rate), &br);
	f_read(&pfile, &header.format.byterate, sizeof(header.format.byterate),
			&br);
	f_read(&pfile, &header.format.block_align,
			sizeof(header.format.block_align), &br);
	f_read(&pfile, &header.format.bits_per_sample,
			sizeof(header.format.bits_per_sample), &br);
	header.fmt_done = true;
}

void pullDataChunk(void) {
	f_lseek(&pfile, f_tell(&pfile) - 4);
	f_read(&pfile, header.data.label, sizeof(header.data.label), &br);
	f_read(&pfile, &header.data.size, sizeof(header.data.size), &br);
	header.data.pStart = f_tell(&pfile);
	f_lseek(&pfile, f_tell(&pfile) + header.data.size);
	header.data_done = true;
}

void pullListChunk(void) {
	f_lseek(&pfile, f_tell(&pfile) - 4);
	f_read(&pfile, header.list.label, sizeof(header.list.label), &br);
	f_read(&pfile, &header.list.size, sizeof(header.list.size), &br);
	f_lseek(&pfile, f_tell(&pfile) + header.list.size);
	header.list_done = true;
}

bool checkHeader(void) {
//check if the first four byte is equal to "RIFF"
	if (strncmp(header.riff, "RIFF", 4) != 0) {
//		trace_printf("RIFF label wrong\n");
		return false;
	}
//check if that four byte is equal to "WAVE"
	if (strncmp(header.wave, "WAVE", 4) != 0) {
//		trace_printf("WAVE label wrong\n");
		return false;
	}
	if (!(header.fmt_done && header.data_done)) {
//		trace_printf("missing format or data tag\n");
		return false;
	}
//check if the format is PCM or not
	if (header.format.type != 1) {
//		trace_printf("Not PCM\n");
		return false;
	}
//check the bits_per_sample is 8 or 16 or more
	switch (header.format.bits_per_sample) {
	case 8:
		s_fmt += 0;
		break;
	case 16:
		s_fmt += 2;
		break;
	default:
//		trace_printf("bit/sample: %d\n", header.format.bits_per_sample);
		return false;
	}
	if ((header.format.channels == 1) || (header.format.channels == 2)) { //check if channels is 1 or 2 (mono or stereo)
		s_fmt += header.format.channels - 1;
	} else {
//		trace_printf("More than two channel");
		return false;
	}
	return true;
}

void closefile(void) {
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
	//HAL_ADC_Stop_DMA(&hadc1);
	HAL_TIM_Base_Stop(&htim6);
	if (type == wav) {
		f_close(&pfile);
	}
//	trace_printf("File close\n");
}

void pullData(void) {
	if (type == wav) {
		if (s_fmt == PCM_8_mono || s_fmt == PCM_8_stereo) {
			f_read(&pfile, output[pdata], sizeof(output[pdata]) / 2, &br);
		} else {
			f_read(&pfile, output[pdata], sizeof(output[pdata]), &br);
		}
		bytes_read = br;
	}
	dataProcess();
}

void dataProcess(void) {
	uint8_t *temp8;
	unsigned short int *temp16;
	temp16 = calloc(bytes_read, sizeof(unsigned short int));
	switch (s_fmt) {
	case PCM_8_mono:
		temp8 = output[pdata];
		for (int i = 0; i < bytes_read; i++) {
			temp16[i] = temp8[i];
			temp16[i] = (temp16[i] * adc / 4096);
		}
		memcpy(output[pdata], temp16, 2 * bytes_read);
		break;
	case PCM_8_stereo:
		temp8 = output[pdata];
		for (int i = 0; i < bytes_read / 2; i++) {
			temp16[i] = temp8[2 * i];
			temp16[i + bytes_read / 2] = temp8[2 * i + 1];
			temp16[i] = (temp16[i] * adc / 4096);
			temp16[i + bytes_read / 2] = (temp16[i + bytes_read / 2] * adc
					/ 4096);

		}
		memcpy(output[pdata], temp16, 2 * bytes_read);
		break;
	case PCM_16_mono:
		for (int i = 0; i < bytes_read / 2; i++) {
			temp16[i] = output[pdata][i] ^ 0x8000;
			temp16[i] = (temp16[i] * adc / 4096);
		}
		memcpy(output[pdata], temp16, bytes_read);
		break;
	case PCM_16_stereo:
		for (int i = 0; i < bytes_read / 4; i++) {
			temp16[i] = (output[pdata][2 * i] ^ 0x8000);
			temp16[i + bytes_read / 4] = (output[pdata][2 * i + 1] ^ 0x8000);
			temp16[i] = (temp16[i] * adc / 4096);
			temp16[i + bytes_read / 4] = (temp16[i + bytes_read / 4] * adc
					/ 4096);

		}
		memcpy(output[pdata], temp16, bytes_read);
		break;
	}
	free(temp16);
}

void Start_DMA(void) {
	switch (s_fmt) {
	case PCM_8_mono:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) output[pdata],
				bytes_read,
				DAC_ALIGN_8B_R);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) output[pdata],
				bytes_read,
				DAC_ALIGN_8B_R);
		break;
	case PCM_8_stereo:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) output[pdata],
				bytes_read / 2,
				DAC_ALIGN_8B_R);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2,
				(uint32_t*) &(output[pdata][bytes_read / 2]), bytes_read / 2,
				DAC_ALIGN_8B_R);
		break;
	case PCM_16_mono:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) output[pdata],
				bytes_read / 2,
				DAC_ALIGN_12B_L);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*) output[pdata],
				bytes_read / 2,
				DAC_ALIGN_12B_L);
		break;
	case PCM_16_stereo:
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*) output[pdata],
				bytes_read / 4,
				DAC_ALIGN_12B_L);
		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2,
				(uint32_t*) &output[pdata][bytes_read / 4], bytes_read / 4,
				DAC_ALIGN_12B_L);
		break;
	}
	HAL_TIM_Base_Start(&htim6);
	pdata = pdata ^ 0x01;
}

void TIM_reINIT(uint16_t sampleRate) {
//	I don't know why,but both number has to be non-zero
//	May be the pluse is not long enough
	HAL_TIM_Base_DeInit(&htim6);
	htim6.Init.Prescaler = 36000000 / sampleRate - 1;
	htim6.Init.Period = 1;
	HAL_TIM_Base_Init(&htim6);

}

bool End(void) {
	if (type == wav) {
		if (bytes_finish > header.data.size) {
			return 1;
		} else {
			bytes_finish += bytes_read;
		}
	} else {
		return 0;
	}
}

int getTimePercentage() {
	float result;
	if (type == wav) {
		result = ((float) bytes_finish / (float) header.data.size) * 200;
	} 
	return ((int) result);
}

int getStatus(void) {
	return status;
}

void setStatus(enum stat input) {
	status = input;
}

void player_play(void) {
	HAL_TIM_Base_Start_IT(&htim6);
	status = Play;
}

void player_pause(void) {
	HAL_TIM_Base_Stop_IT(&htim6);
	status = Pause;
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
	HAL_TIM_Base_Stop(&htim6);
	if (!End()) {
		Start_DMA();
		pullData();
	} else {
		closefile();
	}
}
