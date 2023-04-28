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
	// Amount of time for thread to sleep before writing to spreadsheet each time
	unsigned int sleepSeconds = 15;
	
	// Detach thread (makes it not joinable)
	pthread_detach(tid);
	
	// Obtain all packet sizes
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		bytesPerPacket[i] = packetStats[i].bytesPerPacket;
	}
	
	while (1) {
		// Sleep for an arbitrary amount of time (to let statistics get updated)
		sleep(sleepSeconds);
		
		// Open file to store data produced by network threads
		FILE *outFile = fopen("../data/testingData.csv", "w");
		
		if (outFile == NULL) {
			fputs("Could not open testing file\n", stderr);
			break;
		}
		
		// Write 1st line of file (contains labels for spreadsheet)
		fprintf(outFile, "Packet Size,Packets Sent,Average Round-Trip Time (ms) Average Latency (KB/s),Errors/Packet,Errors/KB,\n");
		
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
		printf("File is written, will be updated in %i seconds\n\n", sleepSeconds);
	}
	
	return 0;
}


void *testingThread(void *param)
{
	// Extract parameters from argument structure
	TestingArgs *parameter = (TestingArgs *) param;
	pthread_t tid = parameter->tid;
	char *hostname = parameter->hostname;
	char *service = parameter->service;
	
	puts("Running Ping");
	runPing(hostname, 10, 1000, 0.5, false);
	puts("Running Traceroute");
	runTraceroute(hostname);
	puts("Running iPerf");
	runIperf(hostname, service, 500, 8000, 1, 10);
	
	// Detach thread (makes it not joinable)
	pthread_detach(tid);
	
	return 0;
}


bool runPing(char *hostname, int numPackets, int numBytes, double interval, bool flood)
{	
	char **args;
	unsigned int numArgs;
	unsigned int buffSize = 20;	
	
	// Check for invalid arguments
	if (flood == false) {
		if (numPackets < 0 || numBytes < 0) {
			fputs("Invalid parameter value\n", stderr);
			return false;
		}
		else if (interval < 0.2) {
			fputs("Interval below 0.2 seconds detected... setting to 0.2\n", stderr);
			interval = 0.2;
		}
		
		// Initialize argument array
		numArgs = 10;
		args = (char **) malloc(numArgs * sizeof(char *));
		for (int i = 0; i < numArgs; i++)
			args[i] = (char *) malloc(buffSize * sizeof(char));
	
		// Run ping with quiet output
		strncpy(args[0], "../data/Ping_Test", buffSize);
		strncpy(args[1], "-q", buffSize);
		// Number of ping requests
		strncpy(args[2], "-c", buffSize);
		snprintf(args[3], buffSize, "%i", numPackets);
		// Size of ping packet (bytes)
		strncpy(args[4], "-s", buffSize);
		snprintf(args[5], buffSize, "%i", numBytes);
		// Time interval between packets (seconds)
		strncpy(args[6], "-i", buffSize);
		if (interval < 0.2)
			interval = 0.2;
		snprintf(args[7], buffSize, "%f", interval);
		// Hostname / IP Address
		strncpy(args[8], hostname, buffSize);
		// Null-terminate argument array
		snprintf(args[9], buffSize, "%p", NULL);
	}
	
	else {
		// Flood testing only requires hostname
		numArgs = 3;
		args = (char **) malloc(numArgs * sizeof(char *));
		for (int i = 0; i < numArgs; i++)
			args[i] = (char *) malloc(buffSize * sizeof(char));
		strncpy(args[0], "../data/Flood_Test", buffSize);
		strncpy(args[1], hostname, buffSize);
		args[2] = NULL;
	}

	// Fork process
	int childExitStatus;
	int pid = fork();
	// Indicates Fork error
	if (pid < 0) {
		fputs("Unable to fork process\n", stderr);
		return false;
	}
	// Child process runs ping program
	else if (pid == 0)
		 if (execv(args[0], args) < 0) {
			fputs("Error calling ping\n", stderr);
			return false;
		 }

	// Parent process frees argument array & waits for program to finish
	else {
		for (int i = 0; i < numArgs; i++)
			free(args[i]);
		free(args);
		wait(&childExitStatus);
	}
	
	puts("Results have been saved to pingData.txt");
	
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
	int childExitStatus;
	int pid = fork();
	// Indicates fork error
	if (pid < 0) {
		fputs("Unable to fork process\n", stderr);
		return false;
	}
	// Child process runs traceroute
	else if (pid == 0)
		if (execv(args[0], args) < 0) {
			fputs("Error calling traceroute\n", stderr);
			return false;
		 }
	// Parent process waits for child to finish executing
	else
		wait(&childExitStatus);
		
	puts("Results have been saved to tracerouteData.txt");
		
	return true;
}


