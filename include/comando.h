#ifndef COMANDO_H
#define COMANDO_H

#include <sys/types.h>

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
    int command_args;
} Comando;

typedef struct Pedido {
    ModoUtilizacao modo;
    Comando dados;
    pid_t runner_pid; 
} Pedido;

#endif 