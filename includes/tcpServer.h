

#ifndef TCPSERVER_H
#define TCPSERVER_H


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


#define BUFFER_SIZE 255



//int tcpSocketInit(int port);

int tcpSocketInit(int port, int *socket_out, int *sockfd) {
    // Initialisation des variables
    //int sockfd, newsockfd;
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
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
    if (bind(*sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur lors du binding");
        exit(1);
    }

    // Mettre le socket en mode ecoute
    printf("TCP Server listening on port %d\n", port);
    listen(*sockfd, 5);

    // Accepter la connexion entrante
    clilen = sizeof(cli_addr);
    *socket_out = accept(*sockfd, (struct sockaddr*)&cli_addr, &clilen);
    if (*socket_out < 0) {
        perror("Erreur lors de l'acceptation de la connexion");
        exit(1);
    }


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


#endif