bool runIperf(char *hostname, char *service, double bandwidth, int numBytes, int interval, int totalTime)
{
	if (service < 0 || bandwidth < 0 || numBytes < 0 || interval < 0 || totalTime < 0) {
		fputs("Invalid parameter value\n", stderr);
		return false;
	}
	
	// Initialize arguments array
	// unsigned int numArgs = 18;

//TESTING
//********************************************************************************************
	unsigned int numArgs = 5;
//********************************************************************************************
	unsigned int buffSize = 20;
	char **args = (char **) malloc(numArgs * sizeof(char *));
	for (int i = 0; i < numArgs; i++)
		args[i] = (char *) malloc(buffSize * sizeof(char));
	
	
//TESTING
//********************************************************************************************
	strncpy(args[0], "../data/Iperf_Test", buffSize);
	strncpy(args[1], "-u", buffSize);
	strncpy(args[2], "-c", buffSize);
	strncpy(args[3], hostname, buffSize);
	snprintf(args[4], buffSize, "%p", NULL);
//********************************************************************************************
	
	/*
	// Run iPerf using UDP, verbose mode, and formatted in KBytes/sec
	strncpy(args[0], "../data/Iperf_Test", buffSize);
	strncpy(args[1], "-u", buffSize);
	strncpy(args[2], "-V", buffSize);
	strncpy(args[3], "-f", buffSize);
	strncpy(args[4], "K", buffSize);
	// Hostname / IP address
	strncpy(args[5], "-c", buffSize);
	strncpy(args[6], hostname, buffSize);
	// Service / Port Number
	strncpy(args[7], "-p", buffSize);
	strncpy(args[8], service, buffSize);
	// Target bandwidth (bits/sec)
	strncpy(args[9], "-b", buffSize);
	snprintf(args[10], buffSize, "%f", bandwidth);
	// Buffer length (bytes)
	strncpy(args[11], "-l", buffSize);
	snprintf(args[12], buffSize, "%i", numBytes);
	// Interval for bandwidth, jitter, & loss reports (seconds)
	strncpy(args[13], "-i", buffSize);
	snprintf(args[14], buffSize, "%i", interval);
	// Total time alloted to iPerf test
	strncpy(args[15], "-t", buffSize);
	snprintf(args[16], buffSize, "%i", totalTime);
	// Null-terminate argument array
	args[17] = NULL;
	*/
	int childExitStatus;
	int pid = fork();
	// Fork error
	if (pid < 0) {
		fputs("Unable to fork process\n", stderr);
		return false;
	}
	// Child process
	else if (pid == 0) {
		if (execv(args[0], args) < 0) {
			fputs("Error calling iPerf\n", stderr);
			return false;
		 }
	}
	// Parent process
	else {
		for (int i = 0; i < numArgs; i++)
			free(args[i]);
		free(args);
		wait(&childExitStatus);
		
	}
	
	// puts("Results have been saved to iperfData.txt", pid);
	printf("Results have been saved to iperfData.txt - PID: %i\n", pid);
	
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
