#include "main.h"

int trata_cgi(int client_fd, char * nome_ficheiro, char * path_ficheiro, char versao_http, int * codigo){
	
	int ok = 0; // variável que indica que um CGI executou dentro dos 5 segundos especificados, 0 para não e 1 para sim
	
	char http[BUFFSIZE];
	char html_padrao[30];
	char mensagem[BUFFSIZE];
	char mensagem_final[BUFFSIZE];
	char execute[BUFFSIZE] = ".";
	char directoria[BUFFSIZE];
	char temp[BUFFSIZE]="/";
	pid_t pid;
	
	struct timespec start;
	struct timespec stop;
	
	long tempo_processamento_ms;
	
	strcpy(html_padrao, "Content-type: text/html\n\n");
	
	if(versao_http == '0'){
		strcpy(http, "HTTP/1.0");
	}
	if(versao_http == '1'){
		strcpy(http, "HTTP/1.1");
	}
	
	if(CGI_ROOT[0] != '\0'){ // foi lido do ficheiro de configuraçao a directoria por default dos CGIs, assim, so podem ser corridos CGIs que se encontrem dentro desta pasta - CGI_ROOT, senão sera pedido com codigo 403		
		sprintf(directoria, "/%s/", CGI_ROOT); 
		strcat(temp, path_ficheiro);
		temp[strlen(directoria)] = '\0';
		
		if(strcmp(temp, directoria) != 0){ // quer-se correr um CGI fora da pasta que define CGI_ROOT
			*codigo = 403; // codigo do pedido passa a ser 403
			strcpy(mensagem, " 403 Forbidden\n");
			strcat(http, mensagem);
			strcat(http, html_padrao); 
			strcpy(mensagem_final, http);
			strcat(mensagem_final, mensagem);
			write(client_fd, mensagem_final, strlen(mensagem_final)); // escrever na socket do cliente a mensagem 
			return 0;
		}
	}
	
	/* inicio da execução do CGI */
	strcpy(mensagem, " 200 OK\n");
	strcat(http, mensagem);
	write(client_fd, http, strlen(http));	
	
	strcat(execute, strrchr(nome_ficheiro, '/'));
		
	if ((pid = fork()) < 0) { // faz fork
		printf("impossivel fazer fork: erro na função trata_cgi()\n"); 
		exit(1); 
	}
	if(pid == 0){ // filho responsável por executar o CGI
		dup2(client_fd, STDOUT_FILENO); // substituir fd do STDOUT_FILENO pelo fd do cliente	
		execlp(path_ficheiro, execute, NULL); // substituir processo filho pelo processo que corre o CGI
		exit(1);			
	}
	else{ // pai
		clock_gettime(CLOCK_MONOTONIC, &start); // aqui começa a contagem do tempo
		while(tempo_processamento_ms < 5000){ // os CGIs só podem executar ate um tempo máximo de 5 segundos, CGIs que demoram mais que isso têm de ser terminados
			if(waitpid(pid, NULL, WNOHANG) != 0){ // espera que o processo filho especificiado por pid mude de estado, se não mudar é devolvido 0
				ok = 1; // se chegou até aqui, entao dentro dos 5 segundos especificados pelo while o processo filho mudou de estado, ou seja, o CGI terminou 
				break;
			}
			clock_gettime(CLOCK_MONOTONIC, &stop); // aqui termina a contagem do tempo
			tempo_processamento_ms = (long)(1000 * (stop.tv_sec - start.tv_sec)) + ((stop.tv_nsec - start.tv_nsec) / 1000000); // actualizar valor que já passou desde que se iniciou a contagem do tempo
		}
		if(ok == 0){ // processo filho nunca mudou de estado
			kill(pid, SIGKILL); // matar o processo filho 
			waitpid(pid, NULL, 0); // garantir que depois de se matar o processo filho não fica um zombie
		}
	}
	
	return 0;
}
