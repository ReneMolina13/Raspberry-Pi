/*
	Rene Molina
	EE 4374 - Operating Systems
	Due Date: 4/12/2023
	Assigned: 3/27/2023
	
	This is the header file for the bank client program. It contains a struct 
	called NetInfo which contains socket information obtained from the command 
	line arguments (cmdIP & cmdPort), the handle to the client socket, and 
	the Sockets API structure specifying an internet address which is used to 
	connect with the bank server. Two global variables are also included to 
	enable the multithreaded portion of the code. The first is the structure 
	to give each thread default attributes, and the other is a pointer to 
	an arbitrary number of thread handles. Each of the functions used in the 
	bank client program are described below
*/


#ifndef	BANK_CLIENT_H
#define	BANK_CLIENT_H


#include "test.h"


// Structure holding connection info (from cmd line & necessary structures)
typedef struct
{
	char *cmdIP;
	unsigned int cmdPort;
	int clientSocket;
	struct sockaddr_in serverAddr;
} NetInfo;


//*******************************************************************************************
// Function Prototypes
//*******************************************************************************************


/*
	serverThread function:
	
	Description:
	Function that each created thread starts from. Each thread creates its own sBANK_PROTOCOL
	structure which is initialized to random, valid values. It then sends that request to
	the bank server before exiting
	
	Inputs:
	- void *param: Holds the address of an integer array of size 2 containing the handle to
			the connected socket (input) and the status of the transaction (output)
	- int clientSocket: Socket handle is used to send the randomly-generated bank request 
			to the server
	
	Outputs:
	- void *retVal: Nothing is returned from void * return value. Actual value 
			returned is 2nd integer in argument array (clientStatus)
	- int clientStatus: The value of the transaction status is -1 if there were 
			transmission errors, 0 if the server closed the socket, and 1 if the 
			transaction was successful
*/
void *serverThread(void *);

/*
	parseCmdArgs function:
	
	Description:
	Parses each of the command line arguments to the program into a NetInfo structure
	(for connection data) and an sBANK_PROTOCOL structure (for transaction request info)
	
	Inputs:
	- int argc: Holds number of command line arguments entered, used to ensure that the
			correct amount of arguments are inputted
	- char **argv: Array of strings containing each of the command line arguments. These are
			parsed into the appropriate variables
	- NetInfo *sockData: Pointer to a an empty structure for connection info
	- sBANK_PROTOCOL *mainRequest: Pointer to an empty structure for bank transaction info
			
	Outputs:
	- bool success: Flag indicating whether the arguments were parsed successfully into
			the NetInfo and sBANK_PROTOCOL structures that were passed by reference
	- NetInfo *sockData: IP address and port number values from command line are stored
			to this structure
	- sBANK_PROTOCOL *mainRequest: Transaction type, bank account number, and transaction
			value are stored to this structure
	
*/
bool parseCmdArgs(int, char **, NetInfo *, sBANK_PROTOCOL *);

/*
	connectToServer function:
	
	Description:
	Creates the client socket and uses command line arguments supplied by user to connect
	to the bank server
	
	Inputs:
	- NetInfo *sockData: Holds IP address and port number entered by user, which is used
			to connect to bank server
			
	Outputs:
	- bool success: Flag indicating whether the client socket was created and connected to
			the bank server successfully
	- NetInfo *sockData: Client socket handle and sockaddr_in structure is stored to 
			this structure
*/
bool connectToServer(NetInfo *sockData);

/*
	makeBankRequest function:
	
	Description:
	Sends the transaction request provided by the calling function to the bank server and
	waits for a receipt of the transaction

	Inputs:
	- int clientSocket: Handle to the connected socket, used communicate with bank server
	- sBANK_PROTOCOL *bankTransaction: Pointer to the structure containing the transaction
			requested by the calling function. This is sent to the bank server for processing
	
	Outputs:
	- int status: Indicates if the transaction was successful. Value is -1 if there was an
			error sending or receiving data on the socket. Value is 0 if the client closed
			the connected socket. Value is 1 if the transaction was successful
	- sBANK_PROTOCOL *bankTransaction: Once transaction is complete, values in this structure
			represent the transaction receipt, which is returned to the calling function
*/
int makeBankRequest(int, sBANK_PROTOCOL *);

/*
	makeThreads function:
	
	Description:
	Creates a random number of threads (0-100) that make random, valid transaction requests
	to the bank server. Each thread is given a 2-element integer array containing the handle
	to the connected socket and a status flag. After all threads complete their transactions, 
	each of the status flags is checked to make sure there were no transmission errors
	
	Inputs:
	- int socket: Handle for the connected socket that will be used by each thread to make
			the random transactions
	
	Outputs:
	- int status: Indicates if the transactions by all threads were successful. Value is -1 
			if at least one thread had an error sending or receiving data on the socket. 
			Value is 0 if the client closed the connected socket. Value is 1 if the 
			transactions made by all threads were successful
*/
int makeThreads(int);

/*
	newTransaction function:
	
	Description:
	Asks the user if they would like to make another transaction. If they do, the user is
	prompted to enter transaction details over the console. These are stored into an argument
	array (along with currently-used IP address and port number). The process then calls
	fork() to make a child, which calls evecv() to call the bankClient program again with
	the new transaction details entered by the user. The parent process waits for the child
	to terminate before exiting. This doesn't occur until the user decides to stop making
	transactions, at which point all processes created in this chain exit
	
	Inputs:
	- NetInfo *sockData: Network-related info is passed to fill argument array
	
	Outputs:
	- bool success: Flag indicating if the function executed properly. Returns true if either
			the user chooses not to make another transaction or if the child process is successfully
			created. Returns false otherwise
*/
bool newTransaction(NetInfo *);


// pthread attributes
pthread_attr_t attr;
// array of threads
pthread_t *tid;


#endif
