/*
	Rene Molina
	EE 4230- Senior Design 2
*/


#include "test.h"


void *dataProcessingThread(void *param)
{	
	// Extract parameters from argument structure
	DataProcessingArgs *parameter = (DataProcessingArgs *) param;
	pthread_t tid = parameter->tid;
	NetStats *packetStats = parameter->packetStats;
	// Temp variables for file output
	unsigned int bytesPerPacket[NUM_PACKET_SIZES];
	unsigned int iteration[NUM_PACKET_SIZES];
	double avgRoundTripTime[NUM_PACKET_SIZES];
	double kiloBytesPerSecond[NUM_PACKET_SIZES];
	double errorsPerPacket[NUM_PACKET_SIZES];
	double errorsPerKB[NUM_PACKET_SIZES];
	// Variables for average over all packet sizes
	unsigned int avgIterations;
	unsigned int totalIterations = 0;
	double totalAvgRTT = 0;
	double avgKBpS = 0;
	double avgEpPk = 0;
	double avgEpKB = 0;
	// True if no transmission errors have occured
	bool retVal = true;
	// Amount of time for thread to sleep before writing to spreadsheet each time
	unsigned int sleepSeconds = 15;
	
	// Obtain all packet sizes
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		bytesPerPacket[i] = packetStats[i].bytesPerPacket;
	}
	
// TESTING
//********************************************************************************
	// for (int i = 0; i < 60; i += sleepSeconds) {
	for (int i = 0; i < 15; i += sleepSeconds) {
//********************************************************************************
		// Sleep for an arbitrary amount of time (to let statistics get updated)
		sleep(sleepSeconds);
		
		// Open file to store data produced by network threads
		FILE *outFile = fopen("../data/customTestData.csv", "w");
		
		if (outFile == NULL) {
			fputs("Could not open testing file\n", stderr);
			retVal = false;
		}
		
		// Write 1st line of file (contains labels for spreadsheet)
		fprintf(outFile, "Packet Size,Packets Sent,Average Round-Trip Time (ms),Average Throughput (KB/s),Errors/Packet,Errors/KB,\n");
		
		// Iterate through one packet size at a time
		for (int i = 0; i < NUM_PACKET_SIZES; i++) {
			// Store current statistical data in temporary buffer
			iteration[i] = packetStats[i].iteration;
			avgRoundTripTime[i] = packetStats[i].avgRoundTripTime;
			errorsPerPacket[i] = packetStats[i].errorsPerPacket;
			// Write packet size to spreadsheet
			fprintf(outFile, "%u-Byte Packet,", bytesPerPacket[i]);
			// Write number of packets sent of this size so far
			fprintf(outFile, "%u,", iteration[i]);
			// Write average round-trip time (ms) to spreadsheet
			fprintf(outFile, "%.2f,", avgRoundTripTime[i]);
			// Calculate and write average latency for this packet (KB/s = B/ms)
			kiloBytesPerSecond[i] = bytesPerPacket[i] / avgRoundTripTime[i];
			fprintf(outFile, "%.2f,", kiloBytesPerSecond[i]);
			// Write number of incorrect bits per packet
			fprintf(outFile, "%.2f,", errorsPerPacket[i]);
			// Calculate and write number of incorrect bits per KB
			errorsPerKB[i] = 1000 * errorsPerPacket[i] / bytesPerPacket[i];
			fprintf(outFile, "%.2f,", errorsPerKB[i]);
			// Add a newline to end row
			fputc('\n', outFile);
		}
		
		// Determine total number of iterations
		for (int i = 0; i < NUM_PACKET_SIZES; i++)
			totalIterations += iteration[i];
		
		// Find averages
		for (int i = 0; i < NUM_PACKET_SIZES; i++) {
			totalAvgRTT += (avgRoundTripTime[i] * iteration[i]) / totalIterations;
			avgKBpS += bytesPerPacket[i] * (iteration[i] / (1000 * NUM_PACKET_SIZES));
			avgEpPk += (errorsPerPacket[i] * iteration[i]) / totalIterations;
			avgEpKB += (errorsPerKB[i] * iteration[i]) / totalIterations;
		}
		avgKBpS /= totalAvgRTT;
		avgIterations = ((double) totalIterations) / NUM_PACKET_SIZES;
		
		// Write averages to spreadsheet
		fprintf(outFile, "Averages,%u,%.2f,%.2f,%.2f,%.2f,\n", avgIterations, totalAvgRTT, avgKBpS, avgEpPk, avgEpKB);
		
		// Reset averages to zero
		totalIterations = 0;
		totalAvgRTT = 0;
		avgKBpS = 0;
		avgEpPk = 0;
		avgEpKB = 0;
		
		// Close file and let user know file has been written
		fclose(outFile);
		printf("File is written, will be updated in %i seconds\n", sleepSeconds);
	}
	
	parameter->status = retVal;
	pthread_exit(0);
}


