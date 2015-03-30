/* \filename utils.c
 * \author Gopalakrishna Hegde, Kiran G B
 * \date 25 Aug 2014
 */

#include <stdio.h>
#include "utils.h"
#include "fft_128.h"
#include "audio.h"
#include "oled.h"
#include "arm_neon.h"

#define UPPER_THRESHOLD 40


/* \brief LED and AUDIO initialization
 *
 */
void system_init()
{
	Xil_Out32(OLED_BASE_ADDR,0xff);
	OLED_Init();							   //OLED initialization
	IicConfig(XPAR_XIICPS_0_DEVICE_ID);
	AudioPllConfig(); 						   //enable core clock for ADAU1761
	AudioConfigure();

}

/* \brief Function to reverse bits of num
 * param[in] 	number	number to be bit reversed
 * param[in] 	N		total bits in the number
 * param[out]	u32		bit reversed number
 */
u32 bit_reverse(u32 number)
{    
    u32 rev_number = 0, i, temp;
 
    for (i = 0; i < S; i++)
    {
        temp = (number & (1 << i));
        if(temp)
            rev_number |= (1 << ((S - 1) - i));
    }
    return rev_number;
}

/* \brief Re-order the array elements in their bit reversed order.
 * param[in] data	pointer to data array
 * param[in] N
 */
void reorder_data_in_bit_reverse(float * __restrict data)
{
	u32 index, rev_index;
	float temp;
	for (index = 0; index < (N & 0xF0); index++)
	{
		
		rev_index = bit_reverse(index);
		if (rev_index > index)
		{
			temp = data[index];
			data[index] = data[rev_index];
			data[rev_index] = temp;
		}
	}
	return;
}

/* \brief Calculate square of the magnitude of FFT coefficients
 *
 * param[in] fft_real	pointer to real part of FFT
 * param[in] fft_img	pointer to imaginary part of the FFT
 * param[in] mag_buff	pointer to output magnitude buffer
 */
void fft_magnitude(float * __restrict fft_real, float * __restrict fft_img, u16 * __restrict mag_buff)
{
	u32 index;

#ifdef NEON  // FIXME: Not working
	float32x4_t real_vec, img_vec;
	uint32x4_t v1,v2;
	for (index = 0; index < N; index+=4)
	{
		real_vec = vld1q_f32(fft_real);
		img_vec = vld1q_f32(fft_img);
		v1 =  vcvtq_u32_f32(vmulq_f32(real_vec, real_vec));
		v2 =  vcvtq_u32_f32(vmulq_f32(img_vec, img_vec));
		v1 =  vqaddq_u32(v1, v2);
		vst1_u16(mag_buff,   vmovn_u32(v1));
	}
#else
	for (index = 0; index < (N & 0xF0); index++)
	{
		// square of the magnitude is stored in fftReal Array.
		mag_buff[index] = (u16)(fft_real[index] * fft_real[index] + fft_img[index] * fft_img[index]);
	}
#endif // NEON

#ifdef DEBUG
	printf("Calculated FFT magnitude \n");
#endif // DEBUG
}

/* \brief suppress noise by thresholding
 * \param[in] magnitude_buff	pointer to FFT magnitude buffer *
 */
void suppress_noise(u8 *magnitude_buff)
{
	u32 index;
#ifdef NEON // FIXME: Not working !!!
	uint8x16_t mag_vec, lower_th, upper_th,cmp_res;
	lower_th = vdupq_n_u8(LOWER_THRESHOLD);
	upper_th = vdupq_n_u8(UPPER_THRESHOLD);
	for (index = 0; index < N; index+=16)
	{
		mag_vec = vld1q_u8(magnitude_buff);
		cmp_res = vcgeq_u8(mag_vec, lower_th);
		cmp_res = vandq_u8(cmp_res, lower_th);
		mag_vec = vsubq_u8(mag_vec, cmp_res);

		cmp_res = vcgtq_u8(mag_vec, upper_th);
		mag_vec = vaddq_u8(vandq_u8(cmp_res, upper_th), vandq_u8(mag_vec, vmvnq_u8(cmp_res)));
		vst1q_u8((magnitude_buff + index), mag_vec);
	}
#else
	for(index = 0; index < (N & 0xF0); index++)
	{
		if (magnitude_buff[index] >= noise_threshold)
		{
			magnitude_buff[index] -= noise_threshold;
			if( UPPER_THRESHOLD < magnitude_buff[index])
			{
				magnitude_buff[index] = UPPER_THRESHOLD;
			}
		}
		else
		{
			magnitude_buff[index] = 0;
		}

	}
#endif // NEON
#ifdef DEBUG
	printf("noise suppression is done \n");
#endif // DEBUG
}

/* \brief Subtract one frame from accumulator
 * \param[in] accumulator	pointer to accumulator
 * \param[in] frame			pointer to frame to be subtracted
 */
void subtract_frame_from_acc(u16 *accumulator, u16 *frame) // NEON optimized
{
	u32 index;
	for (index = 0; index < N; index++)
	{
		accumulator[index] -= frame[index];
	}
#ifdef DEBUG
	printf("Subtracted a frame from accumulator \n");
#endif // DEBUG
}

/* \ brief Add one magnitude frame to accumulator
 * \param[in] accumulator	pointer to accumulator
 * \param[in] frame 		pointer to frame to be added
 */
void add_frame_to_acc(u16 *accumulator, u16 *frame)
{

	u32 index;
	for (index = 0; index < N; index++)
	{
		accumulator[index] += frame[index];
	}
#ifdef DEBUG
	printf("Added new frame magnitude to accumulator \n");
#endif // DEBUG

}

/* \brief Average and scale down to fit into OLED
 * \param[in] accumulator 	pointer to accumulator
 * \param[in] dest_buff		pointer to buffer where final output is stored
 */
void average_acc(u16 *accumulator, u8 *dest_buff)
{
	u32 index;
	for (index = 0; index < N; index++)
	{
		// Scale down before converting into 8 bit number
		dest_buff[index] = (u8)((accumulator[index]) >> 10);

	}
#ifdef DEBUG
	printf("Averaging is done\n");
#endif // DEBUG
}
