#ifndef TESTSPINO_H
#define TESTSPINO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define MAX_DATA_SIZE 222

#define CMD_SET_FREQ_BEACON 	{0xa6, 0xa0, 0x92, 0x9c, 0x9e, 0x88, 0x1e, 0x8c, 0x68, 0x82, 0x96, 0x82, 0x8e, 0x02, 0x00, 0x00, 0x09, 0x00 , 0xf0, 0x0f, 0x65, 0x00, 0x01, 0x01, 0x05, 0x05, 0x99}
#define CMD_GET_SPINO_VALUE 	{0xa6, 0xa0, 0x92, 0x9c, 0x9e, 0x88, 0x1e, 0x8c, 0x68, 0x82, 0x96, 0x82, 0x8e, 0x02, 0x00, 0x00, 0x0a, 0x00, 0xf0, 0x0f, 0x66, 0x00, 0x80, 0x00, 0x66, 0xb8}
#define CMD_GET_SPINO_VALUE_TEST 	{0xa6, 0xa0, 0x92, 0x9c, 0x9e, 0x88, 0x1e, 0x8c, 0x68, 0x82, 0x96, 0x82, 0x8e, 0x02, 0x00, 0x00, 0x0a, 0x00, 0xf0, 0x0f, 0x66, 0x00, 0x80, 0x00}
#define CMD_GET_SPINO_DELAY		{0xa6, 0xa0, 0x92, 0x9c, 0x9e, 0x88, 0x1e, 0x8c, 0x68, 0x82, 0x96, 0x82, 0x8e, 0x02, 0x00, 0x00, 0x0a, 0x00, 0xf0, 0x0f, 0x66, 0x00, 0x01, 0x00, 0x4e, 0x11}
#define CMD_RST					{0xa6, 0xa0, 0x92, 0x9c, 0x9e, 0x88, 0x1e, 0x8c, 0x68, 0x82, 0x96, 0x82, 0x8e, 0x02, 0x00, 0x00, 0x08, 0x00, 0xf0,  0x0f ,0x64, 0x00, 0xb1, 0x9b}

typedef struct ax25_header {
    unsigned char preamble[16];
    unsigned char sync[4];
	unsigned char destinationAdress[6];
	unsigned char ssidDestination;
	unsigned char sourceAdress[6];
	unsigned char ssidSource;
	unsigned char ctrl;
	unsigned char pid;
} s_ax25_header;

typedef struct ax25_packet {
	s_ax25_header header;
	char data[MAX_DATA_SIZE];

} s_ax25_packet;

typedef struct ax25_tc{
	uint8_t preamble[16];
	uint8_t sync[4];
	uint8_t data[MAX_DATA_SIZE];
	
} s_ax25_tc;


uint16_t calculateCRC(unsigned char  *data, int offset, int length) {
	int crc = 0x000;
	for (int i = offset; i < (offset + length); ++i) {
		crc ^= data[i] << 8;
		for (int j = 0; j < 8; ++j) {
			if ((crc & 0x8000) > 0) {
				crc = (crc << 1) ^ 0x1021;
			} else {
				crc = crc << 1;
			}
		}
	}
	return (uint16_t) crc & 0xFFFF;
}


