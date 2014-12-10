#ifndef __LISTAS_H_
#define __LISTAS_H_

#include <pthread.h>

/* estrutura da lista de threads de atendimento de clientes */
typedef struct thread_list{
	pthread_t thread_id; // identificação da thread
	struct thread_list * next; // ponteiro para o próximo elemento da lista
} thread_list;

/* estrutura da lista de relatórios de pedidos (estatisticas) */
typedef struct lista_estatisticas{
	info_estatistica dados_estatistica; // dados do pedido
	struct lista_estatisticas * next; // ponteiro para o próximo elemento da lista
} lista_estatisticas;

thread_list * adiciona_entrada(thread_list * last);
void limpa_threads(thread_list * first);

lista_estatisticas * adiciona_estatisca(lista_estatisticas * last);
void limpa_estatisticas(lista_estatisticas * first);

#endif
