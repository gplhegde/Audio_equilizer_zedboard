/* \file zync_audio.c
* \author Gopalakrishna Hegde, Kiran G B 
* \date 21 Aug 2014
* \brief Application framework
*/

#include <stdio.h>
#include "utils.h"
#include "fft_dma.h"
#include "audio.h"
#include "xparameters.h"
#include "xtime_l.h"
#include "xil_io.h"

#define timer_base 0xF8F00000
#define NOISE_LEARNING_TIME 3
#define SAMPLING_FREQ 48000
#define NOISE_FRAME_COUNT (NOISE_LEARNING_TIME * (SAMPLING_FREQ/N))

void estimate_noise();

/***********************************************************
Timer Registers
************************************************************/
static volatile int *timer_counter_l=(volatile int *)(timer_base+0x200);
static volatile int *timer_counter_h=(volatile int *)(timer_base+0x204);
static volatile int *timer_ctrl=(volatile int *)(timer_base+0x208);
/***********************************************************
***********************************************************
Function definitions
************************************************************/
void init_timer(volatile int *timer_ctrl, volatile int *timer_counter_l, volatile int *timer_counter_h){
        *timer_ctrl=0x0;
        *timer_counter_l=0x1;
        *timer_counter_h=0x0;
        DATA_SYNC;
}

void start_timer(volatile int *timer_ctrl){
        *timer_ctrl=*timer_ctrl | 0x00000001;
        DATA_SYNC;
}

void stop_timer(volatile int *timer_ctrl){
        *timer_ctrl=*timer_ctrl & 0xFFFFFFFE;
        DATA_SYNC;
}


// Buffers required for FFT calculation, holding 8 frame magnitudes and average
// Static memory allocation is preferred for real time systems

s16 real_buff[N] = {0};
s16 img_buff[N] = {0};
u8 noise_th_buff[N/2]= {0};

u16 magnitude_buff[FRAMES_IN_WINDOW][N/2] = {{0}};
u16 accumulator[N/2] = {0};
u8 oled_buff[N];

// API to estimate the noise threshold during noise learning time.
void estimate_noise()
{
	Xint16 noise_in[N];
	u32 *tx_buffer;

	tx_buffer = (u32 *)TX_BUFFER_BASE;

	float temp_th_buff[N/2];
	u16 noise_mag_buff[N/2];
	u32 frame, i, index;
	for (frame = 0; frame < NOISE_FRAME_COUNT; frame++)
	{
		get_audio(noise_in);
		for (index = 0; index < 256; index+=2)
		{
			tx_buffer[index] = noise_in[index/2];
			tx_buffer[index + 1] = 0;
		}
		compute_fft();
		read_fft_output(real_buff, img_buff);
		fft_magnitude(real_buff, img_buff, noise_mag_buff);

		for (i = 0; i < N/2; i++)
		{
			temp_th_buff[i] = (temp_th_buff[i] + noise_mag_buff[i]) /2;
		}
		printf("Please wait... I am learning noise conditions!!!\n\r");
	}
	for (i=0; i<N/2;i++)
	{
		noise_th_buff[i] = (u8)(temp_th_buff[i]+1); // ceil the noise threshold
	}

}
int main(void)
{

	// Local variables
	Xint16 data_in[N];
	s16 *tx_buffer;
	u32 index, frame = 0;
	tx_buffer = (s16 *)TX_BUFFER_BASE;

	// Initialization
	system_init();

	// Estimate noise threshold as per current environmental conditions
	estimate_noise();

	printf("System initialization is done.\n");
	while(1)
	{
		// Read audio frame of length 128
		get_audio(data_in);

		// Timer init and start for latency measurement
		init_timer(timer_ctrl, timer_counter_l, timer_counter_h);
		start_timer(timer_ctrl);

		// Store the data in DMA transmission buffer
		for (index = 0; index < 2*N; index+=2) // 6us
		{
			tx_buffer[index] = data_in[index>>1];
		}

		// Calculate 128 point FFT for the audio frame.
		// DMA data to FFT core. Wait for output and transfer the FFT output back to DDR/OCM
		compute_fft();

		// Read the FFT output from DDR/OCM to real and imaginary o/p buffer for further computations
		read_fft_output(real_buff, img_buff);

		// Subtract a frame from accumulator which is 8 frame behind
		subtract_frame_from_acc(accumulator, &magnitude_buff[frame][0]);

		// calculate the FFT magnitude
		fft_magnitude(real_buff, img_buff, &magnitude_buff[frame][0]);

		// Accumulate. We will not average here.
		add_frame_to_acc(accumulator, &magnitude_buff[frame][0]);

		// Average by dividing by no of frames accumulated.
		average_acc(accumulator, oled_buff);

		// Noise suppression by thresholding
		suppress_noise(oled_buff);

		for (index = 65; index < N; index++)
		{
			oled_buff[index] = oled_buff[128 - index];
		}
		oled_buff[64] = 0;

		stop_timer(timer_ctrl);
		xil_printf("Total time per frame =  %d us\n\r", (*timer_counter_l)/333);

		// Display the output on OLED
		OLED_Clear();
		//print("Writing data to OLED");
		OLED_Equalizer_128(oled_buff);

		frame++;
		frame = frame % FRAMES_IN_WINDOW;
	}
	return 0;
}
