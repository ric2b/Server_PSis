#ifndef __MAIN_H_
#define __MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

#define BUFFSIZE 1024
#define READ 0
#define WRITE 1
#define MARGEM_THREADS 10

/* estrutura info_pedido */
typedef struct info_pedido{
	int client_fd; // file descriptor da socket de cada cliente
	char endereco_cliente[BUFFSIZE]; // endereço IP de cada cliente
	time_t inicio_pedido; // estrutura que armazena a data e hora em que o pedido foi aceite
	struct timespec start; // estrutura que armazena a hora (com precisão de milisegundos) em que o pedido foi aceite, para contar o tempo de resposta ao cliente
}info_pedido; 

/* estrutura info_estatistica */
typedef struct info_estatistica{
	char inicio_pedido[17]; // string com a data e hora em que o pedido foi aceite
	char nome_ficheiro[BUFFSIZE*2]; // nome do pedido 
	int codigo; // código associado a cada pedido
	char endereco_cliente[BUFFSIZE]; // endereço IP de cada cliente
	long tempo_processamento; // tempo de processamento e execução de cada pedido	
}info_estatistica;

#include "comunicacao.h"
#include "sinais.h"
#include "pedidos.h"
#include "threads.h"
#include "listas.h"
#include "estatisticas.h"

char DOCUMENT_ROOT[BUFFSIZE];
char CGI_ROOT[BUFFSIZE];

int fifo_fd;
int fifo_estatistica;
int server_socket;
int fd_pedido[2]; 
int counter;

thread_list * first;
thread_list * last;

int threads_livres;

pthread_t thread_estatisticas;
pthread_t thread_controlo;

pthread_mutex_t pool_criada;

lista_estatisticas * first_estatisticas;
lista_estatisticas * last_estatisticas;

FILE * fp_dir;

int main(int argc, char * argv[]);

#endif
