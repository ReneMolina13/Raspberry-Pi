/*
	Rene Molina
	EE 4230 - Senior Design 2
	
	This is a header file used by both the test client and test server. It contains 
	constants used by the programs, a structure containing packets of different sizes 
	to be sent between the test client and test server, a structure containing statistics 
	to be written to by each network thread during the custom network test, a structure 
	containing input/output parameters for the thread which processes data from custom 
	network test, and separate structures containing important statistics extracted from 
	the output files from all four network tests (custom test, ping, traceroute, and iPerf). 
	Finally, it contains prototypes of the function for the data processing thread, the 
	functions that call run external network testing programs, functions to extract important 
	statistics from the output files of all four networking tests, the function which 
	determines in what order and with which parameters the external network tests are run, and 
	a function to print the hostname and service name of both the client and server sockets
*/


#ifndef TEST_H
#define TEST_H


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/socket.h>	
#include <arpa/inet.h>
#include <netdb.h>


/*
	Description:
	Contains constants used throughout the program
	
	Constants:
	- MAX_PACKET_SIZE_UDP: Max size of a UDP packet
	- INDEX_MAX_SIZE_UDP: Index of packet of largest possible size for UDP in both
			sentPackets and receivedPackets arrays in Packets structure
	- NUM_PACKET_SIZES: Number of different packet sizes in sentPackets and 
			receivedPackets arrays
	- MAX_ITERATIONS: Max number of iterations before calculations start to overflow
	- MAX_BANDWIDTH: Max bandwidth of network used for iPerf testing (can be adjusted
			as needed
*/
enum packetConstants
{
	MAX_PACKET_SIZE_UDP = 65507,
	INDEX_MAX_SIZE_UDP = 16,
	NUM_PACKET_SIZES = 17,
	MAX_ITERATIONS = 65000000,
	MAX_BANDWIDTH = 1500
};


/*
	Description:
	Used in custom network test to send packets to/from test server
	
	Member Variables:
	- packetSize: Contains packet sizes of each string used in both
			sentPackets and receivedPackets arrays
	- sentPackets: Array of strings (packets) to be sent out to test server
			by network testing threads. Note that each packet is initialized to
			random ASCII values at the start of program execution
	- receivedPackets: Array of strings (packets) to be received by test
			server and subsequently compared with sent packets to check for data
			corruption
*/
typedef struct
{
	unsigned int packetSizes[NUM_PACKET_SIZES];
	char *sentPackets[NUM_PACKET_SIZES];
	char *receivedPackets[NUM_PACKET_SIZES];
} Packets;


/*
	Description:
	Used in custom network test to hold statistics generated from threads sending
	packets to/from test server. Each network thread writes to their own instance 
	of this structure
	
	Member Variables:
	- bytesPerPacket: Size of the packet assigned to this thread in bytes
	- iteration: The number of times this packet has been sent to and received from 
			the test server by this thread
	- avgRoundTripTime: The average of all round-trip times for this packet 
	- errorsPerPacket: The average number of incorrect bytes over all times this
			packet has been sent to/from the server
*/
typedef struct
{
	unsigned int bytesPerPacket;
	unsigned int iteration;
	double avgRoundTripTime;
	double errorsPerPacket; 
} NetStats;


/*
	Description:
	Used in custom network test by data processing thread to take real-time
	statistics and output them to customTestData.csv. Specifically, this structure
	is the parameter passed to dataProcessingThread function
	
	Member Variables:
	- tid: Thread ID of the data processing thread
	- packetStats: Pointer to NetStats structures written to by each network thread
	- status: Value indicating whether the thread executed correctly
*/
typedef struct
{
	pthread_t tid;
	NetStats *packetStats;
	bool status;
} DataProcessingArgs;


/*
	Description:
	Contains important statistics generated from the custom network test
	
	Member Variables:
	- totalIterations: Total number of packets sent to/from server
	- avgRTT: Average round-trip time over all threads in ms
	- avgThroughput: Average throughput over all threads in kB/s
	- avgErrorsPerPacket: Average number of incorrect bytes per packet over all
			packets sent to/from test server
	- avgErrorsPerKB: Number of incorrect bytes per KB over all packets sent
			to/from test server
*/
typedef struct
{
	unsigned int totalIterations;
	double avgRTT;
	double avgThroughput;
	double avgErrorsPerPacket;
	double avgErrorsPerKB;
} CustomResults;


