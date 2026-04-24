#ifndef MENSAGEM_H
#define MENSAGEM_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "comando.h"

#define MAX_FIFO_NAME 256

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