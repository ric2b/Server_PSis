CFLAGS= -g -Wall -std=gnu89 
LINKS= -lpthread -lrt

projecto: sinais.o comunicacao.o main.o pedidos.o threads.o listas.o cgi.o estatisticas.o
	gcc $(CFLAGS) -o projecto main.o comunicacao.o sinais.o pedidos.o threads.o listas.o cgi.o estatisticas.o $(LINKS)

main.o: comunicacao.h sinais.h main.h listas.h threads.h pedidos.h cgi.h estatisticas.h main.c 
	gcc $(CFLAGS) -c main.c
		
comunicacao.o: comunicacao.h main.h comunicacao.c
	gcc $(CFLAGS) -c comunicacao.c
	
pedidos.o: pedidos.h main.h pedidos.c
	gcc $(CFLAGS) -c pedidos.c
	
sinais.o: sinais.h main.h sinais.c
	gcc $(CFLAGS) -c sinais.c

threads.o: threads.h main.h threads.c
	gcc $(CFLAGS) -c threads.c

listas.o: listas.h main.h listas.c
	gcc $(CFLAGS) -c listas.c
	
cgi.o: cgi.h main.h cgi.c
	gcc $(CFLAGS) -c cgi.c
	
estatisticas.o: estatisticas.h main.h estatisticas.c
	gcc $(CFLAGS) -c estatisticas.c

clean:
	rm -f *.o *~ projecto
