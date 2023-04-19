/*
	Rene Molina
	EE 4374 - Operating Systems
	Due Date: 4/12/2023
	Assigned: 3/27/2023
	
	This is the header file which contains information used by both the client
	and server programs. This includes the header files described below, the 
	macros for bank transactions, and the structure containing the client-side
	banking protocol (sBANK_PROTOCOL)
*/


#ifndef RMOLINA_BANKING_H
#define RMOLINA_BANKING_H


#include <stdio.h>		// Standard I/O library
#include <stdlib.h>		// Standard library: atoi(), malloc
#include <pthread.h>	// pthreads library
#include <unistd.h>		// Unix system call library
#include <signal.h>		// Signal handler library
#include <time.h>		// srand function
#include <string.h>		// memset function
#include <stdbool.h>	// bool data type
#include <sys/wait.h>	// for wait() function
#include <sys/socket.h>	
#include <arpa/inet.h>
#include <netdb.h>


// Bank Transaction Types
#define BANK_TRANS_DEPOSIT 0
#define BANK_TRANS_WITHDRAW 1
#define BANK_TRANS_INQUIRY 2


// Client-side banking protocol
typedef struct
{
	unsigned int trans; /* transaction type */
	unsigned int acctnum; /* account number */
	unsigned int value; /* value */
} sBANK_PROTOCOL;


#endif