bool extractCustomStats(CustomResults *customResults)
{
	// Open custom file
	FILE *customFile = fopen("../data/customTestData.csv", "r");
	
	// Check to see if there was an error opening custom file
	if (customFile == NULL) {
		fputs("Could not open one of the input files\n", stderr);
		return false;
	}
	
	// Extract data from custom test restults file into CustomResults structure
	unsigned int varsAssigned = 0;
	unsigned int temp;
	customResults->totalIterations = 0;
	
	// Extract total packets sent over the course of the network test
	while (fgetc(customFile) != '\n');		// Skip past first row
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		while (fgetc(customFile) != ',');	// Skip past packet size column
		varsAssigned += fscanf(customFile, "%u", &temp);
		customResults->totalIterations += temp;
		while (fgetc(customFile) != '\n');	// Skip to next row
	}
	varsAssigned /= NUM_PACKET_SIZES;
	
	// Extract averages from final row of customFile
	while (fgetc(customFile) != ',');		// Skip past packet size column
	while (fgetc(customFile) != ',');		// Skip past packets sent column
	varsAssigned += fscanf(customFile, "%lf", &customResults->avgRTT);
	fgetc(customFile);	// Skip past comma
	varsAssigned += fscanf(customFile, "%lf", &customResults->avgThroughput);	
	fgetc(customFile);	// Skip past comma
	varsAssigned += fscanf(customFile, "%lf", &customResults->avgErrorsPerPacket);
	fgetc(customFile);	// Skip past comma
	varsAssigned += fscanf(customFile, "%lf", &customResults->avgErrorsPerKB);
	
	// Make sure all variables in customResults structure have been assigned
	if (varsAssigned != 5) {
		fputs("Incorrect number of variables assinged for customResults structure\n", stderr);
		return false;
	}

/*
// TESTING
//*******************************************************************
	puts("Custom test structure filled");
	printf("Total Iterations: %u\n", customResults->totalIterations);
	printf("Average RTT: %f\n", customResults->avgRTT);
	printf("Average Throughput: %f\n", customResults->avgThroughput);
	printf("Average Errors Per Packet: %f\n", customResults->avgErrorsPerPacket);
	printf("Average Errors Per KB: %f\n", customResults->avgErrorsPerKB);
	fputs("\n", stdout);
//*******************************************************************
*/
	
	// Close custom file
	fclose(customFile);
	
	return true;
}


bool runTests(char *hostname, TestResults *testResults)
{
	// Ensures that each testing program runs successfully
	bool retVal = true;

	// Run ping tests
	unsigned int pingBytes;
	for (int i = 10; i < 15; i++) {
		printf("Ping Iteration number: %i, ", i-9);
		pingBytes = (unsigned int) pow(2, i);
		runPing(hostname, 10, pingBytes, 0.5);
		sleep(5);
		retVal = extractPingStats(&testResults->pingResults, ++(testResults->numPingTests));
		printf("Ping executed with packet size of %i\n", pingBytes);
	}
	fputs("\n", stdout);
	
	// Run traceroute test
	runTraceroute(hostname);
	sleep (5);
	retVal = extractTracerouteStats(&testResults->tracerouteResults);
	puts("Traceroute executed\n");
	
	// Run iPerf tests
		for (int bandwidth = 150; bandwidth <= MAX_BANDWIDTH; bandwidth+= 150) {
			printf("iPerf iteration number: %i, ", bandwidth/150);
			runIperf(hostname, bandwidth, 8, 1);
			sleep(5);
			retVal = extractIperfStats(&testResults->iperfResults, ++(testResults->numIperfTests));
			printf("iPerf executed with bandwidth of %i\n", bandwidth);
		}
	fputs("\n", stdout);
	
	return true;
}


