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


#define BANK_TRANS_DEPOSIT 0
#define BANK_TRANS_WITHDRAW 1
#define BANK_TRANS_INQUIRY 2


typedef struct
{
	unsigned int trans;
	unsigned int acctnum;
	unsigned int value;
} sBANK_PROTOCOL;


// Function prototypes
void printSocketAddress(const struct sockaddr *);


#endif
