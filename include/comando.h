#ifndef COMANDO_H
#define COMANDO_H

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


typedef enum {
    INVALIDO = -1,
    EXECUTAR = 0,
    CONSULTAR = 1,
    TERMINAR = 2
} ModoUtilizacao;

typedef struct Comando {
    int user_id;
    int command_id;
    char command[256];
    struct timeval tempo_entrada
} Comando;

Comando *criar_comando(int user_id, int command_id, const char *command_str);
void libertar_comando(Comando *cmd);
double duracao_execucao (const Comando *c, struct timeval *fim);

/*
typedef struct Pedido {
    ModoUtilizacao modo;
    Comando dados;
    pid_t runner_pid; 
} Pedido;
*/
#endif 

