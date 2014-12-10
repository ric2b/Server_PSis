#ifndef __SINAIS_H_
#define __SINAIS_H_

#include <signal.h>
#include <sys/types.h>
#include <dirent.h>

void interrupcao_clean();
void interrupcao_usr1();
void interrupcao_pipe();
void carrega_config();

#endif
