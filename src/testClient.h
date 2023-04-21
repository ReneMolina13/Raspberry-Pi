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


// Latency Tests
enum
{
	PING = 0,
	TRACEROUTE = 1;
	OWAMP = 2;
	TWAMP = 3;
	IPERF = 4;
};


// Function prototypes
bool parseCmdArgs(int, char **, NetInfo *, sBANK_PROTOCOL *);
bool setupSocket(NetInfo *sockData);
bool sendPackets(NetInfo *, sBANK_PROTOCOL *);
bool latencyTesting(NetInfo *);


#endif