/*
	Description:
	Contains important statistics generated from a particular ping test
	
	Member Variables:
	- packetSize: Size of each ping packet
	- packetsTransmitted: Number of packets transmitted
	- packetLoss: Number of packets dropped by the network
	- minRTT: Smallest round-trip time over all packets sent across the network
	- avgRTT: Average round-trip time over all packets sent across the network
	- maxRTT: Largest round-trip time over all packets sent across the network
	- stdDevRTT: Standard deviation of round-trip times
*/
typedef struct
{
	unsigned int packetSize;
	unsigned int packetsTransmitted;
	double packetLoss;
	double minRTT;
	double avgRTT;
	double maxRTT;
	double stdDevRTT;
} PingResults;


/*
	Description:
	Contains important statistics generated from the traceroute test
	
	Member Variables:
	- numHops: Number of hops taken to reach desired IP address
	- bytesPerPacket: Number of bytes in each packet
	- hopLatency: Pointer to double arrays containing the latency of all three 
			packets sent to each hop on the network
*/
typedef struct
{
	unsigned int numHops;
	unsigned int bytesPerPacket;
	double **hopLatency;
} TracerouteResults;


/*
	Description:
	Contains important statistics generated from a particular iPerf test
	
	Member Variables:
	- secondsPerTest: Length of iPerf test in seconds
	- packetsSent: Number of packets sent to iPerf server
	- dataSent: Total amount of data sent to iPerf server
	- dataSentUnits: Units of dataSent (M = MBytes, G = GBytes)
	- dataReceived: Total amount of data received from iPerf server
	- dataReceivedUnits: Units of dataReceived (M = MBytes, G = GBytes)
	- avgThroughputSent: Average throughput of data sent to iPerf server
	- throughputSentUnits: Units of avgThroughputSent (M = Mbits/sec, G = Gbits/sec) 
	- avgThroughputReceived: Average throughput of data received from iPerf server
	- throughputReceivedUnits: Units of avgThroughputReceived (M = Mbits/sec, 
			G = Gbits/sec)
	- jitterSent: The difference in latency between the highest latency packet
			and lowest latency packet sent
	- jitterReceived: The difference in latency between the highest latency packet
			and lowest latency packet received
	- packetLossSent: Packet loss percentage across all packets sent
	- packetLossReceived: Packet loss percentage across all packets received
*/
typedef struct
{
	double secondsPerTest;
	unsigned int packetsSent;
	double dataSent;
	char dataSentUnits;
	double dataReceived;
	char dataReceivedUnits;
	double avgThroughputSent;
	char throughputSentUnits;
	double avgThroughputReceived;
	char throughputReceivedUnits;
	double jitterSent;
	double jitterReceived;
	double packetLossSent;
	double packetLossReceived;
} IperfResults;


/*
	Description:
	Holds structures containing statistics for each type of network test
	
	Member Variables:
	- numPingTests: Number of ping tests ran so far. Used to determine how many ping
			PingResults structures are needed
	- numIperfTests: Number of iPerf tests ran so far. Used to determine how many
			IperfResults structures are needed
	- customResults: Structure containing important statistics extracted from 
			customTestData.csv following custom network test. Note that this test
			is assumed to have only been run once.
	- pingResults: Pointer to structures containing important statistics extracted
			from pingData.txt following each ping test
	- tracerouteResults: Structure containing important statistics extracted from
			tracerouteData.txt following traceroute test. Note that this test is 
			assumed to have only been run once
	- iperfResults: Pointer to structures containing important statistics extracted 
			from iperfData.txt following each iPerf test
*/
typedef struct
{
	int numPingTests;
	int numIperfTests;
	CustomResults customResults;
	PingResults *pingResults;
	TracerouteResults tracerouteResults;
	IperfResults *iperfResults;
} TestResults;


// Function prototypes

/*
	Description:
	Thread used to output statistics generated by network threads in real-time to
	customTestData.csv runs for an arbitrary amount of time before exiting, at 
	which point main function of testClient.c signals for all network threads to 
	exit
	
	Inputs/Outputs:
	- void *param: Holds a pointer to a DataProcessingArgs structure (described 
			above)
*/
void *dataProcessingThread(void *);