bool runPing(char *hostname, int numPackets, int numBytes, double interval)
{	
	// Check for invalid arguments
	if (numPackets < 0 || numBytes < 0) {
		fputs("Invalid parameter value\n", stderr);
		return false;
	}
	else if (interval < 0.2) {
		fputs("Interval below 0.2 seconds detected... setting to 0.2\n", stderr);
		interval = 0.2;
	}
	
	// Initialize argument array
	unsigned int numArgs = 10;
	unsigned int buffSize = 30;
	char **args = (char **) malloc(numArgs * sizeof(char *));
	for (int i = 0; i < numArgs; i++)
		args[i] = (char *) malloc(buffSize * sizeof(char));
	
	// Run ping
	snprintf(args[0], buffSize, "../data/Ping_Test");
	// Specify number of ping requests
	snprintf(args[1], buffSize, "-c");
	snprintf(args[2], buffSize, "%i", numPackets);
	// Quiet output
	snprintf(args[3], buffSize, "-q");
	// Size of ping packet (bytes)
	snprintf(args[4], buffSize, "-s");
	snprintf(args[5], buffSize, "%i", numBytes);
	// Time interval between packets (seconds)
	snprintf(args[6], buffSize, "-i");
	snprintf(args[7], buffSize, "%f", interval);
	// Hostname / IP Address
	snprintf(args[8], buffSize, "%s", hostname);
	// Null-terminate argument array
	args[9] = NULL;
		
	// Fork process
	pid_t pid = fork();
	// Indicates Fork error
	if (pid < 0) {
		fputs("Unable to fork process\n", stderr);
		return false;
	}
	// Child process runs ping program
	else if (pid == 0)
		 if (execv(args[0], args) < 0) {
			fputs("Child Process: Error calling ping\n", stderr);
			fprintf(stderr,"%s\n", strerror(errno));
			exit(0);
		 }
	// Parent process frees argument array & waits for program to finish
	else {
		waitpid(pid, NULL, 0);
		for (int i = 0; i < numArgs; i++)
			free(args[i]);
		free(args);
		return true;
	}
}


bool extractPingStats(PingResults **pingResults, int numPingTests)
{
	// Open ping file
	FILE *pingFile = fopen("../data/pingData.txt", "r");
	
	// Check to see if there was an error opening ping file
	if (pingFile == NULL) {
		fputs("Could not open one of the input files\n", stderr);
		return false;
	}
	
	// Allocate memory for pingResults structure for this test
	*pingResults = (PingResults *) realloc(*pingResults, numPingTests * sizeof(PingResults));
	
	// Extract data from ping file into the PingResults structure
	unsigned int currentPingTest = numPingTests - 1;
	unsigned int varsAssigned = 0;

	// Extract packet size for this iteration
	while (fgetc(pingFile) != '(');
	while (fgetc(pingFile) != '(');
	varsAssigned += fscanf(pingFile, "%u", &(*pingResults)[currentPingTest].packetSize);
	// Extract packets transmitted
	for (int j = 0; j < 3; j++)
		while (fgetc(pingFile) != '\n');
	varsAssigned += fscanf(pingFile, "%u", &(*pingResults)[currentPingTest].packetsTransmitted);
	// Extract packet loss percentage
	for (int j = 0; j < 5; j++)
		while (fgetc(pingFile) != ' ');
	varsAssigned += fscanf(pingFile, "%lf", &(*pingResults)[currentPingTest].packetLoss);
	// Extract min, max, avg, and std deviation for round-trip-times
	for (int j = 0; j < 7; j++)
		while (fgetc(pingFile) != ' ');		
	varsAssigned += fscanf(pingFile, "%lf", &(*pingResults)[currentPingTest].minRTT);
	fgetc(pingFile);
	varsAssigned += fscanf(pingFile, "%lf", &(*pingResults)[currentPingTest].avgRTT);
	fgetc(pingFile);
	varsAssigned += fscanf(pingFile, "%lf", &(*pingResults)[currentPingTest].maxRTT);
	fgetc(pingFile);
	varsAssigned += fscanf(pingFile, "%lf", &(*pingResults)[currentPingTest].stdDevRTT);
	
	// Make sure all variables from pingResults structure have been assigned
	if (varsAssigned != 7) {
		fputs("Incorrect number of variables assigned for pingResults structure\n", stderr);
		return false;
	}

/*
// TESTING
//*******************************************************************
	puts("Ping test structure filled");
	printf("Packet Size: %u\n", *pingResults[currentPingTest].packetSize);
	printf("Packets Transmitted: %u\n", *pingResults[currentPingTest].packetsTransmitted);
	printf("Packet Loss: %f\n", *pingResults[currentPingTest].packetLoss);
	printf("Min RTT: %f\n", *pingResults[currentPingTest].minRTT);
	printf("Average RTT: %f\n", *pingResults[currentPingTest].avgRTT);
	printf("Max RTT: %f\n", *pingResults[currentPingTest].maxRTT);
	printf("RTT Standard Deviation: %f\n", *pingResults[currentPingTest].stdDevRTT);
	fputs("\n", stdout);
//*******************************************************************
*/
	
	// Close ping file
	fclose(pingFile);
	
	return true;
}


