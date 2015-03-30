/* \file zync_audio.c
* \author Gopalakrishna Hegde, Kiran G B 
* \date 21 Aug 2014
* \brief Application framework
*/

#include <stdio.h>
#include "utils.h"
#include "fft_128.h"
#include "audio.h"
#include "xparameters.h"
#include "xtime_l.h"
#include "xil_io.h"

#define timer_base 0xF8F00000
#define NOISE_LEARNING_TIME 3
#define SAMPLING_FREQ 48000
#define NOISE_FRAME_COUNT (NOISE_LEARNING_TIME * (SAMPLING_FREQ/N))

static u32 estimate_noise();
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

float real_buff[N] = {0.0};
float img_buff[N] = {0.0};
u32 noise_threshold = 0;

u16 magnitude_buff[FRAMES_IN_WINDOW][N] = {{0}};
u16 accumulator[N] = {0};
u8 oled_buff[N];

static u32 estimate_noise()
{
	u16 frame, index;
	float temp_noise_th = 0;
	Xint16 data_in[N];
	u16 noise_mag_buff[N];
	// Learning noise level for first 2s
	init_timer(timer_ctrl, timer_counter_l, timer_counter_h);
	start_timer(timer_ctrl);
	for (frame = 0; frame < NOISE_FRAME_COUNT; frame++)
	{
		float noise_avg = 0;
		get_audio(data_in);
		// Store the data in the real data array
			for (index = 0; index < N; index++)
			{
				real_buff[index] = (float)data_in[index];
				img_buff[index] = 0;
			}
			fft_128(real_buff, img_buff);
			fft_magnitude(real_buff, img_buff, noise_mag_buff);

			for (index = 0; index < N; index++)
			{
				noise_avg += (noise_mag_buff[index]>>7);
			}
			temp_noise_th = (temp_noise_th + noise_avg/N)/2;

		}
		stop_timer(timer_ctrl);
		xil_printf("Noise threshold estimation time =  %d us\n\r", (*timer_counter_l)/333);
		return (u32)(temp_noise_th + 1); // 1 is added to ceil the value

}
int main(void)
{

	// Local variables
	Xint16 data_in[N];
	u16 index, frame = 0;

	// Initialization
	system_init();
	// Estimate noise threshold as per current environmental conditions

	noise_threshold = estimate_noise();
	if (8 < noise_threshold)
	{
		// What if some one is talking when we are learing noise conditions...
		// Speech should not be considered as noise...
		// Hence we limit the threshold by 8(arbitrary)
		noise_threshold = 8;
	}

	xil_printf("Noise threshold = %d \n", noise_threshold);
	printf("System initialization is done.\n");
	while(1)
	{
		init_timer(timer_ctrl, timer_counter_l, timer_counter_h);
		start_timer(timer_ctrl);
		// Read audio frame of length 128

		get_audio(data_in);

		// Store the data in the real data array
		for (index = 0; index < N; index++)
		{
			real_buff[index] = (float)data_in[index];
			img_buff[index] = 0;
		}

		// Calculate 128 point FFT for the audio frame.
		fft_128(real_buff, img_buff);

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

		stop_timer(timer_ctrl);
		xil_printf("Run time per frame excluding OLED display time =  %d us\n\r", (*timer_counter_l)/333);

		// Display the output on OLED
		OLED_Clear();
		//print("Writing data to OLED");
		OLED_Equalizer_128(oled_buff);

		frame++;
		frame = frame % FRAMES_IN_WINDOW;
	}
	return 0;
}
