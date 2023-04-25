/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef	TEST_CLIENT_H
#define	TEST_CLIENT_H


#include "test.h"
#include <pthread.h>


enum packetIndex
{
	ONE_BYTE = 0,
	TWO_BYTES = 1,
	FOUR_BYTES = 2,
	EIGHT_BYTES = 3,
	SIXTEEN_BYTES = 4,
	THIRTY_TWO_BYTES = 5,
	SIXTY_FOUR_BYTES = 6,
	ONE_EIGTH_KB = 7,
	ONE_FOURTH_KB = 8,
	ONE_HALF_KB = 9,
	ONE_KB = 10,
	TWO_KB = 11,
	FOUR_KB = 12,
	EIGHT_KB = 13,
	SIXTEEN_KB = 14,
	THIRTY_TWO_KB = 15,
	MAX_SIZE_UDP = 16,
	NUM_PACKET_SIZES = 17
};


typedef struct
{
	char oneByte;
	char two_bytes[2];
	char four_bytes[4];
	char eight_bytes[8];
	char sixteen_bytes[16];
	char thirty_two_bytes[32];
	char sixty_four_bytes[64];
	char one_eigth_kb[128];
	char one_fourth_kb[256];
	char one_half_kb[512];
	char one_kb[1024];
	char two_kb[2048];
	char four_kb[4096];
	char eight_kb[8192];
	char sixteen_kb[16384];
	char thirty_two_kb[32768];
	char max_size_udp[65507];
} Packets;


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
bool createSocket(NetInfo *);
bool sendPacket(const NetInfo *, char *, unsigned int);
bool receivePacket(const NetInfo *, char *, unsigned int);


// Global Variables
pthread_attr_t attr;
pthread_mutex_t mutex;


#endif
