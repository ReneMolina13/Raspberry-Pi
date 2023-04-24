/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef	TEST_SERVER_H
#define TEST_SERVER_H


#include "test.h"


// Function prototypes
int initServer(char *);
bool handleClient(int, Packets *);
bool receivePacket(int, char *, unsigned int);


#endif
