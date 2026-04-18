#ifndef COMANDO_H
#define COMANDO_H

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

#endif 