bool runTraceroute(char *hostname)
{	
	// Initialize argument array
	char *args[3];
	args[0] = "../data/Traceroute_Test";
	args[1] = hostname;
	args[2] = NULL;

	// Fork process
	pid_t pid = fork();
	// Indicates fork error
	if (pid < 0) {
		fputs("Unable to fork process\n", stderr);
		return false;
	}
	// Child process runs traceroute
	else if (pid == 0)
		if (execv(args[0], args) < 0) {
			fputs("Child Process: Error calling traceroute\n", stderr);
			fprintf(stderr,"%s\n", strerror(errno));
			exit(0);
		 }
	// Parent process waits for child to finish executing
	else {
		waitpid(pid, NULL, 0);
		return true;
	}
}


bool extractTracerouteStats(TracerouteResults *tracerouteResults)
{
	// Open traceroute file
	FILE *tracerouteFile = fopen("../data/tracerouteData.txt", "r");
	
	// Check to see if there was an error opening traceroute file
	if (tracerouteFile == NULL) {
		fputs("Could not open one of the input files\n", stderr);
		return false;
	}
	
	// Extract data from traceroute file into TracerouteResults structure
	tracerouteResults->numHops = -1;
	unsigned int varsAssigned = 0;
	bool hopLine = true;
	char c;
	
	// Determine number of rows and number of hops
	while (!feof(tracerouteFile)) {
		c = fgetc(tracerouteFile);
		if (c == '*')
			hopLine = false;
		else if (c == '\n' && hopLine == true)
			tracerouteResults->numHops++;
	}
	varsAssigned++;
	fseek(tracerouteFile, 0, SEEK_SET);
	
	// Allocate memory for hop latency member variable
	tracerouteResults->hopLatency = (double **) malloc(tracerouteResults->numHops * sizeof(double *));
	for (int i = 0; i < tracerouteResults->numHops; i++)
		tracerouteResults->hopLatency[i] = (double *) malloc(3 * sizeof(double));
	
	// Extract bytes per packet
	for (int i = 0; i < 7; i++)
		while (fgetc(tracerouteFile) != ' ');
	varsAssigned += fscanf(tracerouteFile, "%u", &tracerouteResults->bytesPerPacket);
	while (fgetc(tracerouteFile) != '\n');
	
	// Extract hop latencies
	hopLine = false;
	for (int i = 0; i < tracerouteResults->numHops; i++) {
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
			varsAssigned += fscanf(tracerouteFile, "%lf", &tracerouteResults->hopLatency[i][k]);
		}
		while (fgetc(tracerouteFile) != '\n');
	}
	
	// Make sure all variables from tracerouteResults structure have been assigned
	if (varsAssigned != (tracerouteResults->numHops * 3) + 2) {
		fputs("Incorrect number of variables assigned for tracerouteResults structure\n", stderr);
		return false;
	}

/*
// TESTING
//*******************************************************************
	puts("Traceroute test structure filled");
	printf("Num Hops: %u\n", tracerouteResults->numHops);
	printf("Bytes Per Packet: %u\n", tracerouteResults->bytesPerPacket);
	for (int i = 0; i < tracerouteResults->numHops; i++)
		printf("Hop Latency (Hop %i): %f %f %f\n", i, tracerouteResults->hopLatency[i][0], tracerouteResults->hopLatency[i][1], tracerouteResults->hopLatency[i][2]);
	fputs("\n", stdout);
//*******************************************************************
*/

	// Close traceroute file
	fclose(tracerouteFile);
	
	return true;
}


