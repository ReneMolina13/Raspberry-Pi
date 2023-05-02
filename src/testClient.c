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
	
	// Detach threads
	pthread_detach(tid);
	
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
		fputs("Maximum iterations or 60 seconds reached: ", stderr);
		parameter->status = 0;
	}
	
	// Save status and exit
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


bool formatOutput()
{
	// Open all five data files for reading
	FILE *customFile = fopen("../data/customTestData.csv", "r");
	FILE *pingFile = fopen("../data/pingData.txt", "r");
	FILE *floodFile = fopen("../data/floodData.txt", "r");
	FILE *tracerouteFile = fopen("../data/tracerouteData.txt", "r");
	FILE *iperfClientFile = fopen("../data/iperfDataClient.txt", "r");
	// Check to see if there was an error opening any of the input files
	if (customFile == NULL || pingFile == NULL || floodFile == NULL || tracerouteFile == NULL || iperfClientFile == NULL) {
		fputs("Could not open one of the input files\n", stderr);
		return false;
	}
	// Holds how many variables have been assigned for a particular result structure
	unsigned int varsAssigned;
	
	
	// Extract data from custom test restults file into CustomResults structure
	CustomResults customResults;
	unsigned int customTemp;
	varsAssigned = 0;
	// Extract total packets sent over the course of the network test
	while (fgetc(customFile) != '\n');		// Skip past first row
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		while (fgetc(customFile) != ',');	// Skip past packet size column
		varsAssigned = fscanf(customFile, "%u", &customTemp);
		customResults.totalIterations += customTemp;
		while (fgetc(customFile) != '\n');	// Skip to next row
	}
	// Extract averages from final row of customFile
	while (fgetc(customFile) != ',');		// Skip past packet size column
	while (fgetc(customFile) != ',');		// Skip past packets sent column
	varsAssigned += fscanf(customFile, "%lf", &customResults.avgRTT);
	fgetc(customFile);	// Skip past comma
	varsAssigned += fscanf(customFile, "%lf", &customResults.avgThroughput);	
	fgetc(customFile);	// Skip past comma
	varsAssigned += fscanf(customFile, "%lf", &customResults.avgErrorsPerPacket);
	fgetc(customFile);	// Skip past comma
	varsAssigned += fscanf(customFile, "%lf", &customResults.avgErrorsPerKB);
	// Make sure all variables in customResults structure have been assigned
	if (varsAssigned != 5) {
		fputs("Incorrect number of variables assinged for customResults structure\n", stderr);
		return false;
	}
	
// TESTING
//*******************************************************************
	puts("Custom test structure filled");
	printf("Total Iterations: %u\n", customResults.totalIterations);
	printf("Average RTT: %f\n", customResults.avgRTT);
	printf("Average Throughput: %f\n", customResults.avgThroughput);
	printf("Average Errors Per Packet: %f\n", customResults.avgErrorsPerPacket);
	printf("Average Errors Per KB: %f\n", customResults.avgErrorsPerKB);
	fputs("\n", stdout);
//*******************************************************************
	
	// Extract data from ping and flood files into the PingResults structure
	PingResults pingResults;
	varsAssigned = 0;
	double pingTemp;
	// Determine how many ping tests were executed (5 rows each)
	char c;
	unsigned int numRows = 0;
	do {
		c = fgetc(pingFile);
		if (c == '\n')
			numRows++;
		
// TESTING
//*******************************************************************
		printf("%c", c);
//*******************************************************************
		
	} while (c >= 0 && c <= 127);
	pingResults.numTests = numRows / 5;
	varsAssigned++;
	rewind(pingFile);
		
// TESTING
//*******************************************************************
	puts("Ping - numTests initialized");
