/*
	Rene Molina
	EE 4230 - Senior Design 2
	
	This is the header file for the test client. It contains a structure to hold 
	network info and a structure to hold input/output parameters for the network threads. 
	It also contains global variables specifying the attributes of all threads and a 
	semaphore to be used by network threads to communicate with the test server. Finally, 
	it contains prototypes for functions for the network threads, to initialize relevant 
	structures used by test client, to create client socket, send and receive packets 
	to/from the test server, and to format the output of all four network tests into a 
	single easy to read spreadsheet
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
	- serverAddr: Pointer to an addrinfo structure containing possible IP address / 
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
	this structure is the parameter passed to networkThreads function
	
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
	Parses command-line arguments and initializes NetInfo, Packets, and 
	TestResults structures
	
	Inputs:
	- int argc: Number of command-line arguments
	- char **argv: Command-line argument array
	- NetInfo *sockData: Pointer to a NetInfo structure to be initialized
	- Packets *packets: Pointer to a Packets structure to be initialized
	- TestResults *testResults: Pointer to a TestResults structure to be initialized
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool clientSetup(int, char **, NetInfo *, Packets *, TestResults *);

/*
	Description:
	Creates client socket based on IP address/hostname and port number/service name 
	given as command-line arguments
	
	Inputs:
	- NetInfo *sockData: Pointer to a NetInfo structure which contains information to 
			to create client socket, as well as the resulting client socket handle 
			and a pointer to an addrinfo structure which get initialized to contain 
			the address info of the server socket
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool createSocket(NetInfo *);

/*
	Description:
	Sends the given packet to the server specified by the NetInfo structure
	
	Inputs:
	- const NetInfo *sockData: Pointer to a NetInfo structure containing the client 
			socket and the address info of the server socket
	- const char *packet: The packet to send to the server
	- unsigned int packetSize: The size of the packet in bytes
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool sendPacket(const NetInfo *, const char *, unsigned int);

/*
	Description:
	Receives a packet of a given size back from the server specified by the NetInfo structure 
	
	Inputs:
	- const NetInfo *sockData: Pointer to a NetInfo structure containing the client socket 
			and the address info of the server socket
	- char *packet: The buffer to store the packet received from the server
	- unsigned int packetSize: The size of the buffer in bytes
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool receivePacket(const NetInfo *, char *, unsigned int);

/*
	Description:
	Takes the important network statistics previously extracted from the output files of all 
	four network tests (custom test, ping, traceroute, and iPerf) and outputs them all into 
	a single file (networkTestResults.csv)
	
	Inputs:
	- TestResults *testResults: Pointer to a TestResults structure containing relevant network 
			statistics to be outputted from all four tests
	
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
