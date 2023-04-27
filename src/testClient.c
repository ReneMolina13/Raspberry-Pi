/*
	Rene Molina
	EE 4230- Senior Design 2
*/


#include "testClient.h"


void *networkThreads(void *param)
{
	// Unpack thread arguments structure
	ThreadArgs *parameter = (ThreadArgs *) param;
	pthread_t tid = parameter->tid;
	NetInfo *sockData = parameter->sockData;
	NetStats *stats = parameter->stats;
	char *sentPacket = parameter->sentPacket;
	char *receivedPacket = parameter->receivedPacket;
	// Used to measure round-trip time each iteration
	double duration;
	struct timespec start, end;
	// Holds number of errors each iteration
	unsigned int numErrors;
	// True if no transmission errors have occured
	bool retVal = true;
	
	// Generate network stats by sending packets to/from server
	for (stats->iteration = 1; retVal == true && stats->iteration <= MAX_ITERATIONS; stats->iteration++, numErrors = 0) {
		// Take semaphore and start clock
		pthread_mutex_lock(&mutex);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		// Send & receive packet from server
		retVal *= sendPacket(sockData, sentPacket, stats->bytesPerPacket);
		retVal *= receivePacket(sockData, receivedPacket, stats->bytesPerPacket);
		// Stop clock and release semaphore
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		pthread_mutex_unlock(&mutex);

		// Count number of incorrect bytes in received packet
		for (int i = 0; i < stats->bytesPerPacket; i++)
			if (receivedPacket[i] != sentPacket[i])
				numErrors++;
		
		// Calculate time taken to send & receive data
		duration = (1000.0*end.tv_sec + 1e-6*end.tv_nsec) - (1000.0*start.tv_sec + 1e-6*start.tv_nsec);
		
		// Adjust averages (new_avg = ((i-1)(old_avg) + new_value) / i)
		stats->avgRoundTripTime = (((stats->iteration-1) * stats->avgRoundTripTime) + duration) / stats->iteration;
		stats->errorsPerPacket = (((stats->iteration-1) * stats->errorsPerPacket) + numErrors) / stats->iteration;
	}
	
	// Check if error occured or if max iterations were reached
	if (retVal == false)
		fputs("Transmission error occured: ", stderr);
	else
		fputs("Maximum iterations reached: ", stderr);
	
	// Save status and exit
	parameter->status = retVal;
	printf("Thread for %u byte packets returning\n", stats->bytesPerPacket);
	pthread_exit(0);
}


bool clientSetup(int argc, char **argv ,NetInfo *sockData, Packets *packets)
{
	// Check for correct number of arguments
	if (argc != 3) {
		puts("Incorrect number of arguments entered:");
		puts("1st argument should be IP address of the server");
		puts("2nd argument should be port number of the server");
		return false;
	}
	
	// Extract network info from command line arguments
	sockData->cmdIP = *(argv + 1);
	sockData->cmdPort = *(argv + 2);
	
	// Initialize semaphore
	pthread_mutex_init(&mutex, NULL);
	
	// Initialize data packet sizes
	for (int i = 0; i < INDEX_MAX_SIZE_UDP; i++) {
		packets->packetSizes[i] = (int) pow(2, i);
		packets->sentPackets[i] = (char *) malloc(packets->packetSizes[i] * sizeof(char));
		packets->receivedPackets[i] = (char *) calloc(packets->packetSizes[i], sizeof(char));
	}
	packets->packetSizes[INDEX_MAX_SIZE_UDP] = MAX_PACKET_SIZE_UDP;
	packets->sentPackets[INDEX_MAX_SIZE_UDP] = (char *) malloc(MAX_PACKET_SIZE_UDP * sizeof(char));
	packets->receivedPackets[INDEX_MAX_SIZE_UDP] = (char *) calloc(MAX_PACKET_SIZE_UDP, sizeof(char));
	
	// Initialize data packets to be sent out with random (printable) characters
	srand(time(NULL));
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		for (int j = 0; j < packets->packetSizes[i]-1; j++)
			packets->sentPackets[i][j] = (rand() % 94) + 33;
		// Null-terminate each packet
		packets->sentPackets[i][packets->packetSizes[i]-1] = '\0';
	}
	return true;
}


