
#ifndef __UTILS_H__
#define __UTILS_H__

#include "oled.h"

#define FRAMES_IN_WINDOW	8
#define	N	128
#define S	7

extern u32 noise_threshold;

u32 bit_reverse(u32 number);

void reorder_data_in_bit_reverse(float *data);

void fft_magnitude(float * fft_real, float *fft_img, unsigned short *mag_buff);

void subtract_frame_from_acc(u16 *accumulator, u16 *frame);

void add_frame_to_acc(u16 *accumulator, u16 *frame);

void average_acc(u16 *accumulator, u8 *dest_buff);

void suppress_noise(u8 *magnitude_buff);

void system_init(void);

#endif // __UTILS_H__
