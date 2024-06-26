#include "../includes/fsk.h"
#include <math.h>
#include <stdlib.h>

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>





#define MAIN


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




#include "../includes/testSpino.h"



void interleave(float *i_out, float *q_out, float *output, int length){
    for(int i = 0; i < length; i++){
        output[2*i] = i_out[i];
        output[2*i+1] = q_out[i];
    }
}



void transmitTestSpino(float* samples, size_t *samples_len){
    s_vco vco_config;
    float center_freq = 0;
    float deviation = 2000;
    float vco_max = center_freq + deviation;
    float amplitude = 1;

    float sensitivity = 2*PI*vco_max;

    
    initVco(&vco_config, MODULATION_SAMPLE_RATE, sensitivity, amplitude, 0);

    uint8_t input_data[LENGTH_DATA];
    int frame_size;
    spinoSendTC(input_data, &frame_size);  

    float input_data_interpolated[LENGTH_DATA*8*MODULATION_INTERPOLATION];
    
    if(repeat(input_data, frame_size, MODULATION_INTERPOLATION, input_data_interpolated)<0){
        printf("Error\n");
    }
    float i_out[LENGTH_DATA*8*MODULATION_INTERPOLATION];
    float q_out[LENGTH_DATA*8*MODULATION_INTERPOLATION];

    vco(&vco_config, input_data_interpolated, frame_size*8*MODULATION_INTERPOLATION, i_out, q_out);

    int rf_interpolated_length = frame_size * 8 * MODULATION_INTERPOLATION * RF_INTERPOLATION;
    float *i_out2 = malloc(rf_interpolated_length * sizeof(float));
    float *q_out2 = malloc(rf_interpolated_length * sizeof(float));

    repeat_f(i_out, frame_size*8*MODULATION_INTERPOLATION, RF_INTERPOLATION, i_out2);
    repeat_f(q_out, frame_size*8*MODULATION_INTERPOLATION, RF_INTERPOLATION, q_out2);

    float signal_out[frame_size*8*MODULATION_INTERPOLATION*2*RF_INTERPOLATION];
    int nb = 0;
    int rf_int = 0;
    interleave(i_out2, q_out2, signal_out, frame_size*8*MODULATION_INTERPOLATION*RF_INTERPOLATION);
    samples = signal_out;
    *samples_len = frame_size*8*MODULATION_INTERPOLATION*RF_INTERPOLATION;
}

