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

#include <getopt.h>

#include "../includes/soapySdr.h"
#include "../includes/fsk.h"
#include "../includes/tcpServer.h"


#define PORT 8669 // Port TCP




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
	printf("##########################################################\n");
	
}


int modemInit(int *socket, int *socket2, s_vco *vco_config, s_sdr *sdr, int tcp_port, double gain, double spino_freq) {
	if(selectSDRDevice(sdr)!=0){
		printf("No SDR device found\n");
		return -1;
	}
	if(configSDR(sdr, gain, spino_freq)!=0){
		printf("Fail to configure SDR device\n");
        return -1;
	}
	printf("\n##########################################################\n");
	printf("Connected to SDR Device\n");
	printf("##########################################################\n\n");

	fskInit(vco_config);
	
	tcpSocketInit(tcp_port, socket, socket2);
	return 0;
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

		printf("FSK Modulation done\n");
		for(int i=0; i<2; i++){
			transmitData(sdr, samples, samples_len/2);
		}
    }

    // Fermeture des sockets
	closeSockets(socket, socket2);

}

void print_usage() {
    printf("Usage: ./SpinoMod [--gain <gain(dB)>] [--tcp_port <port>] [--spino_freq <frequency(Hz)>]\n");
}

int main(int argc, char *argv[]) {
    int socket1, socket2;
    s_vco vco_config;
    s_sdr sdr;
    
    // Valeurs par défaut
    int tcp_port = 8888;
    double gain = 35.0;
    double spino_freq = 145.83e6;

    // Options de la ligne de commande
    static struct option long_options[] = {
        {"gain", required_argument, 0, 'g'},
        {"tcp_port", required_argument, 0, 't'},
        {"spino_freq", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    
    // Traitement des options
    while ((opt = getopt_long(argc, argv, "g:t:f:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'g':
                gain = atof(optarg);
                break;
            case 't':
                tcp_port = atoi(optarg);
                break;
            case 'f':
                spino_freq = atof(optarg);
                break;
            default:
                print_usage();
                return -1;
        }
    }
	printf("\n################################################################################\n");
    printf("Using parameters: gain = %.2f, tcp_port = %d, spino_freq = %.2f\n", gain, tcp_port, spino_freq);
	printf("################################################################################\n\n");


    if (modemInit(&socket1, &socket2, &vco_config, &sdr, tcp_port, gain, spino_freq) != 0) {
        printf("Error during modem initialization\n");
        return -1;
    }

    getData(socket1, socket2, &vco_config, &sdr);
    return 0;
}