bool runIperf(char *hostname, int bandwidth, int numBytes, int testTime)
{	
	// Check for valid arguments
	if (bandwidth < 0 || numBytes < 0 || testTime < 0) {
		fputs("Invalid parameter value\n", stderr);
		return false;
	}
	
	// Initialize arguments array
	unsigned int buffSize = 30;
	unsigned int numArgs = 11;
	char **args = (char **) malloc(numArgs * sizeof(char *));
	for (int i = 0; i < numArgs; i++)
		args[i] = (char *) malloc(buffSize * sizeof(char));
	
	// Run iPerf
	snprintf(args[0], buffSize, "../data/Iperf_Test");
	// Client mode at specified IP address
	snprintf(args[1], buffSize, "-c");
	snprintf(args[2], buffSize, "%s", hostname);
	// UDP Protocol
	snprintf(args[3], buffSize, "-u");
	// Specify bandwidth
	snprintf(args[4], buffSize, "-b");
	snprintf(args[5], buffSize, "%im", bandwidth);
	// Specify packet length (bytes)
	snprintf(args[6], buffSize, "-l");
	snprintf(args[7], buffSize, "%ik", numBytes);
	// Specify total time of test
	snprintf(args[8], buffSize, "-t");
	snprintf(args[9], buffSize, "%i", testTime);
	// Null-terminate argument array
	args[10] = NULL;
	
	// Fork process
	pid_t pid = fork();
	// Indicates fork error
	if (pid < 0) {
		fputs("Unable to fork process\n", stderr);
		return false;
	}
	// Child process
	else if (pid == 0) {		
		if (execv(args[0], args) < 0) {
			fputs("Child Process: Error calling iPerf\n", stderr);
			fprintf(stderr,"%s\n", strerror(errno));
			exit(0);
		}
	}
	// Parent process
	else {
		waitpid(pid, NULL, 0);
		for (int i = 0; i < numArgs; i++)
			free(args[i]);
		free(args);
		return true;
	}
}


