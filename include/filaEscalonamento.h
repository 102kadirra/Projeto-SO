#ifndef FILA_ESCALONAMENTO_H
#define FILA_ESCALONAMENTO_H

#include <glib.h>

#include "comando.h"
#include "politicas/roundRobin.h"
#include "politicas/fcfs.h"

typedef GCompareDataFunc PoliticaEscalonamento;

#define MAX_USERS 256

typedef struct {
    GQueue* fila;
    PoliticaEscalonamento politica;
} FilaEscalonamento;

void inicializar_fila (FilaEscalonamento *fila, PoliticaEscalonamento politica);
void inserir_comando (FilaEscalonamento *fila, Comando *cmd);
Comando *fila_pop (FilaEscalonamento *fila);
int tamanho_fila (FilaEscalonamento *fila);
gboolean fila_vazia (FilaEscalonamento *fila);

PoliticaEscalonamento selecionar_politica(const char *nome);

#endif



