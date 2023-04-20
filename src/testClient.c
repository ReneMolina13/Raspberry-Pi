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
	sockData->cmdPort = atoi(*(argv + 2));
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


bool connectToServer(NetInfo *sockData)
{
	// Create TCP client socket
	sockData->clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockData->clientSocket < 0) {
		puts("Error creating socket");
		return false;
	}
	
	// Initialize structure for address
	memset(&sockData->serverAddr, 0, sizeof(sockData->serverAddr));
	sockData->serverAddr.sin_family = AF_INET;
	sockData->serverAddr.sin_addr.s_addr = inet_addr(sockData->cmdIP);
	sockData->serverAddr.sin_port = htons(sockData->cmdPort);
	
	// Connect to server
	if (connect(sockData->clientSocket, (struct sockaddr *) &(sockData->serverAddr), sizeof(struct sockaddr)) < 0) {
		puts("Unable to connect to server");
		return false;
	}
	
	return true;
}


int makeBankRequest(int clientSocket, sBANK_PROTOCOL *bankTransaction)
{
	// Send the requested transaction to the server
	ssize_t bytesSent;
	bytesSent = send(clientSocket, bankTransaction, sizeof(sBANK_PROTOCOL), 0);
	// Indicates transmission error
	if (bytesSent < 0)
		return -1;
	
	// Receive the response from the server
	ssize_t bytesReceived;
	bytesReceived = recv(clientSocket, bankTransaction, sizeof(sBANK_PROTOCOL), 0);
	// Indicates transmission error
	if (bytesReceived < 0)
		return -1;
	// Indicates that the server has closed the socket
	else if (bytesReceived == 0)
		return 0;
	
	// Transaction successful
	return 1;
}


int main(int argc, char **argv)
{	
	puts("\n*************************************************************************************\n");

	// Input structures
	NetInfo sockData;	// Holds TCP Connection information
	sBANK_PROTOCOL mainRequest;	// Holds bank request info from user
	
	// Parse command line arguments
	if (parseCmdArgs(argc, argv, &sockData, &mainRequest) == false) {
		fputs("Unable to parse command line arguments - ", stderr);
		return -1;
	}
	
	// Connect to bank server
	if (connectToServer(&sockData) == false) {
		fputs("Unable to connect to bank server - ", stderr);
		return -1;
	}
	
	puts("Connected to bank server\n");
	puts("Making original transaction:");
	printf("Transaction type (D=0, W=1, I=2): %i\n", mainRequest.trans);
	printf("Account number: %i\n", mainRequest.acctnum);
	printf("Value of transaction: %i\n\n", mainRequest.value);

	
	// Make the transaction specified by the terminal arguments
	int status = makeBankRequest(sockData.clientSocket, &mainRequest);
	if (status < 0) {
		fputs("Original transaction failed due to transmission error - ", stderr);
		return -1;
	}
	else if (status == 0) {
		puts("Socket in close-wait state: Initiating close handshake");
	}
	else {
		puts("Original transaction completed:");
		printf("Transaction type (D=0, W=1, I=2): %i\n", mainRequest.trans);
		printf("Account number: %i\n", mainRequest.acctnum);
		printf("Value of transaction: %i\n\n", mainRequest.value);
	}

	
	// Close client socket
	if (close(sockData.clientSocket) < 0) {
		fputs("Failed to properly close socket - ", stderr);
		return -1;
	}
	
	puts("Successfully closed socket\n");
	puts("Asking user for new transaction:");

	return 0;
}