//*******************************************************************

	// Allocate memory for each pingResults member
	pingResults.packetSize = (unsigned int *) malloc(pingResults.numTests * sizeof(unsigned int));
	pingResults.packetsTransmitted = (unsigned int *) malloc(pingResults.numTests * sizeof(unsigned int));
	pingResults.packetLoss = (double *) malloc(pingResults.numTests * sizeof(double));
	pingResults.minRTT = (double *) malloc(pingResults.numTests * sizeof(double));
	pingResults.avgRTT = (double *) malloc(pingResults.numTests * sizeof(double));
	pingResults.maxRTT = (double *) malloc(pingResults.numTests * sizeof(double));
	pingResults.stdDevRTT = (double *) malloc(pingResults.numTests * sizeof(double));
	// Extract data from ping test outputs
	for (int i = 0; i < pingResults.numTests; i++) {
		// Extract packet size for this iteration
		while (fgetc(pingFile) != '(');
		while (fgetc(pingFile) != '(');
		varsAssigned += fscanf(pingFile, "%u", &pingResults.packetSize[i]);
		// Extract packets transmitted
		for (int j = 0; j < 3; j++)
			while (fgetc(pingFile) != '\n');
		varsAssigned += fscanf(pingFile, "%u", &pingResults.packetsTransmitted[i]);
		// Extract packet loss percentage
		for (int j = 0; j < 5; j++)
			while (fgetc(pingFile) != ' ');
		varsAssigned += fscanf(pingFile, "%lf", &pingResults.packetLoss[i]);
		// Extract min, max, avg, and std deviation for round-trip-times
		for (int j = 0; j < 7; j++)
			while (fgetc(pingFile) != ' ');		
		varsAssigned += fscanf(pingFile, "%lf", &pingResults.minRTT[i]);
		fgetc(pingFile);
		varsAssigned += fscanf(pingFile, "%lf", &pingResults.avgRTT[i]);
		fgetc(pingFile);
		varsAssigned += fscanf(pingFile, "%lf", &pingResults.maxRTT[i]);
		fgetc(pingFile);
		varsAssigned += fscanf(pingFile, "%lf", &pingResults.stdDevRTT[i]);
	}
	// Make sure all variables from pingResults structure have been assigned
	if (varsAssigned != (pingResults.numTests * 7) + 1) {
		fputs("Incorrect number of variables assigned for pingResults structure\n", stderr);
		return false;
	}
	
// TESTING
//*******************************************************************
	puts("Ping test structure filled");
	printf("Num Tests: %u\n", pingResults.numTests);
	for (int i = 0; i < pingResults.numTests; i++)
		printf("Packet Size: %u\n", pingResults.packetSize[i]);
	for (int i = 0; i < pingResults.numTests; i++)
		printf("Packets Transmitted: %u\n", pingResults.packetsTransmitted[i]);
	for (int i = 0; i < pingResults.numTests; i++)
		printf("Packet Loss: %f\n", pingResults.packetLoss[i]);
	for (int i = 0; i < pingResults.numTests; i++)
		printf("Min RTT: %f\n", pingResults.minRTT[i]);
	for (int i = 0; i < pingResults.numTests; i++)
		printf("Average RTT: %f\n", pingResults.avgRTT[i]);
	for (int i = 0; i < pingResults.numTests; i++)
		printf("Max RTT: %f\n", pingResults.maxRTT[i]);
	for (int i = 0; i < pingResults.numTests; i++)
		printf("RTT Standard Deviation: %f\n", pingResults.stdDevRTT[i]);
	fputs("\n", stdout);
//*******************************************************************
	
	// Extract data from traceroute file into TracerouteResults structure
	TracerouteResults tracerouteResults;
	tracerouteResults.numHops = 0;
	varsAssigned = 0;
	// Determine number of rows and number of hops
	while (fgetc(tracerouteFile) != '\n');
	while (fgetc(tracerouteFile) != EOF) {
		for (int i = 0; i < 3; i++)
			fgetc(tracerouteFile);
		if (fgetc(tracerouteFile) != '*')
			tracerouteResults.numHops++;
		while (fgetc(tracerouteFile) != '\n');
	}
	varsAssigned++;
	rewind(tracerouteFile);
	// Allocate memory for hop latency member variable
	tracerouteResults.hopLatency = (double **) malloc(tracerouteResults.numHops * sizeof(double *));
	for (int i = 0; i < tracerouteResults.numHops; i++)
		tracerouteResults.hopLatency[i] = (double *) malloc(3 * sizeof(double));
	// Extract bytes per packet
	for (int i = 0; i < 7; i++)
		while (fgetc(tracerouteFile) != ' ');
	varsAssigned += fscanf(tracerouteFile, "%u", &tracerouteResults.bytesPerPacket);
	while (fgetc(tracerouteFile) != '\n');
	// Extract hop latencies
	bool hopLine = false;
	for (int i = 0; i < tracerouteResults.numHops; i++) {
		// While '*' is present in this line, skip to next line
		while (hopLine == false) {
			for (int j = 0; j < 4; j++)
				fgetc(tracerouteFile);
			if (fgetc(tracerouteFile) == '*')
				while (fgetc(tracerouteFile) != '\n');
			else
				hopLine = true;
		}
		// Extract latencies for this hop
		for (int k = 0; k < 3; k++) {
			for (int l = 0; l < 3; l++)
				while (fgetc(tracerouteFile) != ' ');			
			varsAssigned += fscanf(tracerouteFile, "%lf", &tracerouteResults.hopLatency[i][k]);
		}
		while (fgetc(tracerouteFile) != '\n');
	}
	// Make sure all variables from tracerouteResults structure have been assigned
	if (varsAssigned != (tracerouteResults.numHops * 3) + 2) {
		fputs("Incorrect number of variables assigned for tracerouteResults structure\n", stderr);
		return false;
	}
		
