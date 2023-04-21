/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef	BANK_CLIENT_H
#define	BANK_CLIENT_H


#include "test.h"


typedef struct
{
	char *cmdIP;
	char *cmdPort;
	int clientSocket;
	struct addrinfo *serverAddr;
} NetInfo;


// Function prototypes
bool parseCmdArgs(int, char **, NetInfo *, sBANK_PROTOCOL *);
bool setupSocket(NetInfo *sockData);
bool sendPackets(NetInfo *, sBANK_PROTOCOL *);


#endif
