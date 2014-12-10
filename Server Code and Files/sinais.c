#include "main.h"

/* rotina que efectua código de limpeza e libertação de todos os recursos, aquando de se apanhar um sinal SIGTERM ou SIGINT */
void interrupcao_clean(){
	
	pthread_cancel(thread_controlo); 
	pthread_join(thread_controlo, NULL);
	pthread_mutex_destroy(&pool_criada);
				
	limpa_threads(first); // esta função dá ordem de cancelamento às threads de atendimento de clientes e liberta a memória ocupada pela lista dessas threads
	
	pthread_cancel(thread_estatisticas); // dá-se ordem de cancelamento à thread que armazena as estatísticas
	pthread_join(thread_estatisticas, NULL); // esperar que a thread termine
	limpa_estatisticas(first_estatisticas); // libertar a memória ocupada pela lista de estatíticas
	
	system("rm fifo_cfd");	// apaga-se a FIFO com a lista de espera de clientes para serem atendidos por uma thread
	system("rm fifo_estatistica"); // apaga-se a FIFO com a lista de espera de pedidos a serem armazenados na lista de estatisticas
	
	/* garantir que a socket do servidor é fechada */
	shutdown(server_socket, SHUT_RDWR);
	close(server_socket);
	
	printf("\nacabei por ordem do administrador\n"); 
	exit(0); // parar a execução do servidor
}

/* rotina que lê as configurações do ficheiro "www.config", aquando de se apanhar o sinal SIGUSR1 */
void interrupcao_usr1(){

	/* rotina que efectua de facto o load do ficheiro e verifica se está correctamente configurado */
	carrega_config();
	
	return;	
}

/* rotina que garante que um sinal do tipo SIGPIPE não mata o programa no caso do write */
void interrupcao_pipe(){
	
	return;
}


void carrega_config(){
	
	FILE * config;
	DIR * temp;
	char linha1[BUFFSIZE];
	char linha2[BUFFSIZE];
	char document_temp[BUFFSIZE];
	char directoria[BUFFSIZE];
		
	/* verificar se o ficheiro 'www.config' existe */	
	if((config = fopen("www.config", "r")) == NULL){ 
		return; // se não existe então a função sai
	}
	
	/* se o ficheiro www.config não estiver todo correcto os valores de DOCUMENT_ROOT e CGI_ROOT não são alterados */
	if((fgets(linha1, BUFFSIZE, config)) != NULL){ // primeira linha não está em branco
		if((fgets(linha2, BUFFSIZE, config)) != NULL){ // segunda linha não está em branco
			
			strcpy(directoria, strtok(linha1, "\n"));
			if((temp = opendir(directoria)) != NULL){ // directoria DOCUMENT_ROOT existe
				closedir(temp);
				
				strcpy(document_temp, directoria);
				strcpy(linha1, strtok(linha2, "\n"));
				sprintf(directoria, "%s/%s", document_temp, linha1);		
				if((temp = opendir(directoria)) != NULL){ // directoria CGI_ROOT existe
					closedir(temp);
					strcpy(DOCUMENT_ROOT, document_temp); // guardar as directorias lidas nas variáveis respectivas
					strcpy(CGI_ROOT, directoria);
				}
			}
		}
	}
	
	fclose(config); // fechar o ficheiro www.config 
	
	return;
}
