/*
	Rene Molina
	EE 4230- Senior Design 2
*/


#include "test.h"


void printSocketAddress(struct socketaddr *addr)
{
	// Make sure address isn't null
	if (addr == NULL)
		return;
	
	// Used to hold printable IP address
	void *addrNum;
	char buffer[INET6_ADDRSTRLEN];
	
	// Print out correct IP address type
	switch(addr->sa_family) {
	case AF_INET:
		addrNum = &((struct sockaddr_in *) addr)->sin_addr;
		printf("IPv4 value: %s", inet_ntop(addr->sa_family, addrNum, buffer, sizeof(buffer)));
		printf("Port value: %u", ntohs(((struct sockaddr_in) addr)->sin_port);
		break;
	case AF_INET6:
		addrNum = &((struct sockaddr_in6 *) addr)->sin6_addr;
		printf("IPv6 value: %s", inet_ntop(addr->sa_family, addrNum, buffer, sizeof(buffer)));
		printf("Port value: %u", ntohs(((struct sockaddr_in6) addr)->sin6_port););
		break;
	default:
		puts("Unknown address family");
	}
}