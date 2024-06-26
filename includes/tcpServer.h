#ifndef TCPSERVER_H
#define TCPSERVER_H

#define PORT 8669  // Port sur lequel écouter
#define BUFFER_SIZE 1024


int usrpInit(void);
int tcpSocketInit(int port);



#endif