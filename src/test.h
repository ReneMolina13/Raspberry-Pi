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


// Function prototypes
void printSocketAddress(const struct sockaddr *);
bool runPing(char *, unsigned int, unsigned int, double);
bool runTraceroute(char *);
bool runIperf(char *);


#endif
