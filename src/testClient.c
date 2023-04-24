/*
	Rene Molina
	EE 4230- Senior Design 2
*/


#include "testClient.h"


bool clientSetup(int argc, char **argv ,NetInfo *sockData, Packets *packets)
{
	// Check for correct number of arguments
	if (argc != 3) {
		puts("Not enough arguments entered:");
		puts("1st argument should be IP address of the server");
		puts("2nd argument should be port number of the server");
		return false;
	}
	
	// Extract network info from command line arguments
	sockData->cmdIP = *(argv + 1);
	sockData->cmdPort = *(argv + 2);
	
	// Initialize data packets
	srand(time(NULL));
	for (int i = 0; i < 1024; i++)
		packets->one_kb[i] = rand() % 128;
	for (int i = 0; i < 2048; i++)
		packets->two_kb[i] = rand() % 128;
	for (int i = 0; i < 4096; i++)
		packets->four_kb[i] = rand() % 128;
	for (int i = 0; i < 8192; i++)
		packets->eight_kb[i] = rand() % 128;
	for (int i = 0; i < 16384; i++)
		packets->sixteen_kb[i] = rand() % 128;
	for (int i = 0; i < 32768; i++)
		packets->thirty_two_kb[i] = rand() % 128;
	for (int i = 0; i < 65536; i++)
		packets->sixty_four_kb[i] = rand() % 128;
	for (int i = 0; i < 131072; i++)
		packets->one_eigth_mb[i] = rand() % 128;
	for (int i = 0; i < 262144; i++)
		packets->one_fourth_mb[i] = rand() % 128;
	for (int i = 0; i < 524288; i++)
		packets->one_half_mb[i] = rand() % 128;
	for (int i = 0; i < 1048576; i++)
		packets->one_mb[i] = rand() % 128;
	
	return true;
}


bool setupSocket(NetInfo *sockData)
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


bool makeTraffic(const NetInfo *sockData, Packets *packets)
{
	bool retVal = true;
	retVal *= sendPacket(&sockData, &packets->one_kb, sizeof(one_kb));
	retVal *= sendPacket(&sockData, &packets->two_kb, sizeof(two_kb));
	retVal *= sendPacket(&sockData, &packets->four_kb, sizeof(four_kb));
	retVal *= sendPacket(&sockData, &packets->eight_kb, sizeof(eight_kb));
	retVal *= sendPacket(&sockData, &packets->sixteen_kb, sizeof(sixteen_kb));
	retVal *= sendPacket(&sockData, &packets->thirty_two_kb, sizeof(thirty_two_kb));
	retVal *= sendPacket(&sockData, &packets->sixty_four_kb, sizeof(sixty_four_kb));
	retVal *= sendPacket(&sockData, &packets->one_eigth_mb, sizeof(one_eigth_mb));
	retVal *= sendPacket(&sockData, &packets->one_fourth_mb, sizeof(one_fourth_mb));
	retVal *= sendPacket(&sockData, &packets->one_half_mb, sizeof(one_half_mb));
	retVal *= sendPacket(&sockData, &packets->one_mb, sizeof(one_mb));
	return retVal;
}


bool sendPacket(const NetInfo *sockData, char *packet, unsigned int packetSize)
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
	
	printf("Sent %i bytes to the server\n", bytesSent);
	
	// Receive the response from the server
	ssize_t bytesReceived;
	struct sockaddr_storage fromAddr;
	socklen_t fromAddrLength = sizeof(fromAddr);
	bytesReceived = recvfrom(sockData->clientSocket, packets, packetSize, 0, (struct sockaddr *) &fromAddr, &fromAddrLength);
	// Indicates transmission error
	if (bytesReceived < 0) {
		fputs("Unable to receive data\n", stderr);
		return false;
	}
	else if (bytesReceived != packetSize) {
		fputs("Unexpected number of bytes received\n", stderr);
		return false;
	}
	
	printf("Received %i bytes from the server\n", bytesReceived);
	
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
	if (parseCmdArgs(argc, argv, &sockData, &packets) == false) {
		fputs("Unable to parse command line arguments - ", stderr);
		return -1;
	}
	
	// Connect to server
	if (setupSocket(&sockData) == false) {
		fputs("Unable to connect to bank server - ", stderr);
		return -1;
	}
	
	// Make the transaction specified by the terminal arguments
	Packets serverPackets;
	if (sendPackets(&sockData, &serverPackets) == false ) {
		fputs("Unable to process bank request - ", stderr);
		return -1;
	}
	
	// Free memory allocated to server address
	freeaddrinfo(sockData.serverAddr);

	// Close client socket
	if (close(sockData.clientSocket) < 0) {
		fputs("Failed to properly close socket - ", stderr);
		return -1;
	}
	
	puts("Successfully closed socket");
	puts("\n************************************************\n");

	return 0;
}