// TESTING
//*******************************************************************
	puts("Traceroute test structure filled");
	printf("Num Hops: %u\n", tracerouteResults.numHops);
	printf("Bytes Per Packet: %u\n", tracerouteResults.bytesPerPacket);
	for (int i = 0; i < tracerouteResults.numHops; i++)
		printf("Hop Latency (Hop %i): %f %f %f\n", i, tracerouteResults.hopLatency[i][0], tracerouteResults.hopLatency[i][1], tracerouteResults.hopLatency[i][2]);
	fputs("\n", stdout);
//*******************************************************************
	
	// Extract data from iperf client file into IperfResults structure
	IperfResults iperfResults;
	varsAssigned = 0;
	// Determine how many seconds each test is
	while (fgetc(iperfClientFile) != '-');
	varsAssigned += fscanf(iperfClientFile, "%lf", &iperfResults.secondsPerTest);
	rewind(iperfClientFile);
	// Determine how many tests there are (10 rows per test)
	while (fgetc(iperfClientFile) != EOF)
		for (iperfResults.numTests = 0; fgetc(iperfClientFile) != '\n'; iperfResults.numTests++);
	iperfResults.numTests /= 10;
	varsAssigned++;
	rewind(iperfClientFile);
	// Allocate memory for each iperfResults member
	iperfResults.packetsSent = (unsigned int *) malloc(iperfResults.numTests * sizeof(unsigned int));
	iperfResults.megaBytesSent = (double *) malloc(iperfResults.numTests * sizeof(double));
	iperfResults.megaBytesReceived = (double *) malloc(iperfResults.numTests * sizeof(double));
	iperfResults.avgThroughputSent = (double *) malloc(iperfResults.numTests * sizeof(double));
	iperfResults.avgThroughputReceived = (double *) malloc(iperfResults.numTests * sizeof(double));
	iperfResults.jitterSent = (double *) malloc(iperfResults.numTests * sizeof(double));
	iperfResults.jitterReceived = (double *) malloc(iperfResults.numTests * sizeof(double));
	iperfResults.packetLossSent = (double *) malloc(iperfResults.numTests * sizeof(double));
	iperfResults.packetLossReceived = (double *) malloc(iperfResults.numTests * sizeof(double));
	// Extract results from each test
	for (int i = 0; i < iperfResults.numTests; i++) {
		// Extract packets sent
		for (int j = 0; j < 3; j++)
			while (fgetc(iperfClientFile) != '\n');
		for (int j = 0; j < 16; j++)
			while (fgetc(iperfClientFile) != ' ');
		varsAssigned += fscanf(iperfClientFile, "%u", &iperfResults.packetsSent[i]);
		// Extract MBytes sent
		for (int j = 0; j < 3; j++)
			while (fgetc(iperfClientFile) != '\n');
		for (int j = 0; j < 9; j++)
			while (fgetc(iperfClientFile) != ' ');
		varsAssigned += fscanf(iperfClientFile, "%lf", &iperfResults.megaBytesSent[i]);
		// Extract average throughput of packets sent
		for (int j = 0; j < 3; j++)
			while (fgetc(iperfClientFile) != ' ');
		varsAssigned += fscanf(iperfClientFile, "%lf", &iperfResults.avgThroughputSent[i]);
		// Extract jitter of packets sent
		for (int j = 0; j < 3; j++)
			while (fgetc(iperfClientFile) != ' ');
		varsAssigned += fscanf(iperfClientFile, "%lf", &iperfResults.jitterSent[i]);
		// Extract packet loss percent of sent packets
		while (fgetc(iperfClientFile) != '(');
		varsAssigned += fscanf(iperfClientFile, "%lf", &iperfResults.packetLossSent[i]);
		// Extract MBytes received
		while (fgetc(iperfClientFile) != '\n');
		for (int j = 0; j < 10; j++)
			while (fgetc(iperfClientFile) != ' ');
		varsAssigned += fscanf(iperfClientFile, "%lf", &iperfResults.megaBytesReceived[i]);
		// Extract average throughput of packets received
		for (int j = 0; j < 3; j++)
			while (fgetc(iperfClientFile) != ' ');
		varsAssigned += fscanf(iperfClientFile, "%lf", &iperfResults.avgThroughputReceived[i]);
		// Extract jitter of packets received
		for (int j = 0; j < 3; j++)
			while (fgetc(iperfClientFile) != ' ');
		varsAssigned += fscanf(iperfClientFile, "%lf", &iperfResults.jitterReceived[i]);
		// Extract packet loss percent of received packets
		while (fgetc(iperfClientFile) != '(');
		varsAssigned += fscanf(iperfClientFile, "%lf", &iperfResults.packetLossSent[i]);
	}
	// Make sure all variables from iperfResults structure have been assigned
	if (varsAssigned != (iperfResults.numTests * 9) + 2) {
		fputs("Incorrect number of variables assigned for iperfResults structure\n", stderr);
		return false;
	}
		
