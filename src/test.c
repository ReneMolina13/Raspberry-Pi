/*
	Rene Molina
	EE 4230- Senior Design 2
*/


#include "test.h"


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


void test(char *ipAddr, NetStats *packetStats)
{	
	while (1) {
		for (int i = 0; i < NUM_PACKET_SIZES; i++) {
			printf("Packet size: %i, ", packetStats[i].packetSize);
			printf("Iteration num: %lli\n", packetStats[i].iteration);
		}
		fputs("\n", stdout);
	}
}