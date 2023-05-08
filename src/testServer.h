/*
	Rene Molina
	EE 4230 - Senior Design 2
	
	This is the header file for the test server. It contains prototypes for functions 
	to initialize the test server, initialize the iPerf server, and handle client 
	requests
*/


#ifndef	TEST_SERVER_H
#define TEST_SERVER_H


#include "test.h"


// Function prototypes

/*
	Description:
	Creates a socket for the server at the port number given through command-line
	and using the wildcard IP address (any IP address)
	
	Inputs:
	- char *service: Service name or port number
	
	Outputs:
	- int serverSocket: Socket handle for server socket
*/
int initServer(char *);

/*
	Description:
	Uses fork and execlp system calls to run iPerf in server mode, which will be
	needed once the client process performs iPerf testing
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool iPerfServer();

/*
	Description:
	Waits for a packet to be received from a client, at which point it sends the 
	packet back to the client. Note that the buffer used to receive the incoming
	packet and to echo it back is of the largest size that a UDP packet can be. 
	When the client receives the packet from the server, any extra bytes beyond 
	the size of the client buffer get silently discarded at the client socket
	
	Inputs:
	- int serverSocket: Socket handle for the server socket
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool handleClient(int);


#endif
