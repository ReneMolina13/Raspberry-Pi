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
	for (stats->iteration = 1; parameter->status == 1 && retVal == true && stats->iteration <= MAX_ITERATIONS; stats->iteration++, numErrors = 0) {
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
	if (retVal == false) {
		fputs("Transmission error occured: ", stderr);
		parameter->status = -1;
	}
	else {
		// fputs("Maximum iterations or 60 seconds reached: ", stdout);
		parameter->status = 0;
	}
	
	// Save status and exit
	// printf("Thread for %u byte packets returning\n", stats->bytesPerPacket);
	pthread_exit(0);
}


bool clientSetup(int argc, char **argv ,NetInfo *sockData, Packets *packets, TestResults *testResults)
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
	
	// Initialize semaphore and test statistics structures
	pthread_mutex_init(&mutex, NULL);
	testResults->numPingTests = 0;
	testResults->numIperfTests = 0;
	testResults->pingResults = (PingResults *) calloc(1, sizeof(PingResults));
	testResults->iperfResults = (IperfResults *) calloc(1, sizeof(IperfResults));
	
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
	
	// Print out hostname and service name of server
	char hostname[NI_MAXHOST];
	char service[NI_MAXSERV];
	getnameinfo(sockData->serverAddr->ai_addr, sockData->serverAddr->ai_addrlen, hostname, NI_MAXHOST, service, NI_MAXSERV, 0);
	printf("Hostname: %s\n", hostname);
	printf("Service: %s\n\n", service);

	return true;
}


