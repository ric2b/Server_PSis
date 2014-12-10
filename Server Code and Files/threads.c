#include "main.h"

/* as threads são responsáveis por ler da FIFO de files descriptors as informaçoes de cada cliente e chamar a rotina que analisa o pedido feito */
void * atende_cliente(){
	
	info_pedido dados_thread;
	
	while(1){
		threads_livres++; // a thread ficou livre, incrementa a variável para a thread de controlo saber se é possível fechar algumas threads
		read(fifo_fd, &dados_thread, sizeof(info_pedido)); // ler da FIFO de file descriptors uma estrutura do tipo info_pedido
		threads_livres--; // a thread ficou ocupada, decrementa a variável para a thread de controlo saber se é necessário criar mais threads
		analisa_pedido(dados_thread); // chamar a função analisa_pedido() que recebe como argumento a estrutura lida da FIFO
	}
}


void * controlo_de_threads(){
	
	thread_list * aux;
	int diferenca, i;
	
	pthread_mutex_lock(&pool_criada);
		
	while(1){
			
		sleep(1);	
		if((diferenca = MARGEM_THREADS - threads_livres) > 0){
			 
			printf("a criar  mais threads!\n");	
			
			for(i=0; i<diferenca; i++){
				aux = last; 
				last = adiciona_entrada(last);
					
				if(pthread_create(&(last->thread_id), NULL, atende_cliente, NULL) != 0){ 
					free(last);															 																	 
					last = aux;															 
					last->next = NULL;													 	
					printf("black thread down!!\n");
					pthread_exit(0);																 
				}
			}	
		}	
	}	
}
