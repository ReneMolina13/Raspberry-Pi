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
	
	for (int i = 0; i < 60; i+= sleepSeconds) {
		// Sleep for an arbitrary amount of time (to let statistics get updated)
		sleep(sleepSeconds);
		
		// Open file to store data produced by network threads
		FILE *outFile = fopen("../data/customTestData.csv", "w");
		
		if (outFile == NULL) {
			fputs("Could not open testing file\n", stderr);
			retVal = false;
		}
		
		// Write 1st line of file (contains labels for spreadsheet)
		fprintf(outFile, "Packet Size,Packets Sent,Average Round-Trip Time (ms),Average Latency (KB/s),Errors/Packet,Errors/KB,\n");
		
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
	
	parameter->status = retVal;
	pthread_exit(0);
}


bool runTests(char *hostname)
{
	// Ensures that each testing program runs successfully
	bool retVal = true;

/*
// TESTING
//********************************************************************************************
	puts("Running Ping");
	retVal = runPing(hostname, 10, 1000, 0.5);
	if (retVal == false)
		fputs("Ping program unsuccessful\n", stderr);
	puts("Running Traceroute");
	retVal = runTraceroute(hostname);
	if (retVal == false)
		fputs("Traceroute program unsuccessful\n", stderr);
	puts("Running iPerf");
	retVal = runIperf(hostname, 500, 1000, 1);
	if (retVal == false)
		fputs("iPerf program unsuccessful\n", stderr);
//********************************************************************************************
*/

	// Run ping tests
	int pingBytes;
	for (int i = 10; i < 15; i++) {
		pingBytes = (int) pow(2, i);
		runPing(hostname, 10, pingBytes, 0.5);
	}
	
	/*
	// Run flood test and traceroute tests
	runFlood(hostname);
	runTraceroute(hostname);
	
	// Run iPerf tests
		for (int bandwidth = 150; bandwidth < MAX_BANDWIDTH; bandwidth+= 150)
			runIperf(hostname, bandwidth, 8, 1);
	*/
	
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
		sleep((int) (numPackets * interval) + 1);
		// puts("Results have been saved to pingData.txt / floodData.txt");
		return true;
	}
}


bool runFlood(char *hostname)
{
	char *args[3];
	args[0] = "../data/Flood_Test";
	args[1] = hostname;
	args[2] = NULL;
		
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
		// puts("Results have been saved to pingData.txt / floodData.txt");
		return true;
	}
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
		sleep(3);
		// puts("Results have been saved to tracerouteData.txt");
		return true;
	}
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
	snprintf(args[0], buffSize, "../data/Iperf_Test_Client");
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
		sleep(testTime + 1);
		// puts("Results have been saved to iperfDataClient.txt");
		return true;
	}
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
