#ifndef __THREADS_H_
#define __THREADS_H_

#include <pthread.h>

#define POOL_AMMOUNT 10

void * atende_cliente();
void * controlo_de_threads();

#endif
