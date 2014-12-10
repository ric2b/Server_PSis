#ifndef __COMUNICACAO_H_
#define __COMUNICACAO_H_

#include <netdb.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_PENDING 128 // máximo de clientes em espera

int criar_socket();
int aceita_cliente(int server_socket, char * remote_address);

#endif
