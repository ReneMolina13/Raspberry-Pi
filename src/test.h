/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef TEST_H
#define TEST_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/socket.h>	
#include <arpa/inet.h>
#include <netdb.h>


typedef struct
{
	char *oneByte;
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


// Function prototypes
void printSocketAddress(const struct sockaddr *);
bool runPing(char *, unsigned int, unsigned int, double);
bool runTraceroute(char *);
bool runIperf(char *);


#endif
