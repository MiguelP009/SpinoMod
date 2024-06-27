#include <SoapySDR/Device.h>
  #include <SoapySDR/Formats.h>
  #include <stdio.h> //printf
  #include <stdlib.h> //free
  #include <complex.h>
  #include "../includes/fsk.h"



  #define MAX_DATA_LEN  255


int transmit_data(SoapySDRDevice *sdr, SoapySDRStream *tx_streamer, const float *samples, size_t samples_len, size_t samp_per_buf) {
    size_t num_sent = 0;
    int flags = 0;
    long long timeNs = 0;
    size_t total_sent = 0;
    size_t remaining = samples_len;

    while (remaining > 0) {
        size_t to_send = (remaining > samp_per_buf) ? samp_per_buf : remaining;
        const void *buffs[] = {samples + total_sent};

        printf("Sending %zu samples\n", to_send);
        num_sent = SoapySDRDevice_writeStream(sdr, tx_streamer, buffs, to_send, &flags, timeNs, 1000000);
        printf("to send: %zu\n", to_send);
        if (num_sent < 0) {
            printf("Failed to write to stream: %d\n", (int)num_sent);
            return -1;
        }

        total_sent += num_sent;
        remaining -= num_sent;

        printf("Total sent: %zu, Remaining: %zu\n", total_sent, remaining);
    }

    // Envoyer le signal de fin de transmission
    flags = SOAPY_SDR_END_BURST;
    const void *buffs_end[] = {NULL};
    printf("Sending end burst signal\n");

    //num_sent = SoapySDRDevice_writeStream(sdr, tx_streamer, buffs_end, 0, &flags, timeNs, 1000000);
    if (num_sent < 0) {
        printf("Failed to send end burst signal: %d\n", (int)num_sent);
        return -1;
    }

    return total_sent;
}


int send_samples(SoapySDRDevice *sdr, SoapySDRStream *tx_streamer, const float *samples, size_t samples_len, size_t samp_per_buf) {
    size_t total_sent = 0;
    int flags = 0;
    long long timeNs = 0;
    samples_len = 500000;
    
    while (total_sent < samples_len) {
        size_t to_send = (samples_len - total_sent) > samp_per_buf ? samp_per_buf : (samples_len - total_sent);
        const void *buffs[] = {samples + total_sent};

        int num_sent = SoapySDRDevice_writeStream(sdr, tx_streamer, buffs, to_send, &flags, timeNs, 1000000);
        printf("Sent %d samples\n", num_sent);
        if (num_sent < 0) {
            printf("Failed to write to stream: %d\n", num_sent);
            return -1;
        }

        total_sent += num_sent;
    }

    // Envoyer le signal de fin de transmission
    // flags = SOAPY_SDR_END_BURST;
    // const void *buffs_end[] = {NULL};
    // int num_sent = SoapySDRDevice_writeStream(sdr, tx_streamer, buffs_end, 0, &flags, timeNs, 1000000);
    // if (num_sent < 0) {
    //     printf("Failed to send end burst signal: %d\n", num_sent);
    //     return -1;
    // }

    return total_sent;
}



int main(void)
{
    size_t length;

    //enumerate devices
    SoapySDRKwargs *results = SoapySDRDevice_enumerate(NULL, &length);
    for (size_t i = 0; i < length; i++)
    {
        printf("Found device #%d: ", (int)i);
        for (size_t j = 0; j < results[i].size; j++)
        {
            printf("%s=%s, ", results[i].keys[j], results[i].vals[j]);
        }
        printf("\n");
    }

    int device = 0;
    if (length < 1){
        printf("No device found\n");
        return -1;
    }
    else if (length > 1){
        printf("Choose device: ");
        scanf("%d", &device);
    }

    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, results[device].keys[0], results[device].vals[0]);
    SoapySDRKwargsList_clear(results, length);
    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL)
    {
        printf("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
        return EXIT_FAILURE;
    }

    //query device info
    char** names = SoapySDRDevice_listAntennas(sdr, SOAPY_SDR_TX, 0, &length);
    printf("Tx antennas: ");
    for (size_t i = 0; i < length; i++) printf("%s, ", names[i]);
    printf("\n");
    SoapySDRStrings_clear(&names, length);

    names = SoapySDRDevice_listGains(sdr, SOAPY_SDR_TX, 0, &length);
    printf("Tx gains: ");
    for (size_t i = 0; i < length; i++) printf("%s, ", names[i]);
    printf("\n");
    SoapySDRStrings_clear(&names, length);

    SoapySDRRange *ranges = SoapySDRDevice_getFrequencyRange(sdr, SOAPY_SDR_TX, 0, &length);
    printf("Tx freq ranges: ");
    for (size_t i = 0; i < length; i++) printf("[%g Hz -> %g Hz], ", ranges[i].minimum, ranges[i].maximum);
    printf("\n");
    free(ranges);


    //apply settings
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, 1200000) != 0)
    {
        printf("setSampleRate fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, 145.830e6, NULL) != 0)
    {
        printf("setFrequency fail: %s\n", SoapySDRDevice_lastError());
    }

    //setup a stream (complex floats)
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CF32, NULL, 0, NULL);
    if (txStream == NULL)
    {
        printf("setupStream fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_unmake(sdr);
        return EXIT_FAILURE;
    }

    size_t samp_per_buf = SoapySDRDevice_getStreamMTU(sdr, txStream);

    //SoapySDRDevice_writeStream(sdr, txStream, (const void **)&buffs, 1024, NULL, 1000000, 0);

    float *samples;
    size_t samples_len = 0;

    transmitTestSpino(&samples, &samples_len);
    if (samples == NULL) {
        printf("Error: samples is NULL\n");
        return EXIT_FAILURE;
    }

    printf("First sample: %f, Total samples length: %zu\n", samples[0], samples_len);

    for (int i = 0; i < 5; i++) {
        if (send_samples(sdr, txStream, samples, samples_len, samp_per_buf) < 0) {
            printf("Error during transmission\n");
            break;
        }
    }

    // Free allocated memory for samples
    free(samples);

    // Cleanup
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0); // Deactivate the stream
    SoapySDRDevice_closeStream(sdr, txStream); // Close the stream
    SoapySDRDevice_unmake(sdr); // Unmake the device

    return EXIT_SUCCESS;
}
