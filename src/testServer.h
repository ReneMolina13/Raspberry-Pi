/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef	TEST_SERVER_H
#define TEST_SERVER_H


#include "test.h"
#include <math.h>


// Max size of UDP packet
#define MAX_PACKET_SIZE_UDP	65507


// Function prototypes
int initServer(char *);
bool handleClient(int);


#endif
