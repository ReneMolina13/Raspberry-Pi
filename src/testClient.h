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
	NetStats *stats;
	char *sentPacket;
	char *receivedPacket;
	bool status;
} ThreadArgs;


typedef struct
{
	unsigned int totalIterations;
	double avgRTT;
	double avgThroughput;
	double avgErrorsPerPacket;
	double avgErrorsPerKB;
} CustomResults;


typedef struct
{
	unsigned int numTests
	unsigned int *packetSize;
	unsigned int *packetsTransmitted;
	double *packetLoss;
	double *minRTT;
	double *avgRTT;
	double *maxRTT;
	double *stdDevRTT;
} PingResults;


typedef struct
{
	unsigned int numHops;
	unsigned int bytesPerPacket;
	double **hopLatency;
} TracerouteResults;


typedef struct
{
	unsigned int numTests;
	double secondsPerTest;
	unsigned int *packetsSent;
	double *megaBytesSent;
	double *megaBytesReceived;
	double *avgThroughputSent;
	double *avgThroughputReceived;
	double *jitterSent;
	double *jitterReceived;
	double *packetLossSent;
	double *packetLossReceived;
} IperfResults;


// Function prototypes
void *networkThreads(void *param);
bool clientSetup(int, char **, NetInfo *, Packets *);
bool createSocket(NetInfo *);
bool sendPacket(const NetInfo *, const char *, unsigned int);
bool receivePacket(const NetInfo *, char *, unsigned int);
bool formatOutput();


// Global Variables
pthread_attr_t attr;
pthread_mutex_t mutex;


#endif
