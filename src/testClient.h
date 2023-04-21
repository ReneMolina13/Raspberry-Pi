/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef	BANK_CLIENT_H
#define	BANK_CLIENT_H


#include "test.h"


// Structure holding connection info (from cmd line & necessary structures)
typedef struct
{
	char *cmdIP;
	char *cmdPort;
	int clientSocket;
	struct addrinfo *serverAddr;
} NetInfo;


//*******************************************************************************************
// Function Prototypes
//*******************************************************************************************

bool parseCmdArgs(int, char **, NetInfo *, sBANK_PROTOCOL *);
bool setupSocket(NetInfo *sockData);
bool sendPackets(NetInfo *, sBANK_PROTOCOL *);


#endif
