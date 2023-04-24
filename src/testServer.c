/*
	Rene Molina
	EE 4230 - Senior Design 2
*/


#include "testServer.h"


int initServer(char *service)
{
	// Initialize structure specifying possible connection types
	struct addrinfo addrCriteria;
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC;		// Any address family
	addrCriteria.ai_flags = AI_PASSIVE;		// Accept on any address/port
	addrCriteria.ai_socktype = SOCK_DGRAM;	// Only accept datagram sockets
	addrCriteria.ai_protocol = IPPROTO_UDP;	// Only accept UDP protocol
	
	// Get list of possible server addresses
	struct addrinfo *serverAddr;
	int status = getaddrinfo(NULL, service, &addrCriteria, &serverAddr);
	if (status != 0) {
		fputs("Error getting list of server addresses\n", stderr);
		return -1;
	}
	
	// Create and bind a UDP socket from options given
	int serverSocket = -1;
	status = -1;
	for (struct addrinfo *addr = serverAddr; addr != NULL && serverSocket < 0; addr = addr->ai_next) {
		// Attempt to make socket
		serverSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (serverSocket < 0)
			continue;
		// Attempt to bind socket
		status = bind(serverSocket, addr->ai_addr, addr->ai_addrlen);
		if (status != 0) {
			close(serverSocket);
			serverSocket = -1;
			continue;
		}
		// Upon success, print out address details
		puts("UDP socket created and bound to address:");
		printSocketAddress(addr->ai_addr);
		puts("\n************************************************\n");
	}
		
	// Indicates no options were valid sockets
	if (serverSocket < 0) {
		fputs("Error creating socket and binding to a valid address\n", stderr);
		return -1;
	}
	
	// Free possible address list once bound to an address
	freeaddrinfo(serverAddr);
	
	// Return socket handle
	return serverSocket;
}


bool handleClient(int serverSocket, Packets *packets)
{
	if (receivePacket(serverSocket, &packets->oneByte, sizeof(packets->oneByte)) == false)
		return false;
	if (receivePacket(serverSocket, packets->two_bytes, sizeof(packets->two_bytes)) == false)
		return false;
	if (receivePacket(serverSocket, packets->four_bytes, sizeof(packets->four_bytes)) == false)
		return false;
	if (receivePacket(serverSocket, packets->eight_bytes, sizeof(packets->eight_bytes)) == false)
		return false;
	if (receivePacket(serverSocket, packets->sixteen_bytes, sizeof(packets->sixteen_bytes)) == false)
		return false;
	if (receivePacket(serverSocket, packets->thirty_two_bytes, sizeof(packets->thirty_two_bytes)) == false)
		return false;
	if (receivePacket(serverSocket, packets->sixty_four_bytes, sizeof(packets->sixty_four_bytes)) == false)
		return false;
	if (receivePacket(serverSocket, packets->one_eigth_kb, sizeof(packets->one_eigth_kb)) == false)
		return false;
	if (receivePacket(serverSocket, packets->one_fourth_kb, sizeof(packets->one_fourth_kb)) == false)
		return false;
	if (receivePacket(serverSocket, packets->one_half_kb, sizeof(packets->one_half_kb)) == false)
		return false;
	if (receivePacket(serverSocket, packets->one_kb, sizeof(packets->one_kb)) == false)
		return false;
	if (receivePacket(serverSocket, packets->two_kb, sizeof(packets->two_kb)) == false)
		return false;
	if (receivePacket(serverSocket, packets->four_kb, sizeof(packets->four_kb)) == false)
		return false;
	if (receivePacket(serverSocket, packets->eight_kb, sizeof(packets->eight_kb)) == false)
		return false;
	if (receivePacket(serverSocket, packets->sixteen_kb, sizeof(packets->sixteen_kb)) == false)
		return false;
	if (receivePacket(serverSocket, packets->thirty_two_kb, sizeof(packets->thirty_two_kb)) == false)
		return false;
	if (receivePacket(serverSocket, packets->max_size_udp, sizeof(packets->max_size_udp)) == false)
		return false;
	
	return true;
}


bool receivePacket(int serverSocket, char *packet, unsigned int packetSize)
{
	// Location to store client address & length
	struct sockaddr_storage clientAddr;
	socklen_t clientAddrLength = sizeof(clientAddr);
	// Store data received from client into structure
	ssize_t bytesReceived = recvfrom(serverSocket, packet, packetSize, 0, (struct sockaddr *) &clientAddr, &clientAddrLength);
	if (bytesReceived < 0) {
		fputs("Unable to receive request from client\n", stderr);
		return false;
	}
	else if (bytesReceived != packetSize) {
		fputs("Unexpected number of bytes received\n", stderr);
		return false;
	}
	
	printf("Received %li bytes from the client\n", bytesReceived);
	
	// Send packet to client
	ssize_t bytesSent = sendto(serverSocket, packet, packetSize, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
	if (bytesSent < 0) {
		fputs("Transmission Error\n\n", stderr);
		return false;
	}
	else if (bytesSent != packetSize) {
		fputs("Send unexpected number of bytes\n\n", stderr);
		return false;
	}
	
	printf("Sent %li bytes to the client\n\n", bytesSent);
	
	return true;
}


int main(int argc, char **argv)
{	
	puts("\n************************************************\n");

	// Only argument is port/service, extra arguments ignored
	if (argc < 2) {
		puts("Not enough arguments entered:");
		puts("1st argument should be port address / service of the server");
		return -1;
	}
	
	// Initialize bank server
	int serverSocket = initServer(argv[1]);
	if (serverSocket < 0) {
		fputs("Failed to initialize server - ", stderr);
		return -1;
	}

	// Run forever (assuming no errors)
	Packets packets;
	while (1) {
		// Handle a single request from a client
		if (handleClient(serverSocket, &packets) == false)
			fputs("\nUnable to handle client request\n", stderr);
		puts("************************************************\n");
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
