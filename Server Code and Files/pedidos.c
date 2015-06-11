#include "main.h"
#include "cgi.h"

/* função que recebe como argumento uma estrutura do tipo info_pedido e é responsável por interpretar o tipo de pedido e associá-lo a um código (200, 400, 404 ou 415) */
void analisa_pedido(info_pedido dados_cliente){
	
	char pedido[BUFFSIZE*2], content_type[10];
	char nome_ficheiro[BUFFSIZE*2], path_ficheiro[BUFFSIZE*2*2];
	char versao_http = '1';
	char * linha_pedido;
	char * ponto;
	struct stat teste;
	long tempo_processamento_ms;
	info_estatistica dados_pedido;
	
	/* inicializar valores na estrutura do tipo info_estatistica para que não haja erros de valgrind */
	memset(dados_pedido.nome_ficheiro, '\0', BUFFSIZE*2);
	memset(dados_pedido.endereco_cliente, '\0', BUFFSIZE);
	memset(dados_pedido.inicio_pedido, '\0', 17);
	
	FILE * fp = NULL; // file pointer que irá ser responsável por abrir ficheiros ".html" e ".png"
	
	int codigo = 200; // inicializar a variável de código a 200
	int cgi = 0; // flag que indica se um ficheiro é executável ou não - 0 para não e 1 para sim 
	int n_dados;	
	pedido[2047] = '\0';
	
	struct timespec stop;
	
	struct tm * hora_pedido = localtime(&dados_cliente.inicio_pedido); // converte a estrutura time_t em tm, para ser usada pela função strftime
	strftime(dados_pedido.inicio_pedido, 17, "%d/%m/%Y %H:%M", hora_pedido); // converte para uma string a informação de data e hora armazenadas na estrutura tm
		
	int received_bytes = read(dados_cliente.client_fd, pedido, (BUFFSIZE*2)-1); // ler do file descriptor armazenado na estrutura dados_cliente (passada como argumento para a função) em client_fd o pedido efectuado pelo cliente
	if(received_bytes >= 2){
		pedido[received_bytes] = '\0'; 
	
		if (strcmp(&(pedido[strlen(pedido)-2]), "\r\n") == 0){ // verificar que o pedido acaba em \r\n 
			linha_pedido = strtok(pedido, "\n");
			n_dados = sscanf(linha_pedido, "GET %s HTTP/1.%c", nome_ficheiro, &versao_http); // obter o pedido efectuado - nome_ficheiro e a versão do protocolo - versao_http 
			if(n_dados != 2){
				codigo = 400; // se não leu exactamente o pedido efectuado e a versão do protocolo entao código passa a ser 400, o pedido é inválido
			}	
			else{ 
				if(versao_http != '1' && versao_http !='0'){ // verificar que a versão do protocolo é HTTP 1.0 ou HTTP 1.1
					codigo = 400; // se não for código do pedido passa a ser 400 
				}
				else{
					if(strcmp(nome_ficheiro, "/estatisticas/Pedidos") == 0 || strcmp(nome_ficheiro, "/estatisticas/ClearAll") == 0){
					// pedido é "/estatisticas/Pedidos" ou "/estatisticas/ClearAll 
						codigo = 200; // se sim então código do pedido passa a ser 200	
					}
					else{
					// pedido não está relacionado com estatísticas
						strcpy(path_ficheiro, DOCUMENT_ROOT); // copiar para uma variavel - path_ficheiro o nome da directoria de DOCUMENT_ROOT
						strcat(path_ficheiro, nome_ficheiro); // anexar a path_ficheiro o nome do ficheiro pedido
						if(access(path_ficheiro, F_OK) == -1){ // verificar se o ficheiro pedido existe
							codigo = 404; // variável de código passa a ser 404
						}
						else{ // ficheiro existe
							if(nome_ficheiro[strlen(nome_ficheiro)-1] == '/'){
								codigo = 200; // é uma pasta, variável de código passa a ser 200
							}
							else{							
								if(stat(path_ficheiro, &teste) == 0 && teste.st_mode & S_IXUSR){ 
								// ficheiro não é ".html" nem ".png" , mas sim um executável, que tem permissões para executar portanto 
										codigo = 200; // variável de código passa a ser 200
										cgi = 1; 
								}
								else if((ponto = strrchr(nome_ficheiro, '.')) != NULL){
								// pedido inclui um ponto  
									codigo = 415; // variável de código do pedido inicializada aqui a 415
									if(strcmp(ponto, ".html") == 0 || strcmp(ponto, ".png") == 0 || strcmp(ponto, ".ico") == 0){
									// ficheiro pedido é do tipo ".html", ".png" ou ".ico"
										if (strcmp(ponto, ".html") == 0){ // é do tipo ".html"
											strcpy(content_type, "text/html");
											fp = fopen(path_ficheiro, "r"); // abrir ficheiro pedido em modo READ
											if(fp != NULL){ // verificar se o ficheiro existe
												codigo = 200; // se sim a variável de código passa a ser 200
											}
											else{
												codigo = 404; // se não a variável de código passa a ser 404
											}
										}
										if (strcmp(ponto, ".png") == 0){ // é do tipo ".png"
											strcpy(content_type, "image/png");
											fp = fopen(path_ficheiro, "rb"); // abrir o ficheiro em modo READ BINARY (por se tratar de uma imagem)
											if(fp != NULL){ // verificar se o ficheiro existe
												codigo = 200; // se sim a variável de código passa a ser 200
											}
											else{
												codigo = 404; // se não a variável de código passa a ser 404 
											}
										}
										if (strcmp(ponto, ".ico") == 0){ // é do tipo ".ico"
											strcpy(content_type, "image/ico");
											fp = fopen(path_ficheiro, "rb"); // abrir o ficheiro em modo READ BINARY (por se tratar de uma imagem)
											if(fp != NULL){ // verificar se o ficheiro existe
												codigo = 200; // se sim a variável de código passa a ser 200
											}
											else{
												codigo = 404; // se não a variável de código passa a ser 404 
											}
										}
									}
								}
							}		
						}
					}
				}
			}
		}
		else { // pedido inválido porque não acaba em \r\n
		codigo = 400; // variável de código passa a ser 400
		}
	}
    else { // pedido inválido porque não foram lidos bytes suficientes
		codigo = 400; // variável de código passa a ser 400
	}
	
	/* chama a rotina associada a enviar a resposta do pedido para o browser */			
	envia_pedido(dados_cliente.client_fd, &codigo, cgi, content_type, nome_ficheiro, path_ficheiro, fp, versao_http); 
	
	clock_gettime(CLOCK_MONOTONIC, &stop); // obtém a hora em que o pedido acabou de ser tratado para contar quanto tempo demorou
	tempo_processamento_ms = (long)(1000 * (stop.tv_sec - dados_cliente.start.tv_sec)) + ((stop.tv_nsec - dados_cliente.start.tv_nsec) / 1000000); // calcula o tempo que o pedido demorou a ser tratado
	
	printf("%-30s %-5d %-15s %-5lu [ms]\n", nome_ficheiro, codigo, dados_cliente.endereco_cliente, tempo_processamento_ms);
	// imprimir no terminal informações relativas ao pedido - qual o pedido (nome_ficheiro), qual o código (codigo), qual o endereço IP do cliente (dados_cliente.endereco_cliente) e qual o tempo de processamento do pedido e envio da resposta
	
	/* escrever na estrutura dados_pedido que contém as informações das estatísticas */
	strcpy(dados_pedido.nome_ficheiro, nome_ficheiro);
	dados_pedido.codigo = codigo;
	strcpy(dados_pedido.endereco_cliente, dados_cliente.endereco_cliente);
	dados_pedido.tempo_processamento = tempo_processamento_ms;
	
	write(fifo_estatistica, &dados_pedido, sizeof(info_estatistica));
	// escrever na FIFO das estatisticas a estrutura dados_pedido do tipo info_estatistica, já preenchida 
	
	return;
      
}

