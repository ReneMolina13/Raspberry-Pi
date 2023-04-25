/*
	Rene Molina
	EE 4230- Senior Design 2
*/


#include "testClient.h"


void *networkThreads(void *param)
{
	ThreadArgs *parameter = (ThreadArgs *) param;
	pthread_t tid = parameter->tid;
	NetInfo *sockData = parameter->sockData;
	Packets *sentPackets = parameter->packets;
	Packets receivedPackets;
	bool retVal = true;
	
	switch (tid % NUM_PACKET_SIZES) {
	case ONE_BYTE:
		retVal *= sendPacket(sockData, &sentPackets->oneByte, sizeof(sentPackets->oneByte));
		retVal *= receivePacket(sockData, &receivedPackets.oneByte, sizeof(receivedPackets.oneByte));
		break;
	case TWO_BYTES:
		retVal *= sendPacket(sockData, sentPackets->two_bytes, sizeof(sentPackets->two_bytes));
		retVal *= receivePacket(sockData, receivedPackets.two_bytes, sizeof(receivedPackets.two_bytes));
		break;
	case FOUR_BYTES:
		retVal *= sendPacket(sockData, sentPackets->four_bytes, sizeof(sentPackets->four_bytes));
		retVal *= receivePacket(sockData, receivedPackets.four_bytes, sizeof(receivedPackets.four_bytes));
		break;
	case EIGHT_BYTES:
		retVal *= sendPacket(sockData, sentPackets->eight_bytes, sizeof(sentPackets->eight_bytes));
		retVal *= receivePacket(sockData, receivedPackets.eight_bytes, sizeof(receivedPackets.eight_bytes));
		break;
	case SIXTEEN_BYTES:
		retVal *= sendPacket(sockData, sentPackets->sixteen_bytes, sizeof(sentPackets->sixteen_bytes));
		retVal *= receivePacket(sockData, receivedPackets.sixteen_bytes, sizeof(receivedPackets.sixteen_bytes));
		break;
	case THIRTY_TWO_BYTES:
		retVal *= sendPacket(sockData, sentPackets->thirty_two_bytes, sizeof(sentPackets->thirty_two_bytes));
		retVal *= receivePacket(sockData, receivedPackets.thirty_two_bytes, sizeof(receivedPackets.thirty_two_bytes));
		break;
	case SIXTY_FOUR_BYTES:
		retVal *= sendPacket(sockData, sentPackets->sixty_four_bytes, sizeof(sentPackets->sixty_four_bytes));
		retVal *= receivePacket(sockData, receivedPackets.sixty_four_bytes, sizeof(receivedPackets.sixty_four_bytes));
		break;
	case ONE_EIGTH_KB:
		retVal *= sendPacket(sockData, sentPackets->one_eigth_kb, sizeof(sentPackets->one_eigth_kb));
		retVal *= receivePacket(sockData, receivedPackets.one_eigth_kb, sizeof(receivedPackets.one_eigth_kb));
		break;
	case ONE_FOURTH_KB:
		retVal *= sendPacket(sockData, sentPackets->one_fourth_kb, sizeof(sentPackets->one_fourth_kb));
		retVal *= receivePacket(sockData, receivedPackets.one_fourth_kb, sizeof(receivedPackets.one_fourth_kb));
		break;
	case ONE_HALF_KB:
		retVal *= sendPacket(sockData, sentPackets->one_half_kb, sizeof(sentPackets->one_half_kb));
		retVal *= receivePacket(sockData, receivedPackets.one_half_kb, sizeof(receivedPackets.one_half_kb));
		break;
	case ONE_KB:
		retVal *= sendPacket(sockData, sentPackets->one_kb, sizeof(sentPackets->one_kb));
		retVal *= receivePacket(sockData, receivedPackets.one_kb, sizeof(receivedPackets.one_kb));
		break;
	case TWO_KB:
		retVal *= sendPacket(sockData, sentPackets->two_kb, sizeof(sentPackets->two_kb));
		retVal *= receivePacket(sockData, receivedPackets.two_kb, sizeof(receivedPackets.two_kb));
		break;
	case FOUR_KB:
		retVal *= sendPacket(sockData, sentPackets->four_kb, sizeof(sentPackets->four_kb));
		retVal *= receivePacket(sockData, receivedPackets.four_kb, sizeof(receivedPackets.four_kb));
		break;
	case EIGHT_KB:
		retVal *= sendPacket(sockData, sentPackets->eight_kb, sizeof(sentPackets->eight_kb));
		retVal *= receivePacket(sockData, receivedPackets.eight_kb, sizeof(receivedPackets.eight_kb));
		break;
	case SIXTEEN_KB:
		retVal *= sendPacket(sockData, sentPackets->sixteen_kb, sizeof(sentPackets->sixteen_kb));
		retVal *= receivePacket(sockData, receivedPackets.sixteen_kb, sizeof(receivedPackets.sixteen_kb));
		break;
	case THIRTY_TWO_KB:
		retVal *= sendPacket(sockData, sentPackets->thirty_two_kb, sizeof(sentPackets->thirty_two_kb));
		retVal *= receivePacket(sockData, receivedPackets.thirty_two_kb, sizeof(receivedPackets.thirty_two_kb));
		break;
	case MAX_SIZE_UDP:
		retVal *= sendPacket(sockData, sentPackets->max_size_udp, sizeof(sentPackets->max_size_udp));
		retVal *= receivePacket(sockData, receivedPackets.max_size_udp, sizeof(receivedPackets.max_size_udp));
	}
	
	parameter->status = retVal;
	pthread_exit(0);
}


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
	
	// Initialize data packets (First byte holds index for packet size - defined in test.h)
	srand(time(NULL));
	packets->oneByte = ONE_BYTE;
	packets->two_bytes[0] = TWO_BYTES;
	for (int i = 0; i < 2; i++)
		packets->two_bytes[i] = rand() % 128;
	packets->four_bytes[0] = FOUR_BYTES;
	for (int i = 0; i < 4; i++)
		packets->four_bytes[i] = rand() % 128;
	packets->eight_bytes[0] = EIGHT_BYTES;
	for (int i = 0; i < 8; i++)
		packets->eight_bytes[i] = rand() % 128;
	packets->sixteen_bytes[0] = SIXTEEN_BYTES;
	for (int i = 0; i < 16; i++)
		packets->sixteen_bytes[i] = rand() % 128;
	packets->thirty_two_bytes[0] = THIRTY_TWO_BYTES;
	for (int i = 0; i < 32; i++)
		packets->thirty_two_bytes[i] = rand() % 128;
	packets->sixty_four_bytes[0] = SIXTY_FOUR_BYTES;
	for (int i = 0; i < 64; i++)
		packets->sixty_four_bytes[i] = rand() % 128;
	packets->one_eigth_kb[0] = ONE_EIGTH_KB;
	for (int i = 0; i < 128; i++)
		packets->one_eigth_kb[i] = rand() % 128;
	packets->one_fourth_kb[0] = ONE_FOURTH_KB;
	for (int i = 0; i < 256; i++)
		packets->one_fourth_kb[i] = rand() % 128;
	packets->one_half_kb[0] = ONE_HALF_KB;
	for (int i = 0; i < 512; i++)
		packets->one_half_kb[i] = rand() % 128;
	packets->one_kb[0] = ONE_KB;
	for (int i = 0; i < 1024; i++)
		packets->one_kb[i] = rand() % 128;
	packets->two_kb[0] = TWO_KB;
	for (int i = 0; i < 2048; i++)
		packets->two_kb[i] = rand() % 128;
	packets->four_kb[0] = FOUR_KB;
	for (int i = 0; i < 4096; i++)
		packets->four_kb[i] = rand() % 128;
	packets->eight_kb[0] = EIGHT_KB;
	for (int i = 0; i < 8192; i++)
		packets->eight_kb[i] = rand() % 128
	packets->sixteen_kb[0] = SIXTEEN_KB;
	for (int i = 0; i < 16384; i++)
		packets->sixteen_kb[i] = rand() % 128;
	packets->thirty_two_kb[0] = THIRTY_TWO_KB;
	for (int i = 0; i < 32768; i++)
		packets->thirty_two_kb[i] = rand() % 128;
	packets->max_size_udp[0] = MAX_SIZE_UDP;
	for (int i = 0; i < 65507; i++)
		packets->max_size_udp[i] = rand() % 128;
	
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
	
	printf("Sent %li bytes to the server\n", bytesSent);
	
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
	
	printf("Received %li bytes from the server\n\n", bytesReceived);
	
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
	
	
	// Create threads
	Packets serverPackets;
	pthread_attr_init(&attr);
	ThreadArgs *args = (ThreadArgs *) malloc(NUM_PACKET_SIZES * sizeof(ThreadArgs));
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		args[i].sockData = &sockData;
		args[i].packets = &packets;
		args[i].status = true;
		pthread_create(&args[i].tid, &attr, networkThreads, (void *) &args[i]);
	}
	
	// Wait for threads to terminate (only happens in case of error)
	for (int i = 0; i < NUM_PACKET_SIZES; i++) {
		pthread_join(args[i].tid, NULL);
		if (args[i].status == false) {
			fputs("Error creating network traffic - ", stderr);
			char errorPacket[3] = "-1";
			sendPacket(&sockData, errorPacket, 3);
			return -1;
		}
	}
	
	puts("************************************************\n");
	
	// Free memory allocated to server address and thread argument structures
	freeaddrinfo(sockData.serverAddr);
	free(args);

	// Close client socket
	if (close(sockData.clientSocket) < 0) {
		fputs("Failed to properly close socket - ", stderr);
		return -1;
	}
	
	puts("Successfully closed socket");
	puts("\n************************************************\n");

	return 0;
}