#ifdef MAIN6
#include "uhd.h"
int main(void){

    double gain = 35;
    
    size_t channel = 0;
    double rate = 1.2e6;
    double freq = 145830000;
    uint64_t total_num_samps = 4;
    bool verbose = true;    

    size_t samps_per_buff;
    float *output_data = NULL;
    const void** output_data_ptr = NULL;
    const void** signal_out_ptr = NULL;
    //float complex output_data[LENGTH_DATA * 8 * SAMPLES_PER_BIT];
    //modulateFSK2(input_data, LENGTH_DATA, output_data);

    uhd_usrp_handle usrp;
    uhd_tx_streamer_handle tx_streamer;
    uhd_tx_metadata_handle md;
    uhd_tune_request_t tune_request = { .target_freq = freq,
            .rf_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO,
            .dsp_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO };
    uhd_tune_result_t tune_result;
    uhd_stream_args_t stream_args = { .cpu_format = "fc32",
        .otw_format = "sc16",
        .args = "",
        .channel_list = &channel,
        .n_channels = 1 };

    uhd_usrp_make(&usrp, "type=b200");
    uhd_tx_streamer_make(&tx_streamer);
    uhd_tx_metadata_make(&md, false, 0, 0.1, true, false);

    uhd_usrp_set_tx_rate(usrp, rate, 0);
    uhd_usrp_get_tx_rate(usrp, channel, &rate);
    fprintf(stderr, "Actual TX Rate: %f...\n\n", rate);

    uhd_usrp_set_tx_gain(usrp, gain, 0, "");
    uhd_usrp_get_tx_gain(usrp, channel, "", &gain);
    fprintf(stderr, "Actual TX Gain: %f...\n", gain);

    uhd_usrp_set_tx_freq(usrp, &tune_request, 0, &tune_result);
    uhd_usrp_get_tx_freq(usrp, channel, &freq);
    fprintf(stderr, "Actual TX frequency: %f MHz...\n", freq / 1e6);

    // Set up streamer
    stream_args.channel_list = &channel;
    uhd_usrp_get_tx_stream(usrp, &stream_args, tx_streamer);

    uhd_tx_streamer_max_num_samps(tx_streamer, &samps_per_buff);
    fprintf(stderr, "Buffer size in samples: %zu\n", samps_per_buff);
    output_data = calloc(sizeof(float), samps_per_buff * 2);
    output_data_ptr = (const void **)&output_data;

    s_vco vco_config;
    float center_freq = 0;
    float deviation = 2000;
    float vco_max = center_freq + deviation;
    float amplitude = 1;

    float sensitivity = 2*PI*vco_max;

    
    initVco(&vco_config, MODULATION_SAMPLE_RATE, sensitivity, amplitude, 0);
    


    uint8_t input_data[LENGTH_DATA];
    int frame_size;
    spinoSendTC(input_data, &frame_size);  
    float input_data_interpolated[LENGTH_DATA*8*MODULATION_INTERPOLATION];
    
    if(repeat(input_data, frame_size, MODULATION_INTERPOLATION, input_data_interpolated)<0){
        printf("Error\n");
    }
    float i_out[LENGTH_DATA*8*MODULATION_INTERPOLATION];
    float q_out[LENGTH_DATA*8*MODULATION_INTERPOLATION];

    vco(&vco_config, input_data_interpolated, frame_size*8*MODULATION_INTERPOLATION, i_out, q_out);

    int rf_interpolated_length = frame_size * 8 * MODULATION_INTERPOLATION * RF_INTERPOLATION;
    float *i_out2 = malloc(rf_interpolated_length * sizeof(float));
    float *q_out2 = malloc(rf_interpolated_length * sizeof(float));

    repeat_f(i_out, frame_size*8*MODULATION_INTERPOLATION, RF_INTERPOLATION, i_out2);
    repeat_f(q_out, frame_size*8*MODULATION_INTERPOLATION, RF_INTERPOLATION, q_out2);

    float signal_out[frame_size*8*MODULATION_INTERPOLATION*2*RF_INTERPOLATION];
    int nb = 0;
    int rf_int = 0;
    interleave(i_out2, q_out2, signal_out, frame_size*8*MODULATION_INTERPOLATION*RF_INTERPOLATION);

    signal_out_ptr = (const void **)&signal_out;

    int num_samps = frame_size * 8 * MODULATION_INTERPOLATION * RF_INTERPOLATION;
    size_t sent_samps;
    float number_of_buffers = (frame_size * 8)/BITS_PER_BUFF;
    if(number_of_buffers - (int)number_of_buffers > 0){
        number_of_buffers = (int)number_of_buffers + 1;
    }

    // while (num_samps > 0) {
    //     //sent_samps = (num_samps < samps_per_buff) ? num_samps : samps_per_buff;
    //     uhd_tx_streamer_send(tx_streamer, output_data_ptr,samps_per_buff , &md, 0.1, &sent_samps);
    //     num_samps -= (int)sent_samps;
    //     printf("Sent %zu samples\n", sent_samps);
    //     printf("Remaining %d samples\n", num_samps);
    // }
    // for(int rep = 0; rep < 20; rep++){
    //     for(int i=0; i < (int)(number_of_buffers); i++){
    //         for(int j=0; j<samps_per_buff && ((j * (i * samps_per_buff)) < frame_size*8*MODULATION_INTERPOLATION*RF_INTERPOLATION*2); j++){
    //             output_data[j] = signal_out[j * (i * samps_per_buff)];
    //         }
    //         uhd_tx_streamer_send(tx_streamer, output_data_ptr, samps_per_buff, &md, 0.1, &sent_samps);
    //         num_samps -= sent_samps;
    //     }
    // }
    
    for(int rep=0; rep<5; rep++){
        send_samples(tx_streamer, &md, signal_out, frame_size*8*MODULATION_INTERPOLATION*RF_INTERPOLATION, samps_per_buff);
    }

    return 0;
}



#endif

#ifdef TEST
#include "testSpino.h"

int main(void){

    uint8_t input_data[5] = {0x55, 0xaa, 0x55, 0xaa, 0x55};
    int frame_size;
   
    
    float input_data_interpolated[5*8*2];
    
    if(repeat(input_data, 5, 2, input_data_interpolated)<0){
        printf("Error\n");
    }

    for(int i=0; i<80; i++){
        printf(" ");
        printf("%.0f", input_data_interpolated[i]);
        
    }

    return 0;

}

#endif