bool extractIperfStats(IperfResults **iperfResults, int numIperfTests)
{
	// Open iPerf file
	FILE *iperfFile = fopen("../data/iperfData.txt", "r");
	
	// Check to see if there was an error opening iPerf file
	if (iperfFile == NULL) {
		fputs("Could not open one of the input files\n", stderr);
		return false;
	}
	
	// Allocate memory for IperfResults structure for this test
	*iperfResults = (IperfResults *) realloc(*iperfResults, numIperfTests * sizeof(IperfResults));
	
	// Extract data from iperf client file into IperfResults structure
	unsigned int currentIperfTest = numIperfTests - 1;
	unsigned int varsAssigned = 0;
	char tempDataUnits[20];
	char tempThroughputUnits[20];

	// Determine how many seconds this test is
	while (fgetc(iperfFile) != '-');
	varsAssigned += fscanf(iperfFile, "%lf", &(*iperfResults)[currentIperfTest].secondsPerTest);
	fseek(iperfFile, 0, SEEK_SET);
	
	// Extract packets sent
	for (int j = 0; j < 3; j++)
		while (fgetc(iperfFile) != '\n');
	for (int j = 0; j < 7; j++) {
		while (fgetc(iperfFile) != ' ');
		while (fgetc(iperfFile) == ' ');
	}
	while (fgetc(iperfFile) != ' ');
	varsAssigned += fscanf(iperfFile, "%u", &(*iperfResults)[currentIperfTest].packetsSent);
	
	// Extract data sent
	for (int j = 0; j < 3; j++)
		while (fgetc(iperfFile) != '\n');
	for (int j = 0; j < 3; j++) {
		while (fgetc(iperfFile) != ' ');
		while (fgetc(iperfFile) == ' ');
	}
	while (fgetc(iperfFile) != ' ');
	varsAssigned += fscanf(iperfFile, "%u", &(*iperfResults)[currentIperfTest].dataSent);
	
	// Extract average throughput of packets sent
	while (fgetc(iperfFile) == ' ');
	while (fgetc(iperfFile) != ' ');
	varsAssigned += fscanf(iperfFile, "%lf", &(*iperfResults)[currentIperfTest].avgThroughputSent);
	
	// Extract jitter of packets sent
	while (fgetc(iperfFile) == ' ');
	while (fgetc(iperfFile) != ' ');
	varsAssigned += fscanf(iperfFile, "%lf", &(*iperfResults)[currentIperfTest].jitterSent);
	
	// Extract packet loss percent of sent packets
	while (fgetc(iperfFile) != '(');
	varsAssigned += fscanf(iperfFile, "%lf", &(*iperfResults)[currentIperfTest].packetLossSent);
	
	// Extract data received
	while (fgetc(iperfFile) != '\n');
	for (int j = 0; j < 3; j++) {
		while (fgetc(iperfFile) != ' ');
		while (fgetc(iperfFile) == ' ');
	}
	while (fgetc(iperfFile) != ' ');
	varsAssigned += fscanf(iperfFile, "%u", &(*iperfResults)[currentIperfTest].dataReceived);
	
	// Extract units for data sent/received
	fgetc(iperfFile);
	varsAssigned += fscanf(iperfFile, "%s", (*iperfResults)[currentIperfTest].dataUnits);
	// varsAssigned += fscanf(iperfFile, "%s", tempDataUnits);
	// snprintf(args[0], buffSize, "../data/Iperf_Test");
	// Extract average throughput of packets received
	varsAssigned += fscanf(iperfFile, "%lf", &(*iperfResults)[currentIperfTest].avgThroughputReceived);
	
	// Extract units for average throughput sent/received
	fgetc(iperfFile);
	varsAssigned += fscanf(iperfFile, "%s", (*iperfResults)[currentIperfTest].throughputUnits);
	// varsAssigned += fscanf(iperfFile, "%s", tempThroughputUnits);
	
	
	// Extract jitter of packets received
	varsAssigned += fscanf(iperfFile, "%lf", &(*iperfResults)[currentIperfTest].jitterReceived);
	
	// Extract packet loss percent of received packets
	while (fgetc(iperfFile) != '(');
	varsAssigned += fscanf(iperfFile, "%lf", &(*iperfResults)[currentIperfTest].packetLossReceived);

	// Make sure all variables from iperfResults structure have been assigned
	if (varsAssigned != 12) {
		fputs("Incorrect number of variables assigned for iperfResults structure\n", stderr);
		return false;
	}

/*
// TESTING
//*******************************************************************
	puts("Iperf test structure filled");
	printf("Seconds Per Test: %f\n", *iperfResults[currentIperfTest].secondsPerTest);
	printf("Packets Sent: %u\n", *iperfResults[currentIperfTest].packetsSent);
	printf("Data Sent: %f\n", *iperfResults[currentIperfTest].dataSent);
	printf("Data Received: %f\n", *iperfResults[currentIperfTest].dataReceived);
	printf("Data Units: %s\n", *iperfResults[currentIperfTest].dataUnits);
	printf("Average Throughput Sent: %f\n", *iperfResults[currentIperfTest].avgThroughputSent);
	printf("Average Throughput Received: %f\n", *iperfResults[currentIperfTest].avgThroughputReceived);
	printf("Throughput Units: %s\n", *iperfResults[currentIperfTest].throughputUnits);
	printf("Jitter Sent: %f\n", *iperfResults[currentIperfTest].jitterSent);
	printf("Jitter Received: %f\n", *iperfResults[currentIperfTest].jitterReceived);
	printf("Packet Loss Sent: %f\n", *iperfResults[currentIperfTest].packetLossSent);
	printf("Packet Loss Received: %f\n", *iperfResults[currentIperfTest].packetLossReceived);
//*******************************************************************
*/
	
	// Close iPerf file
	fclose(iperfFile);
	
	return true;
}


void printSocketAddress(const struct sockaddr *addr)
{
	// Make sure address isn't null
	if (addr == NULL)
		return;
	
	// Used to hold printable IP address
	void *addrNum;
	char buffer[INET6_ADDRSTRLEN];
	
	// Print out correct IP address type
	switch(addr->sa_family) {
	case AF_INET:
		addrNum = &(((struct sockaddr_in *) addr)->sin_addr);
		printf("IPv4 value: %s\n", inet_ntop(addr->sa_family, addrNum, buffer, sizeof(buffer)));
		printf("Port value: %u\n", ntohs(((struct sockaddr_in *) addr)->sin_port));
		break;
	case AF_INET6:
		addrNum = &((struct sockaddr_in6 *) addr)->sin6_addr;
		printf("IPv6 value: %s\n", inet_ntop(addr->sa_family, addrNum, buffer, sizeof(buffer)));
		printf("Port value: %u\n", ntohs(((struct sockaddr_in6 *) addr)->sin6_port));
		break;
	default:
		puts("Unknown address family");
	}
}
