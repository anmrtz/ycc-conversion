#include <arm_neon.h>
#include <stdio.h>

#include "cc.h"

#define VECTOR_WIDTH 8

static inline uint8_t avg2(uint8_t e1, uint8_t e2) {
    return (e1 + e2) >> 1;
}

static inline uint8_t avg4(uint8_t e1, uint8_t e2, uint8_t e3, uint8_t e4) {
    return (e1 + e2 + e3 + e4) >> 2;
}

// Vectorized pixel conversion 32 bits
#define VCONV_STEP_32(covec1, coidx1, covec2, coidx2, covec3, coidx3, skewvec, resmax, outbuff) ({\
    temp_low = vmull_lane_s16(rgb_low.val[0], covec1, coidx1);\
    temp_high = vmull_lane_s16(rgb_high.val[0], covec1, coidx1);\
    temp_low = vmlal_lane_s16(temp_low, rgb_low.val[1], covec2, coidx2);\
    temp_high = vmlal_lane_s16(temp_high, rgb_high.val[1], covec2, coidx2);\
    temp_low = vmlal_lane_s16(temp_low, rgb_low.val[2], covec3, coidx3);\
    temp_high = vmlal_lane_s16(temp_high, rgb_high.val[2], covec3, coidx3);\
    temp_low = vshrq_n_s32(temp_low, 8);\
    temp_high = vshrq_n_s32(temp_high, 8);\
    temp_low = vaddq_s32(temp_low, skewvec);\
    temp_high = vaddq_s32(temp_high, skewvec);\
    temp_low = vminq_s32(temp_low, resmax);\
    temp_high = vminq_s32(temp_high, resmax);\
    temp_low = vmaxq_s32(temp_low, val_16);\
    temp_high = vmaxq_s32(temp_high, val_16);\
    \
    outbuff = vreinterpret_u8_s8(vmovn_s16(vcombine_s16(vmovn_s32(temp_low), vmovn_s32(temp_high))));\
})
/*
static void convert_pixels_32(uint8_t* rgb_data, uint32_t num_pixels) {
    uint8x8x3_t buff;

    int32x4_t temp_low, temp_high;
    int16x4x3_t rgb_low, rgb_high;

    int16x4_t coeff1;
    int16x4_t coeff2;

    coeff1[0] = 66;
    coeff1[1] = 129;
    coeff1[2] = 25;

    coeff1[3] = -38;
    coeff2[0] = -74;
    coeff2[1] = 112;

    coeff2[2] = -94;
    coeff2[3] = -18;

    int32x4_t val_16 = vdupq_n_s32(16);
    int32x4_t val_128 = vdupq_n_s32(128);

    int32x4_t val_y_max = vdupq_n_s32(Y_MAX_VAL);
    int32x4_t val_c_max = vdupq_n_s32(C_MAX_VAL);

    __builtin_prefetch(rgb_data);

    for (uint32_t pixel = 0; pixel < num_pixels; pixel += 8) {
        buff = vld3_u8(rgb_data+pixel*3);

        rgb_low.val[0] = vget_low_s16(vreinterpretq_s16_u16(vmovl_u8(buff.val[0])));
        rgb_low.val[1] = vget_low_s16(vreinterpretq_s16_u16(vmovl_u8(buff.val[1])));
        rgb_low.val[2] = vget_low_s16(vreinterpretq_s16_u16(vmovl_u8(buff.val[2])));
        rgb_high.val[0] = vget_high_s16(vreinterpretq_s16_u16(vmovl_u8(buff.val[0])));
        rgb_high.val[1] = vget_high_s16(vreinterpretq_s16_u16(vmovl_u8(buff.val[1])));
        rgb_high.val[2] = vget_high_s16(vreinterpretq_s16_u16(vmovl_u8(buff.val[2])));

        // R pixels
        VCONV_STEP_32(coeff1, 0, coeff1, 1, coeff1, 2, val_16, val_y_max, buff.val[0]);
    
        // G pixels
        VCONV_STEP_32(coeff1, 3, coeff2, 0, coeff2, 1, val_128, val_c_max, buff.val[1]);

        // B pixels
        VCONV_STEP_32(coeff2, 1, coeff2, 2, coeff2, 3, val_128, val_c_max, buff.val[2]);

        vst3_u8(rgb_data+pixel*3, buff);
    }
}
*/
static void convert_pixels_16(uint8_t* rgb_data, uint32_t num_pixels) {
    uint8x8x3_t buff;

    int16x8x3_t rgb;
    int16x8x3_t temp;

    int16x4_t coeff1;
    int16x4_t coeff2;

    coeff1[0] = 33;
    coeff1[1] = 65;
    coeff1[2] = 13;

    coeff1[3] = -19;
    coeff2[0] = -37;
    coeff2[1] = 56;

    coeff2[2] = -47;
    coeff2[3] = -9;

    int16x8_t val_16 = vdupq_n_s16(16);
    int16x8_t val_128 = vdupq_n_s16(128);

    int16x8_t val_y_max = vdupq_n_s16(Y_MAX_VAL);
    int16x8_t val_c_max = vdupq_n_s16(C_MAX_VAL);

    for (uint32_t pixel = 0; pixel < num_pixels; pixel += 8) {
        buff = vld3_u8(rgb_data+pixel*3);

        rgb.val[0] = vreinterpretq_s16_u16(vmovl_u8(buff.val[0]));
        rgb.val[1] = vreinterpretq_s16_u16(vmovl_u8(buff.val[1]));
        rgb.val[2] = vreinterpretq_s16_u16(vmovl_u8(buff.val[2]));

        temp.val[0] = vmulq_lane_s16(rgb.val[0], coeff1, 0);
        temp.val[1] = vmulq_lane_s16(rgb.val[0], coeff1, 3);
        temp.val[2] = vmulq_lane_s16(rgb.val[0], coeff2, 1);

        temp.val[0] = vmlaq_lane_s16(temp.val[0], rgb.val[1], coeff1, 1);
        temp.val[1] = vmlaq_lane_s16(temp.val[1], rgb.val[1], coeff2, 0);
        temp.val[2] = vmlaq_lane_s16(temp.val[2], rgb.val[1], coeff2, 2);

        temp.val[0] = vmlaq_lane_s16(temp.val[0], rgb.val[2], coeff1, 2);
        temp.val[1] = vmlaq_lane_s16(temp.val[1], rgb.val[2], coeff2, 1);
        temp.val[2] = vmlaq_lane_s16(temp.val[2], rgb.val[2], coeff2, 3);

        temp.val[0] = vshrq_n_s16(temp.val[0], 7);
        temp.val[1] = vshrq_n_s16(temp.val[1], 7);
        temp.val[2] = vshrq_n_s16(temp.val[2], 7);

        temp.val[0] = vaddq_s16(temp.val[0], val_16);
        temp.val[1] = vaddq_s16(temp.val[1], val_128);
        temp.val[2] = vaddq_s16(temp.val[2], val_128);

        temp.val[0] = vminq_s16(temp.val[0], val_y_max);
        temp.val[1] = vminq_s16(temp.val[1], val_c_max);
        temp.val[2] = vminq_s16(temp.val[2], val_c_max);

        temp.val[0] = vmaxq_s16(temp.val[0], val_16);    
        temp.val[1] = vmaxq_s16(temp.val[1], val_16);
        temp.val[2] = vmaxq_s16(temp.val[2], val_16);

        buff.val[0] = vreinterpret_u8_s8(vmovn_s16(temp.val[0]));
        buff.val[1] = vreinterpret_u8_s8(vmovn_s16(temp.val[1]));
        buff.val[2] = vreinterpret_u8_s8(vmovn_s16(temp.val[2]));

        vst3_u8(rgb_data+pixel*3, buff);
    }
}

