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
	char *ipAddress = parameter->ipAddress;
	NetStats *packetStats = parameter->packetStats;
	// Temp variables for file output
	unsigned int packetSize;
	unsigned long long int iteration;
	double avgRoundTripTime;
	double errorsPerIteration;
	double errorsPerKB;
	
	// Detach thread (makes it not joinable)
	pthread_detach(tid);
	
	// Open file to store data produced by network threads
	FILE *outFile = fopen("../data/testingData.csv", "r+");
	
	if (outFile == NULL) {
		fputs("Could not open testing file\n", stderr);
		goto exit;
	}
	
	while (1) {
		// Move to start of 2nd line in file & reset for writing
		while (getc(outFile) != '\n');
		fseek(outFile, 0, SEEK_CUR);
		// Iterate through one packet size at a time
		for (int i = 0; i < NUM_PACKET_SIZES; i++) {
			// Store current statistical data in temporary buffer
			packetSize = packetStats[i].packetSize;
			iteration = packetStats[i].iteration;
			avgRoundTripTime = packetStats[i].avgRoundTripTime;
			errorsPerIteration = packetStats[i].errorsPerIteration;
			// Write packet size to spreadsheet
			fprintf(outFile, "%u-Byte Packet,", packetSize);
			// Write average round-trip time (ms) to spreadsheet
			fprintf(outFile, "%.2f,", avgRoundTripTime);
			// Write number of incorrect bits per packet
			fprintf(outFile, "%.2f,", errorsPerIteration);
			// Calculate and write number of incorrect bits per KB
			errorsPerKB = 1000 * errorsPerIteration / packetSize;
			fprintf(outFile, "%.2f,", errorsPerKB);
			// Add a newline to end row & reset for input
			fseek(outFile, 0, SEEK_CUR);
		}
		// Reset to start of file stream
		rewind(outFile);
		break;
	}
	
exit:
	fclose(outFile);
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
