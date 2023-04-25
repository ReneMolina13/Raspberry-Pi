/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef	TEST_CLIENT_H
#define	TEST_CLIENT_H


#include "test.h"


typedef struct
{
	char *cmdIP;
	char *cmdPort;
	int clientSocket;
	struct addrinfo *serverAddr;
} NetInfo;


typedef struct
{
	pthread_t tid;
	NetInfo *sockData;
	Packets *packets;
	bool status;
} ThreadArgs;


// Function prototypes
void *networkThreads(void *param);
bool clientSetup(int, char **, NetInfo *, Packets *);
bool setupSocket(NetInfo *);
bool makeTraffic(const NetInfo *, Packets *);
bool sendPacket(const NetInfo *, char *, unsigned int);


// Global Variables
pthread_attr_t attr;


#endif
