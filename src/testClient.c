/*
	Rene Molina
	EE 4230- Senior Design 2
*/


#include "testClient.h"


bool parseCmdArgs(int argc, char **argv ,NetInfo *sockData, sBANK_PROTOCOL *mainRequest)
{
	// Check for correct number of arguments
	if (argc < 5 || argc > 6) {
		puts("Not enough arguments entered:");
		puts("1st argument should be IP address of the bank server");
		puts("2nd argument should be port number of the bank server");
		puts("3rd argument should be transaction: B = balance inquiry, D = deposit, W = withdraw");
		puts("4th argument should be the account number");
		puts("5th argument should be value of deposit or withdraw in pennies");
		return false;
	}
	
	// Extract command line arguemnts into appropriate structures
	sockData->cmdIP = *(argv + 1);
	sockData->cmdPort = *(argv + 2);
	switch(**(argv + 3)) {
	case 'B':
	case 'b':
		mainRequest->trans = BANK_TRANS_INQUIRY;
		mainRequest->acctnum = atoi(*(argv + 4));
		mainRequest->value = 0;
		break;
	case 'D':
	case 'd':
		mainRequest->trans = BANK_TRANS_DEPOSIT;
		mainRequest->acctnum = atoi(*(argv + 4));
		mainRequest->value = atoi(*(argv + 5));
		break;
	case 'W':
	case 'w':
		mainRequest->trans = BANK_TRANS_WITHDRAW;
		mainRequest->acctnum = atoi(*(argv + 4));
		mainRequest->value = atoi(*(argv + 5));
		break;
	default:
		puts("Invalid transaction");
		return false;
	}
	
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


bool sendPackets(NetInfo *sockData, sBANK_PROTOCOL *bankTransaction)
{
	// Send the requested transaction to the server
	ssize_t bytesSent = sendto(sockData->clientSocket, bankTransaction, sizeof(*bankTransaction), 0, sockData->serverAddr->ai_addr, sockData->serverAddr->ai_addrlen);
	// Indicates transmission error
	if (bytesSent < 0) {
		fputs("Unable to send data\n", stderr);
		return false;
	}
	else if (bytesSent != sizeof(*bankTransaction)) {
		fputs("Unexpected number of bytes sent\n", stderr);
		return false;
	}
	
	printf("Sent %i bytes to the server\n", bytesSent);
	
	// Receive the response from the server
	ssize_t bytesReceived;
	struct sockaddr_storage fromAddr;
	socklen_t fromAddrLength = sizeof(fromAddr);
	bytesReceived = recvfrom(sockData->clientSocket, bankTransaction, sizeof(*bankTransaction), 0, (struct sockaddr *) &fromAddr, &fromAddrLength);
	// Indicates transmission error
	if (bytesReceived < 0) {
		fputs("Unable to receive data\n", stderr);
		return false;
	}
	else if (bytesReceived != sizeof(*bankTransaction)) {
		fputs("Unexpected number of bytes received\n", stderr);
		return false;
	}
	
	printf("Received %i bytes from the server\n", bytesReceived);
	
	// Transaction successful
	return true;
}


int main(int argc, char **argv)
{	
	puts("\n********************************************************************************\n");

	// Input structures
	NetInfo sockData;	// Holds TCP Connection information
	sBANK_PROTOCOL mainRequest;	// Holds bank request info from user
	
	// Parse command line arguments
	if (parseCmdArgs(argc, argv, &sockData, &mainRequest) == false) {
		fputs("Unable to parse command line arguments - ", stderr);
		return -1;
	}
	
	// Connect to bank server
	if (setupSocket(&sockData) == false) {
		fputs("Unable to connect to bank server - ", stderr);
		return -1;
	}
	
	// Make the transaction specified by the terminal arguments
	if (sendPackets(&sockData, &mainRequest) == false ) {
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
	
	puts("Successfully closed socket\n");

	return 0;
}
