#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char** args)
{
	if(args[1] == NULL)
	{
		printf("You must enter an IP address and a number of port\n");
		return 0;
	}

	if(args[2] == NULL)
	{
		printf("You must enter an IP address and a number of port\n");
		return 0;
	}

	int sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Recherche du protocole
	struct protoent* protocol;
	protocol = getprotobyname("tcp");

	//Recherche du service
	struct servent* service;
	service = getservbyname("kermit", protocol -> p_name);

	//Recherche du nom de l'hôte
	struct hostent* host;
	host = gethostbyname("localhost");

	//Initialisation de la structure sockaddr_in
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = service -> s_port;
	addr.sin_addr.s_addr = ((struct in_addr*)(host -> h_addr)) -> s_addr;

	//Nommage
	int code;
	code = bind(sock, (struct sockaddr*) & addr, sizeof(struct sockaddr_in));
	if(code == -1)
	{
		printf("Erreur bind !\n");
		perror("bind");
		exit(1);
	}
	else printf("Success !\n");

	//Définition du serveur
	struct sockaddr_in server;
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons((int)args[2]);
	inet_aton("127.0.0.1", &server.sin_addr);

	shutdown(sock, 2);

	

	return 0;

}
