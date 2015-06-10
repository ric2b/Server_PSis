#include "main.h"

/* rotina responsável por criar a socket do servidor que irá escutar os clientes */
int criar_socket(){
	
	int server_socket;
	struct sockaddr_in server_addr;
	
	port_number = 8080; // porto é 8080
	
	/* criar socket do servidor */
	if ((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("impossível criar socket: erro na função socket()\n");
		exit(1);
    }
    
	/* estrutura que contém o endereço IP */
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port_number);
	
	port_number--;
	
	do{
		port_number++;
		server_addr.sin_port = htons(port_number);
		//perror("impossível associar porto: erro na função bind()\n");
	} while(bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0);
	
	printf("port - %d\n", port_number);
    
    /* socket do servidor a escutar pedidos */
    if (listen(server_socket, MAX_PENDING) < 0) { // fila de espera de escuta pela parte da socket do servidor é de 128 clientes, valor definido pelo MAX_PENDING
        perror("impossível criar socket: erro na função listen()\n");
		exit(1);
    }
    
    return server_socket; // função devolve o file descriptor da socket do servidor
}

/* função responsável por aceitar a ligação de cada cliente */
int aceita_cliente(int server_socket, char * remote_address){

	int client_socket; // file descriptor que irá ser associado a cada cliente
	struct sockaddr_in client_addr;
	unsigned int client_lenght = sizeof(client_addr);
	
	/* espera por ligação do cliente */
	if ((client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_lenght)) < 0) { // aceita o pedido do cliente e a cada cliente associa um file descriptor - client_socket
        perror("impossível aceitar ligação do cliente: erro na função client_socket()\n");
		exit(1);
    }
    
    sprintf(remote_address, "%s", inet_ntoa(client_addr.sin_addr));   
    
	return client_socket; // função devolve o file descriptor da socket do cliente
}
