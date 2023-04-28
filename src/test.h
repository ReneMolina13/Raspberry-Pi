/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef TEST_H
#define TEST_H


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/socket.h>	
#include <arpa/inet.h>
#include <netdb.h>


enum packetConstants
{
	// Max size of UDP packet & index of this packet in packets arrays
	MAX_PACKET_SIZE_UDP = 65507,
	INDEX_MAX_SIZE_UDP = 16,
	// Number of different packet sizes in packets array
	NUM_PACKET_SIZES = 17,
	// Max number of iterations before calculations begin to overflow
	MAX_ITERATIONS = 65000000
};


typedef struct
{
	unsigned int packetSizes[NUM_PACKET_SIZES];
	char *sentPackets[NUM_PACKET_SIZES];
	char *receivedPackets[NUM_PACKET_SIZES];
} Packets;


typedef struct
{
	unsigned int bytesPerPacket;
	unsigned int iteration;
	double avgRoundTripTime;
	double errorsPerPacket; 
} NetStats;


typedef struct
{
	pthread_t tid;
	NetStats *packetStats;
} DataProcessingArgs;


typedef struct
{
	pthread_t tid;
	char *hostname;
	char *service;
} TestingArgs;


// Function prototypes
void *dataProcessingThread(void *);
void *testingThread(void *);
bool runPing(char *, int, int, double, bool);
bool runTraceroute(char *);
bool runIperf(char *, int, double, int, int, int);
void printSocketAddress(const struct sockaddr *);

#endif
