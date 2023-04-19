/*
	Rene Molina
	EE 4374 - Operating Systems
	Due Date: 4/12/2023
	Assigned: 3/27/2023
	
	This program creates a bank server which allows clients to perform 
	deposit, withdraw, and bank inquiry operations. Once the operation 
	has been completed, the client is given a receipt of the transaction. 
	Note that each time the bank server program is run, each of the 100 
	bank accounts are set to contain random amounts of money
*/


#include "bankServer.h"


void *clientThread(void *param)
{
	// Extract input arguments
	ThreadArgs *parameter = (ThreadArgs *) param;
	pthread_t tid = parameter->tid;
	int serverSocket = parameter->serverSocket;
	int errorCode = parameter->errorCode;
	
	// Run forever (assuming no errors)
	while (errorCode >= 0) {
		// Accept client connection
		char clientName[INET_ADDRSTRLEN];
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLength = sizeof(clientAddr);
		int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &clientAddrLength);
		if (clientSocket < 0) {
			errorCode = ACCEPT_ERROR;
			break;
		}
		inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, clientName, sizeof(clientName));
			
		puts("Server accepted connection request:");
		printf("Client socket value: %i\n", clientSocket);
		printf("Client family value: %i\n", clientAddr.sin_family);
		printf("Client IP value: %s\n", clientName);
		printf("Client port value: %i\n", ntohs(clientAddr.sin_port));
		puts("\n************************************************\n");
		
		// Handle requests until client ends connection
		int status;
		while (1) {
			status = handleClient(clientSocket);
			if (status < 0) {
				errorCode = TRANSMISSION_ERROR;
				break;
			}
			else if (status == 0) {
				puts("Socket in close-wait state: Initiating close handshake");
				break;
			}
		}
		
		// Client closed socket
		if (errorCode >= 0)
			if (close(clientSocket) < 0)
				errorCode = CLOSE_ERROR;
	
		puts("Successfully closed client socket");
		puts("\n************************************************\n");
	}
	
	// Error has occured: kill all other threads and exit
	int i;
	for (i = 0; args[i].tid != tid; i++)
		pthread_kill(args[i].tid, SIGKILL);
	for (++i; i < NUM_ACCTS; i++)
		pthread_kill(args[i].tid, SIGKILL);
	parameter->errorCode = errorCode;
	pthread_exit(0);
}


int initBank(struct sockaddr_in *serverAddr)
{
	// Initialize bank accounts
	srand(time(NULL));
	for (int i = 0; i < NUM_ACCTS; i++) {
		// Set account balances to random values
		acctData[i].balance = rand();
		// Initialize account mutexes
		pthread_mutex_init(&acctData[i].mutex, NULL);
	}
	
	// Create TCP server socket
	int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket < 0) {
		puts("Error creating socket");
		return -1;
	}

	// Initialize address structure
	memset(serverAddr, 0, sizeof(*serverAddr));
	serverAddr->sin_family = AF_INET;
	serverAddr->sin_addr.s_addr = htonl(INADDR_ANY);	// Allows connection to any IP address
	serverAddr->sin_port = htons(26207);
	
	puts("TCP socket created:");
	printf("Socket value: %i\n", serverSocket);
	puts("\n************************************************\n");
	
	// Bind local address to socket
	if (bind(serverSocket, (struct sockaddr *) serverAddr, sizeof(struct sockaddr)) < 0) {
		puts("Error binding local address to socket");
		return -1;
	}
	
	puts("TCP socket bound to address");
	printf("Server family value: %i\n", serverAddr->sin_family);
	printf("Server IP value: %i\n", serverAddr->sin_addr.s_addr);
	printf("Server port value: %i\n", ntohs(serverAddr->sin_port));
	
	// Have server listen for bank customers
	if (listen(serverSocket, NUM_ACCTS) < 0) {
		puts("Server unable to listen for traffic");
		return -1;
	}
	
	puts("\n************************************************\n");
	puts("Server is now listening for incoming connections");
	puts("\n************************************************\n");

	// Return socket handle
	return serverSocket;
}


int handleClient(int clientSocket)
{
	static int transactionNum = 0;
	printf("Transaction Number: %i\n", transactionNum++);
	
	// Receive request from client
	sBANK_PROTOCOL clientRequest;
	ssize_t bytesReceived;
	bytesReceived = recv(clientSocket, &clientRequest, sizeof(sBANK_PROTOCOL), 0);
	if (bytesReceived < 0) {
		puts("Unable to receive request from client");
		puts("\n************************************************\n");
		return -1;
	}
	else if (bytesReceived == 0) {
		puts("Client has closed socket");
		puts("\n************************************************\n");
		transactionNum = 0;
		return 0;
	}
	else
		printf("Received %li bytes out of a possible %lu\n\n", bytesReceived, sizeof(sBANK_PROTOCOL));
				
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
	bytesSent = send(clientSocket, &clientRequest, sizeof(sBANK_PROTOCOL), 0);
	if (bytesSent < 0) {
		puts("Unable to confirm completion of request to client");
		puts("\n************************************************\n");
		return -1;
	}
	else
		printf("Sent %li bytes out of a possible %lu\n", bytesSent, sizeof(sBANK_PROTOCOL));
	
	puts("Receipt received by client");
	puts("\n************************************************\n");

	return 1;
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
	struct sockaddr_in serverAddr;
	int serverSocket = initBank(&serverAddr);
	if (serverSocket < 0) {
		fputs("Failed to initialize bank server - ", stderr);
		return -1;
	}

	// Initialize threads to handle client requests
	pthread_attr_init(&attr);
	for (int i = 0; i < NUM_ACCTS; i++) {
		args[i].serverSocket = serverSocket;
		args[i].errorCode = 0;
		pthread_create(&(args[i].tid), &attr, clientThread, (void *) &(args[i]));
	}
	
	// Wait for threads to return (indicates error)
	for (int i = 0; i < NUM_ACCTS; i++) {
		pthread_join(args[i].tid, NULL);
		switch(args[i].errorCode) {
		case ACCEPT_ERROR:
			fputs("Unable to accept client connection - ", stderr);
			return -1;
		case TRANSMISSION_ERROR:
			fputs("Unable to handle client request - ", stderr);
			return -1;
		case CLOSE_ERROR:
			fputs("Unable to properly close client socket - ", stderr);
			return -1;
		}
	}
	
	// Never reached
		
	// Close server socket
	if (close(serverSocket) < 0) {
		fputs("Unable to properly close server socket - ", stderr);
		return -1;
	}
	
	puts("Successfully closed server socket\n");
	
	// Exit program
	return 0;
}
