/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef	TEST_SERVER_H
#define TEST_SERVER_H


#include "test.h"


// Function prototypes
int initServer(char *);
bool handleClient(int);


// Global variable: Max UDP packet size
const unsigned int MAX_PACKET_SIZE_UDP = 65507;


#endif
