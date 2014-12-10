#include "main.h"

int main(int argc, char * argv[]){
	
	int i;
	char directoria[BUFFSIZE];
	info_pedido dados;
	struct stat cfd;
	struct stat estatistica;
	
	thread_list * aux;
	
	/* associar handlers aos diversos sinais */
	signal(SIGINT, interrupcao_clean);
	signal(SIGTERM, interrupcao_clean);
	signal(SIGUSR1, interrupcao_usr1);
	signal(SIGPIPE, interrupcao_pipe);
	
	strcpy(DOCUMENT_ROOT, "http_docs"); // por default a directoria base do servidor é http_docs
	strcpy(CGI_ROOT, "\0");
	
	carrega_config(); // rotina associada a carregar ficheiro "www.config" 
			
	server_socket = criar_socket(); // rotina associada a criar socket do servidor 
	
	/* FIFO de file descriptors da socket de cada cliente */ 
	if(stat("fifo_cfd", &cfd) != -1){
		system("rm fifo_cfd"); // se já existe uma FIFO de uma sessão anterior é eliminada
	}
	if(mkfifo(strcat(getcwd(directoria, BUFFSIZE),"/fifo_cfd"), 0660) != 0){ // é criada a FIFO na directoria em que se encontra o executável do servidor com permissões 660
		perror("erro a criar named pipe: fifo_cfd (FIFO)\n");
		exit(1);
	}
	fifo_fd = open("fifo_cfd", O_RDWR); // abrir a FIFO em modo READ e WRITE
	
	/* FIFO das estatísticas */
	if(stat("fifo_estatistica", &estatistica) != -1){
		system("rm fifo_estatistica"); // se já existe uma FIFO de uma sessão anterior é eliminada
	}
	if(mkfifo(strcat(getcwd(directoria, BUFFSIZE),"/fifo_estatistica"), 0660) != 0){
		perror("erro a criar named pipe: fifo_estatistica (FIFO)\n"); // é criada a FIFO na directoria em que se encontra o executável do servidor com permissões 660
		exit(1);
	}
	fifo_estatistica = open("fifo_estatistica", O_RDWR); // abrir a FIFO em modo READ e WRITE	
	
	pthread_create(&thread_estatisticas, NULL, guarda_estatistica, NULL); // cria a thread que armazena na lista de estatisticas os dados sobre os vários pedidos

	/* criação da pool de threads  */
	pthread_mutex_init(&pool_criada, NULL);
	pthread_mutex_lock(&pool_criada);	
	
	pthread_create(&thread_controlo, NULL, controlo_de_threads, NULL);
	
	first = last = NULL; // inicializar os ponteiros para ter a certeza que a função adiciona entrada funciona bem
		
	for(i = 0; i < 150; i++){ // criar pool base com 150 threads
	// para o caso de pthread_create falhar é preciso apagar a última entrada da lista de threads, uma vez que não está a representar uma thread
		aux = last; // aux passa a apontar para a ultima entrada da lista
		last = adiciona_entrada(last); // como vai ser necessário apagar last, é mais prático ficar com aux a apontar para o penúltimo elemento do que percorrer toda a lista para recuperar um ponteiro para o último elemento
		if(first == NULL){ 
			first = last; // se é a primeira entrada na lista, guarda-se um ponteiro para esse elemento
		}
		if(pthread_create(&(last->thread_id), NULL, atende_cliente, NULL) != 0){ // criação das threads
		// criação das threads falhou se entrar dentro do if	
			free(last); // apagar o elemento da lista que acabou de ser criado, pois não representa uma thread, e pode ser problemático ao terminar o servidor										 
			// assume-se que é sempre possível criar pelo menos 1 thread de atendimento de cliente pelo que não se altera o ponteiro 'first' em caso de falha	
			last = aux; // ultimo da lista passa a ser aux, que antes se pos a apontar para o penultimo elemento
			last->next = NULL; // elemento a seguir ao ultimo é NULL	
			printf("threads: %d\n", i);
			break;																 
		}	
	}
	
	pthread_mutex_unlock(&pool_criada);
		
	memset(dados.endereco_cliente, '\0', BUFFSIZE); // inicialização da string, devido ao aviso do valgrind de que se a string não tiver todos os caracteres inicializados pode causar problemas ao fazer write
	
	while(1){ 
		dados.client_fd = aceita_cliente(server_socket, dados.endereco_cliente); // rotina associada a aceitar a ligação de cada cliente e a armazenar na estrutura dados em client_fd o file descriptor da socket de cada cliente
		time(&dados.inicio_pedido);	//data e hora em que o pedido foi aceite
		clock_gettime(CLOCK_MONOTONIC, &dados.start); //inicio da contagem em milisegundos da duração da resposta ao cliente
		write(fifo_fd, &dados, sizeof(info_pedido)); // escrever na FIFO de file descriptors a estrutura dados
	}
			
	exit(0);
}
