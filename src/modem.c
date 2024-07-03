#include "../includes/modem.h"
#include "../includes/soapySdr.h"

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "../includes/fsk.h"



void fskInit(s_vco *vco_config){
	float center_freq = 0;
    float deviation = 2000;
    float vco_max = center_freq + deviation;
    float amplitude = 1;
    float sensitivity = 2 * PI * vco_max;
	initVco(vco_config, MODULATION_SAMPLE_RATE, sensitivity, amplitude, 0);

	printf("\n##########################################################\n");
	printf("FSK Modulator Configuration:\n");
    printf("	Sample Rate: %.2f Hz\n", vco_config->samp_rate);
    printf("	Frequency Deviation: %.2f Hz\n", deviation);
    printf("	Center Frequency: %.1f Hz\n", center_freq);
    printf("	Amplitude: %.2f\n", vco_config->amplitude);
    printf("	Sensitivity: %.2f rad/s\n", vco_config->sensitivity);
	printf("##########################################################\n");
	
}


int modemInit(int *socket, int *socket2, s_vco *vco_config, s_sdr *sdr) {
	if(selectSDRDevice(sdr)!=0){
		printf("No SDR device found\n");
		return -1;
	}
	if(configSDR(sdr)!=0){
		printf("Fail to configure SDR device\n");
	}
	printf("\n##########################################################\n");
	printf("Connected to SDR Device\n");
	printf("##########################################################\n\n");

	fskInit(vco_config);
	
	tcpSocketInit(PORT, socket, socket2);
	return 0;
}

void closeSockets(int socket, int socket2){
#ifdef _WIN32
    closesocket(socket);
	closesocket(socket2);
    WSACleanup();
#else
    close(socket);
	close(socket2);
#endif
}

void print_hex(const unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
}

void getData(int socket, int socket2, s_vco *vco_config, s_sdr *sdr){
	// Reception des donnees
	char buffer[BUFFER_SIZE];
	float *samples;
	size_t samples_len;
	int n;
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(socket, buffer, BUFFER_SIZE, 0);
        if (n < 0) {
            perror("Erreur lors de la reception du socket");
            exit(1);
        }
        if (n == 0) {
            printf("Connexion fermee par le client\n");
            break;
        }

        // Affichage des donnees recues
        print_hex(buffer, n);

        //  Modulation FSK
		fskModulate(vco_config, buffer, n, &samples, &samples_len);
		//printf("sample : %.1f, %.1f\n", samples[1029], samples[1030]);

		printf("FSK Modulation done\n");
		for(int i=0; i<10; i++){
			transmitData3(sdr, samples, samples_len/2);
		}
    }

    // Fermeture des sockets
	closeSockets(socket, socket2);

}

int main(){
	int socket1, socket2;
	s_vco vco_config;
	s_sdr sdr;
	if(modemInit(&socket1, &socket2, &vco_config, &sdr)!=0){
		return -1;
	}

	getData(socket1, socket2, &vco_config, &sdr);
	return 0;

}