/* função que envia a resposta ao pedido para o browser */
void envia_pedido(int client_fd, int * codigo, int cgi, char content_type[10], char * nome_ficheiro, char * path_ficheiro, FILE * fp, char versao_http){

	char http[BUFFSIZE];
	char mensagem_final[BUFFSIZE];
	char mensagem[BUFFSIZE];	
	char html_padrao[30];
	char png_padrao[30];
	char ico_padrao[30];
	char buffer_image[BUFFSIZE];
	char buffer_char;
	char buffer_dir[BUFFSIZE];
	char directoria[BUFFSIZE];
	char directoria_temp[BUFFSIZE];
	char ficheiro_na_dir[BUFFSIZE];
	char tipo[BUFFSIZE];
	char * ponto;
	char host_name[BUFFSIZE];
	
	lista_estatisticas * aux;

	DIR * temp;
	pid_t pid;
	
	strcpy(html_padrao, "Content-type: text/html\n\n");
	strcpy(png_padrao, "Content-type: image/png\n\n");
	strcpy(ico_padrao, "Content-type: image/ico\n\n");
	
	if(versao_http == '0'){
		strcpy(http, "HTTP/1.0");
	}
	if(versao_http == '1'){
		strcpy(http, "HTTP/1.1");
	}
	
	/* 400 */	
	if(*codigo == 400){
		sprintf(mensagem, "%s 400 Bad Request\n%s 400 Bad Request\n", http, html_padrao); 
		write(client_fd, mensagem, strlen(mensagem)); // escreve na socket do cliente a mensagem a enviar para o browser 
		/* garantir que a socket do cliente é fechada */
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);
	}
	
	else {
		/* 404 */
		if(*codigo == 404){
			sprintf(mensagem, "%s 404 Not Found\n%s<HTML><HEAD> <TITLE>404 Not Found</TITLE></HEAD><BODY>ficheiro %s nao foi encontrado.</BODY></HTML>", http, html_padrao, nome_ficheiro); 
			write(client_fd, mensagem, strlen(mensagem)); // escreve na socket do cliente a mensagem a enviar para o browser 
			/* garantir que a socket do cliente é fechada */
			shutdown(client_fd, SHUT_RDWR);
			close(client_fd);
		}
		/* 200 */
		if(*codigo == 200){
			if(cgi == 1){ // pedido é do tipo cgi
				/* chama a rotina associada a executar o cgi para o browser */		
				cgi = trata_cgi(client_fd, nome_ficheiro, path_ficheiro, versao_http, codigo); 	
			}
			else{
				// verica-se se o pedido é para listar uma directoria						
				strcpy(directoria, DOCUMENT_ROOT);
				strcat(directoria, nome_ficheiro);				
				if(nome_ficheiro[strlen(nome_ficheiro)-1] == '/'){ // confirma-se que o pedido é para listar uma directoria
					if((temp = opendir(directoria)) == NULL){
					// directoria inexistente
						sprintf(mensagem, "%s 404 Not Found\n%s<HTML><HEAD> <TITLE>404 Not Found</TITLE></HEAD><BODY>ficheiro %s nao foi encontrado.</BODY></HTML>", http, html_padrao, nome_ficheiro); 
						write(client_fd, mensagem, strlen(mensagem)); // escreve na socket do cliente a mensagem a enviar para o browser 
						/* garantir que a socket do cliente é fechada */
						shutdown(client_fd, SHUT_RDWR);
						close(client_fd);
					}
					else{
					// directoria da qual se pretende listar os ficheiros existe
						closedir(temp); // fechar a directoria para não dar erros de valgrind
						if (pipe(fd_pedido) != 0) { // faz pipe
							printf("impossivel criar pipe: erro na função envia_pedido()\n"); 
							exit(1); 
						}		
						if ((pid = fork()) < 0) { // faz fork
							printf("impossivel fazer fork: erro na função envia_pedido()\n"); 
							exit(1); 
						}
						if (pid == 0){ // filho que vai executar o ls, ou seja, listar os ficheiros existentes na directoria
							close(fd_pedido[READ]); // fechar a reading end do pipe entre pai e filho
							dup2(fd_pedido[WRITE], STDOUT_FILENO); // substituir fd do STDOUT_FILENO pelo fd do pipe, porque o ls tem como default o terminal
							// assim, o resultado do ls é escrito no pipe entre pai e filho, para que depois o pai possa ler a informação do ls
							close(fd_pedido[WRITE]); // fechar a writing end do pipe entre pai e filho
							execlp("ls","ls","-l",directoria,NULL); // executar o ls, sem retorno ao filho, o processo do filho é substituido pelo processo do execlp
						}
						else{ // pai
							strcpy(mensagem, " 200 OK\n");
							strcat(http, mensagem);
							strcat(http, html_padrao); 
							write(client_fd, http, strlen(http));
							sprintf(mensagem, "<h3>lista de ficheiros existentes na directoria %s\n</h3>", directoria);
							write(client_fd, mensagem, strlen(mensagem));
							close(fd_pedido[WRITE]); // fechar a writing end do pipe entre pai e filho
							fp_dir = fdopen(fd_pedido[READ], "r"); // fp_dir é um FILE * obtido através da abertura do file descriptor fd_pedido em modo READ, ou seja, a extremidade de leitura do processo pai
							// em vez de se fazer read do pipe, associa-se ao file descriptor do pipe um FILE * e lê-se como se fosse um ficheiro
							fgets(buffer_dir, BUFFSIZE, fp_dir); // a primeira linha do ls (que lista o total de ficheiros na directoria) é para ser ignorada
							while(fgets(buffer_dir, BUFFSIZE,fp_dir) != NULL){ // enquanto não se acabar de ler toda a informação do ls contida em fp_dir
								sscanf(buffer_dir,"%s %*d %*s %*s %*d %*s %*d %*s %s",tipo,ficheiro_na_dir); // os valores com asterisco não são relevantes para o que se pretende enviar para o browser, e como tal ignorados no sscanf
								if(tipo[0] == 'd'){ // primeiro caracter da primeira coluna que o ls lista é um 'd' então trata-se de uma directoria dentro de outra
									strcat(ficheiro_na_dir, "/"); // inserir uma barra no final do nome dessa pasta para que depois possa-se listar o conteudo dessa mesma pasta no browser, com recurso a links
								}
								strcpy(directoria_temp, nome_ficheiro);
								strcat(directoria_temp, ficheiro_na_dir);
								
								if(strcmp(extIP,"NoneDefined"))
									strcpy(host_name, extIP);
								else									
									gethostname(host_name, BUFFSIZE); // retirar o nome do cliente

								sprintf(mensagem, "<a href=\"http://%s:%d%s\">%s</a><br>", host_name, port_number, directoria_temp, ficheiro_na_dir); // imprimir numa string o ficheiro/directoria listado pelo ls, com link
								write(client_fd, mensagem, strlen(mensagem)); // escreve na socket do cliente a mensagem a enviar para o browser
							}
							/* garantir que a socket do cliente é fechada */
							shutdown(client_fd, SHUT_RDWR);
							close(client_fd);
							close(fd_pedido[READ]); // fechar a reading end do pipe entre pai e filho
							fclose(fp_dir); // fechar o FILE * associado ao file descriptor de modo leitura do pipe entre pai e filho
						}
					}
				}				
				else{ 
					if((ponto = strrchr(nome_ficheiro, '.')) == NULL){
						// pedido não tem nenhum ponto
						if(strcmp(nome_ficheiro, "/estatisticas/Pedidos") == 0 || strcmp(nome_ficheiro, "/estatisticas/ClearAll") == 0){
						// é um pedido de estatisticas
							if(strcmp(nome_ficheiro, "/estatisticas/Pedidos") == 0){ // pedido efectuado é "/estatisticas/Pedidos"	
								strcpy(mensagem, " 200 OK\n");
								strcat(http, mensagem);
								strcat(http, html_padrao); 
								strcpy(mensagem_final, http);
								strcat(mensagem_final, "<html><head></head><body>");
								strcat(mensagem_final, "<h3>Lista de Pedidos\n</h3>");
								strcat(mensagem_final, "<p><table cellspacing=""10"" >"); // tabela que contém as estatisticas
								write(client_fd, mensagem_final, strlen(mensagem_final)); // escreve na socket do cliente inicio da formatação da tabela
								
								aux = first_estatisticas; // ponteiro aux aponta para o inicio da lista que contem as estruturas do tipo info_estatisticas, cada estrutura têm as informações relevantes a cada pedido que chegou 
								while(aux != NULL){ // enquanto não se atingir o final da lista 							
									sprintf(mensagem,"<tr><td> %s </td><td> %s </td><td> %d </td><td> %s </td><td> %lu [ms]</td></tr>", 
										aux->dados_estatistica.inicio_pedido, aux->dados_estatistica.nome_ficheiro, 
										aux->dados_estatistica.codigo, aux->dados_estatistica.endereco_cliente, aux->dados_estatistica.tempo_processamento);
										// escreve numa string todos os dados lidos da estrutura referente a um determinado pedido  
									write(client_fd, mensagem, strlen(mensagem)); // escreve string na socket do cliente							
									aux = aux->next; // ponteiro aux avança na lista	
								}
								write(client_fd, "</table></p></body></html>", strlen("</table></p></body></html>")); // escreve na socket do cliente final da formatação da tabela						
								/* garantir que a socket do cliente é fechada */
								shutdown(client_fd, SHUT_RDWR);
								close(client_fd);					
							}
							if(strcmp(nome_ficheiro, "/estatisticas/ClearAll") == 0){  // pedido efectuado é "/estatisticas/ClearAll"				
								
								while(first_estatisticas != NULL){ // enquanto não se atingir o final da lista
									aux = first_estatisticas; // ponteiro aux aponta para o inicio da lista
									first_estatisticas = first_estatisticas->next; // o primeiro na lista passa agora a ser o elemento seguinte do primeiro, ou seja, o segundo
									free(aux); // libertar o primeiro 
								}
								
								last_estatisticas = NULL; // último da lista passa a ser NULL
								counter = 0;					
								
								strcpy(mensagem, " 200 OK\n");
								strcat(http, mensagem);
								strcat(http, html_padrao); 
								strcpy(mensagem_final, http);
								strcat(mensagem_final, "<h3>Todos os registos eliminados\n</h3>");
								write(client_fd, mensagem_final, strlen(mensagem_final)); // escreve na socket do cliente a mensagem a enviar para o browser 
								/* garantir que a socket do cliente é fechada */
								shutdown(client_fd, SHUT_RDWR);
								close(client_fd);			
							}
						}					
					}
					else{
					// pedido tem um ponto
						strcpy(mensagem, " 200 OK\n");
						strcat(http, mensagem);
						

						if(strcmp(content_type, "text/html") == 0){ // pedido é para um ficheiro ".html"
							strcat(http, html_padrao); 
							write(client_fd, http, strlen(http));
							while(((int)(buffer_char = getc(fp))) != EOF){ // ler caracter a caracter do ficheiro html pedido
								if((write(client_fd, &buffer_char, 1)) < 1){ // escrever na socket do cliente caracter a caracter				
									perror("write");
									break; // se algum caracter falhar na escrita então esta é interrompida
								}		
							}							
						}
						
/*						if(strcmp(content_type, "text/html") == 0){ // pedido é para um ficheiro ".png"
							strcat(http, html_padrao); 
							write(client_fd, http, strlen(http));
							while((fread(buffer_image, 1, sizeof(buffer_image), fp)) > 0){ // ler caracter a caracter do ficheiro png pedido
								if((write(client_fd, &buffer_image, sizeof(buffer_image))) < 1){ // escrever na socket do cliente caracter a caracter	
									perror("write");
									break; // se algum caracter falhar na escrita então esta é interrompida
								}		
							}	
						}
*/
						if(strcmp(content_type, "image/png") == 0){ // pedido é para um ficheiro ".png"
							strcat(http, png_padrao); 
							write(client_fd, http, strlen(http));
							while((fread(buffer_image, 1, sizeof(buffer_image), fp)) > 0){ // ler caracter a caracter do ficheiro png pedido
								if((write(client_fd, &buffer_image, sizeof(buffer_image))) < 1){ // escrever na socket do cliente caracter a caracter	
									perror("write");
									break; // se algum caracter falhar na escrita então esta é interrompida
								}		
							}	
						}
						if(strcmp(content_type, "image/ico") == 0){ // pedido é para um ficheiro ".ico"
							strcat(http, ico_padrao); 
							write(client_fd, http, strlen(http));
							while((fread(buffer_image, 1, sizeof(buffer_image), fp)) > 0){ // ler caracter a caracter do ficheiro png pedido
								if((write(client_fd, &buffer_image, sizeof(buffer_image))) < 1){ // escrever na socket do cliente caracter a caracter	
									perror("write");
									break; // se algum caracter falhar na escrita então esta é interrompida
								}		
							}	
						}						
						/* garantir que o FILE * fp responsável por abrir os ficheiros ".html" e ".png" é devidamente fechado */
						fflush(fp);  
						fclose(fp); 
					}					
				}
			}
			/* garantir que a socket do cliente é fechada */
			shutdown(client_fd, SHUT_RDWR);
			close(client_fd);
		}
		/* 415 */
		if(*codigo == 415){
			sprintf(mensagem, "%s 415 Unsupported Media Type\n%s 415 Unsupported Media Type\n", http, html_padrao); 
			write(client_fd, mensagem, strlen(mensagem));
			/* garantir que a socket do cliente é fechada */
			shutdown(client_fd, SHUT_RDWR);
			close(client_fd);
		}
	}
	return;
}
