/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef	TEST_SERVER_H
#define TEST_SERVER_H


#include "test.h"
#include <math.h>


// Function prototypes
int initServer(char *);
bool handleClient(int);


// Global variable: max packet size constant
const unsigned int MAX_PACKET_SIZE_UDP = 65507;


#endif
