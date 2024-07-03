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


#define GAIN_TX 38
/*

int usrpInit(void){//double gain, double rate, double tx_frequency) {

    double rate = 1e6; // 1 Msps
    double gain = 0;
    const char* device_args = "type=b200";
    size_t channel = 0;
    uint64_t total_num_samps = 0;
    double freq = 144e6;
    size_t samps_per_buff;
    
    // Create USRP
    
    uhd_usrp_handle usrp;
    printf("Creating the USRP device with args \"%s\"...\n", device_args);
    uhd_usrp_make(&usrp, device_args);

    uhd_tx_streamer_handle tx_streamer;
    uhd_tx_streamer_make(&tx_streamer);

    // Create TX metadata
    uhd_tx_metadata_handle md;
    uhd_tx_metadata_make(&md, false, 0, 0.1, true, false);
    uhd_tune_request_t tune_request = { .target_freq = freq,
           .rf_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO,
           .dsp_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO };
    uhd_tune_result_t tune_result;
    uhd_stream_args_t stream_args = { .cpu_format = "fc32",
        .otw_format = "sc16",
        .args = "",
        .channel_list = &channel,
        .n_channels = 1 };



    //set rate
    printf("Setting TX Rate: %f ...\n", rate);
    uhd_usrp_set_tx_rate(usrp, rate, channel);
    uhd_usrp_get_tx_rate(usrp, channel, &rate);
    printf("Actual TX Rate: %f \n", rate);

    //set Gain
    printf("Setting TX Gain: %f db...\n", gain);
    uhd_usrp_set_tx_gain(usrp, gain, 0,"");
    uhd_usrp_get_tx_gain(usrp, channel, "", &gain);
    printf("Actual TX Gain: %f db\n", gain);

    //set frequency
    printf("Setting TX Frequency: %f MHz...\n", freq/1e6);
    uhd_usrp_set_tx_freq(usrp, &tune_request, channel, &tune_result);
    uhd_usrp_get_tx_freq(usrp, channel, &freq);
    printf("Actual TX Frequency: %f MHz\n", freq/1e6);

    // Set up streamer
    stream_args.channel_list = &channel;
    uhd_usrp_get_tx_stream(usrp, &stream_args, tx_streamer);

    //Set up buffer
    uhd_tx_streamer_max_num_samps(tx_streamer, &samps_per_buff);
    printf("Buffer size in samples: %zu\n", samps_per_buff);

}
*/



#define BUFFER_SIZE 1024


int tcpSocketInit(int port) {
    // Initialisation des variables
    int sockfd, newsockfd;
    socklen_t clilen;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }
#endif

    // Creer un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erreur lors de l'ouverture du socket");
        exit(1);
    }

    // Initialiser la structure sockaddr_in
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // Lier le socket a l'adresse et au port
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur lors du binding");
        exit(1);
    }

    // Mettre le socket en mode ecoute
    listen(sockfd, 5);

    // Accepter la connexion entrante
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
    if (newsockfd < 0) {
        perror("Erreur lors de l'acceptation de la connexion");
        exit(1);
    }

    // Reception des donnees
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(newsockfd, buffer, BUFFER_SIZE, 0);
        if (n < 0) {
            perror("Erreur lors de la reception du socket");
            exit(1);
        }
        if (n == 0) {
            printf("Connexion fermee par le client\n");
            break;
        }

        // Affichage des donnees recues
        printf("Donnees recues: %s\n", buffer);

        // Ici vous pouvez traiter les donnees recues et les transmettre a votre modulation FSK
    }

    // Fermeture des sockets
#ifdef _WIN32
    closesocket(newsockfd);
    closesocket(sockfd);
    WSACleanup();
#else
    close(newsockfd);
    close(sockfd);
#endif

    return 0;
}

int main() {
	tcpSocketInit(PORT);
	return 0;
}