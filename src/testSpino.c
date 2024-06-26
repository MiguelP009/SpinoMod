#include "testSpino.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAIN2


#ifdef MAIN1
int main(){
	uint8_t data[240]={0};
    int size;
	spinoSendTC(data, &size);
    for(int i = 0; i < size; i++){
        printf("%.2x ", data[i]);
    }
	return 0;
}
#endif


#ifdef MAIN2
int main(){
    uint8_t data[]=CMD_GET_SPINO_VALUE_TEST;
    int size;
    uint16_t crc = calculateCRC(data, 0, 24);
    printf("CRC: %.4x\n", crc);
    return 0;
}

#endif  