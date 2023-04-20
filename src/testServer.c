/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#include "testServer.h"


int initBank()
{
	// Initialize bank accounts
	srand(time(NULL));
	for (int i = 0; i < NUM_ACCTS; i++) {
		// Set account balances to random values
		acctData[i].balance = rand();
		// Initialize account mutexes
		pthread_mutex_init(&acctData[i].mutex, NULL);
	}
	
	// Initialize structure specifying possible connection types
	char *service = "26207";
	struct addrinfo addrCriteria;
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria->ai_family = AF_UNSPEC;		// Any address family
	addrCriteria->ai_flags = AI_PASSIVE;		// Accept on any address/port
	addrCriteria->ai_socktype = SOCK_DGRAM;	// Only accept datagram sockets
	addrCriteria->ai_protocol = IPPROTO_UDP;	// Only accept UDP protocol
	
	// Get list of possible server addresses
	struct addrinfo *serverAddr;
	int status = getaddrinfo(NULL, service, &addrCriteria, &serverAddr);
	if (status != 0) {
		fputs("Error getting list of server addresses\n", stderr);
		return -1;
	}
	
	// Create UDP server socket
	int serverSocket = socket(serverAddr->ai_family, serverAddr->ai_socktype, serverAddr->ai_protocol);
	if (serverSocket < 0) {
		fputs("Error creating socket\n", stderr);
		return -1;
	}

	puts("UDP socket created:");
	printf("Socket value: %i\n", serverSocket);
	puts("\n************************************************\n");
	
	// Bind local address to socket
	status = bind(serverSocket, serverAddr->ai_addr, serverAddr->ai_addrlen);
	if (status < 0) {
		fputs("Error binding local address to socket\n", stderr);
		return -1;
	}
	
	puts("UDP socket bound to address");
	printf("Server family value: %i\n", serverAddr->ai_family);
	printf("Server IP value: %i\n", serverAddr->ai_addr);
	printf("Server port value: %s\n", service);
	puts("\n************************************************\n");
	
	// Free possible address list once bound to an address
	free(serverAddr);
	
	// Return socket handle
	return serverSocket;
}


bool handleClient(int serverSocket)
{
	// Receive request from client
	struct sockaddr_storage clientAddr;
	socklen_t clientAddrLength = sizeof(clientAddr);
	sBANK_PROTOCOL clientRequest;
	ssize_t bytesReceived;
	bytesReceived = recvfrom(serverSocket, &clientRequest, sizeof(sBANK_PROTOCOL), 0, (struct sockaddr *) &clientAddr, &clientAddrLength);
	if (bytesReceived < 0) {
		fputs("Unable to receive request from client\n", stderr);
		fputs("\n************************************************\n\n", stderr);
		return false;
	}
	
	puts("Received request from client:");
	printf("Transaction type (D=0, W=1, I=2): %i\n", clientRequest.trans);
	printf("Account number: %i\n", clientRequest.acctnum);
	printf("Value of transaction: %i\n\n", clientRequest.value);
	
	// Perform requested transaction 
	if (processTransaction(&clientRequest) == false)
		puts("Unable to complete transaction");
	else
		puts("Transaction Completed");
	
	puts("Receipt for client: ");
	printf("Transaction type (D=0, W=1, I=2): %i\n", clientRequest.trans);
	printf("Account number: %i\n", clientRequest.acctnum);
	printf("Value of transaction: %i\n\n", clientRequest.value);
	
	// Confirm with client that request was completed
	ssize_t bytesSent;
	bytesSent = sendto(serverSocket, &clientRequest, sizeof(sBANK_PROTOCOL), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
	if (bytesSent < 0) {
		fputs("Unable to confirm completion of request to client\n", stderr);
		fputs("\n************************************************\n\n", stderr);
		return false;
	}
	else if (bytesSent != bytesReceived) {
		fputs("Send unexpected number of bytes\n", stderr);
		fputs("\n************************************************\n\n", stderr);
		return false;
	}
	
	puts("Receipt received by client");
	puts("\n************************************************\n");

	return true;
}


bool processTransaction(sBANK_PROTOCOL *request)
{	
	// Checks for a valid account number
	if (request->acctnum < 0 || request->acctnum >= NUM_ACCTS) {
		puts("Invalid account number");
		return false;
	}
	
	// Use mutex to prevent race conditions
	pthread_mutex_lock(&acctData[request->acctnum].mutex);

	// Check for valid request
	bool success = true;
	switch(request->trans) {
	
	// Deposit
	case BANK_TRANS_DEPOSIT:
		acctData[request->acctnum].balance += request->value;
		break;
	
	// Withdraw
	case BANK_TRANS_WITHDRAW:
		// Check for sufficient funds
		if (acctData[request->acctnum].balance < request->value) {
			puts("Insufficient Funds");
			request->value = 0;
			success = false;
		}
		else 
			acctData[request->acctnum].balance -= request->value;
		break;
		
	// Show account balance
	case BANK_TRANS_INQUIRY:
		request->value = acctData[request->acctnum].balance;
		break;
		
	default:
		puts("Invalid transaction");
		success = false;
	}
	
	// Unlock mutex and return whether transaction was successful
	pthread_mutex_unlock(&acctData[request->acctnum].mutex);
	return success;
}


int main()
{	
	puts("\n************************************************\n");

	// Initialize bank server
	int serverSocket = initBank();
	if (serverSocket < 0) {
		fputs("Failed to initialize bank server - ", stderr);
		return -1;
	}

	// Run forever (assuming no errors)
	while (1) {
		// Handle a single request from a client
		if (handleClient(serverSocket) == false) {
			fputs("Unable to handle client request - ", stderr);
			return -1;
		}
	}
		
	// Close server socket (never reached)
	if (close(serverSocket) < 0) {
		fputs("Unable to properly close server socket - ", stderr);
		return -1;
	}
	
	puts("Successfully closed server socket\n");
	
	// Exit program
	return 0;
}