bool createSocket(NetInfo *sockData)
{
	// Initialize structure specifying possible connection types
	struct addrinfo addrCriteria;
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC;	// Any address family
	addrCriteria.ai_socktype = SOCK_DGRAM;	// Only accept datagram sockets
	addrCriteria.ai_protocol = IPPROTO_UDP;	// Only accept UDP protocol
	
	// Get list of server addresses
	int status = getaddrinfo(sockData->cmdIP, sockData->cmdPort, &addrCriteria, &(sockData->serverAddr));
	if (status != 0) {
		fputs("Error getting list of server addresses\n", stderr);
		return false;
	}
	
	// Create a UDP client socket from options given
	sockData->clientSocket = -1;
	for(struct addrinfo *addr = sockData->serverAddr; addr != NULL; addr = addr->ai_next) {
		sockData->clientSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (sockData->clientSocket >= 0) {
			puts("UDP socket successfully created:");
			printSocketAddress(addr->ai_addr);
			fputs("\n", stdout);
			break;
		}
	}
	
	// Indicates no options were valid sockets
	if (sockData->clientSocket < 0) {
		fputs("Error creating socket\n", stderr);
		return false;
	}
	
	return true;
}


bool sendPacket(const NetInfo *sockData, const char *packet, unsigned int packetSize)
{
	// Send the requested transaction to the server
	// pthread_mutex_lock(&mutex);
	ssize_t bytesSent = sendto(sockData->clientSocket, packet, packetSize, 0, sockData->serverAddr->ai_addr, sockData->serverAddr->ai_addrlen);
	// pthread_mutex_unlock(&mutex);
	// Indicates transmission error
	if (bytesSent < 0) {
		fputs("Unable to send data\n", stderr);
		return false;
	}
	else if (bytesSent != packetSize) {
		fputs("Unexpected number of bytes sent\n", stderr);
		return false;
	}
	
	// Transaction successful
	return true;
}


bool receivePacket(const NetInfo *sockData, char *packet, unsigned int packetSize)
{
	// Receive the response from the server
	ssize_t bytesReceived;
	struct sockaddr_storage fromAddr;
	socklen_t fromAddrLength = sizeof(fromAddr);
	bytesReceived = recvfrom(sockData->clientSocket, packet, packetSize, 0, (struct sockaddr *) &fromAddr, &fromAddrLength);
	// Indicates transmission error
	if (bytesReceived < 0) {
		fputs("Unable to receive data\n\n", stderr);
		return false;
	}
	else if (bytesReceived != packetSize) {
		fputs("Unexpected number of bytes received\n\n", stderr);
		return false;
	}
	
	// Transaction successful
	return true;
}


int main(int argc, char **argv)
{	
	puts("\n************************************************\n");

	// Input structures
	NetInfo sockData;	// Holds TCP Connection information
	Packets packets;	// Holds different size data packets (1KB - 1MB)
	
	// Parse command line arguments
	if (clientSetup(argc, argv, &sockData, &packets) == false) {
		fputs("Unable to parse command line arguments - ", stderr);
		return -1;
	}
	
	// Connect to server
	if (createSocket(&sockData) == false) {
		fputs("Unable to connect to server - ", stderr);
		return -1;
	}
	
	puts("************************************************\n");
	
	// Initialize thread argument structures and create network threads
	pthread_attr_init(&attr);
	NetStats *packetStats = (NetStats *) calloc(NUM_PACKET_SIZES, sizeof(NetStats));
	ThreadArgs *thArgs = (ThreadArgs *) malloc(NUM_PACKET_SIZES * sizeof(ThreadArgs));
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		thArgs[i].sockData = &sockData;
		thArgs[i].stats = &packetStats[i];
		thArgs[i].stats->bytesPerPacket = packets.packetSizes[i];
		thArgs[i].sentPacket = packets.sentPackets[i];
		thArgs[i].receivedPacket = packets.receivedPackets[i];
		thArgs[i].status = true;
		pthread_create(&thArgs[i].tid, &attr, networkThreads, (void *) &thArgs[i]);
	}
	
	// Create data processing thread
	DataProcessingArgs dpArgs;
	dpArgs.packetStats = packetStats;
	pthread_create(&dpArgs.tid, &attr, dataProcessingThread, (void *) &dpArgs);
	
	// Create testing thread
	TestingArgs teArgs;
	pthread_create(&teArgs.tid, &attr, testingThread, (void *) &teArgs);
	
	// Wait for network threads to terminate (only happens in case of error or overflow)
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		pthread_join(thArgs[i].tid, NULL);
		if (thArgs[i].status == false) {
			fputs("Error creating network traffic - ", stderr);
			return -1;
		}
	}
	
	puts("************************************************\n");
	
	// Free memory allocated to server address and thread argument structures
	freeaddrinfo(sockData.serverAddr);
	free(packetStats);
	free(thArgs);
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		free(packets.sentPackets[i]);
		free(packets.receivedPackets[i]);
	}

	// Close client socket
	if (close(sockData.clientSocket) < 0) {
		fputs("Failed to properly close socket - ", stderr);
		return -1;
	}
	
	puts("Successfully closed socket");
	puts("\n************************************************\n");

	return 0;
}
