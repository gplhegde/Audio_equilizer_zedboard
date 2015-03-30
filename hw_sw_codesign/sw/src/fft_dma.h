/*
 * fft_dma.h
 *
 *  Created on: Oct 31, 2014
 *      Author: Gopalakrishna Hegde
 */

#ifndef FFT_DMA_H_
#define FFT_DMA_H_

#include "xaxidma.h"
#include "xparameters.h"

#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

#define TX_BUFFER_BASE		(0x00000000) // TX and RX buffers are in OCM
#define RX_BUFFER_BASE		(0x00001000)

#define MAX_PKT_LEN_WORDS	128
#define MAX_PKT_LEN			MAX_PKT_LEN_WORDS*4

int compute_fft();
int DMA_Congfig(u16 DeviceId);
void read_fft_output(s16 *real_buff, s16 *img_buff);

#endif /* FFT_DMA_H_ */