// TESTING
//*******************************************************************
	puts("Iperf test structure filled");
	printf("Num Tests: %u\n", iperfResults.numTests);
	for (int i = 0; i < iperfResults.numTests; i++)
		printf("Seconds Per Test: %u\n", iperfResults.secondsPerTest);
	for (int i = 0; i < iperfResults.numTests; i++)
		printf("Packets Sent: %u\n", iperfResults.packetsSent[i]);
	for (int i = 0; i < iperfResults.numTests; i++)
		printf("MB Sent: %f\n", iperfResults.megaBytesSent[i]);
	for (int i = 0; i < iperfResults.numTests; i++)
		printf("MB Received: %f\n", iperfResults.megaBytesReceived[i]);
	for (int i = 0; i < iperfResults.numTests; i++)
		printf("Average Throughput Sent: %f\n", iperfResults.avgThroughputSent[i]);
	for (int i = 0; i < iperfResults.numTests; i++)
		printf("Average Throughput Received: %f\n", iperfResults.avgThroughputReceived[i]);
	for (int i = 0; i < iperfResults.numTests; i++)
		printf("Jitter Sent: %f\n", iperfResults.jitterSent[i]);
	for (int i = 0; i < iperfResults.numTests; i++)
		printf("Jitter Received: %f\n", iperfResults.jitterReceived[i]);
	for (int i = 0; i < iperfResults.numTests; i++)
		printf("Packet Loss Sent: %f\n", iperfResults.packetLossSent[i]);
	for (int i = 0; i < iperfResults.numTests; i++)
		printf("Packet Loss Received: %f\n", iperfResults.packetLossReceived[i]);