/*
	Description:
	Extracts important statistics from customTestData.csv into a CustomResults 
	structure. This function is assumed to be called only once during program 
	execution, directly after custom network test concludes
	
	Inputs:
	- CustomResults *customResults: Pointer to a structure to store relevant 
			statistics from custom network test
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool extractCustomStats(CustomResults *);

/*
	Description:
	Calls functions to run external network tests (ping, traceroute, and iPerf) and
	store relevant statistics outputted by these programs into structures to later 
	be outputted to a single file.
	
	Inputs:
	- char *hostname: Destination IP address or hostname to perform these network 
			tests on
	- TestResults *testResults: Pointer to structures being used to store network 
			testing statistics across all tests
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool runTests(char *, TestResults *);

/*
	Description:
	Uses fork and execv system calls to run a bash script (Ping_Test) which runs 
	ping program with given parameters and redirects the output to pingData.txt
	
	Inputs:
	- char *hostname: Destination IP address or hostname to perform ping test on
	- int numPackets: Number of packets sent during ping test
	- int numBytes: Number of bytes sent per packet (not including ICMP header) 
	- double interval: Amount of time in seconds to wait before sending each 
			ICMP packet (must be at least 0.2 seconds)
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool runPing(char *, int, int, double);

/*
	Description:
	Extracts important statistics from pingData.txt into a PingResults structure. 
	This function is assumed to be called each time directly after calling runPing(). 
	Note that the size of the PingResults array passed by reference to this function 
	is dynamically increased using realloc() each time this function executes to 
	make room for the structure which the most recent ping test's results will be
	written to
	
	Inputs:
	- PingResults **pingResults: Contains the address of a pointer to a PingResults
			array. This is because the change in array size needs to be seen by the 
			calling function
	- int numPingTests: Number of times the ping program has been run (used each 
			time this function is called to resize the PingResults array)
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool extractPingStats(PingResults **, int);

/*
	Description:
	Uses fork and execv system calls to run a bash script (TracerouteTest) which 
	runs traceroute program with given parameters and redirects the output to 
	tracerouteData.txt
	
	Inputs:
	- char *hostname: Destination IP address or hostname to perform traceroute 
			test on
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool runTraceroute(char *);

/*
	Description:
	Extracts important statistics from tracerouteData.txt into a TracerouteResults 
	structure. This function is assumed to be called only once during program 
	execution, directly after calling runTraceroute()
	
	Inputs:
	- TracerouteResults *tracerouteResults: Pointer to a structure to store relevant 
			statistics from traceroute test
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool extractTracerouteStats(TracerouteResults *);

/*
	Description:
	Uses fork and execv system calls to run a bash script (Iperf_Test) which runs 
	iPerf program with given parameters and redirects the output to iperfData.txt
	
	Inputs:
	- char *hostname: Destination IP address or hostname to perform iPerf test on 
	- int bandwidth: The maximum allowable throughput that the test can reach
	- int numBytes: The number of bytes in each packet
	- int testTime: The amount of time to run the test for
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool runIperf(char *, int, int, int);

/*
	Description:
	Extracts important statistics from iperfData.txt into an IperfResults structure. 
	This function is assumed to be called each time directly after calling
	runIperf(). Note that the size of the IperfResults array passed by reference to 
	this function is dynamically increased using realloc() each time the function 
	executes to make room for the structure which the most recent iPerf test results
	will be written to
	
	Inputs:
	- IperfResults **iperfResults: Contains the address of a pointer to an
			IperfResults array. This is because the change in array size needs to 
			be seen by the calling function
	- int numIperfTests: Number of times the iPerf program has been run (used each 
			time this function is called to resize the IperfResults array)
	
	Outputs:
	- bool retVal: Flag determining if the function executed correctly
*/
bool extractIperfStats(IperfResults **, int);

/*
	Description:
	Prints out the hostname and service name of the client/server socket being used 
	to perform the custom network test (if available) to the console. If no hostname 
	or service names are available, the IP address or port number will be printed 
	out instead
	
	Inputs:
	- const struct sockaddr *addr: Pointer to a structure containing the IP address 
			 and port number of the socket
*/
void printSocketAddress(const struct sockaddr *);


#endif
