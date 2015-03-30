/* \file fft_128.c
* \author Gopalakrishna Hegde, Kiran G B
* \date 21 Aug 2014
* \brief This file contains 128 point floating point FFT implementation
*/

#include "fft_128.h"
#include "utils.h"


const float WkNReal[N/2] = {1.000000, 0.998795, 0.995185, 0.989177, 0.980785, 0.970031, 0.956940, 0.941544, 0.923880,
							0.903989, 0.881921, 0.857729, 0.831470, 0.803208, 0.773010, 0.740951, 0.707107, 0.671559,
							0.634393, 0.595699, 0.555570, 0.514103, 0.471397, 0.427555, 0.382683, 0.336890, 0.290285,
							0.242980, 0.195090, 0.146730, 0.098017, 0.049068, 0.000000, -0.049068, -0.098017, -0.146730,
							-0.195090, -0.242980, -0.290285, -0.336890, -0.382683, -0.427555, -0.471397, -0.514103, 
							-0.555570, -0.595699, -0.634393, -0.671559, -0.707107, -0.740951, -0.773010, -0.803208,
							-0.831470, -0.857729, -0.881921, -0.903989, -0.923880, -0.941544, -0.956940, -0.970031,
							-0.980785, -0.989177, -0.995185, -0.998795};
const float WkNImg[N/2] = {	0.000000, -0.049068, -0.098017, -0.146730, -0.195090, -0.242980, -0.290285, -0.336890, -0.382683, 
						   -0.427555, -0.471397, -0.514103, -0.555570, -0.595699, -0.634393, -0.671559, -0.707107, -0.740951,
						   -0.773010, -0.803208, -0.831470, -0.857729, -0.881921, -0.903989, -0.923880, -0.941544, -0.956940,
						   -0.970031, -0.980785, -0.989177, -0.995185, -0.998795, -1.000000, -0.998795, -0.995185, -0.989177,
						   -0.980785, -0.970031, -0.956940, -0.941544, -0.923880, -0.903989, -0.881921, -0.857729, -0.831470,
						   -0.803208, -0.773010, -0.740951, -0.707107, -0.671559, -0.634393, -0.595699, -0.555570, -0.514103,
						   -0.471397, -0.427555, -0.382683, -0.336890, -0.290285, -0.242980, -0.195090, -0.146730, -0.098017, 
						   -0.049068};
/* \brief API to calculate 128 point Radix 2 FFT
 * \param[in] data_real		pointer to real data buffer
 * \param[in] data_img		pointer to imaginary data buffer
 */
void fft_128(float * __restrict data_real, float * __restrict data_img)
{
	u32 n,mul,stage;
	float Wr, Wi, temp_real, temp_img;


	// Arrange data in bit reversed order
	reorder_data_in_bit_reverse(data_real);

	for (stage = 1; stage <= S; stage++)
	{
		u32 pow_2_stage = 1 << stage;
		u32 pow_2_stage_1 = 1 << (stage -1);

		for (n = 0; n < (N & 0xF0); n += pow_2_stage)
		{
			for (mul = 0; mul < pow_2_stage_1; mul++)
			{
				Wr = WkNReal[mul * (N/pow_2_stage)];
				Wi = WkNImg[mul * (N/pow_2_stage)];
			
				temp_real = data_real[n+ pow_2_stage_1 + mul] * Wr - data_img[n+ pow_2_stage_1 + mul] * Wi;
				temp_img = data_real[n+ pow_2_stage_1 + mul] * Wi + data_img[n+ pow_2_stage_1 + mul] * Wr;
			
				data_real[n+ pow_2_stage_1 + mul] = data_real[n + mul] - temp_real;
				data_img[n+ pow_2_stage_1 + mul] = data_img[n + mul] - temp_img;

				data_real[n + mul] = data_real[n + mul] + temp_real;
				data_img[n + mul] = data_img[n + mul] + temp_img;
			}		
		}		
	}
}