void testSpino(uint8_t *data){
	s_ax25_packet ax25_frame;
    s_ax25_header ax25_hdr;
    ax25_frame.header = ax25_hdr;
    uint8_t preamble[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    //ax25_hdr.preamble = preamble;
    memcpy(ax25_hdr.preamble, preamble, 16);
    uint8_t sync[] = {0x2e, 0xfc, 0x98, 0x27};
    //ax25_hdr.sync = sync;
    memcpy(ax25_hdr.sync, sync, 4);
    char dest[] = { 'S', 'P', 'I', 'N', 'O', 'D' }; //6 bytes
	
	//memcpy(ax25_hdr.destinationAdress, destinationAddress, 6);
    uint8_t ssidDest = 15;
	ax25_hdr.ssidDestination= (unsigned char)(ssidDest<<1 & 0xfe);
	
    char src[] = { 'I', 'S', 'P', 'A', 'C', 'E' }; //6bytes
	//memcpy(ax25_hdr.sourceAdress, sourceAddress, 6);
    uint8_t ssidSource = 1;
	ax25_hdr.ssidSource = (unsigned char)(ssidSource<<1 & 0xfe);

    uint8_t controlbit = 0x01;
	ax25_hdr.ctrl = controlbit;
    uint8_t pid = 255;
	ax25_hdr.pid = pid;
    
	ax25_frame.header = ax25_hdr;


    uint8_t message[]= {0xa6, 0xa0, 0x92, 0x9c, 0x9e, 0x88, 0x1e, 0x8c, 0x68, 0x82, 0x96, 0x82, 0x8e, 0x02, 0x00, 0x00, 0x0b, 0x00, 0xf0, 0x0f, 0x65, 0x00, 0x09, 0x00, 0x04, 0x54, 0xde};
	uint8_t size = (uint8_t)sizeof(ax25_hdr) + (uint8_t)sizeof(message);//36+27 message
    uint16_t crc = 0; // 2bytes
    uint8_t flag[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
	int i;
	unsigned char c;
	
	for (i = 0; i < 6; i++) {
		c = (unsigned char) dest[i];
		ax25_hdr.destinationAdress[i] = (unsigned char) (c << 1 & 0xFE);
		;
	}

	for (i = 0; i < 6; i++) {
		c = (unsigned char) src[i];
		ax25_hdr.sourceAdress[i] = (unsigned char) (c << 1 & 0xFE);
		;
	}
	for (i = 0; i < 27; i++) {
		ax25_frame.data[i] = message[i];
	}

	uint8_t tx_frame[240];

	memcpy(&tx_frame, &ax25_frame, size);

	crc = calculateCRC((uint8_t*)&tx_frame, 0, size);
	tx_frame[size] = (uint8_t)(crc & 0xff);
	tx_frame[size + 1] = (uint8_t)((crc >> 8) & 0xff);
	for(int i=0; i<16; i++){
		tx_frame[size + 2 + i] = flag[i];
	}	



	printf("frame: \n");
	for(int i=0; i<size+(int)(sizeof(crc)+sizeof(flag)); i++){
		printf("%.2x ", tx_frame[i]);
	}

}


void spinoSendTC(uint8_t* data, int *size){
	s_ax25_tc ax25_tc;
    
    uint8_t preamble[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
	memcpy(ax25_tc.preamble, preamble, 16);
	 uint8_t sync[] = {0x2e, 0xfc, 0x98, 0x27};
    memcpy(ax25_tc.sync, sync, sizeof(sync));

	//Get value spino version
	//uint8_t message[]= {0xa6, 0xa0, 0x92, 0x9c, 0x9e, 0x88, 0x1e, 0x8c, 0x68, 0x82, 0x96, 0x82, 0x8e, 0x02, 0x00, 0x00, 0x0a, 0x00, 0xf0, 0x0f, 0x66, 0x00, 0x80, 0x00, 0x66, 0xb8};
	//Set frequency beacon
	uint8_t message[] = CMD_SET_FREQ_BEACON;
	memcpy(ax25_tc.data, message, sizeof(message));
	uint8_t tx_frame[240];
	*size = sizeof(sync)+sizeof(message)+sizeof(preamble);
	
	memcpy(data, &ax25_tc, *size);
	uint8_t flag[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
	for(int i=0; i<16; i++){
		data[*size + i] = flag[i];
	}	
	*size += sizeof(flag);

	// printf("\n Frame: \n");
	// for(int i=0; i<size+(int)(sizeof(flag)); i++){
	// 	printf("%.2x ", tx_frame[i]);
	// }

}


#endif