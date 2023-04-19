/*
	Rene Molina
	EE 4374 - Operating Systems
	Due Date: 4/12/2023
	Assigned: 3/27/2023
	
	This is the header file for the bank server program. It contains two structures
	The firest is the one specified in the assignment instructions (sBANK_ACCT_DATA) 
	containing the account balance and a mutex semaphore for a single bank account. 
	The second one is the structure passsed to the thread function (ThreadArgs) containing 
	the thread's ID, the handle for the bank server's listening socket, and an error code. 
	Also included as global variables are an array of bank accounts (sBANK_ACCT_DATA) and an
	array of thread arguments (ThreadArgs), both set to a length of the macro NUM_ACCOUNTS.
	The macros used in this program are NUM_ACCTS (100) and the error codes used for threads. 
	Each of the functions used in the bankServer program are described below
*/


#ifndef	BANK_SERVER_H
#define BANK_SERVER_H


#include "rmolina_banking.h"


// Number of bank accounts
#define NUM_ACCTS 100
// Error codes for threads
#define ACCEPT_ERROR		-1
#define TRANSMISSION_ERROR	-2
#define CLOSE_ERROR			-3


// Server-side banking information
typedef struct
{
	unsigned int balance;
	pthread_mutex_t mutex;
} sBANK_ACCT_DATA;

// Structure holding thread arguments
typedef struct
{
	pthread_t tid;
	int serverSocket;
	int errorCode;
} ThreadArgs;


//*******************************************************************************************
// Function Prototypes
//*******************************************************************************************


/*
	clientThread function:
	
	Description:
	Assigns a thread to each client currently making a request to the bank server. For each
	client, the thread accepts the connection using the server's TCP socket and handles 
	the client's request on a connected socket. Once a thread completes a client's request, 
	assuming no errors, it blocks until it is chosen to handle a new client trying to connect. 
	If an error does occur on a thread (accepting a connection, transmission errors, or with 
	closing a socket), the thread sends a kill signal to all other threads and returns an 
	error code to the main function, which then prints the error and exits
	
	Inputs:
	- void *param: Holds the input structure for the threads (ThreadArgs). Contains 3 
			variables: tid (intput), serverSocket (input), and errorCode (output)
	- pthread_t tid: Thread ID of a particular thread. Used when killing other threads in 
			the case of an error
	- int serverSocket: The handle to the server (listening) socket, used to accept client 
			connections
			
	Outputs:
	- void *retVal: Nothing is returned from void * return value. Actual value returned is 
			3rd variable from ThreadArgs structure (int errorCode)
	- int errorCode: Error code to be returned to main if an error occurs when a thread is 
			handling a client request. Value if -1 if there was an error accepting a client 
			connection, -2 if there was a transmission error (sending or receiving), and -3 
			if there was an error closing the connected socket with the client. Value is 0 if 
			no error has occurred in this thread
*/
void *clientThread(void *);

/*
	initBank function: 
	
	Description:
	Initializes the bank accounts to random values, creates the TCP socket for the bank 
	server, initializes the sockaddr_in structure from the Sockets API, binds the TCP
	socket to port 26207, and sets it to be listening to incoming connections
	
	Inputs:
	- struct sockaddr_in *serverAddr: A pointer to an empty sockaddr_in structure
	
	Outputs:
	- int serverSocket: The handle for the server socket
	- struct sockaddr_in *serverAddr: By the time the function returns, this structure has been
			filled with the connection info for the bank server
*/
int initBank(struct sockaddr_in *);

/*
	handleClient function:
	
	Description:
	Handles communication with the client. Waits to receive the transaction request from 
	the connected client, calls a function to process the transaction, and sends the receipt 
	of the transaction back to the client
	
	Inputs:
	- int clientSocket: The handle for the connected socket between the bank server and a
			particular client. Used to send and receive transaction request and send
			transaction receipts
	
	Outputs:
	- int status: Indicates if the transaction was successful. Value is -1 if there was an
			error sending or receiving data on the socket. Value is 0 if the client closed
			the connected socket. Value is 1 if the transaction was successful
*/
int handleClient(int clientSocket);

/*
	processTransaction function:
	
	Description:
	Processes the requested transaction received by the server
	
	Inputs:
	- sBANK_PROTOCOL *request: A pointer to the sBANK_PROTOCOL structure received by the
			bank server, representing the client's requested transaction
		
	Outputs:
	- bool success: A flag signaling if the transaction was successful or not. Its value is
			false if the provided account number is invalid, the requested transaction is 
			invalid, or if more money is asked to be withdrawn than is available in the 
			provided bank account. Its value is true otherwise
	- sBANK_PROTOCOL *request: The receipt of the transaction is stored back in this structure
			to be sent back to the client
*/
bool processTransaction();


// Array of bank accounts
sBANK_ACCT_DATA acctData[NUM_ACCTS];

// Attribute structure for threads
pthread_attr_t attr;

// Array of thread structures
ThreadArgs args[NUM_ACCTS];

#endif
