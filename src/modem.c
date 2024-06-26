#include "modem.h"



int modulateFSK(const char* buffer, size_t buffer_size, short* output, size_t output_size) {

}


int modemInit() {
	tcpSocketInit(PORT);
	return 0;
}