bool sendPacket(const NetInfo *sockData, const char *packet, unsigned int packetSize)
{
	// Send the requested transaction to the server
	ssize_t bytesSent = sendto(sockData->clientSocket, packet, packetSize, 0, sockData->serverAddr->ai_addr, sockData->serverAddr->ai_addrlen);
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


bool formatOutput(TestResults *testResults)
{
	// Write data from structures holding test results to networkTestResults.csv
	FILE *outFile = fopen("../networkTestResults.csv", "w");
	
	// Check to see if there was an error opening the output file
	if (outFile == NULL) {
		fputs("Could not open the output file\n", stderr);
		return false;
	}
	
	// Print out custom test results
	fprintf(outFile, "Custom Test,\n");
	fprintf(outFile, "Total Iterations,Average Round-Trip-Time (ms),Average Throughput (kB/s),Average Errors Per Packet,Average Errors Per KB,\n");
	fprintf(outFile, "%u,%.3f,%.3f,%.3f,%.3f,\n", testResults->customResults.totalIterations, testResults->customResults.avgRTT, testResults->customResults.avgThroughput, testResults->customResults.avgErrorsPerPacket,testResults->customResults.avgErrorsPerKB);
	fputs("\n", outFile);
		
	// Print out ping test results
	fprintf(outFile, "Ping Test,\n");
	fprintf(outFile, "Test Number,Packet Size (bytes),Packets Transmitted,Packet Loss %%,Minimum Round-Trip-Time (ms),Average Round-Trip-Time (ms),Maximum Round-Trip-Time (ms),Round-Trip-Time Standard Deviation (ms),\n");
	for (int i = 0; i < testResults->numPingTests; i++)
		fprintf(outFile, "%i,%u,%u,%.3f,%.3f,%.3f,%.3f,%.3f,\n", i+1, testResults->pingResults[i].packetSize, testResults->pingResults[i].packetsTransmitted, testResults->pingResults[i].packetLoss, testResults->pingResults[i].minRTT, testResults->pingResults[i].avgRTT, testResults->pingResults[i].maxRTT, testResults->pingResults[i].stdDevRTT);
	fputs("\n", outFile);
	
	// Print out traceroute test results
	fprintf(outFile, "Traceroute Test,\n");
	fprintf(outFile, "Hop Number,Packet Size (bytes),Packet 1 Latency (ms),Packet 2 Latency (ms),Packet 3 Latency (ms),\n");
	for (int i = 0; i < testResults->tracerouteResults.numHops; i++)
		fprintf(outFile, "%i,%u,%.3f,%.3f,%.3f,\n", i+1, testResults->tracerouteResults.bytesPerPacket, testResults->tracerouteResults.hopLatency[i][0], testResults->tracerouteResults.hopLatency[i][1], testResults->tracerouteResults.hopLatency[i][2]);
	fputs("\n", outFile);
	
	// Print out iPerf test results
	double timeline = 0;
	char *dataUnits;
	char *throughputUnits;
	fprintf(outFile, "Iperf Test,\n");
	fprintf(outFile, "Time (s),Packets Sent,Total Data Sent,Total Data Received,Total Data Units,Average Throughput Sent,Average Throughput Received,Average Throughput Units,Jitter Sent (ms),Jitter Received (ms),Packet Loss %% Sent,Packet Loss %% Received,\n");
	for (int i = 0; i < testResults->numIperfTests; i++) {
		switch (testResults->iperfResults[i].dataUnits) {
		case 'M':
			dataUnits = "MBytes";
			break;
		case 'G':
			dataUnits = "GBytes";
			break;
		default:
			dataUnits = "";
		}
		switch (testResults->iperfResults[i].throughputUnits) {
		case 'M':
			throughputUnits = "Mbits/sec";
			break;
		case 'G':
			throughputUnits = "Gbits/sec";
			break;
		default:
			throughputUnits = "";
		}
		printf("Iteration %i - Data Units: %c, Throughput Units: %c\n", i, testResults->iperfResults[i].dataUnits, testResults->iperfResults[i].throughputUnits);
		timeline += testResults->iperfResults[i].secondsPerTest;
		fprintf(outFile, "%.3f,%u,%.3f,%.3f,%s,%.3f,%.3f,%s,%.3f,%.3f,%.3f,%.3f,\n", timeline, testResults->iperfResults[i].packetsSent, testResults->iperfResults[i].dataSent, testResults->iperfResults[i].dataReceived, dataUnits, testResults->iperfResults[i].avgThroughputSent, testResults->iperfResults[i].avgThroughputReceived, throughputUnits, testResults->iperfResults[i].jitterSent, testResults->iperfResults[i].jitterReceived, testResults->iperfResults[i].packetLossSent, testResults->iperfResults[i].packetLossReceived);
	}
	fputs("\n", outFile);
	
	// Close output file
	fclose(outFile);
	
	return true;
}


int main(int argc, char **argv)
{	
	puts("\n************************************************\n");

	// Holds TCP Connection information
	NetInfo sockData;
	// Holds different size data packets (1B - Max size for UDP packets)
	Packets packets;
	// Holds stats extracted from network tests
	TestResults testResults;
	
	// Parse command line arguments and initialize structures
	if (clientSetup(argc, argv, &sockData, &packets, &testResults) == false) {
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
	ThreadArgs *thread_args = (ThreadArgs *) malloc(NUM_PACKET_SIZES * sizeof(ThreadArgs));
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		thread_args[i].sockData = &sockData;
		thread_args[i].stats = &packetStats[i];
		thread_args[i].stats->bytesPerPacket = packets.packetSizes[i];
		thread_args[i].sentPacket = packets.sentPackets[i];
		thread_args[i].receivedPacket = packets.receivedPackets[i];
		thread_args[i].status = 1;
		pthread_create(&thread_args[i].tid, &attr, networkThreads, (void *) &thread_args[i]);
	}
	
	// Create data processing thread
	DataProcessingArgs data_processing_args;
	data_processing_args.packetStats = packetStats;
	pthread_create(&data_processing_args.tid, &attr, dataProcessingThread, (void *) &data_processing_args);
	
	// Wait for data processing thread to complete (~60 seconds)
	pthread_join(data_processing_args.tid, NULL);
	if (data_processing_args.status == false) {
		fputs("Error processing network data - ", stderr);
		return -1;
	}
	
	// Custom network test complete, signal to all network threads to terminate
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		thread_args[i].status = 0;
	}
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		pthread_join(thread_args[i].tid, NULL);
		if (thread_args[i].status == -1) {
			fputs("Error creating network traffic - ", stderr);
			return -1;
		}
	}
	
	// Save results of custom test to CustomResults structure
	if (extractCustomStats(&testResults.customResults) == false) {
		fputs("Error extracting custom stats - ", stderr);
		return -1;
	}
	
	puts("\nCustom network test completed");
	puts("\n************************************************\n");
	
	// Run ping, traceroute, and iPerf tests on the network
	if (runTests(sockData.cmdIP, &testResults) == false) {
		fputs("Error running extra network tests - ", stderr);
		return -1;
	}
	
	puts("External network tests completed");
	puts("\n************************************************\n");
	
	// Close client socket
	if (close(sockData.clientSocket) < 0) {
		fputs("Failed to properly close socket - ", stderr);
		return -1;
	}
	
	puts("Successfully closed socket");
	puts("\n************************************************\n");
	
	// Output all test results into a single file
	if (formatOutput(&testResults) == false) {
		fputs("Error outputting test results - ", stderr);
		return -1;
	}
	
	puts("Network test results successfully processed");
	puts("\n************************************************\n");
	
	// Free memory allocated to server address and thread argument structures
	freeaddrinfo(sockData.serverAddr);
	free(packetStats);
	free(thread_args);
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		free(packets.sentPackets[i]);
		free(packets.receivedPackets[i]);
	}
	
	// Free memory allocated to ping statistics
	free(testResults.pingResults);
	// Free memory allocated to traceroute statistics
	for (int i = 0; i < testResults.tracerouteResults.numHops; i++)
		free(testResults.tracerouteResults.hopLatency[i]);
	free(testResults.tracerouteResults.hopLatency);
	// Free memory allocated to iPerf statistics
	free(testResults.iperfResults);
	
	return 0;
}
