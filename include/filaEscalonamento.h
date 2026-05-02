#ifndef FILA_ESCALONAMENTO_H
#define FILA_ESCALONAMENTO_H

#include <glib.h>

#include "comando.h"
#include "politicas/roundRobin.h"
#include "politicas/fcfs.h"

typedef GCompareDataFunc PoliticaEscalonamento;

#define MAX_USERS 256

typedef struct {
    int user_id;
    int contador;
} ContadorUtilizador;

typedef struct {
    GQueue* fila;
    PoliticaEscalonamento politica;
    ContadorUtilizador contadores[MAX_USERS];
    int num_users;
} FilaEscalonamento;

void inicializar_fila (FilaEscalonamento *fila, PoliticaEscalonamento politica, ContadorUtilizador contadores[MAX_USERS], int num_users);
void inserir_comando (FilaEscalonamento *fila, Comando *cmd);
Comando *fila_pop (FilaEscalonamento *fila);
int tamanho_fila (FilaEscalonamento *fila);
gboolean fila_vazia (FilaEscalonamento *fila);

PoliticaEscalonamento selecionar_politica(const char *nome);

#endif