void cc_vector(uint8_t* rgb_data, uint32_t rgb_width, uint32_t rgb_height, uint8_t* ycc_data) {
    convert_pixels_16(rgb_data, rgb_height*rgb_width);
/*
    uint8x8x3_t buff_top;
    uint8x8x3_t buff_bottom;

    uint8_t top_row_temp[24];
    uint8_t bottom_row_temp[24];

    // Downsample converted pixels
    for (uint32_t row = 0; row < rgb_height; row += 2) {
        for (uint32_t col = 0; col < rgb_width; col += 8) {
            // Destination luma sections
            const uint32_t ycc_y_top_idx = (col + (row * rgb_width));
            const uint32_t ycc_y_bottom_idx = (col + ((row+1) * rgb_width));

            // Destination chroma sections
            const uint32_t ycc_cb_top_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
            const uint32_t ycc_cr_top_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
            const uint32_t ycc_cb_bottom_idx = (rgb_width * rgb_height) + (col >> 1) + ((row+1) >> 1)*(rgb_width >> 1);
            const uint32_t ycc_cr_bottom_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + ((row+1) >> 1)*(rgb_width >> 1);

            // Base source pixel
            const uint32_t pixel_top = row*col;
            const uint32_t pixel_bottom = (row+1)*col;

            memcpy(top_row_temp, rgb_data+pixel_top*3, 24);

            // Full row?
            buff_top = vld3_u8(rgb_data+pixel_top*3);

        }
    }
*/

    // Downsample converted pixels
    for (uint32_t row = 0; row < rgb_height; ++row) {
        for (uint32_t col = 0; col < rgb_width; ++col) {
            const uint32_t ycc_y_idx = (col + (row * rgb_width));
            const uint32_t idx = 3 * ycc_y_idx;

            ycc_data[ycc_y_idx] = rgb_data[idx];
            uint8_t* g = rgb_data+(idx+1);
            uint8_t* b = rgb_data+(idx+2);  

            // Process 4x4 clusters for downsampling
            if ((row % 2 == 1) && (col % 2 == 1)) {
                const uint32_t left_idx = idx - 3;
                const uint32_t up_idx = idx - (rgb_width*3);
                const uint32_t up_left_idx = (idx - 3) - (rgb_width*3);

                *g = avg4(*g, rgb_data[left_idx+1], rgb_data[up_idx+1], rgb_data[up_left_idx+1]);
                *b = avg4(*b, rgb_data[left_idx+2], rgb_data[up_idx+2], rgb_data[up_left_idx+2]);

                // Write downsampled chroma plane values to output array
                const uint32_t ycc_cb_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
                const uint32_t ycc_cr_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);

                ycc_data[ycc_cb_idx] = *g; // Cb
                ycc_data[ycc_cr_idx] = *b; // Cr
            }
            // Process 1x2 terminal row clusters
            else if ((row == rgb_height - 1) && (col % 2 == 1)) {
                const uint32_t left_idx = idx - 3;

                *g = avg2(*g, rgb_data[left_idx+1]);
                *b = avg2(*b, rgb_data[left_idx+2]);

                // Write downsampled chroma plane values to output array
                const uint32_t ycc_cb_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
                const uint32_t ycc_cr_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);

                ycc_data[ycc_cb_idx] = *g; // Cb
                ycc_data[ycc_cr_idx] = *b; // Cr
            }
            // Process 2x1 terminal column clusters
            else if ((col == rgb_width - 1) && (row % 2 == 1)) {
                const uint32_t up_idx = idx - (rgb_width*3);

                *g = avg2(*g, rgb_data[up_idx+1]);
                *b = avg2(*b, rgb_data[up_idx+2]);

                // Write downsampled chroma plane values to output array
                const uint32_t ycc_cb_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
                const uint32_t ycc_cr_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);

                ycc_data[ycc_cb_idx] = *g; // Cb
                ycc_data[ycc_cr_idx] = *b; // Cr
           }
           // Process terminal corner pixel
           else if (((row == rgb_height - 1) && (col == rgb_width - 1)) && ((row % 2 == 0) && (col % 2 == 0))) {
                // Write downsampled chroma plane values to output array
                const uint32_t ycc_cb_idx = (rgb_width * rgb_height) + (col >> 1) + (row >> 1)*(rgb_width >> 1);
                const uint32_t ycc_cr_idx = (rgb_width * rgb_height) + ((rgb_width*rgb_height) >> 2) + (col >> 1) + (row >> 1)*(rgb_width >> 1);

                ycc_data[ycc_cb_idx] = *g; // Cb
                ycc_data[ycc_cr_idx] = *b; // Cr
           }
        }
    }

}
