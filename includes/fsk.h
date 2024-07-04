#ifndef FSK_H
#define FSK_H

#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define PI 3.14159265
#define FC0 4000
#define FC1 5200
#define BR 1200
#define TB 1.0 / BR
#define BITS_LENGTH 10
#define T_LENGTH 1000

#define FREQ_CENTER 2210
#define BAUD_RATE 2400
#define DEVIATION (BAUD_RATE - 400)
#define FREQ_LOW (FREQ_CENTER - (DEVIATION/2))
#define FREQ_HIGH (FREQ_CENTER + (DEVIATION/2))
#define MODULATION_SAMPLE_RATE 48000
#define MODULATION_INTERPOLATION (MODULATION_SAMPLE_RATE / BAUD_RATE)
#define SAMPS_PER_BUFF (2040)
#define BITS_PER_BUFF (SAMPS_PER_BUFF / MODULATION_INTERPOLATION)
#define RF_SAMP_RATE 1200000
#define RF_INTERPOLATION (RF_SAMP_RATE/(BAUD_RATE*MODULATION_INTERPOLATION)) 



#define LENGTH_DATA 256

typedef struct vco{
    float samp_rate;
    float sensitivity;
    float amplitude;
    float k;
    double phase;
} s_vco;

void initVco(s_vco *vco, float samp_rate, float sensitivity, float amplitude, double phase){
    vco->samp_rate = samp_rate;
    vco->sensitivity = sensitivity;
    vco->amplitude = amplitude;
    vco->k = sensitivity/samp_rate;
    vco->phase = phase;
}

void adjustPhase(s_vco *vco, float delta_phase){
    vco->phase += delta_phase;
    //printf("Phase: %f\n", vco->phase);
    if(fabs(vco->phase) > PI){
        while(vco->phase > PI){
            vco->phase -= 2*PI;
        }
        while(vco->phase < -PI){
            vco->phase += 2*PI;
        }
    }
}

void get_sincos(s_vco *vco, float *input, int noutput_items, float *i_out, float *q_out){
    for(int i = 0; i < noutput_items; i++){
        i_out[i] = cos(vco->phase);
        q_out[i] = sin(vco->phase);
        adjustPhase(vco, vco->k*(input[i]));
    }
}


int repeat(uint8_t *input, int input_length, int interpolation, float *output) {
    if (input == NULL || output == NULL || input_length <= 0 || interpolation <= 0) {
        return -1; // Return an error code if input is invalid
    }

    int output_index = 0; // Initialize the output index
    for (int i = 0; i < input_length; i++) {
        for (int bit = 7; bit >= 0; bit--) {  // Process bits from the most significant to the least significant
            int bit_value = (input[i] >> bit) & 0x01;
            for (int j = 0; j < interpolation; j++) {
                output[output_index++] = (float)bit_value;
            }
        }
    }

    return 0;
}


int repeat_f(float *input, int input_length, int interpolation, float *output){
    for(int i = 0; i < input_length; i++){
        for(int j = 0; j < interpolation; j++){
            output[i*interpolation+j] = input[i];
        }
    }

    return 0;
        
}


void vco(s_vco *vco, float *input, int length, float *i_out, float *q_out){
    get_sincos(vco, input, length, i_out, q_out);
}


void interleave(float *i_out, float *q_out, float *output, int length){
    for(int i = 0; i < length; i++){
        output[2*i] = i_out[i];
        output[2*i+1] = q_out[i];
    }
}

void fskModulate(s_vco *vco_config, uint8_t *input_data, size_t len_input_data, float **samples, size_t *samples_len){
    float input_data_interpolated[LENGTH_DATA * 8 * MODULATION_INTERPOLATION];

    if(repeat(input_data, len_input_data, MODULATION_INTERPOLATION, input_data_interpolated) < 0) {
        printf("Error\n");
        return;
    }

    float i_out[LENGTH_DATA * 8 * MODULATION_INTERPOLATION];
    float q_out[LENGTH_DATA * 8 * MODULATION_INTERPOLATION];

    vco(vco_config, input_data_interpolated, len_input_data * 8 * MODULATION_INTERPOLATION, i_out, q_out);

    int rf_interpolated_length = len_input_data * 8 * MODULATION_INTERPOLATION * RF_INTERPOLATION;
    float *i_out2 = malloc(rf_interpolated_length * sizeof(float));
    float *q_out2 = malloc(rf_interpolated_length * sizeof(float));

    repeat_f(i_out, len_input_data * 8 * MODULATION_INTERPOLATION, RF_INTERPOLATION, i_out2);
    repeat_f(q_out, len_input_data * 8 * MODULATION_INTERPOLATION, RF_INTERPOLATION, q_out2);

    *samples_len = len_input_data * 8 * MODULATION_INTERPOLATION * 2 * RF_INTERPOLATION;
    *samples = malloc(*samples_len * sizeof(float));

    if (*samples == NULL) {
        printf("Error allocating memory for samples\n");
        free(i_out2);
        free(q_out2);
        return;
    }

    interleave(i_out2, q_out2, *samples, rf_interpolated_length);

    free(i_out2);
    free(q_out2);
}


#endif