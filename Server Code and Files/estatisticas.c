#include "main.h"

/* função que armazena na lista de estatisticas os dados lidos da FIFO de estatisticas */
void * guarda_estatistica(void * pipe){
	
	info_estatistica dados_pipe_estatistica;
	first_estatisticas = NULL;
	last_estatisticas = NULL;
	lista_estatisticas * aux = NULL;
	
	counter = 0; // variável que conta o numero de entradas na lista que guarda a estrutura das estatisticas
	
	while(1){
		read(fifo_estatistica, &dados_pipe_estatistica, sizeof(info_estatistica)); // ler da FIFO das estatisticas a estrutura dados_pipe_estatistica do tipo info_estatistica
		last_estatisticas = adiciona_estatisca(last_estatisticas); // adicionar uma entrada na lista de estatisticas
		
		if(counter == 0){ // ainda não há nenhum elemento na lista
			first_estatisticas = last_estatisticas; // primeiro da lista passa a ser o ultimo
		}
		
		last_estatisticas->dados_estatistica = dados_pipe_estatistica; // armazenar o que foi lido do FIFO na lista		
		counter++; // incrementar a variavel que conta o numero de entradas na lista	
		if(counter >= 1000){ // quando se atinge 1000 entradas na lista vai-se apagando a mais antiga, ou seja, a primeira de todas na lista, isto porque os elementos são inseridos na lista no final de tudo
			aux = first_estatisticas; // ponteiro aux passa a pontar para o primeiro elemento na lista
			first_estatisticas = first_estatisticas->next; // o primeiro na lista passa agora a ser o elemento seguinte do primeiro, ou seja, o segundo
			free(aux); // libertar o primeiro elemento na lista
			counter--; // diminuir o numero de entradas na lista por um, uma vez que uma entrada acabou de ser apagada		
		}
	}	
}
