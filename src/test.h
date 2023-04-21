/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef RMOLINA_BANKING_H
#define RMOLINA_BANKING_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>	
#include <arpa/inet.h>
#include <netdb.h>


// Client-side banking protocol
typedef struct
{
	unsigned int trans; /* transaction type */
	unsigned int acctnum; /* account number */
	unsigned int value; /* value */
} sBANK_PROTOCOL;


//*******************************************************************************************
// Function Prototypes
//*******************************************************************************************


void printSocketAddress(const struct sockaddr *);


#endif
