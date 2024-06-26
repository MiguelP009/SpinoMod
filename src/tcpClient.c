#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Pour close

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#define SERVER_ADDR "127.0.0.1" // Adresse IP du serveur (localhost dans cet exemple)
#define SERVER_PORT 8669        // Port du serveur

#include <time.h>

void delay(int number_of_seconds) {
#ifdef _WIN32
    // Converting time into milli_seconds
    long milli_seconds = 1000 * number_of_seconds;

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds);
#else
    sleep(number_of_seconds);

#endif
}

#ifdef _WIN32
int inet_pton(int af, const char* src, void* dst) {
    struct sockaddr_storage ss;
    int size = sizeof(ss);
    char src_copy[INET6_ADDRSTRLEN + 1];

    ZeroMemory(&ss, sizeof(ss));
    /* stupid non-const API */
    strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
    src_copy[INET6_ADDRSTRLEN] = 0;

    if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr*)&ss, &size) == 0) {
        switch (af) {
        case AF_INET:
            *(struct in_addr*)dst = ((struct sockaddr_in*)&ss)->sin_addr;
            return 1;
        case AF_INET6:
            *(struct in6_addr*)dst = ((struct sockaddr_in6*)&ss)->sin6_addr;
            return 1;
        }
    }
    return 0;
}
#endif
#include "testSpino.h"

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }
#endif

    // Création du socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_ADDR, &serv_addr.sin_addr) <= 0) {
        perror("Erreur lors de la conversion de l'adresse");
        exit(EXIT_FAILURE);
    }

    // Connexion au serveur
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur lors de la connexion au serveur");
        exit(EXIT_FAILURE);
    }
    delay(1);
    
    uint8_t message[244];
    int frame_size;
    spinoSendTC(message, &frame_size);
    printf("size: %d\n", frame_size);

    printf("Commande envoyée au serveur : %s\n", message);
    for(int rep =0; rep<5; rep++){
    //while (1) {
        // Envoi des données au serveur (simulation de la commande de SpinoController)
        if (send(sockfd, message, frame_size, 0) < 0) {
            perror("Erreur lors de l'envoi des données au serveur");
            exit(EXIT_FAILURE);
        }
    }
        
    //}

    // Fermeture du socket
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif

    return 0;
}
