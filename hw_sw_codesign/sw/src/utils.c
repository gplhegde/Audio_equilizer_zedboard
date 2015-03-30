/* \filename utils.c
 * \author Gopalakrishna Hegde
 * \date 25 Oct 2014
 */

#include <stdio.h>
#include "utils.h"
#include "fft_128.h"
#include "audio.h"
#include "oled.h"
#include "arm_neon.h"
#include "fft_dma.h"
#include "xil_mmu.h"

#define UPPER_THRESHOLD 40

// comment below software switch to turnoff all hand coded NEON optimization
#define NEON_ON

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
	DMA_Congfig(DMA_DEV_ID);
	//Disable cache on OCM
	Xil_SetTlbAttributes(0x00000000,0x14de2);

}

/* \brief Calculate square of the magnitude of FFT coefficients
 *
 * param[in] fft_real	pointer to real part of FFT
 * param[in] fft_img	pointer to imaginary part of the FFT
 * param[in] mag_buff	pointer to output magnitude buffer
 */
void fft_magnitude(s16 * __restrict fft_real, s16 * __restrict fft_img, u16 * __restrict mag_buff)
{
	u32 index;

#ifdef NEON_ON
	int16x8_t real_vec, img_vec;
	uint16x8_t mag_vec;
	for (index = 0; index < (N/2); index+=16)
	{
		real_vec = vld1q_s16(fft_real+index);
		img_vec = vld1q_s16(fft_img+index);
		mag_vec  = vaddq_u16((uint16x8_t)vmulq_s16(real_vec, real_vec) , (uint16x8_t)vmulq_s16(img_vec, img_vec));
		vst1q_u16(mag_buff+index, mag_vec);

		real_vec = vld1q_s16(fft_real+index+8);
		img_vec = vld1q_s16(fft_img+index+8);
		mag_vec  = vaddq_u16((uint16x8_t)vmulq_s16(real_vec, real_vec) , (uint16x8_t)vmulq_s16(img_vec, img_vec));
		vst1q_u16(mag_buff+index+8, mag_vec);
	}
#else
	for (index = 0; index < (N/2); index++)
	{
		// square of the magnitude is stored in fftReal Array.
		mag_buff[index] = (u16)(((u16)(fft_real[index] * fft_real[index] )+ (u16)(fft_img[index] * fft_img[index])));
	}
#endif // NEON_ON
}

/* \brief suppress noise by thresholding
 * \param[in] magnitude_buff	pointer to FFT magnitude buffer *
 */
void suppress_noise(u8 *magnitude_buff)
{
	u32 index;

#ifdef NEON_ON
	uint8x16_t mag_vec, lower_th, upper_th, cmp_res;
	upper_th = vdupq_n_u8(UPPER_THRESHOLD);
	for (index = 0; index < N/2; index+=16)
	{
		// This logic is little tricky.... have to scratch head a little...:P
		mag_vec = vld1q_u8(magnitude_buff + index);
		lower_th = vld1q_u8(noise_th_buff + index);

		cmp_res = vcgeq_u8(mag_vec, lower_th);
		mag_vec = vandq_u8(mag_vec, cmp_res); // else case

		cmp_res = vandq_u8(cmp_res, lower_th);
		mag_vec = vsubq_u8(mag_vec, cmp_res);

		cmp_res = vcgtq_u8(mag_vec, upper_th);
		mag_vec = vaddq_u8(vandq_u8(cmp_res, upper_th), vandq_u8(mag_vec, vmvnq_u8(cmp_res)));
		vst1q_u8((magnitude_buff + index), mag_vec);
	}
#else
	for(index = 0; index < (N/2); index++)
	{
		if (magnitude_buff[index] >= noise_th_buff[index])
		{
			magnitude_buff[index] -= noise_th_buff[index];
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
#endif // NEON_ON

}

/* \brief Subtract one frame from accumulator
 * \param[in] accumulator	pointer to accumulator
 * \param[in] frame			pointer to frame to be subtracted
 */
void subtract_frame_from_acc(u16 *accumulator, u16 *frame) // NEON optimized
{
	u32 index;
#ifdef NEON_ON
	uint16x8_t acc_vec, frame_vec;
	for (index = 0; index < N/2; index += 16)
	{
		acc_vec = vld1q_u16(accumulator + index);
		frame_vec = vld1q_u16(frame + index);
		acc_vec = vsubq_u16(acc_vec, frame_vec);
		vst1q_u16 (accumulator + index, acc_vec);

		// unroll by 2
		acc_vec = vld1q_u16(accumulator + index + 8);
		frame_vec = vld1q_u16(frame + index + 8);
		acc_vec = vsubq_u16(acc_vec, frame_vec);
		vst1q_u16 (accumulator + index + 8, acc_vec);
	}

#else
	for (index = 0; index < N/2; index++)
	{
		accumulator[index] -= frame[index];
	}
#endif // NEON
}

/* \ brief Add one magnitude frame to accumulator
 * \param[in] accumulator	pointer to accumulator
 * \param[in] frame 		pointer to frame to be added
 */
void add_frame_to_acc(u16 *accumulator, u16 *frame)
{
	u32 index;
#ifdef NEON_ON
	uint16x8_t acc_vec, frame_vec;
	for (index = 0; index < N/2; index += 16)
	{
		acc_vec = vld1q_u16(accumulator + index);
		frame_vec = vld1q_u16(frame + index);
		acc_vec = vaddq_u16(acc_vec, frame_vec);
		vst1q_u16 (accumulator + index, acc_vec);

		// unroll by 2
		acc_vec = vld1q_u16(accumulator + index + 8);
		frame_vec = vld1q_u16(frame + index + 8);
		acc_vec = vaddq_u16(acc_vec, frame_vec);
		vst1q_u16 (accumulator + index + 8, acc_vec);
	}
#else
	for (index = 0; index < N/2; index++)
	{
		accumulator[index] += frame[index];
	}
#endif // NEON_ON
}

/* \brief Average and scale down to fit into OLED
 * \param[in] accumulator 	pointer to accumulator
 * \param[in] dest_buff		pointer to buffer where final output is stored
 */
void average_acc(u16 *accumulator, u8 *dest_buff)
{
	u32 index;
#ifdef NEON_ON
	uint16x8_t acc_vec;
	uint8x8_t dest_buff_vec;
	for (index = 0; index < N/2; index += 16)
	{
		acc_vec = vld1q_u16(accumulator + index);
		dest_buff_vec = vshrn_n_u16(acc_vec, 3);
		//dest_buff_vec = vshr_n_u8(dest_buff_vec, 2);
		vst1_u8(dest_buff + index, dest_buff_vec);

		// unroll by 2
		acc_vec = vld1q_u16(accumulator + index + 8);
		dest_buff_vec = vshrn_n_u16(acc_vec, 3);
		//dest_buff_vec = vshr_n_u8(dest_buff_vec, 2);
		vst1_u8(dest_buff + index + 8, dest_buff_vec);
	}
#else
	for (index = 0; index < N/2; index++)
	{
		// Scale down before converting into 8 bit number
		dest_buff[index] = (u8)((accumulator[index]) >> 3);

	}
#endif // NEON_ON

}