//*******************************************************************
	
	// Close all input files
	fclose(customFile);
	fclose(pingFile);
	fclose(floodFile);
	fclose(tracerouteFile);
	fclose(iperfClientFile);
	
	
	// Write data from structures holding test results to networkTestResults.csv
	FILE *outFile = fopen("../networkTestResults.csv", "w");
	// Check to see if there was an error opening the output file
	if (outFile == NULL) {
		fputs("Could not open the output file\n", stderr);
		return false;
	}
	// Print out custom test results
	fprintf(outFile, "Custom Test,\n");
	fprintf(outFile, "Total Iterations,Average Round-Trip-Time,Average Throughput,Average Errors Per Packet,Average Errors Per KB,\n");
	fprintf(outFile, "%u,%f,%f,%f,%f,", customResults.totalIterations, customResults.avgRTT, customResults.avgThroughput, customResults.avgErrorsPerPacket,customResults.avgErrorsPerKB);
	fputs("\n", outFile);
	// Print out ping test results
	fprintf(outFile, "Ping Test,\n");
	fprintf(outFile, "Test Number,Packet Size,Packets Transmitted,Packet Loss,Minimum Round-Trip-Time,Average Round-Trip-Time,Maximum Round-Trip-Time,Round-Trip-Time Standard Deviation,\n");
	for (int i = 0; i < pingResults.numTests; i++)
		fprintf(outFile, "%i,%u,%u,%f,%f,%f,%f,%f,\n", i+1, pingResults.packetSize[i], pingResults.packetsTransmitted[i], pingResults.packetLoss[i], pingResults.minRTT[i], pingResults.avgRTT[i], pingResults.maxRTT[i], pingResults.stdDevRTT[i]);
	fputs("\n", outFile);
	// Print out traceroute test results
	fprintf(outFile, "Traceroute Test,\n");
	fprintf(outFile, "Hop Number,Packet Size,Packet 1 Latency,Packet 2 Latency,Packet 3 Latency,\n");
	for (int i = 0; i < tracerouteResults.numHops; i++)
		fprintf(outFile, "%i,%u,%f,%f,%f,\n", i+1, tracerouteResults.bytesPerPacket, tracerouteResults.hopLatency[i][0], tracerouteResults.hopLatency[i][1], tracerouteResults.hopLatency[i][2]);
	fputs("\n", outFile);
	// Print out iPerf test results
	double timeline = 0;
	fprintf(outFile, "Iperf Test,\n");
	fprintf(outFile, "Time (s),Packets Sent,MB Sent,MB Received,Average Throughput Sent,Average Throughput Received,Jitter Sent,Jitter Received,Packet Loss Sent,Packet Loss Received,\n");
	for (int i = 0; i < iperfResults.numTests; i++) {
		timeline += iperfResults.secondsPerTest;
		fprintf(outFile, "%f,%u,%f,%f,%f,%f,%f,%f,%f,%f,", timeline, iperfResults.packetsSent[i], iperfResults.megaBytesSent[i], iperfResults.megaBytesReceived[i], iperfResults.avgThroughputSent[i], iperfResults.avgThroughputReceived[i], iperfResults.jitterSent[i], iperfResults.jitterReceived[i], iperfResults.packetLossSent[i], iperfResults.packetLossReceived[i]);
	}
	fputs("\n", outFile);
	// Close output file
	fclose(outFile);
		
// TESTING
//*******************************************************************
	puts("Data written to output file");
//*******************************************************************

	// Free dynamic memory
	free(pingResults.packetSize);
	free(pingResults.packetsTransmitted);
	free(pingResults.packetLoss);
	free(pingResults.minRTT);
	free(pingResults.avgRTT);
	free(pingResults.maxRTT);
	free(pingResults.stdDevRTT);
	for (int i = 0; i < tracerouteResults.numHops; i++)
		free(tracerouteResults.hopLatency[i]);
	free(tracerouteResults.hopLatency);
	free(iperfResults.packetsSent);
	free(iperfResults.megaBytesSent);
	free(iperfResults.megaBytesReceived);
	free(iperfResults.avgThroughputSent);
	free(iperfResults.avgThroughputReceived);
	free(iperfResults.jitterSent);
	free(iperfResults.jitterReceived);
	free(iperfResults.packetLossSent);
	free(iperfResults.packetLossReceived);
	
	return true;
}


int main(int argc, char **argv)
{	
	puts("\n************************************************\n");

	// Input structures
	NetInfo sockData;	// Holds TCP Connection information
	Packets packets;	// Holds different size data packets (1B - Max size for UDP packets)
	
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
	
	/*
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
		if (thread_args[i].status == -1) {
			fputs("Error creating network traffic - ", stderr);
			return -1;
		}
	}
	
	puts("Custom network test completed");
	puts("\n************************************************\n");
	*/
	
	// Run ping, traceroute, and iPerf tests on the network
	if (runTests(sockData.cmdIP) == false) {
		fputs("Error running extra network tests - ", stderr);
		return -1;
	}
	
	puts("ping, traceroute, and iPerf tests completed");
	puts("\n************************************************\n");
	
	// Close client socket
	if (close(sockData.clientSocket) < 0) {
		fputs("Failed to properly close socket - ", stderr);
		return -1;
	}
	
	puts("Successfully closed socket");
	puts("\n************************************************\n");
	
	// Output all test results into a single file
	if (formatOutput() == false) {
		fputs("Error outputting test results - ", stderr);
		return -1;
	}
	
	puts("Network test results successfully outputted");
	puts("\n************************************************\n");
	
	// Free memory allocated to server address and thread argument structures
	freeaddrinfo(sockData.serverAddr);
	// free(packetStats);
	// free(thread_args);
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		free(packets.sentPackets[i]);
		free(packets.receivedPackets[i]);
	}

	return 0;
}
