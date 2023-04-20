/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#ifndef RMOLINA_BANKING_H
#define RMOLINA_BANKING_H


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
// #include <signal.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
// #include <sys/wait.h>
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
