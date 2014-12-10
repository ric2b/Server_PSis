#ifndef __PEDIDOS_H_
#define __PEDIDOS_H_

#include <sys/wait.h>
#include <time.h>
#include <errno.h>

#define READ 0
#define WRITE 1

void analisa_pedido(info_pedido dados_cliente);
void envia_pedido(int client_fd, int * codigo, int cgi, char content_type[10], char * nome_ficheiro, char * path_ficheiro, FILE * fd, char versao_http);

#endif

