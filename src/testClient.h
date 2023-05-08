/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef	TEST_CLIENT_H
#define	TEST_CLIENT_H


#include "test.h"


/*
	Description:
	Contains network information needed to run custom network test
	
	Member variables:
	- cmdIP: IP address or hostname given to the program as a command-line argument
	- cmdPort: Port number or service name given to the program as a command-line
			argument
	- clientSocket: Socket handle of the client socket used for custom network test
	- serverAddr: Pointer to an addrinfo structure containing possible IP addresss / 
			port number combinations for the client socket (one of which is used)
*/
typedef struct
{
	char *cmdIP;
	char *cmdPort;
	int clientSocket;
	struct addrinfo *serverAddr;
} NetInfo;


/*
	Description:
	Used in custom network test by each network testing thread to send data to/from 
	test server and record statistics related to network performance. Specifically, 
	this strucutre is the parameter passed to networkThreads function
	
	Member Variables:
	- tid: Thread ID of a particular network testing thread
	- sockData: Pointer to a NetInfo structure containing network information for 
			the client socket (used by the thread to send data to/from test server)
	- stats: Pointer to a NetStats structure containing network statistics that the 
			thread will write to after each packet is sent to / received from the
			test server 
	- sentPacket: String (packet) that the thread is assigned to which will be sent
			to the test server
	- receivedPacket: String buffer (of same length of sentPacket) that will be 
			received from test server and compared with sentPacket to check for 
			errors
	- status: Value indicating whether the thread executed correctly
*/
typedef struct
{
	pthread_t tid;
	NetInfo *sockData;
	NetStats *stats;
	char *sentPacket;
	char *receivedPacket;
	volatile int status; 
} ThreadArgs;


// Function prototypes

/*
	Description:
	Threads used to perform network testing by communicating with the test server.
	Each thread is assigned a different packet size and writes network statistics 
	to a separate instance of the NetStats structure
	
	Inputs/Outputs:
	- void *param: Holds a pointer to a ThreadArgs structure
*/
void *networkThreads(void *param);

/*
	Description:
	
	Inputs:
	- int argc:
	- char **argv:
	- NetInfo *sockData:
	- Packets *packets:
	- TestResults *testResults:
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool clientSetup(int, char **, NetInfo *, Packets *, TestResults *);

/*
	Description:
	
	Inputs:
	- NetInfo *sockData:
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool createSocket(NetInfo *);

/*
	Description:
	
	Inputs:
	- const NetInfo *sockData:
	- const char *packet:
	- unsigned int packetSize:
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool sendPacket(const NetInfo *, const char *, unsigned int);

/*
	Description:
	
	Inputs:
	- const NetInfo *sockData:
	- char *packet:
	- unsigned int packetSize:
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool receivePacket(const NetInfo *, char *, unsigned int);

/*
	Description:
	
	Inputs:
	- TestResults *testResults:
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool formatOutput(TestResults *testResults);


// Global Variables

// Structure containing attributes of each thread
pthread_attr_t attr;
// Semaphore used by all threads to communicate with server
pthread_mutex_t mutex;


#endif

