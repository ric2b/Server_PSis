#include "main.h"

/* listas de threads - função que anexa um elemento à lista, se a lista estiver vazia esse elemento será a primeira entrada  */
thread_list * adiciona_entrada(thread_list * last){
	
	thread_list * new_entry;
	
	if(last == NULL){ // lista ainda está vazia
		if((last = malloc(sizeof(thread_list))) == NULL){ // criar uma entrada na lista
			perror("memoria cheia - malloc falhou\n");
			exit(1);
		}
		last->next = NULL; // elemento a seguir ao ultimo (o que foi criado agora) é NULL
	}
	else{ // a lista já não está vazia
		if((new_entry = malloc(sizeof(thread_list))) == NULL){ // anexa-se uma entrada na lista, ajustando o ponteiro do último elemento para esta nova entrada. é uma lista com inserção no final
			perror("memoria cheia - malloc falhou\n");
			exit(1);
		}
		last->next = new_entry; // o proximo elemento do anterior ultimo elemento passa a ser a nova entrada criada
		new_entry->next = NULL; // o elemento seguinte à nova entrada é NULL 
		last = new_entry; // o ultimo da lista passa a ser o novo elemento criado
	} 
		
	return last;	
}

/* função que termina a execução das threads de atendimento a clientes e liberta a memória da respectiva lista */
void limpa_threads(thread_list * first){
	
	thread_list * aux;
	
	while(first != NULL){ // enquanto ainda existirem elementos na lista
		aux = first; // aux passa a apontar para o primeiro elemento da lista
		pthread_cancel(aux->thread_id);	// dar ordem de cancelamento à thread da entrada actual
		if(pthread_join(aux->thread_id, NULL) != 0){ // esperar que essa thread acabe
			printf("erro a cancelar\n");
		}
		first = first->next; // o novo primeiro elemento sera o segundo							
		free(aux); // apagar o anterior primeiroelemento da lista e passar ao próximo
	}
	
	return;
}

/* listas de estatísticas - função que anexa um relatório de pedido à lista de pedidos */
lista_estatisticas * adiciona_estatisca(lista_estatisticas * last){ 
	
	lista_estatisticas * new_entry;
	
	if(last == NULL){ // lista ainda está vazia
		if((last = malloc(sizeof(lista_estatisticas))) == NULL){ // criar uma entrada na lista
			perror("memoria cheia - malloc falhou\n");
			exit(1);
		}
		last->next = NULL; // elemento a seguir ao ultimo (o que foi criado agora) é NULL
	}
	else{
		if((new_entry = malloc(sizeof(lista_estatisticas))) == NULL){ // anexa-se uma entrada na lista, ajustando o ponteiro do último elemento para esta nova entrada. é uma lista com inserção no final 
			perror("memoria cheia - malloc falhou\n");
			exit(1);
		}
			
		last->next = new_entry; // o proximo elemento do anterior ultimo elemento passa a ser a nova entrada criada
		new_entry->next = NULL; // o elemento seguinte à nova entrada é NULL 
		last = new_entry; // o ultimo da lista passa a ser o novo elemento criado
	} 
		
	return last;	
}

/* esta função liberta a memória da lista de relatórios de pedidos */
void limpa_estatisticas(lista_estatisticas * first){
	
	lista_estatisticas * aux;
	
	if(first != NULL){ // enquanto ainda existirem elementos na lista
		do{
			aux = first->next; // aux passa a apontar para o segundo elemento da lista 
			free(first); // libertar o primeiro elemento da lista
			first = aux; // o novo primeiro elemento passa a ser aux
		} while(aux != NULL);
	}	
	
	return;
}

