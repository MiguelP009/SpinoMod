#ifndef MODEM_H
#define MODEM_H


#include "tcpServer.h"

#define PORT 8669  // Port sur lequel écouter

void modulateFSK(const char* buffer, size_t buffer_size, short* output, size_t output_size);

int modemInit();




#endif