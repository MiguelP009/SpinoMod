#ifndef SOAPYSDR_H
#define SOAPYSDR_H


#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <complex.h>

typedef struct sdr{
    SoapySDRDevice *device;
    SoapySDRStream *txStream;
    size_t samp_per_buf;
}s_sdr;

int selectSDRDevice(s_sdr *sdr){
    size_t length;

    // Enumerate devices
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
        return -1;
    }
    else if (length > 1){
        printf("Choose device: ");
        scanf("%d", &device);
    }
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, results[device].keys[0], results[device].vals[0]);
    SoapySDRKwargsList_clear(results, length);
    sdr->device = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);
    if (sdr->device == NULL)
    {
        printf("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
        return EXIT_FAILURE;
    }
    return 0;
}

int configSDR(s_sdr *sdr, double gain, double freq){
    size_t length;
    SoapySDRDevice *device = sdr->device;

    // Query device info
    char **names = SoapySDRDevice_listAntennas(device, SOAPY_SDR_TX, 0, &length);
    printf("Tx antennas: ");
    for (size_t i = 0; i < length; i++) printf("%s, ", names[i]);
    printf("\n");
    SoapySDRStrings_clear(&names, length);

    names = SoapySDRDevice_listGains(device, SOAPY_SDR_TX, 0, &length);
    printf("Tx gains: ");
    for (size_t i = 0; i < length; i++) printf("%s, ", names[i]);
    printf("\n");
    SoapySDRStrings_clear(&names, length);

    SoapySDRRange *ranges = SoapySDRDevice_getFrequencyRange(device, SOAPY_SDR_TX, 0, &length);
    printf("Tx freq ranges: ");
    for (size_t i = 0; i < length; i++) printf("[%g Hz -> %g Hz], ", ranges[i].minimum, ranges[i].maximum);
    printf("\n");
    free(ranges);

    // Apply settings
    if (SoapySDRDevice_setSampleRate(device, SOAPY_SDR_TX, 0, 1200000) != 0)
    {
        printf("setSampleRate fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(device, SOAPY_SDR_TX, 0, freq, NULL) != 0)
    {
        printf("setFrequency fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setGain(device, SOAPY_SDR_TX, 0, gain))
    {
        printf("setGain fail: %s\n", SoapySDRDevice_lastError());

    }
    double gainRes = SoapySDRDevice_getGain(device, SOAPY_SDR_TX, 0);
    printf("Gain (dB) : %f\n", gainRes);


    // Setup a stream (complex floats)
    sdr->txStream = SoapySDRDevice_setupStream(device, SOAPY_SDR_TX, SOAPY_SDR_CF32, NULL, 0, NULL);
    if (sdr->txStream == NULL)
    {
        printf("setupStream fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_unmake(device);
        return EXIT_FAILURE;
    }

    sdr->samp_per_buf = SoapySDRDevice_getStreamMTU(device, sdr->txStream);
    return 0;
}

int transmitData(s_sdr *sdr, const float *data, size_t num_samples) {
    size_t num_sent = 0;
    int flags = 0;
    long long timeNs = 0;
    size_t total_sent = 0;
    size_t remaining = num_samples;
    printf("sample : %.1f, %.1f\n", data[1029], data[1030]);
    printf("Sending %zu samples\n", num_samples);
    while (remaining > 0) {
        size_t to_send = (remaining > sdr->samp_per_buf) ? sdr->samp_per_buf : remaining;
        const void *buffs[1];
        buffs[0]= &data[total_sent*2];

        //printf("Sending %zu samples\n", to_send);
        num_sent = SoapySDRDevice_writeStream(sdr->device, sdr->txStream, buffs, to_send, &flags, timeNs, 1000000);
        //printf("to send: %zu\n", to_send);
        if (num_sent < 0) {
            printf("Failed to write to stream: %d\n", (int)num_sent);
            return -1;
        }
        //printf("Samples sent : %zu", num_sent);
        
        total_sent += num_sent;
        remaining -= num_sent;

        //printf("Total sent: %zu, Remaining: %zu\n", total_sent, remaining);
    }

    // Envoyer le signal de fin de transmission
    flags = SOAPY_SDR_END_BURST;
    const void *buffs_end[] = {NULL};
    printf("Sending end burst signal\n");

    num_sent = SoapySDRDevice_writeStream(sdr->device, sdr->txStream, buffs_end, 0, &flags, timeNs, 1000000);
    if (num_sent < 0) {
        printf("Failed to send end burst signal: %d\n", (int)num_sent);
        return -1;
    }

    return total_sent;
}

int transmitData2(s_sdr *sdr, const float *data, size_t num_samples) {
    size_t num_sent = 0;
    int flags = 0;


    while (num_sent < num_samples) {
        size_t to_send = num_samples - num_sent;
        if (to_send > sdr->samp_per_buf) {
            to_send = sdr->samp_per_buf;
        }

        size_t sent_samples;

        // Create a pointer array pointing to the current position in the samples array
        const void* samples_ptr[1];
        samples_ptr[0] = &data[num_sent * 2];  // Correctly set the pointer to the current sample

        //int ret = uhd_tx_streamer_send(tx_streamer, samples_ptr, to_send, md, 0.1, &sent_samples);

        sent_samples = SoapySDRDevice_writeStream(sdr->device, sdr->txStream, samples_ptr, to_send, 0, 0, 1000);

        num_sent += sent_samples;
    }

    flags = SOAPY_SDR_END_BURST;
    const void *buffs_end[] = {NULL};
    printf("Sending end burst signal\n");

    num_sent = SoapySDRDevice_writeStream(sdr->device, sdr->txStream, buffs_end, 0, &flags, 0, 1000000);
    if (num_sent < 0) {
        printf("Failed to send end burst signal: %d\n", (int)num_sent);
        return -1;
    }
    return 0;
}

int transmitData3(s_sdr *sdr, const float *data, size_t num_samples) {
    int flags = 0;
    long long timeNs = 0;
    size_t total_sent = 0;
    size_t remaining = num_samples;
    size_t num_sent = 0;

    //printf("sample : %.1f, %.1f\n", data[1029], data[1030]);
    printf("Sending %zu samples\n", num_samples);

    // Activer le flux avant de commencer la transmission
    if (SoapySDRDevice_activateStream(sdr->device, sdr->txStream, flags, timeNs, 0) != 0) {
        printf("Failed to activate stream: %s\n", SoapySDRDevice_lastError());
        return -1;
    }

    while (remaining > 0) {
        size_t to_send = (remaining > sdr->samp_per_buf) ? sdr->samp_per_buf : remaining;
        const void *buffs[] = { &data[total_sent * 2] };

        num_sent = SoapySDRDevice_writeStream(sdr->device, sdr->txStream, buffs, to_send, &flags, timeNs, 1000000);
        if (num_sent < 0) {
            printf("Failed to write to stream: %d\n", (int)num_sent);
            SoapySDRDevice_deactivateStream(sdr->device, sdr->txStream, flags, timeNs);
            return -1;
        }

        total_sent += num_sent;
        remaining -= num_sent;
    }

    // Envoyer le signal de fin de transmission
    flags = SOAPY_SDR_END_BURST;
    const void *buffs_end[] = { NULL };
    printf("Sending end burst signal\n");

    num_sent = SoapySDRDevice_writeStream(sdr->device, sdr->txStream, buffs_end, 0, &flags, timeNs, 1000000);
    if (num_sent < 0) {
        printf("Failed to send end burst signal: %d\n", (int)num_sent);
        SoapySDRDevice_deactivateStream(sdr->device, sdr->txStream, flags, timeNs);
        return -1;
    }

    // Désactiver le flux après la transmission
    if (SoapySDRDevice_deactivateStream(sdr->device, sdr->txStream, flags, timeNs) != 0) {
        printf("Failed to deactivate stream: %s\n", SoapySDRDevice_lastError());
        return -1;
    }

    return total_sent;
}




#endif