#ifndef COMANDO_H
#define COMANDO_H

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

typedef struct Comando {
    int user_id;
    int command_id;
    int turno;
    char command[256];
    char runner_FIFO[256];
    struct timeval tempo_entrada;
} Comando;

Comando *criar_comando(int user_id, int command_id, const char *command_str, const char *runner_fifo);
void libertar_comando(Comando *cmd);
double duracao_execucao (const Comando *c, struct timeval *fim);

#endif 

