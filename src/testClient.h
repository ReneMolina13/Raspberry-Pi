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


// Command line tests
enum
{
	PING = 0,		// Round trip latency, packet loss (approximate)
	TRACEROUTE = 1,	// Round trip delay, packet loss (approximate), network path
	IPERF = 2,		// Jitter, packet loss, throughput
};


// Function prototypes
bool parseCmdArgs(int, char **, NetInfo *, sBANK_PROTOCOL *);
bool setupSocket(NetInfo *sockData);
bool sendPackets(NetInfo *, sBANK_PROTOCOL *);
bool runCmdTest(char *, int);


#endif
