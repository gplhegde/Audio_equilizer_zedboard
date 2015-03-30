/*
 * fft_dma.c
 *
 *  Created on: Oct 31, 2014
 *      Author:
 */
#include <stdio.h>
#include "utils.h"
#include "fft_dma.h"
#include "arm_neon.h"

XAxiDma AxiDma;

// DMA related Code Reference : Xilinx
// PL AXI DMA configuration. DMA interrupt is disabled. We use polling here.
int DMA_Congfig(u16 DeviceId)
{
	int Status;
	XAxiDma_Config *CfgPtr;
	/* Initialize the XAxiDma device.
		 */
		CfgPtr = XAxiDma_LookupConfig(DeviceId);
		if (!CfgPtr) {
			xil_printf("No config found for %d\r\n", DeviceId);
			return XST_FAILURE;
		}

		Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("Initialization failed %d\r\n", Status);
			return XST_FAILURE;
		}

		if(XAxiDma_HasSg(&AxiDma)){
			xil_printf("Device configured as SG mode \r\n");
			return XST_FAILURE;
		}

		/* Disable interrupts, we use polling mode
		 */
		XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
							XAXIDMA_DEVICE_TO_DMA);
		XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
							XAXIDMA_DMA_TO_DEVICE);
		return Status;
}

// Initiate DMA on the audio data and wait for FFT computation
// DMA back the FFT output to OCM
int compute_fft()
{
	int Status;
	u32 *tx_buffer;
	u32 *rx_buffer;

	tx_buffer = (u32 *)TX_BUFFER_BASE;
	rx_buffer = (u32 *)RX_BUFFER_BASE;

	// Cache flush is not needed since we disabled cache on OCM
	//Xil_DCacheFlushRange((u32)tx_buffer, MAX_PKT_LEN);

		Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) rx_buffer,
					MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) tx_buffer,
					MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		while (XAxiDma_Busy(&AxiDma,XAXIDMA_DMA_TO_DEVICE)) {

		}
		while (XAxiDma_Busy(&AxiDma,XAXIDMA_DEVICE_TO_DMA)) {
				// Wait
		}

	return XST_SUCCESS;
}
// read the FFT output buffer in OCM and store in real and imaginary buffer
void read_fft_output(s16 *real_buff, s16 *img_buff)
{
	u32 index;
	s16 *rx_buff;

	rx_buff = (s16 *) RX_BUFFER_BASE;

	// Cache flush is not needed since we disabled cache on OCM
	//Xil_DCacheInvalidateRange((u32)rx_buff, MAX_PKT_LEN);
	for(index = 0; index < N; index+=2)
	{
		real_buff[index/2]= rx_buff[index];
		img_buff[index/2] = rx_buff[index+1];
	}
}
