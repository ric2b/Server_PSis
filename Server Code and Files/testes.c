#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFSIZE 1024

void * client_thread(void * arguments)
{
	int socket_fd;
	char pedido[BUFFSIZE] = "GET /index.html HTTP/1.1\r\n";
	char resposta[BUFFSIZE];
	struct sockaddr_in serv_addr;
	unsigned int server_lenght = sizeof(serv_addr);	
	
	char ** argv = (char **)arguments;
	
	if ((socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("client: can't open stream socket");
		return 0;
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	
	if (connect(socket_fd, (struct sockaddr *) &serv_addr, server_lenght) < 0)
	{
		perror("client_thread: can't connect to server\n");
		return 0;
	}	
	
	if(write(socket_fd, &pedido, BUFFSIZE) == -1)
	{
		printf("erro\n");
	}
	
	read(socket_fd, &resposta, BUFFSIZE);
	printf("resposta recebida: %s\n", resposta);
	printf("***thread over***\n");
	
	close(socket_fd);
	
	return 0;
}

int main(int argc, char ** argv)
{
	int npedidos, i,n;
	

	if(argc != 4)
	{
		printf("usage: ip porta nthreads\n");
		exit(-1);
	}
	
	npedidos = atoi(argv[3]);
	
	pthread_t thread_id[npedidos];

	for(i=0; i<npedidos; i++)
	{		
		if(pthread_create(&(thread_id[i]), NULL, client_thread, (void *)argv) != 0)
		{
			if(fork() == 0)
			{
				npedidos = npedidos - i;
				i = 0;								
			}
			else
			{
				break;
			}
		}		
	}


	n = i;
	
	for(i=0; i<n; i++)
	{
		pthread_join(thread_id[i], NULL);
	}

	printf("n: %d\n", n);
	exit(0);
}


