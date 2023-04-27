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
			avgKBpS += (kiloBytesPerSecond[i] * iteration[i]) / totalIterations;
			avgEpPk += (errorsPerPacket[i] * iteration[i]) / totalIterations;
			avgEpKB += (errorsPerKB[i] * iteration[i]) / totalIterations;
		}
		avgIterations = ((double) totalIterations) / NUM_PACKET_SIZES;
		
		// Write averages to spreadsheet
		fprintf(outFile, "Averages,%u,%.2f,%.2f,%.2f,%.2f,\n", avgIterations, totalAvgRTT, avgKBpS, avgEpPk, avgEpKB);
		
		// Reset averages to zero
		totalIterations = 0;
		totalAvgRTT = 0;
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
	
	// Detach thread (makes it not joinable)
	pthread_detach(tid);
	
	return 0;
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


bool runPing(char *ipAddr, unsigned int numPackets, unsigned int numBytes, double interval)
{
	char **args;

	args = (char **) malloc(5 * sizeof(char *));
	args[0] = "../data/Ping_Test";
	args[1] = "-c";
	args[2] = "10";
	args[3] = ipAddr;
	args[4] = NULL;
	
	int childExitStatus;
	int pid = fork();
	// Fork error
	if (pid < 0) {
		fputs("Unable to fork process\n", stderr);
		return false;
	}
	// Child process
	else if (pid == 0)
		execvp(args[0], args);
	// Parent process
	else {
		free(args);
		wait(&childExitStatus);
		return true;
	}
}


bool runTraceroute(char *ipAddr)
{
	char **args;
	
	args = (char **) malloc(3 * sizeof(char *));
	args[0] = "../data/Traceroute_Test";
	args[1] = ipAddr;
	args[2] = NULL;

	int childExitStatus;
	int pid = fork();
	// Fork error
	if (pid < 0) {
		fputs("Unable to fork process\n", stderr);
		return false;
	}
	// Child process
	else if (pid == 0)
		execvp(args[0], args);
	// Parent process
	else {
		free(args);
		wait(&childExitStatus);
		return true;
	}	
}


bool runIperf(char *ipAddr)
{
	char **args;

	args = (char **) malloc(6 * sizeof(char *));
	args[0] = "../data/Iperf_Test";
	args[1] = "-c";
	args[2] = ipAddr;
	args[3] = "-u";
	args[4] = "100";
	args[5] = NULL;
	
	int childExitStatus;
	int pid = fork();
	// Fork error
	if (pid < 0) {
		fputs("Unable to fork process\n", stderr);
		return false;
	}
	// Child process
	else if (pid == 0)
		execvp(args[0], args);
	// Parent process
	else {
		free(args);
		wait(&childExitStatus);
		return true;
	}
}
