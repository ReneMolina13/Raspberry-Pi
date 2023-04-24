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
	char one_kb[1024];
	char two_kb[2048];
	char four_kb[4096];
	char eight_kb[8192];
	char sixteen_kb[16384];
	char thirty_two_kb[32768];
	char sixty_four_kb[65536];
	char one_eigth_mb[131072];
	char one_fourth_mb[262144];
	char one_half_mb[524288];
	char one_mb[1048576];
} Packets;


// Function prototypes
void printSocketAddress(const struct sockaddr *);
bool runPing(char *, unsigned int, unsigned int, double);
bool runTraceroute(char *);
bool runIperf(char *);


#endif
