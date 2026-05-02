#ifndef MENSAGEM_H
#define MENSAGEM_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "comando.h"

#define MAX_FIFO_NAME 256
#define FIFO_RUNNER_TO_CONTROLLER "fifos/runnerToController"

typedef enum {
    SUBMIT, 
    OK,
    EXECUTADO,
    CONSULTA,
    SHUTDOWN, 
} TipoMensagem;

typedef struct Mensagem {
    TipoMensagem tipo;
    Comando comando;
    char runner_FIFO[MAX_FIFO_NAME];
} Mensagem;

#endif