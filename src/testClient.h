/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef	BANK_CLIENT_H
#define	BANK_CLIENT_H


#include "test.h"


// Structure holding connection info (from cmd line & necessary structures)
typedef struct
{
	char *cmdIP;
	char *cmdPort;
	int clientSocket;
	struct addrinfo *serverAddr;
} NetInfo;


//*******************************************************************************************
// Function Prototypes
//*******************************************************************************************


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
bool setupSocket(NetInfo *sockData);

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


#endif
